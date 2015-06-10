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

#include "LConnectToServerNetWork.h"
#include "LServerManager.h"
#include "LGateServerMainLogic.h"
#include "../NetWork/LPacketBroadCast.h"
#include "../include/Server_To_Server_Packet_Define.h"
#include "LServer.h"

LConnectToServerNetWork::LConnectToServerNetWork()
{
	m_pGateServerMainLogic = NULL;
}

LConnectToServerNetWork::~LConnectToServerNetWork()
{
}

	//	初始化网络层
bool LConnectToServerNetWork::Initialize(char* pNetWorkConfigFile, unsigned int unMaxProcessPacketOnce)
{
	if (m_pGateServerMainLogic == NULL)
	{
		return false;
	}
	if (!InitializeNetWork(pNetWorkConfigFile, unMaxProcessPacketOnce, "GateServer_For_Servers_1", false))
	{
		return false;
	}
	if (!NetWorkStart())
	{
		return false;
	}
	return true;
}

bool LConnectToServerNetWork::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	LServerManager* pServerManager = m_pGateServerMainLogic->GetServerManager();
	if (!pServerManager->AddNewServer(u64SessionID, tsa.szExtData, tsa.usExtDataLen, tsa.nRecvThreadID, tsa.nSendThreadID, tsa.szIp, tsa.usPort))
	{
		//	添加服务器失败，那么端开该连接
		KickOutOneSession(u64SessionID);
		return false;
	}
	//	添加成功，那么向服务器发送数据包初始序列包
	LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
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

void LConnectToServerNetWork::OnRemoveSession(uint64_t u64SessionID)
{ 
	LServerManager* pServerManager = m_pGateServerMainLogic->GetServerManager();
	pServerManager->RemoveServerBySessionID(u64SessionID);
}

void LConnectToServerNetWork::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	//	tRecvedPacket.u64SessionID, tRecvedPacket.pPacket
	E_Packet_From_Type eFromType = E_Packet_From_OtherServer;
	m_pGateServerMainLogic->GetPacketProcessManager()->DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, eFromType);
}

bool LConnectToServerNetWork::StopConnectToServerNetWork()
{
	NetWorkDown();
	return true;
}

void LConnectToServerNetWork::ReleaseConnectToServerNetWorkResource()
{

}

void LConnectToServerNetWork::SetGateServerMainLogic(LGateServerMainLogic* pgsml)
{
	m_pGateServerMainLogic = pgsml;
}

LGateServerMainLogic* LConnectToServerNetWork::GetGateServerMainLogic()
{
	return m_pGateServerMainLogic;
}


