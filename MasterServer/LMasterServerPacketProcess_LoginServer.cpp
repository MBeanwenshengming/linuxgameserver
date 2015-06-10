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

#include "LMasterServer_PacketProcess_Proc.h"
#include "../NetWork/LPacketSingle.h"
#include "LMainLogicThread.h" 
#include "LServerManager.h"
#include "../NetWork/LPacketBroadCast.h"
#include "../include/Server_To_Server_Packet_Define.h"
#include "LMasterServerMainLogic.h"
#include "LServer.h"
#include "LUserManager.h"
#include "LUser.h"
#include "../include/Global_Error_Define.h"

DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_L2M_USER_ONLINE)
{
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();
	LUserManager* pUserManager = pMainLogic->GetUserManager();

	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();
	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		return ;
	}
	unsigned char ucServerType = pServer->GetServerType();
	E_Server_Type eServerType = (E_Server_Type)ucServerType;
	if (eServerType != E_Server_Type_Login_Server)
	{
		return ;
	}

	uint64_t u64UserSessionIDInLoginServer = 0;
	char szUserID[MAX_USER_ID_LEN + 1];
	uint64_t u64UserUniqueIDInDB = 0;
	memset(szUserID, 0, sizeof(szUserID));

	pPacket->GetULongLong(u64UserSessionIDInLoginServer);
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetULongLong(u64UserUniqueIDInDB);

	int nResult = LMasterServerMainLogic::UserOnlineFromLoginServer(pMainLogic, szUserID, u64UserUniqueIDInDB);
	if (nResult == 0)
	{	
		//	加入玩家成功，那么发送消息给GateServer，让GateServer准备玩家登录事议
		uint64_t u64SelecttedGateServerSessionID = 0;
		bool bSelectSuccess = LServerManager::GetServerManagerInstance()->SelectBestGateServer(u64SelecttedGateServerSessionID);
		if (bSelectSuccess)		//	发送信息给GateServer服务器
		{
			if (pUserManager->UpdateUserState(u64UserUniqueIDInDB, E_User_Login_Waiting_GateServer_Reponse))
			{
				//	查找GateServer
				LServer* pGateServer = LServerManager::GetServerManagerInstance()->FindServerBySessionID(u64SelecttedGateServerSessionID);
				if (pGateServer == NULL)		//	错误
				{
					unsigned short usPacketLen = 128;
					LPacketBroadCast* pBSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
					if (pBSendPacket == NULL)
					{
						return ;
					}
					nResult = Global_Error_Unknown;
					pBSendPacket->SetPacketID(Packet_M2L_USER_ONLINE_VERIFY);
					pBSendPacket->AddInt(nResult);
					pBSendPacket->AddULongLong(u64UserSessionIDInLoginServer);
					pBSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
					pBSendPacket->AddULongLong(u64UserUniqueIDInDB);
					pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pBSendPacket);
					pMainLogic->FlushSendPacket();	
				}
				else
				{
					//	发送数据包给GateServer
					unsigned short usPacketLen = 128;
					LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
					if (pSendPacket == NULL)
					{
						return ;
					}
					pSendPacket->SetPacketID(Packet_M2GA_USER_WILL_LOGIN);
					pSendPacket->AddULongLong(u64UserSessionIDInLoginServer);
					pSendPacket->AddLongLong(pServer->GetNetWorkSessionID());
					pSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
					pSendPacket->AddULongLong(u64UserUniqueIDInDB);
					pMainLogic->SendOnePacket(pGateServer->GetNetWorkSessionID(), pGateServer->GetSendThreadID(), pSendPacket);
					pMainLogic->FlushSendPacket();	

				}
			}
			else	//	失败，那么发送失败给登录服务器
			{
				unsigned short usPacketLen = 128;
				LPacketBroadCast* pBSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
				if (pBSendPacket == NULL)
				{
					return ;
				}
				nResult = Global_Error_Unknown;
				pBSendPacket->SetPacketID(Packet_M2L_USER_ONLINE_VERIFY);
				pBSendPacket->AddInt(nResult);
				pBSendPacket->AddULongLong(u64UserSessionIDInLoginServer);
				pBSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
				pBSendPacket->AddULongLong(u64UserUniqueIDInDB);
				pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pBSendPacket);
				pMainLogic->FlushSendPacket();	
			}
		}
		else	//	选择服务器失败，那么发送登录失败给登录服务器
		{
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pBSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
			if (pBSendPacket == NULL)
			{
				return ;
			}	
			nResult = Global_Error_Unknown;
			pBSendPacket->SetPacketID(Packet_M2L_USER_ONLINE_VERIFY);
			pBSendPacket->AddInt(nResult);
			pBSendPacket->AddULongLong(u64UserSessionIDInLoginServer);
			pBSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
			pBSendPacket->AddULongLong(u64UserUniqueIDInDB);
			pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pBSendPacket);
			pMainLogic->FlushSendPacket();	
		} 
	}
	else	//	加入玩家失败，那么发送给客户端，登录失败
	{
		unsigned short usPacketLen = 128;
		LPacketBroadCast* pBSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
		if (pBSendPacket == NULL)
		{
			return ;
		}
		pBSendPacket->SetPacketID(Packet_M2L_USER_ONLINE_VERIFY);
		pBSendPacket->AddInt(nResult);
		pBSendPacket->AddULongLong(u64UserSessionIDInLoginServer);
		pBSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
		pBSendPacket->AddULongLong(u64UserUniqueIDInDB);
		pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pBSendPacket);
		pMainLogic->FlushSendPacket();	
	}
}

DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_L2M_USER_OFFLINE)
{ 
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();
	LUserManager* pUserManager = pMainLogic->GetUserManager();

	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();
	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		return ;
	}
	char szUserID[MAX_USER_ID_LEN + 1];
	uint64_t u64UserUniqueIDInDB = 0;
	memset(szUserID, 0, sizeof(szUserID));

	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetULongLong(u64UserUniqueIDInDB);

	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}
	LUser* pUser = pUserManager->FindUser(u64UserUniqueIDInDB);
	if (pUser == NULL)
	{
		return ;
	}
	E_User_State eUserState = pUser->GetUserState();
	if (eUserState <= E_User_Login_Waiting_GateServer_Reponse)	//	等待gateserver返回，那么从UserManager中移除玩家
	{
		pUserManager->RemoveUser(u64UserUniqueIDInDB);
	}
}

