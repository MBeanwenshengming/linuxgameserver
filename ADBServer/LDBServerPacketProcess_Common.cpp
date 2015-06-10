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

#include "LDBServerPacketProcess.h"
#include "../NetWork/LPacketBroadCast.h"
#include "LServerManager.h"
#include "LServer.h"
#include "LDBServerMainLogicThread.h"
#include "LDBServerConnectToMasterServer.h"
#include "LUserInfoManager.h"
#include "LUserInfo.h"
#include "LDBOpThread.h"
#include "LWorkItem.h"
#include "../include/Global_Error_Define.h"

DEFINE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req)
{
	if (eFromType == E_DBServer_Packet_From_Client)
	{ 
		LDBServerMainLogicThread* pDBServerMainLogic = pDBServerPacketProcessProc->GetDBServerMainLogicThread();

		LServerManager* pServerManager = pDBServerMainLogic->GetServerManager();
		LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
		if (pServer == NULL)
		{
			return ;
		}
		unsigned short usPacketLen = 100;
		LPacketBroadCast* pPacketBroadCast = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
		pPacketBroadCast->SetPacketID(Packet_SS_Start_Res);
		pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pPacketBroadCast);
		pDBServerMainLogic->FlushSendPacket(); 
	}
	else
	{
		return ;
	}
}

DEFINE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res)
{
	if (eFromType == E_DBServer_Packet_From_Master)
	{ 
		LDBServerMainLogicThread* pDBServerMainLogic = pDBServerPacketProcessProc->GetDBServerMainLogicThread();

		LDBServerConnectToMasterServer* pConToMasterServer = pDBServerMainLogic->GetDBServerConnectToMasterServer();

		uint64_t u64ServerUniqueID = pDBServerMainLogic->GetServerUniqueID();
		unsigned short usPacketLen = 100;
		LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Register_Server_Req1);
		pSendPacket->AddULongLong(u64ServerUniqueID);
		pConToMasterServer->AddOneSendPacket(pSendPacket);
	}
}

DEFINE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Res)
{
}

DEFINE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req)
{ 
	if (eFromType == E_DBServer_Packet_From_Master)
	{
		uint64_t u64RequestServerID = 0;	//	请求的服务器
		uint64_t u64DestServerID	= 0;	//	目标服务器

		pPacket->GetULongLong(u64RequestServerID); 
		pPacket->GetULongLong(u64DestServerID);


		LDBServerMainLogicThread* pDBServerMainLogic = pDBServerPacketProcessProc->GetDBServerMainLogicThread(); 
		LServerManager* pServerManager = pDBServerMainLogic->GetServerManager();

		LDBServerConnectToMasterServer* pConToMasterServer = pDBServerMainLogic->GetDBServerConnectToMasterServer();

		char szIp[20]; memset(szIp, 0, sizeof(szIp));
		unsigned short usPort = 0;
		pDBServerMainLogic->GetListenIpAndPort(szIp, sizeof(szIp) - 1, usPort);

		uint64_t u64ServerUniqueID = pDBServerMainLogic->GetServerUniqueID();
		if (u64ServerUniqueID != u64DestServerID)
		{
			return ;
		}

		if (!pServerManager->AddWillConnectToServer(u64RequestServerID))
		{
			return ;
		}

		int nResCode = 0;
		unsigned short usPacketLen = 100;
		LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Res);
		pSendPacket->AddInt(nResCode);
		pSendPacket->AddULongLong(u64RequestServerID);
		pSendPacket->AddULongLong(u64DestServerID);
		pSendPacket->AddData(szIp, 20);
		pSendPacket->AddUShort(usPort);
		pConToMasterServer->AddOneSendPacket(pSendPacket);
	}
}

DEFINE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1)
{
	LDBServerMainLogicThread* pDBServerMainLogic = pDBServerPacketProcessProc->GetDBServerMainLogicThread(); 
	if (eFromType == E_DBServer_Packet_From_Client)
	{
		uint64_t u64UniqueServerID = 0;
		pPacket->GetULongLong(u64UniqueServerID);
		if (pPacket->GetErrorCode() != 0)
		{
			return ;
		}
		LServerID serverID;
		if (!serverID.SetServerID(u64UniqueServerID))
		{
			return ;
		}
		unsigned char ucServerType = serverID.GetServerType();
		E_Server_Type eServerType = (E_Server_Type)ucServerType;
		if (eServerType != E_Server_Type_Game_Server && eServerType != E_Server_Type_Lobby_Server)
		{
			return ;
		}

		LServerManager* pServerManager = pDBServerMainLogic->GetServerManager();
		if (pServerManager->FindServerBySessionID(u64SessionID) == NULL)
		{
			return;
		}
		if (pServerManager->FindServerByServerID(u64UniqueServerID))
		{
			return ;
		}
		pServerManager->SetServerID(u64SessionID, u64UniqueServerID);
	}
}


DEFINE_DBSERVER_PACKET_PROCESS_PROC(Packet_L2ADB_USER_VERIFY)
{
	char szUserID[MAX_USER_ID_LEN + 1];
	char szPassWord[MAX_PASSWORD_LEN + 1];
	uint64_t u64UserSessionID = 0;		//	验证用户的连接信息，返回给AccountServer时，用来查找玩家
	memset(szUserID, 0, sizeof(szUserID));
	memset(szPassWord, 0, sizeof(szPassWord));

	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetData(szPassWord, MAX_PASSWORD_LEN);
	pPacket->GetULongLong(u64UserSessionID);

	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}

	LDBServerMainLogicThread* pDBServerMainLogic = pDBServerPacketProcessProc->GetDBServerMainLogicThread(); 
	LServerManager* pServerManager = pDBServerMainLogic->GetServerManager();

	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		return ;
	}
	LUserInfoManager* pUserInfoManager = pDBServerMainLogic->GetUserInfoManager();
	LUserInfo* pUserInfo = pUserInfoManager->FindUserInfo(szUserID);
	if (pUserInfo == NULL)
	{
		LUserInfo* pUserTemp = pUserInfoManager->AllocOneUserInfo();
		if (pUserTemp == NULL)
		{
			//	发送错误给客户端服务器，告诉验证帐号失败
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
			int nResult = Global_Error_Unknown;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64UserSessionID);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			pDBServerMainLogic->FlushSendPacket();
			return ;
		}
		pUserTemp->SetUserID(szUserID);
		pUserTemp->SetUserPassWord(szPassWord);
		pUserTemp->SetUserState(E_User_State_Getting_Info_From_DB);
		pUserInfoManager->AddUserInfo(pUserTemp);

		//	发送一个数据库操作给数据库线程，检查玩家名称和密码是否一致
		LDBOpThread* pDBOpThread = pDBServerMainLogic->GetDBOpThread();

		unsigned short usPacketLen = 128;
		LWorkItem* pWorkItem = pDBOpThread->GetOneFreeWorkItemFromPool(usPacketLen);
		if (pWorkItem == NULL)
		{
			//	发送错误给客户端服务器，告诉验证帐号失败
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
			int nResult = Global_Error_Unknown;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64UserSessionID);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			pDBServerMainLogic->FlushSendPacket();
			return ;
		}
		//	workUniqueID设置为0,表明本地没有与之对应的数据，数据库返回后逻辑线程不需要额外的数据来处理
		pWorkItem->SetWorkUniqueID(0);
		pWorkItem->SetWorkID(ADB_SERVER_MT2DBT_READ_USER_INFO);
		pWorkItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkItem->AddULongLong(u64SessionID);
		pWorkItem->AddULongLong(u64UserSessionID);
		//	pWorkItem
		if (!pDBOpThread->AddWorkItemToWorkQueue(pWorkItem))
		{
			delete pWorkItem;
			//	发送错误给客户端服务器，告诉验证帐号失败
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
			int nResult = Global_Error_Unknown;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64UserSessionID);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			pDBServerMainLogic->FlushSendPacket();
			return ;
		}
		//	等待数据库处理返回，主线程收到消息包后，会处理	
	}
	else
	{
		E_User_State eUserState = pUserInfo->GetUserState();
		if (eUserState <= E_User_State_Getting_Info_From_DB)
		{
			//	正在读取或者创建玩家，返回失败给客户端
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
			int nResult = Global_Error_Unknown;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64UserSessionID);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			pDBServerMainLogic->FlushSendPacket();
			return;
		}
		//	状态正常，那么发送数据给客户端服务器
		if (pUserInfo->CheckUserAndPassWord(szUserID, szPassWord))
		{
			//	验证通过
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
			int nResult = 0;
			uint64_t u64UserUniqueIDInDB = pUserInfo->GetUserUniqueIDInDB();
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64UserSessionID);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			pDBServerMainLogic->FlushSendPacket();
		}
		else
		{
			//	验证失败
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_USER_VERIFY);
			int nResult = Global_Error_User_Password_Error;
			uint64_t u64UserUniqueIDInDB = pUserInfo->GetUserUniqueIDInDB();
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64UserSessionID);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			pDBServerMainLogic->FlushSendPacket();
		}
	}
}

DEFINE_DBSERVER_PACKET_PROCESS_PROC(Packet_L2ADB_ADD_USER)
{
	char szUserID[MAX_USER_ID_LEN + 1];
	char szPassWord[MAX_PASSWORD_LEN + 1];
	uint64_t u64UserSessionID = 0;		//	验证用户的连接信息，返回给AccountServer时，用来查找玩家
	
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetData(szPassWord, MAX_PASSWORD_LEN);
	pPacket->GetULongLong(u64UserSessionID);
	if (pPacket->GetErrorCode() != 0)
	{
		return;
	}

	LDBServerMainLogicThread* pDBServerMainLogic = pDBServerPacketProcessProc->GetDBServerMainLogicThread(); 
	LServerManager* pServerManager = pDBServerMainLogic->GetServerManager();

	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		return ;
	}
	LUserInfoManager* pUserInfoManager = pDBServerMainLogic->GetUserInfoManager();
	LUserInfo* pUserInfo = pUserInfoManager->FindUserInfo(szUserID);
	if (pUserInfo != NULL)	
	{
		E_User_State eUserState = pUserInfo->GetUserState();
		if (eUserState > E_User_State_Getting_Info_From_DB)
		{
			//	已经存在该帐号，无法创建，这样就不用进行数据库操作了
			unsigned short usPacketLen = 128;
			LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_ADB2L_ADD_USER);
			int nResult = Global_Error_User_Already_Online;
			uint64_t u64UserUniqueIDInDB = 0;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64UserSessionID);
			pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
			pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
			pDBServerMainLogic->FlushSendPacket();
			return;
		}
	}
	//	给数据库线程发送创建帐号的指令 
	LDBOpThread* pDBOpThread = pDBServerMainLogic->GetDBOpThread();

	unsigned short usPacketLen = 128;
	LWorkItem* pWorkItem = pDBOpThread->GetOneFreeWorkItemFromPool(usPacketLen);
	if (pWorkItem == NULL)
	{
		//	发送错误给客户端服务器，告诉验证帐号失败
		unsigned short usPacketLen = 128;
		LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_ADB2L_ADD_USER);
		int nResult = Global_Error_Unknown;
		uint64_t u64UserUniqueIDInDB = 0;
		pSendPacket->AddInt(nResult);
		pSendPacket->AddULongLong(u64UserSessionID);
		pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
		pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
		pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
		pDBServerMainLogic->FlushSendPacket();
		return ;
	}
	//	workUniqueID设置为0,表明本地没有与之对应的数据，数据库返回后逻辑线程不需要额外的数据来处理
	pWorkItem->SetWorkUniqueID(0);
	pWorkItem->SetWorkID(ADB_SERVER_MT2DBT_ADD_USER_INFO);
	pWorkItem->AddData(szUserID, MAX_USER_ID_LEN);
	pWorkItem->AddData(szPassWord, MAX_PASSWORD_LEN);
	pWorkItem->AddULongLong(u64SessionID);
	pWorkItem->AddULongLong(u64UserSessionID);
	//	pWorkItem
	if (!pDBOpThread->AddWorkItemToWorkQueue(pWorkItem))
	{
		delete pWorkItem;
		//	发送错误给客户端服务器，告诉验证帐号失败
		unsigned short usPacketLen = 128;
		LPacketBroadCast* pSendPacket = pDBServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_ADB2L_ADD_USER);
		int nResult = Global_Error_Unknown;
		uint64_t u64UserUniqueIDInDB = 0;
		pSendPacket->AddInt(nResult);
		pSendPacket->AddULongLong(u64UserSessionID);
		pSendPacket->AddData(szUserID, MAX_PASSWORD_LEN);
		pSendPacket->AddULongLong(u64UserUniqueIDInDB);	
		pDBServerMainLogic->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
		pDBServerMainLogic->FlushSendPacket();
		return ;
	}
}

