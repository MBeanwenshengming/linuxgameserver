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

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_ADB2L_USER_VERIFY)
{ 
	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();
	if (eFromType != E_LoginServer_Packet_From_Servers)
	{
		return ;
	}
	int nResultID 					= 0;
	uint64_t u64UserSessionID 		= 0;
	char szUserID[MAX_USER_ID_LEN + 1];	//	用户帐号名
	uint64_t u64UserUniqueIDInDB 	= 0;	//	用户的数字化帐号ID
	memset(szUserID, 0, sizeof(szUserID));
	
	pPacket->GetInt(nResultID);
	pPacket->GetULongLong(u64UserSessionID);
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetULongLong(u64UserUniqueIDInDB);
	if (pPacket == NULL)
	{
		return ;
	}
	LClientManager* pClientManager = pLoginServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FindClientBySessionID(u64UserSessionID);
	if (pClient == NULL)	//	表明玩家已经下线，那么直接返回，不用处理
	{
		return ;
	}
	if (nResultID != 0)		//	验证出错，那么告诉客户端，并且，下一个主循环踢掉客户端
	{ 
		unsigned short usPacketLen = 16;
		LPacketBroadCast* pSendPacket = pLoginServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_SC_USERID_AND_PASSWORD);
		pSendPacket->AddInt(nResultID);
		pLoginServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pLoginServerMainLogic->FlushSendPacket();

		//	添加删除连接
		pLoginServerMainLogic->AddSessionIDToKickOutQueue(pClient->GetSessionID(), 2);
		return ;
	}

	pClient->SetClientUniqueIDInADB(u64UserUniqueIDInDB);
	pClient->SetUserID(szUserID);

	//	处理成功，那么发送注册请求给MasterServer，注册该用户
	LConnectToMaster* pConToMaster = pLoginServerMainLogic->GetConnectToMaster();
	unsigned short usPacketLen = 128;
	LPacketSingle* pSendPacket = pConToMaster->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_L2M_USER_ONLINE);
	pSendPacket->AddULongLong(u64UserSessionID);
	pSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
	pSendPacket->AddULongLong(u64UserUniqueIDInDB);
	pConToMaster->AddOneSendPacket(pSendPacket);
}


DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_ADB2L_ADD_USER)
{
	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();
	if (eFromType != E_LoginServer_Packet_From_Servers)
	{
		return ;
	}

	int nResult = 0;
	uint64_t u64UserSessionID = 0;
	char szUserID[MAX_USER_ID_LEN + 1];	//	用户帐号名
	uint64_t u64UserUniqueIDInDB = 0;	//	用户的数字化帐号ID
	memset(szUserID, 0, sizeof(szUserID));

	pPacket->GetInt(nResult);
	pPacket->GetULongLong(u64UserSessionID);
	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetULongLong(u64UserUniqueIDInDB);

	if (pPacket->GetErrorCode() != 0)
	{
		return ;
	}

	LClientManager* pClientManager = pLoginServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FindClientBySessionID(u64UserSessionID);
	if (pClient == NULL)	//	表明玩家已经下线，那么直接返回，不用处理
	{
		return ;
	}
	unsigned short usPacketLen = 32;
	LPacketBroadCast* pSendPacket = pLoginServerMainLogic->GetOneSendPacket(usPacketLen);
	pSendPacket->SetPacketID(PACKET_LS2C_CREATE_USER);
	pSendPacket->AddInt(nResult);
	pSendPacket->AddData(szUserID, MAX_USER_ID_LEN);
	pLoginServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
	pLoginServerMainLogic->FlushSendPacket();

	//	添加删除连接
	pLoginServerMainLogic->AddSessionIDToKickOutQueue(pClient->GetSessionID(), 2);
}


