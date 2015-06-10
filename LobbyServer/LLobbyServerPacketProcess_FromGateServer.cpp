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
#include "LUser.h"

DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_GA2LB_USER_ONLINE) 
{ 
	LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();

	LLBGateServerManager* pGateServerManager = pLobbyServerMainLogic->GetGateServerManager();
	LLBGateServer* pGateServer = pGateServerManager->FindGateServerBySessionID(u64SessionID);
	if (pGateServer == NULL)
	{
		return ;
	}

	uint64_t u64SessionIDInGateServer = 0;
	uint64_t u64UserUniqueIDInDB = 0;
	char szUserID[MAX_USER_ID_LEN + 1];
	memset(szUserID, 0, sizeof(szUserID));

	pPacket->GetULongLong(u64SessionIDInGateServer); 
	pPacket->GetULongLong(u64UserUniqueIDInDB);
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}
	LUserManager* pUserManager = pLobbyServerMainLogic->GetUserManager();
	if (!pUserManager->ExistsWillConnectUser(u64UserUniqueIDInDB))
	{
		//	不存在该玩家，那么发送登录错误消息
		int nResult = -1;
		unsigned short usPacketLen = 64;
		LPacketBroadCast* pSendPacket = pLobbyServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_LB2GA_USER_ONLINE);
		pSendPacket->AddInt(nResult);
		pSendPacket->AddULongLong(u64SessionIDInGateServer);
		pSendPacket->AddULongLong(u64UserUniqueIDInDB);
		pLobbyServerMainLogic->SendOnePacket(pGateServer->GetSessionID(), pGateServer->GetSendThreadID(), pSendPacket);
		pLobbyServerMainLogic->FlushSendPacket();
		return ;
	}
	else
	{
		//	存在玩家，那么将玩家添加到信息列表，发送登录成功信息包
		LUser* pUser = pUserManager->AllocUserFromPool();
		if (pUser == NULL)
		{
			int nResult = -1;
			unsigned short usPacketLen = 64;
			LPacketBroadCast* pSendPacket = pLobbyServerMainLogic->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_LB2GA_USER_ONLINE);
			pSendPacket->AddInt(nResult);
			pSendPacket->AddULongLong(u64SessionIDInGateServer);
			pSendPacket->AddULongLong(u64UserUniqueIDInDB);
			pLobbyServerMainLogic->SendOnePacket(pGateServer->GetSessionID(), pGateServer->GetSendThreadID(), pSendPacket);
			pLobbyServerMainLogic->FlushSendPacket();
			return;
		}
		pUser->SetUniqueUserIDInDB(u64UserUniqueIDInDB);
		pUser->SetUserID(szUserID);
		pUserManager->AddUser(pUser);

		int nResult = 0;
		unsigned short usPacketLen = 64;
		LPacketBroadCast* pSendPacket = pLobbyServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_LB2GA_USER_ONLINE);
		pSendPacket->AddInt(nResult);
		pSendPacket->AddULongLong(u64SessionIDInGateServer);
		pSendPacket->AddULongLong(u64UserUniqueIDInDB);
		pLobbyServerMainLogic->SendOnePacket(pGateServer->GetSessionID(), pGateServer->GetSendThreadID(), pSendPacket);
		pLobbyServerMainLogic->FlushSendPacket(); 
	}
}


//DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_GA2LB_USER_ONLINE)
//{
//	LLobbyServerMainLogic* pLobbyServerMainLogic = pLobbyServerPacketProcessProc->GetLobbyServerMainLogic();
//
//	LLBGateServerManager* pGateServerManager = pLobbyServerMainLogic->GetGateServerManager();
//	LLBGateServer* pGateServer = pGateServerManager->FindGateServerBySessionID(u64SessionID);
//	if (pGateServer == NULL)
//	{
//		return ;
//	}
//
//	uint64_t u64SessionIDInGateServer 	= 0;
//	uint64_t u64UserUniqueIDInDB 		= 0;
//	char szUserID[MAX_USER_ID_LEN + 1];
//	memset(szUserID, 0, sizeof(szUserID));
//
//	pPacket->GetULongLong(u64SessionIDInGateServer);
//	pPacket->GetULongLong(u64UserUniqueIDInDB);
//	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
//	if (pPacket->GetErrorCode() != 0)
//	{
//		return ;
//	}
//	LUserManager* pUserManager = pLobbyServerMainLogic->GetUserManager();
//	LUser* pUser = pUserManager->FineUser(u64UserUniqueIDInDB);
//	if (pUser == NULL)		//	如果User不存在，那么创建一个User，设置为正在从数据库读取数据的状态
//	{
//	}
//	else		//	已经存在，那么判断数据是否已经读取好，如果数据读取好，那么直接将数据发送的GateServer
//	{
//	}
//
//}


