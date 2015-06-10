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
#include "../include/Client_To_Server_Packet_Define.h"
#include "LClientManager.h"
#include "LClient.h"
#include "LLSServerManager.h"

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_CS_START_REQ)
{
	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();
	if (eFromType == E_LoginServer_Packet_From_Client)
	{
		LClientManager* pClientManager = pLoginServerMainLogic->GetClientManager();
		LClient* pClient = pClientManager->FindClientBySessionID(u64SessionID);
		if (pClient == NULL)
		{
			return ;
		}
		unsigned short usPacketLen = 16;
		LPacketBroadCast* pSendPacket = pLoginServerMainLogic->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(PACKET_SC_START_RES);
		pLoginServerMainLogic->SendOnePacket(pClient->GetSessionID(), pClient->GetSendThreadID(), pSendPacket);
		pLoginServerMainLogic->FlushSendPacket();
	}
}

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_CS_2LS_USERID_AND_PASSWORD)
{
	if (eFromType != E_LoginServer_Packet_From_Client)
	{
		return ;
	}

	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();
	LClientManager* pClientManager = pLoginServerMainLogic->GetClientManager();
	
	LClient* pClient = pClientManager->FindClientBySessionID(u64SessionID);
	if (pClient == NULL)
	{
		return ;
	}
	
	uint64_t u64ClientSessionID = pClient->GetSessionID();

	char szUserID[MAX_USER_ID_LEN + 1];
	char szPassWord[MAX_PASSWORD_LEN + 1];
	memset(szUserID, 0, sizeof(szUserID));
	memset(szPassWord, 0, sizeof(szPassWord));

	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetData(szPassWord, MAX_PASSWORD_LEN);

	if (pPacket->GetErrorCode() != 0)
	{
		return;
	}
	LLSServerManager* pServerManager = pLoginServerMainLogic->GetConnectToServerManager();
	LLSServer* pADBServer = pServerManager->SelectAccountDBServerForClient(szUserID);
	if (pADBServer == NULL)
	{
		return ;
	}

	//	向AccountDB发送帐号验证请求
	LLoginServerConnectToServerNetWork* pLSConToServerNetWork = pLoginServerMainLogic->GetLoginServerConToServerNetWork();
	unsigned short usPacketLen = MAX_USER_ID_LEN + MAX_PASSWORD_LEN + 32;
	LPacketBroadCast* pSendPacketToADB = pLSConToServerNetWork->GetOneSendPacket(usPacketLen);
	pSendPacketToADB->SetPacketID(Packet_L2ADB_USER_VERIFY);
	pSendPacketToADB->AddData(szUserID, MAX_USER_ID_LEN);
	pSendPacketToADB->AddData(szPassWord, MAX_PASSWORD_LEN);
	pSendPacketToADB->AddULongLong(u64ClientSessionID);
	pLSConToServerNetWork->SendOnePacket(pADBServer->GetSessionID(), pADBServer->GetSendThreadID(), pSendPacketToADB);
	pLSConToServerNetWork->FlushSendPacket();
}

DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_C2LS_CREATE_USER)
{ 
	char szUserID[MAX_USER_ID_LEN + 1];
	char szPassWord[MAX_PASSWORD_LEN + 1];

	memset(szUserID, 0, sizeof(szUserID));
	memset(szPassWord, 0, sizeof(szPassWord));

	pPacket->GetData(szUserID, MAX_USER_ID_LEN);
	pPacket->GetData(szPassWord, MAX_PASSWORD_LEN);

	if (pPacket->GetErrorCode() != 0)
	{
		return;
	}

	LLoginServerMainLogic* pLoginServerMainLogic = pLoginServerPacketProcessProc->GetLoginServerMainLogic();
	LClientManager* pClientManager = pLoginServerMainLogic->GetClientManager();
	
	LClient* pClient = pClientManager->FindClientBySessionID(u64SessionID);
	if (pClient == NULL)
	{
		return ;
	}
	
	uint64_t u64ClientSessionID = pClient->GetSessionID();


	LLSServerManager* pServerManager = pLoginServerMainLogic->GetConnectToServerManager();
	LLSServer* pADBServer = pServerManager->SelectAccountDBServerForClient(szUserID);
	if (pADBServer == NULL)
	{
		return ;
	}

	//	向AccountDB发送帐号验证请求
	LLoginServerConnectToServerNetWork* pLSConToServerNetWork = pLoginServerMainLogic->GetLoginServerConToServerNetWork();
	unsigned short usPacketLen = MAX_USER_ID_LEN + MAX_PASSWORD_LEN + 32;
	LPacketBroadCast* pSendPacketToADB = pLSConToServerNetWork->GetOneSendPacket(usPacketLen);
	pSendPacketToADB->SetPacketID(Packet_L2ADB_ADD_USER);
	pSendPacketToADB->AddData(szUserID, MAX_USER_ID_LEN);
	pSendPacketToADB->AddData(szPassWord, MAX_PASSWORD_LEN);
	pSendPacketToADB->AddULongLong(u64ClientSessionID);
	pLSConToServerNetWork->SendOnePacket(pADBServer->GetSessionID(), pADBServer->GetSendThreadID(), pSendPacketToADB);
	pLSConToServerNetWork->FlushSendPacket();
}
