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

#include "LGameServerConnectToServerNetWork.h"
#include "LGameServerMainLogic.h" 
#include "../NetWork/LPacketBroadCast.h"
#include "../include/Server_To_Server_Packet_Define.h" 
#include "LGSPacketProcessManager.h"
#include "LGSServerManager.h"
#include "LGSServer.h"


LGameServerConnectToServerNetWork::LGameServerConnectToServerNetWork()
{
	m_pGameServerMainLogic = NULL;
}
LGameServerConnectToServerNetWork::~LGameServerConnectToServerNetWork()
{
}
bool LGameServerConnectToServerNetWork::InitialzeGSConToServerNetWork(char* pConfigFile, unsigned int unMaxProcessPacketOnce, char* pConfigHeader)
{
	if (pConfigFile == NULL || pConfigHeader == NULL || m_pGameServerMainLogic == NULL)
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

bool LGameServerConnectToServerNetWork::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	LGSServerManager* pGSServerManager = m_pGameServerMainLogic->GetGSServerManager();
	if (!pGSServerManager->AddNewServer(u64SessionID, tsa.szExtData, tsa.usExtDataLen, tsa.nRecvThreadID, tsa.nSendThreadID, tsa.szIp, tsa.usPort))
	{
		return false;
	}

	LGSServer* pServer = pGSServerManager->FindGSServerBySessionID(u64SessionID);
	if (pServer == NULL)
	{
		return false;
	}
	unsigned short usPacketLen = 16;
	LPacketBroadCast* pSendPacket = GetOneSendPacket(usPacketLen);
	pSendPacket->SetPacketID(Packet_SS_Start_Req);
	SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
	FlushSendPacket();
	return true;
}
void LGameServerConnectToServerNetWork::OnRemoveSession(uint64_t u64SessionID)
{
	LGSServerManager* pGSServerManager = m_pGameServerMainLogic->GetGSServerManager();
	pGSServerManager->RemoveGSServerBySessionID(u64SessionID);
}
void LGameServerConnectToServerNetWork::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	LGSPacketProcessManager* pPacketProcessManager = m_pGameServerMainLogic->GetGSPacketProcessManager();

	E_GameServer_Packet_From_Type eFromType = E_GS_Packet_From_Con_Server;
	pPacketProcessManager->DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, eFromType);
}
bool LGameServerConnectToServerNetWork::StopGSConnectToServerNetWork()
{
	NetWorkDown();
	return true;
}
void LGameServerConnectToServerNetWork::ReleaseGSConnectToServerNetWork()
{
}
void LGameServerConnectToServerNetWork::SetGameServerMainLogic(LGameServerMainLogic* pgsml)
{
	m_pGameServerMainLogic = pgsml;
}
LGameServerMainLogic* LGameServerConnectToServerNetWork::GetGameServerMainLogic()
{
	return m_pGameServerMainLogic;
}
