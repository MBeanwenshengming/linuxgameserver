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

#include "LLBConnectToServersNetWork.h"
#include "LLobbyServerMainLogic.h"
#include "LLBServerManager.h"
#include "../NetWork/LPacketBroadCast.h"
#include "../include/Server_To_Server_Packet_Define.h"

LLBConnectToServersNetWork::LLBConnectToServersNetWork()
{
	m_pLobbyServerMainLogic = NULL;
}
LLBConnectToServersNetWork::~LLBConnectToServersNetWork()
{
}

bool LLBConnectToServersNetWork::Initialize(char* pConfigFile, unsigned int unMaxProcessPacketOnce, char* pConfigHeader)
{
	if (m_pLobbyServerMainLogic == NULL || pConfigFile == NULL || pConfigHeader == NULL)
	{
		return false;
	}
	if (!InitializeNetWork(pConfigFile, unMaxProcessPacketOnce, pConfigHeader, false))
	{
		return false;
	}
	if (!NetWorkStart())
	{
		return false;
	}
	return true;
}

bool LLBConnectToServersNetWork::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	LLBServerManager* pLBServerManager = m_pLobbyServerMainLogic->GetConnectToServerManager();
	if (!pLBServerManager->AddNewServer(u64SessionID, tsa.szExtData, tsa.usExtDataLen, tsa.nRecvThreadID, tsa.nSendThreadID, tsa.szIp, tsa.usPort))
	{
		//	添加服务器失败，那么端开该连接
		KickOutOneSession(u64SessionID);
		return false;
	}

	LLBServer* pServer = pLBServerManager->FindServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		KickOutOneSession(u64SessionID);
		return false;
	}
	unsigned short usDataLen = 32;
	LPacketBroadCast* pSendPacket = GetOneSendPacket(usDataLen);
	pSendPacket->SetPacketID(Packet_SS_Start_Req); 
	SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
	FlushSendPacket();

	return true;
}

void LLBConnectToServersNetWork::OnRemoveSession(uint64_t u64SessionID)
{
	LLBServerManager* pLBServerManager = m_pLobbyServerMainLogic->GetConnectToServerManager();
	pLBServerManager->RemoveServerBySessionID(u64SessionID);
}

void LLBConnectToServersNetWork::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	E_LobbyServer_Packet_From_Type eFromType = E_LobbyServer_Packet_From_ConnectToServer;
	m_pLobbyServerMainLogic->GetPacketProcessProcManager()->DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, eFromType); 
}

bool LLBConnectToServersNetWork::StopConnectToServersNetWork()
{
	NetWorkDown();
	return true;
}

void LLBConnectToServersNetWork::ReleaseConnectToServersNetWork()
{
}

void LLBConnectToServersNetWork::SetLobbyServerMainLogic(LLobbyServerMainLogic* plsm)
{
	m_pLobbyServerMainLogic = plsm;
}

LLobbyServerMainLogic* LLBConnectToServersNetWork::GetLobbyServerMainLogic()
{
	return m_pLobbyServerMainLogic;
}
