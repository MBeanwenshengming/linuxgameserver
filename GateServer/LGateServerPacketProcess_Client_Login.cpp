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



DEFINE_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_USER_LOGINKEY)
{ 
	LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();

	uint64_t u64UserUniqueIDInDB = 0;
	char szUniqueKeyToLogin[128];
	memset(szUniqueKeyToLogin, 0, sizeof(szUniqueKeyToLogin));

	pPacket->GetULongLong(u64UserUniqueIDInDB);
	pPacket->GetData(szUniqueKeyToLogin, 128);
	if (pPacket->GetErrorCode() != 0)
	{
		return;
	}
	
	LClientManager* pClientManager = pGateServerMainLogic->GetClientManager();
	LClient* pClient = pClientManager->FindClientBySessionID(u64SessionID);
	if (pClient == NULL)
	{
		return ;
	}
	if (!pClientManager->CheckClientInfo(u64SessionID, u64UserUniqueIDInDB, szUniqueKeyToLogin))
	{
		//	登录验证错误，关闭连接
		pGateServerMainLogic->KickOutOneSession(u64SessionID);
		return ;
	}
	//	向MasterServer发送玩家登录验证消息
	LConnectToMasterServer* pConToMasterServer = pGateServerMainLogic->GetConnectToMasterServer();
	unsigned short usPacketLen = 128;
	LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
	pSendPacket->SetPacketID(Packet_GA2M_USER_GATESERVER_ONLINE);
	pSendPacket->AddULongLong(u64SessionID);
	pSendPacket->AddLongLong(u64UserUniqueIDInDB);
	pConToMasterServer->AddOneSendPacket(pSendPacket);

	pClient->SetClientState(E_Client_State_Login_Waiting_Master_Confirm);
}

