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

#include "LLoginServerPacketProcess_Proc.h"
#include "../include/Server_To_Server_Packet_Define.h" 
#include "../NetWork/LPacketSingle.h"
#include "LLoginServerMainLogic.h" 
#include "LConnectToMaster.h"
#include "../NetWork/LPacketBroadCast.h"
#include "LClientManager.h"
#include "LClient.h"
#include "LLSServerManager.h"
#include "LLSServer.h"
#include "../include/Client_To_Server_Packet_Define.h"

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res)
{
	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();

	if (eFromType == E_LoginServer_Packet_From_MasterServer)
	{ 
		LConnectToMaster* pConnectToMaster = pLoginServerMainLogic->GetConnectToMaster();

		uint64_t u64UniqueServerID =pLoginServerMainLogic->GetServerUniqueID();

		LPacketSingle* pSendPacket = pConnectToMaster->GetOneSendPacketPool(128);	

		pSendPacket->SetPacketID(Packet_SS_Register_Server_Req1);
		pSendPacket->AddULongLong(u64UniqueServerID);

		pConnectToMaster->AddOneSendPacket(pSendPacket);

		LClientManager* pClientManager = pLoginServerMainLogic->GetClientManager();
		unsigned int unClientCount = pClientManager->GetClientCount();

		pSendPacket = pConnectToMaster->GetOneSendPacketPool(128);
		pSendPacket->SetPacketID(Packet_SS_Server_Current_Serve_Count);
		pSendPacket->AddUInt(unClientCount);
		pConnectToMaster->AddOneSendPacket(pSendPacket);
	}
	else if (eFromType == E_LoginServer_Packet_From_Servers)
	{ 
		uint64_t u64UniqueServerID =pLoginServerMainLogic->GetServerUniqueID();
		LLoginServerConnectToServerNetWork* pLSConToServerNetWork = pLoginServerMainLogic->GetLoginServerConToServerNetWork();
		LLSServerManager* pLSServerManager = pLoginServerMainLogic->GetConnectToServerManager();
		LLSServer* pServer = pLSServerManager->FindServerBySessionID(u64SessionID);
		if (pServer == NULL)
		{
			return;
		}
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pLSConToServerNetWork->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Register_Server_Req1);
		pSendPacket->AddULongLong(u64UniqueServerID);
		pLSConToServerNetWork->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
		pLSConToServerNetWork->FlushSendPacket();
	}
}

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1)
{ 
	if (eFromType != E_LoginServer_Packet_From_MasterServer)
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
		if (eServerType == E_Server_Type_Account_DB_Server)
		{ 
			LLoginServerMainLogic* pLSMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();

			uint64_t u64UniqueServerID = pLSMainLogic->GetServerUniqueID();
			LConnectToMaster* pConnectToMaster = pLSMainLogic->GetConnectToMaster(); 
			unsigned short usPacketLen = 32; 
			LPacketSingle* pSendPacket = pConnectToMaster->GetOneSendPacketPool(usPacketLen);			  
			pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Req);
			pSendPacket->AddULongLong(u64UniqueServerID);
			pSendPacket->AddULongLong(u64ServerID);
			pConnectToMaster->AddOneSendPacket(pSendPacket);
		}
	}
}

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res)
{
	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();

	LLSServerManager* pLSServerManager = pLoginServerMainLogic->GetConnectToServerManager();

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

	uint64_t u64UniqueServerID =pLoginServerMainLogic->GetServerUniqueID();
	if (u64UniqueServerID != u64RequestServerID)
	{
		return ;
	}
	LServerID serverID;
	if (!serverID.SetServerID(u64DestServerID))
	{
		return ;
	}
	if (pLSServerManager->FindServerByServerID(u64DestServerID))
	{
		return ;
	}
	char* pbuf = new char[256];
	memset(pbuf, 0, 256);
	memcpy(pbuf, &u64DestServerID, sizeof(u64DestServerID));

	if (!pLSServerManager->AddConnectWork(szIp, usPort, pbuf, 256))
	{
		//	error log
	} 
}

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos)
{ 
	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();

	LLSServerManager* pLSServerManager = pLoginServerMainLogic->GetConnectToServerManager();

	uint64_t u64RecvServerID			= 0;		//	接收该数据包的服务器的ID
	uint64_t u64CanConnectToServerID	= 0;
	pPacket->GetULongLong(u64RecvServerID);
	pPacket->GetULongLong(u64CanConnectToServerID);

	uint64_t u64UniqueServerID =pLoginServerMainLogic->GetServerUniqueID();
	if (u64UniqueServerID != u64RecvServerID)
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
	if (eServerType != E_Server_Type_Account_DB_Server)
	{
		return ;
	}

	if (pLSServerManager->FindServerByServerID(u64CanConnectToServerID))
	{
		return ;
	}

	LConnectToMaster* pConnectToMaster = pLoginServerMainLogic->GetConnectToMaster();

	unsigned short usPacketLen = 32;
	LPacketSingle* pSendPacket = pConnectToMaster->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_SS_Server_Will_Connect_Req);
	pSendPacket->AddULongLong(u64UniqueServerID);
	pSendPacket->AddULongLong(u64CanConnectToServerID);
	pConnectToMaster->AddOneSendPacket(pSendPacket); 
}

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_M2L_USER_ONLINE_VERIFY)
{
	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();
	if (eFromType != E_LoginServer_Packet_From_MasterServer)
	{
		return ;
	}

	int nResult								= 0;					//	玩家登录结果，为0表示没有错误发生
	uint64_t u64UserSessionIDInLoginServer	= 0;
	uint64_t u64UserUniqueIDInDB			= 0;
	char szUserID[MAX_USER_ID_LEN + 1];
	memset(szUserID, 0, sizeof(szUserID));

	pPacket->GetInt(nResult);
	pPacket->GetULongLong(u64UserSessionIDInLoginServer);
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetULongLong(u64UserUniqueIDInDB);
	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}
	LClientManager* pClientManager = pLoginServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FindClientBySessionID(u64UserSessionIDInLoginServer);
	if (pClient == NULL)	// 客户端已经下线
	{
		return ;
	}

	if (nResult != 0)		//	在MasterServer上注册登录失败
	{
		//	发送数据给客户端，告诉客户端验证失败
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pLoginServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_SC_USERID_AND_PASSWORD);
		pSendPacket->AddInt(nResult);
		pLoginServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pLoginServerMainLogic->FlushSendPacket();

		//	下一个循环删除该连接
		pLoginServerMainLogic->AddSessionIDToKickOutQueue(pClient->GetSessionID(), 2);
		return;
	}
	//	注册成功
	// 如果登录没有错误，那么才有如下信息在消息包中
	uint64_t u64GateServerUniqueID 	= 0;	
	char szGateServerIP[MAX_SERVER_IP + 1];
	unsigned short usGateServerPort	= 0;
	char szConnectToGateServerUniqueKey[128];
	memset(szGateServerIP, 0, sizeof(szGateServerIP));
	memset(szConnectToGateServerUniqueKey, 0, sizeof(szConnectToGateServerUniqueKey));
	pPacket->GetULongLong(u64GateServerUniqueID);
	pPacket->GetData(szGateServerIP, MAX_SERVER_IP);
	pPacket->GetUShort(usGateServerPort);
	pPacket->GetData(szConnectToGateServerUniqueKey, 128);
	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}
	unsigned short usPacketLen = 256;
	LPacketBroadCast* pSendPacket = pLoginServerMainLogic->GetOneSendPacket(usPacketLen);
	pSendPacket->SetPacketID(PACKET_SC_USERID_AND_PASSWORD);
	pSendPacket->AddInt(nResult); 
	pSendPacket->AddULongLong(u64UserUniqueIDInDB);
	pSendPacket->AddData(szConnectToGateServerUniqueKey, 128);
	pSendPacket->AddData(szGateServerIP, MAX_SERVER_IP);
	pSendPacket->AddUShort(usGateServerPort);
	pLoginServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
	pLoginServerMainLogic->FlushSendPacket();

	//	下一个循环删除该连接
	pLoginServerMainLogic->AddSessionIDToKickOutQueue(pClient->GetSessionID(), 2);
}

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Current_Serve_Count_BroadCast)
{
	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();
	if (eFromType != E_LoginServer_Packet_From_MasterServer)
	{
		return ;
	}
	uint64_t uServerUniqueID 	= 0;
	unsigned int unServeCount 	= 0;

	pPacket->GetULongLong(uServerUniqueID);
	pPacket->GetUInt(unServeCount);

	LLSServerManager* pLSServerManager = pLoginServerMainLogic->GetConnectToServerManager();
	LLSServer* pServer = pLSServerManager->FindServerByServerID(uServerUniqueID);
	if (pServer == NULL)
	{
		return;
	}
	pServer->SetServeCount(unServeCount);
}
