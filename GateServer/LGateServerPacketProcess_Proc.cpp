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
#include "../NetWork/LPacketBroadCast.h"
#include "LServerManager.h"
#include "LServer.h"

LGateServerPacketProcess_Proc::LGateServerPacketProcess_Proc()
{
	m_pgsml = NULL;
}
LGateServerPacketProcess_Proc::~LGateServerPacketProcess_Proc()
{
}

void LGateServerPacketProcess_Proc::SetGateServerMainLogic(LGateServerMainLogic* pgsml)
{
	m_pgsml = pgsml;
}
LGateServerMainLogic* LGateServerPacketProcess_Proc::GetGateServerMainLogic()
{
	return m_pgsml;
}

//	初始化化处理函数
bool LGateServerPacketProcess_Proc::Initialize()
{ 
	if (m_pgsml == NULL)
	{
		return false;
	}
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res);
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast);
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1);
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res);
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos);
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_USER_WILL_LOGIN);

	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_USER_GATESERVER_ONLINE);

	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_SELECT_LOBBYSERVER);

	REGISTER_GATESERVER_PACKET_PROCESS_PROC(Packet_LB2GA_USER_ONLINE);
	//	客户端数据包 
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_USER_LOGINKEY);
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_ENTER_LOBBYSERVER);
	REGISTER_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_GET_ROLE_INFO);
	return true;
}
//	注册函数
bool LGateServerPacketProcess_Proc::Register(unsigned int unPacketID, GATESERVER_PACKET_PROCESS_PROC pProc)
{ 
	map<unsigned int, GATESERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapPacketProcessProc.find(unPacketID);
	if (_ito != m_mapPacketProcessProc.end())
	{
		//	error write
		return false;
	}
	m_mapPacketProcessProc[unPacketID] = pProc;
	return true;
}
//	派遣处理函数
void LGateServerPacketProcess_Proc::DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket, E_Packet_From_Type eFromType)
{
	if (pPacket == NULL)
	{
		return ;
	}
	unsigned int unPacketID = pPacket->GetPacketID();
	if (unPacketID == 0)
	{
		return ;
	}

	map<unsigned int, GATESERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapPacketProcessProc.find(unPacketID);
	if (_ito == m_mapPacketProcessProc.end())
	{
		return;
	}
	_ito->second(this, u64SessionID, pPacket, eFromType);
}

DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res)
{
	if (eFromType == E_Packet_From_MasterServer)	//	从MasterServer来的信息包，那么发送服务器ID给MasterServer
	{
		pGateServerPacketProcessProc->GetGateServerMainLogic()->GetConnectToMasterServer()->ReportServerIDToMasterServer();
	}
	else if (eFromType == E_Packet_From_OtherServer)
	{
		LGateServerMainLogic* pGateServerMainLogic = pGateServerPacketProcessProc->GetGateServerMainLogic();
		LConnectToServerNetWork* pConToServerNetWork = pGateServerMainLogic->GetConnectToServerNetWork();

		LServerManager* pServerManager = pGateServerMainLogic->GetServerManager();
		LServer* pServer = pServerManager->FindServerBySessionID(u64SessionID);
		if (pServer == NULL)
		{
			return ;
		}
		uint64_t u64ServerUniqueID = pGateServerMainLogic->GetServerUniqueID();
		unsigned short usPacketLen = 32;
		LPacketBroadCast* pSendPacket = pConToServerNetWork->GetOneSendPacket(usPacketLen);
		pSendPacket->SetPacketID(Packet_SS_Register_Server_Req1);
		pSendPacket->AddULongLong(u64ServerUniqueID);
		pConToServerNetWork->SendOnePacket(pServer->GetSessionID(), pServer->GetSendThreadID(), pSendPacket);
		pConToServerNetWork->FlushSendPacket();
	}
}

DEFINE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast)
{
}
