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
