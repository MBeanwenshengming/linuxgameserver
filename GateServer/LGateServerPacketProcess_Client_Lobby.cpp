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

#include "LGateServerPacketProcess_Proc.h"
#include "../NetWork/LPacketSingle.h"
#include "LGateServerMainLogic.h"
#include "LConnectToMasterServer.h"
#include "LConnectToServerNetWork.h"
#include "LServerManager.h"
#include "../NetWork/LPacketBroadCast.h"
#include "LClient.h"
#include "LClientManager.h"
#include "LServer.h"


DEFINE_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_ENTER_LOBBYSERVER)
{ 
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();

	LClientManager* pClientManager = pGateServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FindClientBySessionID(u64SessionID);
	if (pClient == NULL)
	{
		return ;
	}
	E_Client_State eClientState = pClient->GetClientState();
	if (eClientState != E_Client_State_Logined)
	{
		return ;
	}
	
	//	向MasterServer请求LobbyServer和GameDBServer
	pClient->SetClientState(E_Client_State_Request_Lobby);
	uint64_t u64UserUniqueIDInDB = pClient->GetUniqueIDInDB();

	LConnectToMasterServer* pConToMaster = pGateServerMainLogic->GetConnectToMasterServer();
	unsigned short usPacketLen = 32;
	LPacketSingle* pSendPacket = pConToMaster->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_GA2M_SELECT_LOBBYSERVER);
	pSendPacket->AddULongLong(u64SessionID); 
	pSendPacket->AddULongLong(u64UserUniqueIDInDB);
	pConToMaster->AddOneSendPacket(pSendPacket);
}

DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_SELECT_LOBBYSERVER)
{
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();

	int nResult = 0;
	uint64_t u64SessionIDInGateServer = 0;
	uint64_t u64UserUniqueIDInDBOnlineFromGateServer = 0;
	uint64_t u64LobbyServerUniqueServerID = 0;

	pPacket->GetInt(nResult);
	pPacket->GetULongLong(u64SessionIDInGateServer);
	pPacket->GetULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pPacket->GetULongLong(u64LobbyServerUniqueServerID);
	if (pPacket->GetErrorCode() != 0)
	{
		return;
	}
	LClientManager* pClientManager = pGateServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FineClientByUserUniqueIDInDB(u64UserUniqueIDInDBOnlineFromGateServer);
	if (pClient == NULL)
	{
		return ;
	}
	if (nResult != 0)
	{ 
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pGateServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_GA2C_ENTER_LOBBYSERVER);
		pSendPacket->AddInt(nResult);
		pGateServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pGateServerMainLogic->FlushSendPacket();

		pGateServerMainLogic->AddSessionIDToKickOutQueue(pClient->GetSessionID(), 3);
		return ;
	}

	LServerManager* pServerManager = pGateServerMainLogic->GetServerManager();
	LServer* pServer = pServerManager->FindServerByServerID(u64LobbyServerUniqueServerID);
	if (pServer == NULL)
	{ 
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pGateServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_GA2C_ENTER_LOBBYSERVER);
		pSendPacket->AddInt(nResult);
		pGateServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pGateServerMainLogic->FlushSendPacket();

		pGateServerMainLogic->AddSessionIDToKickOutQueue(pClient->GetSessionID(), 3);
		return ;
	}

	//	设置玩家状态
	pClient->SetClientState(E_Client_State_Loginning_Lobby);

	char szUserID[MAX_USER_ID_LEN + 1];
	memset(szUserID, 0, sizeof(szUserID));
	pClient->GetUserID(szUserID, MAX_USER_ID_LEN);

	//	向LobbyServer发送登录协议包
	LConnectToServerNetWork* pConToServerNetWork = pGateServerMainLogic->GetConnectToServerNetWork();
	unsigned short usPacketLen = 64;
	LPacketBroadCast* pSendPacketToLobby = pConToServerNetWork->GetOneSendPacket(usPacketLen);
	pSendPacketToLobby->SetPacketID(Packet_GA2LB_USER_ONLINE);
	pSendPacketToLobby->AddULongLong(pClient->GetSessionID());
	pSendPacketToLobby->AddULongLong(pClient->GetUniqueIDInDB());
	pSendPacketToLobby->AddData(szUserID, MAX_USER_ID_LEN);
	pConToServerNetWork->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacketToLobby);
	pConToServerNetWork->FlushSendPacket();
}

DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_LB2GA_USER_ONLINE)
{
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();


	int nResult = 0;
	uint64_t u64SessionIDInGateServer = 0;
	uint64_t u64UserUniqueIDInDB = 0;

	pPacket->GetInt(nResult);
	pPacket->GetULongLong(u64SessionIDInGateServer);
	pPacket->GetULongLong(u64UserUniqueIDInDB);
	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}

	LClientManager* pClientManager = pGateServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FindClientBySessionID(u64SessionIDInGateServer);
	if (pClient == NULL)
	{
		return ;
	}
	if (nResult == 0)
	{
		pClient->SetClientState(E_Client_State_In_Lobby);
		unsigned short usPacketLen = 16;
		LPacketBroadCast* pSendPacket = pGateServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_GA2C_ENTER_LOBBYSERVER);
		pSendPacket->AddInt(nResult);
		pGateServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pGateServerMainLogic->FlushSendPacket(); 
	}
	else
	{ 
		unsigned short usPacketLen = 16;
		LPacketBroadCast* pSendPacket = pGateServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_GA2C_ENTER_LOBBYSERVER);
		pSendPacket->AddInt(nResult);
		pGateServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pGateServerMainLogic->FlushSendPacket();

		pGateServerMainLogic->AddSessionIDToKickOutQueue(pClient->GetSessionID(), 5);
	}
}


DEFINE_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_GET_ROLE_INFO)
{ 
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();


	LClientManager* pClientManager = pGateServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FindClientBySessionID(u64SessionID);
	if (pClient == NULL)
	{
		return ;
	}
	E_Client_State eClientState = pClient->GetClientState();
	if (eClientState != E_Client_State_In_Lobby)		//	不是在登录在LobbyServer下的客户端无法发送该数据包
	{
		return;
	}
	uint64_t u64CurrentServerUniqueID = pClient->GetCurrentServerUniqueID();
	LServerManager* pServerManager = pGateServerMainLogic->GetServerManager();
	LServer* pServer = pServerManager->FindServerByServerID(u64CurrentServerUniqueID);
	if (pServer == NULL)
	{
		return;
	}
	//	向该Server发送读取玩家角色的信息包
	LConnectToServerNetWork* pConToServerNetWork = pGateServerMainLogic->GetConnectToServerNetWork();

	char szUserID[MAX_USER_ID_LEN + 1];
	memset(szUserID, 0, sizeof(szUserID));
	pClient->GetUserID(szUserID, MAX_USER_ID_LEN);

	unsigned short usPacketLen = 64;
	LPacketBroadCast* pSendPacket = pConToServerNetWork->GetOneSendPacket(usPacketLen);
	pSendPacket->SetPacketID(Packet_GA2LB_USER_ONLINE);
	pSendPacket->AddULongLong(pClient->GetSessionID());
	pSendPacket->AddULongLong(pClient->GetUniqueIDInDB());
	pSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
	pConToServerNetWork->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
	pConToServerNetWork->FlushSendPacket();
}
