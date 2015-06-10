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

#include "LGSPacketProcessManager.h"
#include "LGSServer.h"
#include "LGSServerManager.h"
#include "LGSClientServer.h"
#include "LGSClientServerManager.h"
#include "LGameServerConnectToMasterServer.h"
#include "LGameServerConnectToServerNetWork.h"
#include "LGameServerMainLogic.h"
#include "../include/Server_To_Server_Packet_Define.h"
#include "../include/Server_Define.h"
#include "../NetWork/LPacketBroadCast.h"

DEFINE_GAMESERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req) 
{
	LGameServerMainLogic* pGSMainLogic = pGameServerPacketProcessProc->GetGameServerMainLogic();
	if (eFromType == E_GS_Packet_From_Client_Server)
	{
		LGSClientServerManager* pGSClientServerManager = pGSMainLogic->GetGSClientServerManager();
		LGSClientServer* pClientServer = pGSClientServerManager->FindGSClientServerBySessionID(u64SessionID);
		if (pClientServer == NULL)
		{
			return ;
		}
		//LGameServerConnectToServerNetWork* pGSConToServerNetWork = pGSMainLogic->GetGSConToServerNetWork();
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pGSMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Start_Res);
		pGSMainLogic->SendOnePacket(pClientServer->GetSessionID(), pClientServer->GetSendThreadID(), pSendPacket);
		pGSMainLogic->FlushSendPacket();
	}
}

DEFINE_GAMESERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res)
{
	LGameServerMainLogic* pGSMainLogic = pGameServerPacketProcessProc->GetGameServerMainLogic();
	if (eFromType == E_GS_Packet_From_Master_Server)
	{
		LGameServerConnectToMasterServer* pConToMasterServer = pGSMainLogic->GetConToMasterServer();
		//	发送服务器ID给MasterServer
		uint64_t u64MyServerUniqueID = pGSMainLogic->GetServerUniqueID();
		unsigned short usPacketLen = 32;
		LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Register_Server_Req1);
		pSendPacket->AddULongLong(u64MyServerUniqueID);
		pConToMasterServer->AddOneSendPacket(pSendPacket);
	}
	else if (eFromType == E_GS_Packet_From_Con_Server)
	{ 
		LGSServerManager* pGSServerManager = pGSMainLogic->GetGSServerManager();
		LGSServer* pGSServer = pGSServerManager->FindGSServerBySessionID(u64SessionID);
		if (pGSServer == NULL)
		{
			return ;
		}
		LGameServerConnectToServerNetWork* pGSConToServerNetWork = pGSMainLogic->GetGSConToServerNetWork();

		uint64_t u64MyServerUniqueID = pGSMainLogic->GetServerUniqueID();
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pGSConToServerNetWork->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Register_Server_Req1);
		pSendPacket->AddULongLong(u64MyServerUniqueID);
		pGSConToServerNetWork->SendOnePacket(pGSServer->GetSessionID(), pGSServer->GetSendThreadID(), pSendPacket);
		pGSConToServerNetWork->FlushSendPacket(); 
	}
}

DEFINE_GAMESERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1)
{
	LGameServerMainLogic* pGSMainLogic = pGameServerPacketProcessProc->GetGameServerMainLogic();
	if (eFromType == E_GS_Packet_From_Client_Server)
	{ 
		LGSClientServerManager* pGSClientServerManager = pGSMainLogic->GetGSClientServerManager();
		LGSClientServer* pClientServer = pGSClientServerManager->FindGSClientServerBySessionID(u64SessionID);
		if (pClientServer == NULL)
		{
			return ;
		}
		uint64_t u64ServerUniqueID = 0;
		pPacket->GetULongLong(u64ServerUniqueID);
		if (pPacket->GetErrorCode() != 0)
		{
			return ;
		}
		//LGameServerConnectToServerNetWork* pGSConToServerNetWork = pGSMainLogic->GetGSConToServerNetWork();
		if (!pGSClientServerManager->UpdateServerIDForServer(u64SessionID, u64ServerUniqueID))
		{
			pGSMainLogic->KickOutOneSession(u64SessionID);
			return ;
		}
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pGSMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Register_Server_Res);
		pGSMainLogic->SendOnePacket(pClientServer->GetSessionID(), pClientServer->GetSendThreadID(), pSendPacket);
		pGSMainLogic->FlushSendPacket();
	}
}

DEFINE_GAMESERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Res)
{
	LGameServerMainLogic* pGSMainLogic = pGameServerPacketProcessProc->GetGameServerMainLogic();
}

DEFINE_GAMESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1)
{
	LGameServerMainLogic* pGSMainLogic = pGameServerPacketProcessProc->GetGameServerMainLogic();
	if (eFromType != E_GS_Packet_From_Master_Server)
	{
		return ;
	}
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
			LGameServerConnectToMasterServer* pConToMasterServer = pGSMainLogic->GetConToMasterServer();
			//	发送服务器ID给MasterServer
			uint64_t u64MyServerUniqueID = pGSMainLogic->GetServerUniqueID();
			unsigned short usPacketLen = 32;
			LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen); 
			//	发送连接请求
			pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Req);
			pSendPacket->AddULongLong(u64MyServerUniqueID);
			pSendPacket->AddULongLong(u64ServerID);
			pConToMasterServer->AddOneSendPacket(pSendPacket);
		} 
	}
}

DEFINE_GAMESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req)
{
	LGameServerMainLogic* pGSMainLogic = pGameServerPacketProcessProc->GetGameServerMainLogic();
	if (eFromType != E_GS_Packet_From_Master_Server)
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

	uint64_t u64MyServerUniqueID = pGSMainLogic->GetServerUniqueID();
	if (u64MyServerUniqueID != u64DestServerID)
	{
		return ;
	}
	char szListenIp[20]; memset(szListenIp, 0, sizeof(szListenIp));
	unsigned short usPort = 0;
	pGSMainLogic->GetListenIpAndPort(szListenIp, sizeof(szListenIp) - 1, usPort);

	LGameServerConnectToMasterServer* pConToMasterServer = pGSMainLogic->GetConToMasterServer();

	//	发送应答数据包
	unsigned short usPacketLen = 200;
	LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Res);
	int nRetCode = 0;
	pSendPacket->AddInt(nRetCode);
	pSendPacket->AddULongLong(u64SrcServerID);
	pSendPacket->AddULongLong(u64DestServerID);
	pSendPacket->AddData(szListenIp, 20);
	pSendPacket->AddUShort(usPort);

	pConToMasterServer->AddOneSendPacket(pSendPacket);
}

DEFINE_GAMESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res)
{
	LGameServerMainLogic* pGSMainLogic = pGameServerPacketProcessProc->GetGameServerMainLogic();
	if (eFromType != E_GS_Packet_From_Master_Server)
	{
		return ;
	}

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

	uint64_t u64MyServerID = pGSMainLogic->GetServerUniqueID();
	if (u64MyServerID != u64RequestServerID)
	{
		return ;
	}
	LGSServerManager* pGSServerManager = pGSMainLogic->GetGSServerManager();
	
	LGSServer* pServer = pGSServerManager->FineGSServerByServerID(u64DestServerID);
	if (pServer != NULL)
	{
		return ;
	}

	char* pbuf = new char[256];
	memset(pbuf, 0, 256);
	memcpy(pbuf, &u64DestServerID, sizeof(u64DestServerID));
	if (!pGSServerManager->AddConnectWork(szIp, usPort, pbuf, 256))
	{
		// error
	}
}

DEFINE_GAMESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos)
{
	LGameServerMainLogic* pGSMainLogic = pGameServerPacketProcessProc->GetGameServerMainLogic();
	if (eFromType != E_GS_Packet_From_Master_Server)
	{
		return ;
	}

	uint64_t u64RecvServerID			= 0;		//	接收该数据包的服务器的ID
	uint64_t u64CanConnectToServerID	= 0;
	pPacket->GetULongLong(u64RecvServerID);
	pPacket->GetULongLong(u64CanConnectToServerID);


	uint64_t u64MyServerID = pGSMainLogic->GetServerUniqueID();
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


	LGSServerManager* pGSServerManager = pGSMainLogic->GetGSServerManager();
	
	LGSServer* pServer = pGSServerManager->FineGSServerByServerID(u64CanConnectToServerID);
	if (pServer != NULL)
	{
		return ;
	}

	LGameServerConnectToMasterServer* pConToMasterServer = pGSMainLogic->GetConToMasterServer();

	//	发送应答数据包
	unsigned short usPacketLen = 32;
	LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Req);
	pSendPacket->AddULongLong(u64MyServerID);
	pSendPacket->AddULongLong(u64CanConnectToServerID);
	pConToMasterServer->AddOneSendPacket(pSendPacket);
}


