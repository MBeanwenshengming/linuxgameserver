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
#include "../include/Server_Define.h"

DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req)
{
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();

	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();
	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		return ;
	}

	LPacketBroadCast* pBSendPacket = pMainLogic->GetOneSendPacket(120);
	if (pBSendPacket == NULL)
	{
		return ;
	}
	pBSendPacket->SetPacketID(Packet_SS_Start_Res);

	pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pBSendPacket);
	pMainLogic->FlushSendPacket();	
}

DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1)
{ 
	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();

	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();

	uint64_t u64ServerID = 0;
	pPacket->GetULongLong(u64ServerID);

	LServerID serverID;
	if (!serverID.SetServerID(u64ServerID))
	{
		//	error;
		pMainLogic->KickOutOneSession(u64SessionID);
		return ;
	}
	if (!pServerManager->AddServer(u64SessionID, serverID.GetServerType(), serverID.GetAreaID(), serverID.GetGroupID(), serverID.GetServerID()))
	{
		pMainLogic->KickOutOneSession(u64SessionID);
		return ;
	}
	char szServerInfo[64];
	memset(szServerInfo, 0, sizeof(szServerInfo));
	LServerID::ToServeTypeString(serverID.GetServerType(), szServerInfo, 61);

	//	测试，打印登录上来的服务器
	printf("Server Online:ServerType:%s, AreaID:%d, GroupID:%d, ServerID:%d\n", szServerInfo, serverID.GetAreaID(), serverID.GetGroupID(), serverID.GetServerID());

	//	把本服务器注册的消息广播给其它服务器
	LMasterServerMainLogic::BroadCastServerToServers(pMainLogic, u64ServerID);
	//	把该注册服务器可以连接的服务器发送给注册服务器
	LMasterServerMainLogic::SendConnectableServerInfosToServer(pMainLogic, u64ServerID);

	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		return ;
	}

	LPacketBroadCast* pBSendPacket = pMainLogic->GetOneSendPacket(120);
	if (pBSendPacket == NULL)
	{
		return ;
	}
	pBSendPacket->SetPacketID(Packet_SS_Register_Server_Res); 
	pMainLogic->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pBSendPacket);
	pMainLogic->FlushSendPacket();	
}


DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req)
{
	uint64_t uSrcServerID = 0;
	pPacket->GetULongLong(uSrcServerID);	
	uint64_t uDestServerID = 0;
	pPacket->GetULongLong(uDestServerID);


	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();

	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();

	LServer* pSrcServer = pServerManager->FindServer(uSrcServerID);
	LServer* pDestServer = pServerManager->FindServer(uDestServerID);
	if (pSrcServer == NULL)
	{
		return ;
	}
	if (pDestServer == NULL)
	{ 
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pBSendPacket2Src = pMainLogic->GetOneSendPacket(usPacketLen);
		pBSendPacket2Src->SetPacketID(Packet_SS_Server_Will_Connect_Res);
		int nRetCode = -1;
		pBSendPacket2Src->AddInt(nRetCode);
		pBSendPacket2Src->AddULongLong(uSrcServerID);
		pBSendPacket2Src->AddULongLong(uDestServerID); 
		
		pMainLogic->SendOnePacket(pSrcServer->GetNetWorkSessionID(), pSrcServer->GetSendThreadID(), pBSendPacket2Src);
		pMainLogic->FlushSendPacket();	
		return;
	}

	unsigned short usPacketLen = 32;
	LPacketBroadCast* pBSendPacket2Dest = pMainLogic->GetOneSendPacket(usPacketLen);
	pBSendPacket2Dest->SetPacketID(Packet_SS_Server_Will_Connect_Req);
	pBSendPacket2Dest->AddULongLong(uSrcServerID);
	pBSendPacket2Dest->AddULongLong(uDestServerID);

	pMainLogic->SendOnePacket(pDestServer->GetNetWorkSessionID(), pDestServer->GetSendThreadID(), pBSendPacket2Dest);
	pMainLogic->FlushSendPacket();	
}


DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res)
{
	int nRetCode = -1;
	pPacket->GetInt(nRetCode);
	uint64_t uSrcServerID = 0;
	pPacket->GetULongLong(uSrcServerID);	
	uint64_t uDestServerID = 0;
	pPacket->GetULongLong(uDestServerID);

	char szIp[20]; memset(szIp, 0, sizeof(szIp));
	unsigned short usPort = 0;
	if (nRetCode == 0)
	{
		pPacket->GetData(szIp, 20);
		pPacket->GetUShort(usPort);
	}


	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();

	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();

	LServer* pSrcServer = pServerManager->FindServer(uSrcServerID); 
	if (pSrcServer == NULL)
	{
		return ;
	}

	unsigned short usPacketLen = 120;
	LPacketBroadCast* pBSendPacket2Src = pMainLogic->GetOneSendPacket(usPacketLen);
	pBSendPacket2Src->SetPacketID(Packet_SS_Server_Will_Connect_Res);
	pBSendPacket2Src->AddInt(nRetCode);
	pBSendPacket2Src->AddULongLong(uSrcServerID);
	pBSendPacket2Src->AddULongLong(uDestServerID); 
	pBSendPacket2Src->AddData(szIp, 20);
	pBSendPacket2Src->AddUShort(usPort);

	pMainLogic->SendOnePacket(pSrcServer->GetNetWorkSessionID(), pSrcServer->GetSendThreadID(), pBSendPacket2Src);
	pMainLogic->FlushSendPacket();	
}

DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Current_Serve_Count)
{
	unsigned int unServeCount = 0;
	pPacket->GetUInt(unServeCount);

	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();
	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
	if (pServer != NULL)
	{
		pServer->SetCurrentServeCount(unServeCount);
	}

	uint64_t uServerID = pServer->GetServerUniqueID();
	LMainLogicThread* pMainLogic = pMasterServerPacketProcessProc->GetMainLogicThread();
	LPacketBroadCast* pBSendPacket2Src = pMainLogic->GetOneSendPacket(128);
	pBSendPacket2Src->SetPacketID(Packet_SS_Server_Current_Serve_Count_BroadCast);
	pBSendPacket2Src->AddULongLong(uServerID);
	pBSendPacket2Src->AddUInt(unServeCount);

	E_Server_Type eServerType = pServer->GetServerType();
	switch(eServerType)
	{
		case E_Server_Type_Account_DB_Server:
		{
			pServerManager->BroadCastToServersByServerType(E_Server_Type_Login_Server, pBSendPacket2Src);
			pMainLogic->FlushSendPacket();
		}
		break;
		case E_Server_Type_DB_Server:
		{
			pServerManager->BroadCastToServersByServerType(E_Server_Type_Game_Server, pBSendPacket2Src);
			pServerManager->BroadCastToServersByServerType(E_Server_Type_Lobby_Server, pBSendPacket2Src);
			pMainLogic->FlushSendPacket();
		}
		break;
		case E_Server_Type_Game_Server:
		{
			pServerManager->BroadCastToServersByServerType(E_Server_Type_Lobby_Server, pBSendPacket2Src);
			pMainLogic->FlushSendPacket();
		}
		break;
		case E_Server_Type_Lobby_Server:
		{
			pServerManager->BroadCastToServersByServerType(E_Server_Type_Game_Server, pBSendPacket2Src);
			pMainLogic->FlushSendPacket();
		}
		break;
		default:
			break;
	}

}
