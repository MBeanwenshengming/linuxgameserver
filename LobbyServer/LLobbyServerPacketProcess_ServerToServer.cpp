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

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req)
{ 
	if (eFromType == E_LobbyServer_Packet_From_GateServer)
	{
		LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();

		LLBGateServerManager* pGateServerManager = pLobbyServerMainLogic->GetGateServerManager();
		LLBGateServer* pGateServer = pGateServerManager->FindGateServerBySessionID(u64SessionID);
		if (pGateServer == NULL)
		{
			return;
		}

		uint64_t u64SessionIDFind = pGateServer->GetSessionID();
		if (u64SessionIDFind != u64SessionID)
		{
			return;
		}
		int nSendThreadID = pGateServer->GetSendThreadID();

		unsigned short usPacketLen = 20;
		LPacketBroadCast* pSendPacket = pLobbyServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Start_Res);
		pLobbyServerMainLogic->SendOnePacket(u64SessionID, nSendThreadID, pSendPacket);
		pLobbyServerMainLogic->FlushSendPacket();
	}
}

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res)
{
	LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();
	if (eFromType == E_LobbyServer_Packet_From_MasterServer)
	{
		LLBConnectToMasterServer* plbctms = pLobbyServerMainLogic->GetConnectToMasterServer();
		plbctms->ReportServerIDToMasterServer();

		unsigned int unServeCount = pLobbyServerMainLogic->GetUserManager()->GetServeCount();
		LLBConnectToMasterServer* pConToMasterServer = pLobbyServerMainLogic->GetConnectToMasterServer();
		LPacketSingle* pPacket = pConToMasterServer->GetOneSendPacketPool(128);
		pPacket->SetPacketID(Packet_SS_Server_Current_Serve_Count);
		pPacket->AddUInt(unServeCount);
		pConToMasterServer->AddOneSendPacket(pPacket);
	}
	else if (eFromType == E_LobbyServer_Packet_From_ConnectToServer)
	{
		//	发送服务器ID给接受本服务器的服务器
		LLBConnectToServersNetWork* pConToServerNetWork = pLobbyServerMainLogic->GetConnectToServerNetWork();
		
		LLBServerManager* pLBServerManager = pLobbyServerMainLogic->GetConnectToServerManager();

		LLBServer* pServer = pLBServerManager->FindServerBySessionID(u64SessionID);
		if (pServer == NULL)		//	已经连接了，不用再连接，那么直接返回
		{
			return;
		}
		uint64_t u64MyServerID = pLobbyServerMainLogic->GetServerUniqueID();
		unsigned short usPacketLen = 32; 
		LPacketBroadCast* pSendPacket = pConToServerNetWork->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Register_Server_Req1);
		pSendPacket->AddULongLong(u64MyServerID);
		pConToServerNetWork->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
		pConToServerNetWork->FlushSendPacket();
	}
}

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1)
{
	if (eFromType == E_LobbyServer_Packet_From_GateServer)
	{
		uint64_t u64ServerUniqueID = 0;
		pPacket->GetULongLong(u64ServerUniqueID);

		
		LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();

		LLBGateServerManager* pGateServerManager = pLobbyServerMainLogic->GetGateServerManager();

		LLBGateServer* pGateServer = pGateServerManager->FindGateServerBySessionID(u64SessionID);
		if (pGateServer == NULL)
		{
			return;
		}
		if (!pGateServer->SetServerID(u64ServerUniqueID))
		{
			return;
		}
		if (!pGateServerManager->UpdateServerID(u64SessionID, u64ServerUniqueID))
		{
			return;
		}
		int nErrorCode = 0;
		unsigned short usPacketLen = 10;
		LPacketBroadCast* pSendPacket = pLobbyServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Register_Server_Res);
		pSendPacket->AddInt(nErrorCode);
		pLobbyServerMainLogic->SendOnePacket(pGateServer->GetSessionID(), pGateServer->GetSendThreadID(), pSendPacket);
		pLobbyServerMainLogic->FlushSendPacket();
	}
}

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Res)
{
}

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast)
{
}

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1)
{ 
	unsigned char ucServerCount = 0;
	pPacket->GetByte(ucServerCount);

	uint64_t u64ServerID = 0;
	for (unsigned char ucIndex = 0; ucIndex < ucServerCount; ++ucIndex)
	{
		u64ServerID = 0;
		pPacket->GetULongLong(u64ServerID);
		if (u64ServerID == 0)
		{
			continue;
		}
		LServerID serverID;
		if (!serverID.SetServerID(u64ServerID))
		{
			continue;
		}


		E_Server_Type eServerType = (E_Server_Type)serverID.GetServerType();
		if (eServerType == E_Server_Type_DB_Server)
		{ 
			
			LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();
			LLBConnectToMasterServer* pConToMasterServer = pLobbyServerMainLogic->GetConnectToMasterServer();

			LLBServerManager* pLBServerManager = pLobbyServerMainLogic->GetConnectToServerManager();

			LLBServer* pServer = pLBServerManager->FindServerByServerID(u64ServerID);
			if (pServer != NULL)		//	已经连接了，不用再连接，那么直接返回
			{
				return;
			}
			uint64_t u64MyServerID = pLobbyServerMainLogic->GetServerUniqueID();
			//	发送连接请求
			unsigned short usPacketLen = 32;
			LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
			pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Req);
			pSendPacket->AddULongLong(u64MyServerID);
			pSendPacket->AddULongLong(u64ServerID);
			pConToMasterServer->AddOneSendPacket(pSendPacket);
		} 
	}
}

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req)
{
	//	只有MasterServer发送该消息
	if (eFromType != E_LobbyServer_Packet_From_MasterServer)
	{
		return ;
	}
	uint64_t u64SrcServerID = 0;
	uint64_t u64DestServerID = 0;
	pPacket->GetULongLong(u64SrcServerID);
	pPacket->GetULongLong(u64DestServerID);
	if (u64SrcServerID == 0 || u64DestServerID == 0)
	{
		return ;
	}
	LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();

	uint64_t u64MyServerID = pLobbyServerMainLogic->GetServerUniqueID();
	if (u64MyServerID != u64DestServerID)
	{
		return ;
	}
	
	LLBGateServerManager* pGateServerManager = pLobbyServerMainLogic->GetGateServerManager();

	pGateServerManager->AddWillConnectServer(u64SrcServerID);

	LLBConnectToMasterServer* pConToMaster = pLobbyServerMainLogic->GetConnectToMasterServer();

	char szListenIp[20]; memset(szListenIp, 0, sizeof(szListenIp));
	unsigned short usPort = 0;
	pLobbyServerMainLogic->GetListenIpAndPort(szListenIp, sizeof(szListenIp) - 1, usPort);

	//	发送应答数据包
	unsigned short usPacketLen = 200;
	LPacketSingle* pSendPacket = pConToMaster->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Res);
	int nRetCode = 0;
	pSendPacket->AddInt(nRetCode);
	pSendPacket->AddULongLong(u64SrcServerID);
	pSendPacket->AddULongLong(u64DestServerID);
	pSendPacket->AddData(szListenIp, 20);
	pSendPacket->AddUShort(usPort);

	pConToMaster->AddOneSendPacket(pSendPacket);
}

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res)
{ 
	if (eFromType == E_LobbyServer_Packet_From_MasterServer)
	{ 
		int nResErrorID 			= 0;				//	请求结果, 为0表示成功
		uint64_t u64RequestServerID = 0;
		uint64_t u64DestServerID 	= 0;
		char szIp[20]; memset(szIp, 0, sizeof(szIp));
		unsigned short usPort		= 0;
		
		pPacket->GetInt(nResErrorID);
		pPacket->GetULongLong(u64RequestServerID);
		pPacket->GetULongLong(u64DestServerID);
		if (nResErrorID != 0)
		{
			return ;
		}
		if (nResErrorID == 0)
		{
			pPacket->GetData(szIp, 20);
			pPacket->GetUShort(usPort);
		}
		if (pPacket->GetErrorCode() != 0)
		{
			return ;
		}
		LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();

		uint64_t u64MyServerID = pLobbyServerMainLogic->GetServerUniqueID();
		if (u64MyServerID != u64RequestServerID)
		{
			return ;
		}

		LLBServerManager* pLBServerManager = pLobbyServerMainLogic->GetConnectToServerManager();

		LLBServer* pServer = pLBServerManager->FindServerByServerID(u64DestServerID);
		if (pServer != NULL)		//	已经连接了，不用再连接，那么直接返回
		{
			return;
		}
		char* pbuf = new char[256];
		memset(pbuf, 0, 256);
		memcpy(pbuf, &u64DestServerID, sizeof(u64DestServerID));

		if (!pLBServerManager->AddConnectWork(szIp, usPort, pbuf, 256))
		{
			//	error log
		} 
	}
}
DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos)
{
	if (eFromType != E_LobbyServer_Packet_From_MasterServer)
	{
		return ;
	}

	uint64_t u64RecvServerID			= 0;		//	接收该数据包的服务器的ID
	uint64_t u64CanConnectToServerID	= 0;
	pPacket->GetULongLong(u64RecvServerID);
	pPacket->GetULongLong(u64CanConnectToServerID);

	LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();


	uint64_t u64MyServerID = pLobbyServerMainLogic->GetServerUniqueID();
	if (u64MyServerID != u64RecvServerID)
	{
		return ;
	}
	LServerID serverID;
	if (!serverID.SetServerID(u64CanConnectToServerID))
	{
		return ;
	}
	unsigned char ucServerType = serverID.GetServerType();
	E_Server_Type eServerType = (E_Server_Type)ucServerType; 
	if (eServerType != E_Server_Type_DB_Server)
	{
		return ;
	}
	
	LLBServerManager* pLBServerManager = pLobbyServerMainLogic->GetConnectToServerManager();

	LLBServer* pServer = pLBServerManager->FindServerByServerID(u64CanConnectToServerID);
	if (pServer != NULL)		//	已经连接了，不用再连接，那么直接返回
	{
		return;
	}

	//	发送将连接的数据包 
	LLBConnectToMasterServer* pConToMaster = pLobbyServerMainLogic->GetConnectToMasterServer();

	unsigned short usPacketLen = 32;
	LPacketSingle* pSendPacket = pConToMaster->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Req);
	pSendPacket->AddULongLong(u64MyServerID);
	pSendPacket->AddULongLong(u64CanConnectToServerID);
	pConToMaster->AddOneSendPacket(pSendPacket);
}
