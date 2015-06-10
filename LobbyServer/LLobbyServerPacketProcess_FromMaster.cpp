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

#include "LLobbyServerPacketProcess_Proc.h"
#include "../NetWork/LPacketSingle.h"
#include "LLobbyServerMainLogic.h"
#include "../include/Server_Define.h"
#include "../include/Server_To_Server_Packet_Define.h"
#include "LLBConnectToServersNetWork.h" 
#include "LLBGateServerManager.h"
#include "LLBServerManager.h"
#include "LLBConnectToMasterServer.h"
#include "../NetWork/LPacketBroadCast.h"
#include "LLBGateServer.h"



DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_M2LB_NOTIFY_USER_WILL_ONLINE)
{ 
	LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();

	uint64_t u64SessionIDInGateServer = 0;
	uint64_t u64UserUniqueIDInDBOnlineFromGateServer = 0;
	uint64_t u64GateServerSessionIDInMasterServer = 0;
	uint64_t u64GameDBUniqueServerID = 0;		//	被选择的GameDBServer的服务器ID
	uint64_t u64LobbyServerUniqueID = 0;		//	被选择的LobbyServer的服务器ID
	char szUserID[MAX_USER_ID_LEN + 1];			//	玩家角色名称
	memset(szUserID, 0, sizeof(szUserID));

	pPacket->GetULongLong(u64SessionIDInGateServer);
	pPacket->GetULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pPacket->GetULongLong(u64GateServerSessionIDInMasterServer);
	pPacket->GetULongLong(u64GameDBUniqueServerID);
	pPacket->GetULongLong(u64LobbyServerUniqueID);
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	if (pPacket->GetErrorCode() != 0)
	{
		return;
	}

	LLBServerManager* pServerManager = pLobbyServerMainLogic->GetConnectToServerManager();
	LLBServer* pGameDBServer = pServerManager->FindServerByServerID(u64GameDBUniqueServerID);

	LLBConnectToMasterServer* pConToMasterServer = pLobbyServerMainLogic->GetConnectToMasterServer();
	if (pGameDBServer == NULL)
	{
		//	向MasterServer发送消息，推荐的GameDBServer不存在	
		int nResult = -1;
		unsigned short usPacketLen = 64;
		LPacketSingle* pPacketSingle = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
		pPacketSingle->SetPacketID(Packet_LB2M_NOTIFY_USER_WILL_ONLINE);
		pPacketSingle->AddInt(nResult);
		pPacketSingle->AddULongLong(u64SessionIDInGateServer);
		pPacketSingle->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
		pPacketSingle->AddULongLong(u64GateServerSessionIDInMasterServer);
		pPacketSingle->AddULongLong(u64LobbyServerUniqueID);
		pConToMasterServer->AddOneSendPacket(pPacketSingle);
		return ;
	}
	//	向GameDBServer发送消息，预读玩家角色信息
	LLBConnectToServersNetWork* pConToServersNetWork = pLobbyServerMainLogic->GetConnectToServerNetWork();
	unsigned short usPacketLen = 64;
	LPacketBroadCast* pSendToGameDBServer = pConToServersNetWork->GetOneSendPacket(usPacketLen);
	pSendToGameDBServer->SetPacketID(Packet_LB2GDB_NOTIFY_USER_WILL_ONLINE);
	pSendToGameDBServer->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pSendToGameDBServer->AddData(szUserID, MAX_USER_ID_LEN);
	pConToServersNetWork->SendOnePacket(pGameDBServer->GetSessionID(), pGameDBServer->GetSendThreadID(), pSendToGameDBServer);
	pConToServersNetWork->FlushSendPacket();
	//	添加即将登录的玩家到预登录管理器中
	LUserManager* pUserManager = pLobbyServerMainLogic->GetUserManager();
	pUserManager->AddNewWillConnectUser(u64UserUniqueIDInDBOnlineFromGateServer);
	//	向MasterServer发送消息，LobbyServer已经可以让该玩家登录


	int nResult = 0;
	usPacketLen = 64;
	LPacketSingle* pPacketSingle = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
	pPacketSingle->SetPacketID(Packet_LB2M_NOTIFY_USER_WILL_ONLINE);
	pPacketSingle->AddInt(nResult);
	pPacketSingle->AddULongLong(u64SessionIDInGateServer);
	pPacketSingle->AddULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pPacketSingle->AddULongLong(u64GateServerSessionIDInMasterServer);
	pPacketSingle->AddULongLong(u64LobbyServerUniqueID);
	pConToMasterServer->AddOneSendPacket(pPacketSingle);
}
