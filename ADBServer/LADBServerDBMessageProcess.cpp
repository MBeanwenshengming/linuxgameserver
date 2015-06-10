/*
The MIT License (MIT)

Copyright (c) <2010-2020> <wenshengming>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "LADBServerDBMessageProcess.h"
#include "LDBServerMainLogicThread.h"
#include "LWorkItem.h"
#include "LUserInfoManager.h"
#include "LUserInfo.h"
#include "LServerManager.h"
#include "LServer.h"
#include "../NetWork/LPacketBroadCast.h"

LADBServerDBMessageProcess::LADBServerDBMessageProcess()
{
	m_pDBServerMainLogicThread = NULL;
}
LADBServerDBMessageProcess::~LADBServerDBMessageProcess()
{
}
//	分发消息包进行处理
void LADBServerDBMessageProcess::DispatchMessageToProcess(LWorkItem* pWorkItem)
{
	if (pWorkItem == NULL)
	{
		return ;
	}
	unsigned int unWorkID = pWorkItem->GetWorkID();
	switch (unWorkID)
	{
		case ADB_SERVER_DBT2MT_READ_USER_INFO:
			{
				DBMessageProcess_Read_User_Info(pWorkItem);
			}
			break;
		case ADB_SERVER_DBT2MT_ADD_USER_INFO:
			{
				DBMessageProcess_AddUser_Info(pWorkItem);
			}
			break;
		default:
			break;
	}
}
void LADBServerDBMessageProcess::SetDBServerMainLogicThread(LDBServerMainLogicThread* pdbsmlt)
{
	m_pDBServerMainLogicThread = pdbsmlt;
}
LDBServerMainLogicThread* LADBServerDBMessageProcess::GetDBServerMainLogicThread()
{
	return m_pDBServerMainLogicThread;
}

void LADBServerDBMessageProcess::DBMessageProcess_Read_User_Info(LWorkItem* pWorkItem)
{ 
	int nResult = 0;				//	数据库操作是否成功
	char szUserID[MAX_USER_ID_LEN];	
	char szPassWord[MAX_PASSWORD_LEN];
	uint64_t u64UniqueIDInDB = 0;
	uint64_t u64ServerSessionID = 0;
	uint64_t u64SessionIDInClient = 0;
	memset(szUserID, 0, sizeof(szUserID)); 
	memset(szPassWord, 0, sizeof(szPassWord)); 

	pWorkItem->GetInt(nResult);
	pWorkItem->GetData(szUserID		, MAX_USER_ID_LEN);
	pWorkItem->GetData(szPassWord	, MAX_PASSWORD_LEN);
	pWorkItem->GetULongLong(u64UniqueIDInDB);
	pWorkItem->GetULongLong(u64ServerSessionID);
	pWorkItem->GetULongLong(u64SessionIDInClient);

	if (pWorkItem->GetErrorCode() != 0)
	{
		return ;
	}
	LServerManager* pServerManager = m_pDBServerMainLogicThread->GetServerManager();
	LServer* pServer = pServerManager->FindServerBySessionID(u64ServerSessionID);
	if (pServer == NULL)
	{
		return ;
	}
	LUserInfoManager* pUserInfoManager = m_pDBServerMainLogicThread->GetUserInfoManager();
	LUserInfo* pUserInfo = pUserInfoManager->FindUserInfo(szUserID);
	if (pUserInfo == NULL)
	{
		if (nResult == 0)		//	成功
		{
			pUserInfo = pUserInfoManager->AllocOneUserInfo();
			if (pUserInfo == NULL)
			{
				return ;
			}
			pUserInfo->SetUserID(szUserID);
			pUserInfo->SetUserPassWord(szPassWord);
			pUserInfo->SetUserUniqueIDInDB(u64UniqueIDInDB);
			pUserInfo->SetUserState(E_User_State_Data_Info_Initialized);
			pUserInfoManager->AddUserInfo(pUserInfo);
		}
		else
		{
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = m_pDBServerMainLogicThread->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
			int nResult = nResult;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64SessionIDInClient);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			m_pDBServerMainLogicThread->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			m_pDBServerMainLogicThread->FlushSendPacket();
			//	发送验证失败消息给客户端
			return ;
		}
	}
	else
	{
		if (nResult == 0)
		{
			pUserInfo->SetUserID(szUserID);
			pUserInfo->SetUserPassWord(szPassWord);
			pUserInfo->SetUserUniqueIDInDB(u64UniqueIDInDB);
			pUserInfo->SetUserState(E_User_State_Data_Info_Initialized);
		}
		else
		{ 
			//	从玩家管理器删除玩家信息
			pUserInfoManager->RemoveUserInfo(szUserID);
			//	发送验证失败消息给客户端
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = m_pDBServerMainLogicThread->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
			int nResult = nResult;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64SessionIDInClient);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			m_pDBServerMainLogicThread->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			m_pDBServerMainLogicThread->FlushSendPacket();
			return ;
		}
	}
//	if (pUserInfo->CheckUserAndPassWord(szUserID, szPassWord))
//	{
		//	发送验证成功的数据包
		unsigned short usPacketLen = 128;
		LPacketBroadCast* pSendPacket = m_pDBServerMainLogicThread->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
		nResult = 0;
		uint64_t u64UserUniqueIDInDB = u64UniqueIDInDB;
		pSendPacket->AddInt(nResult);
		pSendPacket->AddULongLong(u64SessionIDInClient);
		pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
		pSendPacket->AddULongLong(u64UniqueIDInDB);	
		m_pDBServerMainLogicThread->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
		m_pDBServerMainLogicThread->FlushSendPacket();
//	}
//	else
//	{
//		unsigned short usPacketLen = 128;
//		LPacketBroadCast* pSendPacket = m_pDBServerMainLogicThread->GetOneSendPacket(usPacketLen);
//		pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
//		int nResult = -2;
//		uint64_t u64UserUniqueIDInDB = 0;
//		pSendPacket->AddInt(nResult);
//		pSendPacket->AddULongLong(u64SessionIDInClient);
//		pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
//		pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
//		m_pDBServerMainLogicThread->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
//		m_pDBServerMainLogicThread->FlushSendPacket();
//	} 
}

void LADBServerDBMessageProcess::DBMessageProcess_AddUser_Info(LWorkItem* pWorkItem)
{ 
	int nResult = 0;
	char szUserID[MAX_USER_ID_LEN + 1];
	char szPassWord[MAX_PASSWORD_LEN + 1];
	uint64_t u64UniqueIDInDB = 0;				//	创建成功时，在数据库中的唯一ID
	uint64_t u64ServerSessionID = 0;
	uint64_t u64SessionIDInClient = 0;			//	玩家在对应的客户端服务器中的SessionID
	memset(szUserID, 0, sizeof(szUserID));
	memset(szPassWord, 0, sizeof(szPassWord));
	
	pWorkItem->GetInt(nResult);
	pWorkItem->GetData(szUserID, MAX_USER_ID_LEN); 
	pWorkItem->GetData(szPassWord, MAX_PASSWORD_LEN);
	pWorkItem->GetULongLong(u64UniqueIDInDB);
	pWorkItem->GetULongLong(u64ServerSessionID);
	pWorkItem->GetULongLong(u64SessionIDInClient);

	if (pWorkItem->GetErrorCode() != 0)
	{
		return ;
	}

	LServerManager* pServerManager = m_pDBServerMainLogicThread->GetServerManager();
	LServer* pServer = pServerManager->FindServerBySessionID(u64ServerSessionID);
	if (pServer == NULL)
	{
		return ;
	}
	LUserInfoManager* pUserInfoManager = m_pDBServerMainLogicThread->GetUserInfoManager();
	LUserInfo* pUserInfo = pUserInfoManager->FindUserInfo(szUserID);
	if (pUserInfo == NULL)
	{ 
		if (nResult == 0)		//	成功
		{
			pUserInfo = pUserInfoManager->AllocOneUserInfo();
			if (pUserInfo == NULL)
			{
				return ;
			}
			pUserInfo->SetUserID(szUserID);
			pUserInfo->SetUserPassWord(szPassWord);
			pUserInfo->SetUserUniqueIDInDB(u64UniqueIDInDB);
			pUserInfo->SetUserState(E_User_State_Data_Info_Initialized);
			pUserInfoManager->AddUserInfo(pUserInfo);
		}
		else
		{
			//	发送添加用户失败的数据包
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = m_pDBServerMainLogicThread->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_ADD_USER);
			int nResult = nResult;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64SessionIDInClient);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			m_pDBServerMainLogicThread->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			m_pDBServerMainLogicThread->FlushSendPacket();
			return ;
		}
	}
	else
	{
		if (nResult == 0)
		{
			pUserInfo->SetUserID(szUserID);
			pUserInfo->SetUserPassWord(szPassWord);
			pUserInfo->SetUserUniqueIDInDB(u64UniqueIDInDB);
			pUserInfo->SetUserState(E_User_State_Data_Info_Initialized);
		}
		else
		{ 
			//	从玩家管理器删除玩家信息
			pUserInfoManager->RemoveUserInfo(szUserID);
			//	发送添加用户失败的数据包
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = m_pDBServerMainLogicThread->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_ADD_USER);
			int nResult = nResult;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64SessionIDInClient);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			m_pDBServerMainLogicThread->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			m_pDBServerMainLogicThread->FlushSendPacket();
			return ;
		}
	}
	//	发送添加用户成功的数据包
	unsigned short usPacketLen = 128;
	LPacketBroadCast* pSendPacket = m_pDBServerMainLogicThread->GetOneSendPacket(usPacketLen);
	pSendPacket->SetPacketID(Packet_ADB2L_ADD_USER);
	nResult = 0;
	uint64_t u64UserUniqueIDInDB = u64UniqueIDInDB;
	pSendPacket->AddInt(nResult);
	pSendPacket->AddULongLong(u64SessionIDInClient);
	pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
	pSendPacket->AddULongLong(u64UniqueIDInDB);	
	m_pDBServerMainLogicThread->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
	m_pDBServerMainLogicThread->FlushSendPacket();
}
