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
#include "../include/Global_Error_Define.h"
#include "../NetWork/LPacketBroadCast.h"
#include "../include/Server_To_Server_Packet_Define.h"
#include "LServer.h"

DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_WILL_LOGIN)
{
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();
	LUserManager* pUserManager = pMainLogic->GetUserManager();

	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();

	int nResult 									= 0;
	uint64_t u64UserSessionIDInLoginServer 			= 0;
	uint64_t u64LoginServerSessionIDInMasterServer 	= 0;
	char szUserID[MAX_USER_ID_LEN + 1];
	uint64_t u64UserUniqueIDInDB 					= 0;
	char szConnectToGateServerUniqueKey[128];
	char szServerIP[MAX_USER_ID_LEN + 1];
	unsigned short usPort 							= 0;
	
	memset(szUserID, 0, sizeof(szUserID));
	memset(szConnectToGateServerUniqueKey, 0, sizeof(szConnectToGateServerUniqueKey));
	memset(szServerIP, 0, sizeof(szServerIP));

	pPacket->GetInt(nResult);
	pPacket->GetULongLong(u64UserSessionIDInLoginServer);
	pPacket->GetULongLong(u64LoginServerSessionIDInMasterServer);
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetULongLong(u64UserUniqueIDInDB);
	if (pPacket->GetErrorCode() != 0)
	{
		return;
	}

	//	查找LoginServer
	LServer* pServer = pServerManager->FindServerBySessionID(u64LoginServerSessionIDInMasterServer);
	if (pServer == NULL)
	{
		//	查找玩家，删除玩家信息
		LUser* pUser = pUserManager->FindUser(u64UserUniqueIDInDB);
		if (pUser != NULL)
		{
			pUserManager->RemoveUser(u64UserUniqueIDInDB); 
		}
		return ;
	}
	//	玩家已经下线，那么直接返回
	LUser* pUser = pUserManager->FindUser(u64UserUniqueIDInDB);
	if (pUser == NULL)
	{
		return ;
	}
	if (nResult == 0)
	{
		pPacket->GetData(szConnectToGateServerUniqueKey, 128);
		pPacket->GetData(szServerIP, MAX_SERVER_IP);
		pPacket->GetUShort(usPort);
		if (pPacket->GetErrorCode() != 0)
		{
			//	给LoginServer发送消息，告诉LS，验证失败
			unsigned short usPacketLen = 512;
			LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_M2L_USER_ONLINE_VERIFY);
			nResult = Global_Error_Unknown;
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64UserSessionIDInLoginServer);
			pSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);

			pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pSendPacket);
			pMainLogic->FlushSendPacket();

			//	移除玩家信息
			pUserManager->RemoveUser(u64UserUniqueIDInDB); 
			 
			return ;
		}
		//	发送成功的数据包给LS
		unsigned short usPacketLen = 512;
		LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_M2L_USER_ONLINE_VERIFY);
		pSendPacket->AddInt(nResult);
		pSendPacket->AddULongLong(u64UserSessionIDInLoginServer);
		pSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
		pSendPacket->AddULongLong(u64UserUniqueIDInDB);
		pSendPacket->AddData(szConnectToGateServerUniqueKey, 128);
		pSendPacket->AddData(szServerIP, MAX_USER_ID_LEN);
		pSendPacket->AddUShort(usPort);
		pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pSendPacket);
		pMainLogic->FlushSendPacket();

		pUser->SetUserState(E_User_Login_Waiting_GateServer_Login);
	}
	else		//	GateServer验证失败，发送给LS失败信息
	{ 
		unsigned short usPacketLen = 512;
		LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_M2L_USER_ONLINE_VERIFY);
		pSendPacket->AddInt(nResult);
		pSendPacket->AddULongLong(u64UserSessionIDInLoginServer);
		pSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
		pSendPacket->AddULongLong(u64UserUniqueIDInDB);

		pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pSendPacket);
		pMainLogic->FlushSendPacket();
		//	移除玩家信息
		pUserManager->RemoveUser(u64UserUniqueIDInDB);
	}
}

//	玩家在gateserver退出，或者登录超时
DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_GATESERVER_OFFLINE)
{ 
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();
	LUserManager* pUserManager = pMainLogic->GetUserManager();

	uint64_t u64UserUniqueIDInDB = 0;
	pPacket->GetULongLong(u64UserUniqueIDInDB);
	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}
	LUser* pUser = pUserManager->FindUser(u64UserUniqueIDInDB);
	if (pUser == NULL)
	{
		return;
	}
	E_User_State eUserState = pUser->GetUserState();
	if (eUserState >= E_User_Login_Waiting_GateServer_Reponse)
	{ 
		pUserManager->RemoveUser(u64UserUniqueIDInDB);
	}
}


DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_GATESERVER_ONLINE)
{
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();
	LUserManager* pUserManager = pMainLogic->GetUserManager();
	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();
	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		return ;
	}

	uint64_t u64SessionIDInGateServer = 0;
	uint64_t u64UserUniqueIDInDBOnlineFromGateServer = 0;

	pPacket->GetULongLong(u64SessionIDInGateServer);
	pPacket->GetULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	
	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}
	LUser* pUser = pUserManager->FindUser(u64UserUniqueIDInDBOnlineFromGateServer);
	if (pUser == NULL)
	{
		//	发送错误消息包给GateServer
		int nResult = -1;
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_M2GA_USER_GATESERVER_ONLINE);
		pSendPacket->AddInt(nResult);
		pSendPacket->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
		pSendPacket->AddULongLong(u64SessionIDInGateServer);
		pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pSendPacket);
		pMainLogic->FlushSendPacket();
		return ;
	}
	pUser->SetUserState(E_User_Connectted_To_GateServer);
	//	发送成功消息包给GateServer
	
	int nResult = 0;
	unsigned short usPacketLen = 32;
	LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
	pSendPacket->SetPacketID(Packet_M2GA_USER_GATESERVER_ONLINE);
	pSendPacket->AddInt(nResult);
	pSendPacket->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pSendPacket->AddULongLong(u64SessionIDInGateServer);
	pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pSendPacket);
	pMainLogic->FlushSendPacket();

}


DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_SELECT_LOBBYSERVER)
{
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();
	LUserManager* pUserManager = pMainLogic->GetUserManager();
	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();

	LServer* pSrcServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pSrcServer == NULL)
	{
		return ;
	}

	uint64_t u64SessionIDInGateServer = 0;
	uint64_t u64UserUniqueIDInDBOnlineFromGateServer = 0;
	
	pPacket->GetULongLong(u64SessionIDInGateServer);
	pPacket->GetULongLong(u64UserUniqueIDInDBOnlineFromGateServer);

	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}
	LUser* pUser = pUserManager->FindUser(u64UserUniqueIDInDBOnlineFromGateServer);
	if (pUser == NULL)
	{
		//	发送进入Lobby失败的数据包
		int nEnterResult = -1;
		uint64_t u64LobbyServerUniqueServerID = 0;

		unsigned short usPacketLen = 128;
		LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_M2GA_SELECT_LOBBYSERVER);
		pSendPacket->AddInt(nEnterResult);
		pSendPacket->AddULongLong(u64SessionIDInGateServer);
		pSendPacket->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
		pSendPacket->AddULongLong(u64LobbyServerUniqueServerID);
		pMainLogic->SendOnePacket(pSrcServer->GetNetWorkSessionID(), pSrcServer->GetSendThreadID(), pSendPacket);
		pMainLogic->FlushSendPacket();
		return ;
	}
	E_User_State eUserState = pUser->GetUserState();
	if (eUserState < E_User_Connectted_To_GateServer)
	{ 
		return ;
	}
	//	给玩家选择LobbyServer和GameDBServer
	uint64_t u64SelectedLobbyServerUniqueID = 0;
	uint64_t u64SelectedGameDBServerUniqueID = 0;
	if (!pServerManager->SelectBestLobbyServerAndGameDBServer(u64SelectedLobbyServerUniqueID, u64SelectedGameDBServerUniqueID))
	{
		//	发送失败信息
		int nEnterResult = -1;
		uint64_t u64LobbyServerUniqueServerID = 0;

		unsigned short usPacketLen = 128;
		LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_M2GA_SELECT_LOBBYSERVER);
		pSendPacket->AddInt(nEnterResult);
		pSendPacket->AddULongLong(u64SessionIDInGateServer);
		pSendPacket->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
		pSendPacket->AddULongLong(u64LobbyServerUniqueServerID);
		pMainLogic->SendOnePacket(pSrcServer->GetNetWorkSessionID(), pSrcServer->GetSendThreadID(), pSendPacket);
		pMainLogic->FlushSendPacket();
		return ;
	}
	//	选择服务器成功，那么通知LobbyServer，有玩家登录上来
	LServer* pLobbyServer = pServerManager->FindServer(u64SelectedLobbyServerUniqueID);
	if (pLobbyServer == NULL)
	{
		//	发送失败信息
		int nEnterResult = -1;
		uint64_t u64LobbyServerUniqueServerID = 0;

		unsigned short usPacketLen = 128;
		LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_M2GA_SELECT_LOBBYSERVER);
		pSendPacket->AddInt(nEnterResult);
		pSendPacket->AddULongLong(u64SessionIDInGateServer);
		pSendPacket->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
		pSendPacket->AddULongLong(u64LobbyServerUniqueServerID);
		pMainLogic->SendOnePacket(pSrcServer->GetNetWorkSessionID(), pSrcServer->GetSendThreadID(), pSendPacket);
		pMainLogic->FlushSendPacket();
		return ;
	}
	char szUserID[MAX_USER_ID_LEN + 1];
	memset(szUserID, 0, sizeof(szUserID));
	pUser->GetUserID(szUserID, sizeof(szUserID));

	unsigned short usPacketLen = 128;
	LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
	pSendPacket->SetPacketID(Packet_M2LB_NOTIFY_USER_WILL_ONLINE);
	pSendPacket->AddULongLong(u64SessionIDInGateServer);
	pSendPacket->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pSendPacket->AddULongLong(pSrcServer->GetNetWorkSessionID());
	pSendPacket->AddULongLong(u64SelectedGameDBServerUniqueID);
	pSendPacket->AddULongLong(u64SelectedLobbyServerUniqueID);
	pSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
	pMainLogic->SendOnePacket(pLobbyServer->GetNetWorkSessionID(), pLobbyServer->GetSendThreadID(), pSendPacket);
	pMainLogic->FlushSendPacket();

}

DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_LB2M_NOTIFY_USER_WILL_ONLINE)
{
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();
	LUserManager* pUserManager = pMainLogic->GetUserManager();
	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();


	int nResult = 0;
	uint64_t u64SessionIDInGateServer = 0;
	uint64_t u64UserUniqueIDInDBOnlineFromGateServer = 0;
	uint64_t u64GateServerSessionIDInMasterServer = 0;
	uint64_t u64LobbyServerUniqueID = 0; 

	pPacket->GetInt(nResult);
	pPacket->GetULongLong(u64SessionIDInGateServer);
	pPacket->GetULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pPacket->GetULongLong(u64GateServerSessionIDInMasterServer);
	pPacket->GetULongLong(u64LobbyServerUniqueID);

	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}
	LServer* pGateServer = pServerManager->FindServerBySessionID(u64GateServerSessionIDInMasterServer);
	if (pGateServer == NULL)
	{
		return ;
	}

	unsigned short usPacketLen = 128;
	LPacketBroadCast* pSendPacket = pMainLogic->GetOneSendPacket(usPacketLen);
	pSendPacket->SetPacketID(Packet_M2GA_SELECT_LOBBYSERVER);
	pSendPacket->AddInt(nResult);
	pSendPacket->AddULongLong(u64SessionIDInGateServer);
	pSendPacket->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pSendPacket->AddULongLong(u64LobbyServerUniqueID);
	pMainLogic->SendOnePacket(pGateServer->GetNetWorkSessionID(), pGateServer->GetSendThreadID(), pSendPacket);
	pMainLogic->FlushSendPacket();
}
