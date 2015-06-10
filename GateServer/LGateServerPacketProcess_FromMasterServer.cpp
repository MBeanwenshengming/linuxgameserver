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


DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1)
{ 
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();
	LConnectToMasterServer* pConToMasterServer = pGateServerMainLogic->GetConnectToMasterServer();

	uint64_t u64MyServerUniqueID = pGateServerMainLogic->GetServerUniqueID();

	unsigned char ucServerCount = 0;
	uint64_t u64ServerID = 0;
	pPacket->GetByte(ucServerCount);

	for (unsigned char ucIndex = 0; ucIndex < ucServerCount; ++ucIndex)
	{
		u64ServerID = 0;
		pPacket->GetULongLong(u64ServerID);
		LServerID serverID;
		if (!serverID.SetServerID(u64ServerID))
		{
			continue;
		}
		unsigned int unServerType = serverID.GetServerType();
		E_Server_Type eServerType = (E_Server_Type)unServerType;
		if (eServerType == E_Server_Type_Lobby_Server || eServerType == E_Server_Type_Game_Server)
		{
			if (eFromType == E_Packet_From_MasterServer)
			{
				unsigned short usPacketLen = 120;
				LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
				pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Req);
				pSendPacket->AddULongLong(u64MyServerUniqueID);
				pSendPacket->AddULongLong(u64ServerID);
				pConToMasterServer->AddOneSendPacket(pSendPacket);
			}
		} 
	}
}

DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res)
{ 
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();
	LConnectToMasterServer* pConToMasterServer = pGateServerMainLogic->GetConnectToMasterServer();
	LConnectToServerNetWork* pConToServerNetWork = pGateServerMainLogic->GetConnectToServerNetWork();


	int nResErrorID 			= 0;				//	请求结果, 为0表示成功
	uint64_t u64RequestServerID = 0;
	uint64_t u64DestServerID 	= 0;
	char szIp[20];
	unsigned short usPort 		= 0;
	memset(szIp, 0, sizeof(szIp));

	pPacket->GetInt(nResErrorID);
	pPacket->GetULongLong(u64RequestServerID);
	pPacket->GetULongLong(u64DestServerID);
	if (nResErrorID == 0)
	{
		pPacket->GetData(szIp, 20);
		pPacket->GetUShort(usPort);
	}
	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}

	uint64_t u64MyServerUniqueID = pGateServerMainLogic->GetServerUniqueID();

	//	不是本服务器的请求，退出处理
	if (u64RequestServerID != u64MyServerUniqueID)
	{
		return ;
	}
	LServerManager* pServerManager = pGateServerMainLogic->GetServerManager();
	char* pbuf = new char[256];
	memset(pbuf, 0, 256);
	memcpy(pbuf, &u64DestServerID, sizeof(u64DestServerID));

	if (!pServerManager->AddConnectWork(szIp, usPort, pbuf, 256))
	{
		//	error log
	}
}
DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos)
{
	if (eFromType != E_Packet_From_MasterServer)
	{
		return ;
	}

	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();
	LConnectToMasterServer* pConToMasterServer = pGateServerMainLogic->GetConnectToMasterServer();
	LConnectToServerNetWork* pConToServerNetWork = pGateServerMainLogic->GetConnectToServerNetWork();

	uint64_t u64RecvServerID			= 0;		//	接收该数据包的服务器的ID
	uint64_t u64CanConnectToServerID	= 0;
	pPacket->GetULongLong(u64RecvServerID);
	pPacket->GetULongLong(u64CanConnectToServerID);

	uint64_t u64MyServerUniqueID = pGateServerMainLogic->GetServerUniqueID();

	//	不是本服务器的请求，退出处理
	if (u64RecvServerID != u64MyServerUniqueID)
	{
		return ;
	}

	unsigned short usPacketLen = 120;
	LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Req);
	pSendPacket->AddULongLong(u64MyServerUniqueID);
	pSendPacket->AddULongLong(u64CanConnectToServerID);
	pConToMasterServer->AddOneSendPacket(pSendPacket);
}

DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_USER_WILL_LOGIN)
{ 
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();
	LConnectToMasterServer* pConToMasterServer = pGateServerMainLogic->GetConnectToMasterServer();

	uint64_t u64UserSessionIDInLoginServer 			= 0;
	uint64_t u64LoginServerSessionIDInMasterServer 	= 0;
	char szUserID[MAX_USER_ID_LEN + 1];
	uint64_t u64UserUniqueIDInDB 					= 0;
	memset(szUserID, 0, sizeof(szUserID));

	pPacket->GetULongLong(u64UserSessionIDInLoginServer);
	pPacket->GetULongLong(u64LoginServerSessionIDInMasterServer);
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetULongLong(u64UserUniqueIDInDB);
	if (pPacket->GetErrorCode() != 0)
	{
		return;
	}
	LClientManager* pClientManager = pGateServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FineClientByUserUniqueIDInDB(u64UserUniqueIDInDB);
	if (pClient != NULL)
	{
		unsigned short usPacketLen = 120;
		LPacketSingle* pSendPacketToMaster = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
		int nResult = -1;
		pSendPacketToMaster->SetPacketID(Packet_GA2M_USER_WILL_LOGIN);
		pSendPacketToMaster->AddInt(nResult);
		pSendPacketToMaster->AddULongLong(u64UserSessionIDInLoginServer);
		pSendPacketToMaster->AddULongLong(u64LoginServerSessionIDInMasterServer);
		pSendPacketToMaster->AddData(szUserID, MAX_USER_ID_LEN);
		pSendPacketToMaster->AddULongLong(u64UserUniqueIDInDB);
		pConToMasterServer->AddOneSendPacket(pSendPacketToMaster);
		return ;
	}
	char szbuf[128];
	memset(szbuf, 0, sizeof(szbuf));
	if (!pClientManager->BuildLoginKey(szbuf, sizeof(szbuf)))
	{
		unsigned short usPacketLen = 120;
		LPacketSingle* pSendPacketToMaster = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
		int nResult = -1;
		pSendPacketToMaster->SetPacketID(Packet_GA2M_USER_WILL_LOGIN);
		pSendPacketToMaster->AddInt(nResult);
		pSendPacketToMaster->AddULongLong(u64UserSessionIDInLoginServer);
		pSendPacketToMaster->AddULongLong(u64LoginServerSessionIDInMasterServer);
		pSendPacketToMaster->AddData(szUserID, MAX_USER_ID_LEN);
		pSendPacketToMaster->AddULongLong(u64UserUniqueIDInDB);
		pConToMasterServer->AddOneSendPacket(pSendPacketToMaster);
		return ;
	}
	if (!pClientManager->AddWillConnectClientInfo(u64UserUniqueIDInDB, szbuf, szUserID))
	{ 
		unsigned short usPacketLen = 120;
		LPacketSingle* pSendPacketToMaster = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
		int nResult = -1;
		pSendPacketToMaster->SetPacketID(Packet_GA2M_USER_WILL_LOGIN);
		pSendPacketToMaster->AddInt(nResult);
		pSendPacketToMaster->AddULongLong(u64UserSessionIDInLoginServer);
		pSendPacketToMaster->AddULongLong(u64LoginServerSessionIDInMasterServer);
		pSendPacketToMaster->AddData(szUserID, MAX_USER_ID_LEN);
		pSendPacketToMaster->AddULongLong(u64UserUniqueIDInDB);
		pConToMasterServer->AddOneSendPacket(pSendPacketToMaster);
		return ;
	}

	char szIP[MAX_SERVER_IP + 1];
	unsigned short usListenPort = 0;
	pGateServerMainLogic->GetListenIpAndPort(szIP, MAX_SERVER_IP, usListenPort);
	unsigned short usPacketLen = 120;
	LPacketSingle* pSendPacketToMaster = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
	int nResult = 0;
	pSendPacketToMaster->SetPacketID(Packet_GA2M_USER_WILL_LOGIN);
	pSendPacketToMaster->AddInt(nResult);
	pSendPacketToMaster->AddULongLong(u64UserSessionIDInLoginServer);
	pSendPacketToMaster->AddULongLong(u64LoginServerSessionIDInMasterServer);
	pSendPacketToMaster->AddData(szUserID, MAX_USER_ID_LEN);
	pSendPacketToMaster->AddULongLong(u64UserUniqueIDInDB);
	pSendPacketToMaster->AddData(szbuf, 128);
	pSendPacketToMaster->AddData(szIP, MAX_SERVER_IP);
	pSendPacketToMaster->AddUShort(usListenPort);
	pConToMasterServer->AddOneSendPacket(pSendPacketToMaster);
}

DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_USER_GATESERVER_ONLINE)
{
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();
	int nResult = 0;
	uint64_t u64UserUniqueIDInDBOnlineFromGateServer = 0;
	uint64_t u64SessionIDInGateServer = 0;

	pPacket->GetInt(nResult);
	pPacket->GetULongLong(u64UserUniqueIDInDBOnlineFromGateServer);
	pPacket->GetULongLong(u64SessionIDInGateServer);

	LClientManager* pClientManager = pGateServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FindClientBySessionID(u64SessionIDInGateServer);
	if (pClient == NULL)
	{
		return ;
	}
	if (nResult == 0)
	{
		//	MasterServer验证通过
		pClient->SetClientState(E_Client_State_Logined);

		//	向客户端发送登录成功 
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pGateServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_GA2C_USER_LOGIN);
		pSendPacket->AddInt(nResult);
		pGateServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pGateServerMainLogic->FlushSendPacket();
	}
	else
	{
		//	发送登录失败
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pGateServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_GA2C_USER_LOGIN);
		pSendPacket->AddInt(nResult);
		pGateServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pGateServerMainLogic->FlushSendPacket();
		//	踢出玩家
		pGateServerMainLogic->AddSessionIDToKickOutQueue(u64SessionIDInGateServer, 2); 
	}
}

