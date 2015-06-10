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

#include "LLoginServerConnectToServerNetWork.h"
#include "LLoginServerMainLogic.h"

LLoginServerConnectToServerNetWork::LLoginServerConnectToServerNetWork()
{
	m_pLoginServerMainLogic = NULL;
}
LLoginServerConnectToServerNetWork::~LLoginServerConnectToServerNetWork()
{
}
bool LLoginServerConnectToServerNetWork::Initialize(char* pConfigFile, unsigned int unMaxProcessPacketOnce, char* pConfigHeader)
{
	if (m_pLoginServerMainLogic == NULL)
	{
		return false;
	}
	if (pConfigFile == NULL || pConfigHeader == NULL)
	{
		return false;
	}
	if (unMaxProcessPacketOnce == 0)
	{
		unMaxProcessPacketOnce = 100;
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
bool LLoginServerConnectToServerNetWork::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{ 
	LLSServerManager* pLSServerManager = m_pLoginServerMainLogic->GetConnectToServerManager();
	if (!pLSServerManager->AddNewServer(u64SessionID, tsa.szExtData, tsa.usExtDataLen, tsa.nRecvThreadID, tsa.nSendThreadID, tsa.szIp, tsa.usPort))
	{
		//	添加服务器失败，那么端开该连接
		KickOutOneSession(u64SessionID);
		return false;
	}
	return true;
}
void LLoginServerConnectToServerNetWork::OnRemoveSession(uint64_t u64SessionID)
{ 
	LLSServerManager* pLSServerManager = m_pLoginServerMainLogic->GetConnectToServerManager();
	pLSServerManager->RemoveServerBySessionID(u64SessionID);
}
void LLoginServerConnectToServerNetWork::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	E_LoginServer_Packet_From_Type eFromType = E_LoginServer_Packet_From_Servers;
	m_pLoginServerMainLogic->GetPacketProcessProcManager()->DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, eFromType);
}

bool LLoginServerConnectToServerNetWork::StopConnectToServersNetWork()
{
	NetWorkDown();
	return true;
}
void LLoginServerConnectToServerNetWork::ReleaseConnectToServersResource()
{
}

void LLoginServerConnectToServerNetWork::SetLoginServerMainLogic(LLoginServerMainLogic* pLoginServerMainLogic)
{
	m_pLoginServerMainLogic = pLoginServerMainLogic;
}
LLoginServerMainLogic* LLoginServerConnectToServerNetWork::GetLoginServerMainLogic()
{
	return m_pLoginServerMainLogic;
}
