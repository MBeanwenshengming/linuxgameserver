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
#include "../include/Server_To_Server_Packet_Define.h"

LMasterServerPacketProcessProc::LMasterServerPacketProcessProc()
{
	m_pMainLogicThread = NULL;
}

LMasterServerPacketProcessProc::~LMasterServerPacketProcessProc()
{
}

void LMasterServerPacketProcessProc::SetMainLogicThread(LMainLogicThread* pmlt)
{
	m_pMainLogicThread = pmlt;
} 

LMainLogicThread* LMasterServerPacketProcessProc::GetMainLogicThread()
{
	return m_pMainLogicThread;
}


bool LMasterServerPacketProcessProc::Initialize()
{
	if (m_pMainLogicThread == NULL)
	{
		return false;
	}
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req);
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1);
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req);
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res);
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Current_Serve_Count);

	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_L2M_USER_ONLINE);
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_WILL_LOGIN);


	;
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_GATESERVER_OFFLINE);

	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_GATESERVER_ONLINE);
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_SELECT_LOBBYSERVER);
	REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(Packet_LB2M_NOTIFY_USER_WILL_ONLINE);
	return true;
}

//	注册函数
bool LMasterServerPacketProcessProc::Register(unsigned int unPacketID, MASTERSERVER_PACKET_PROCESS_PROC pProc)
{
	if (pProc == NULL)
	{
		return false;
	}
	map<unsigned int, MASTERSERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapPacketProcessProcManager.find(unPacketID);
	if (_ito != m_mapPacketProcessProcManager.end())
	{
		return false;
	}
	m_mapPacketProcessProcManager[unPacketID] = pProc;
	return true;
}

//	派遣处理函数
void LMasterServerPacketProcessProc::DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return ;
	}

	unsigned int unPacketID = pPacket->GetPacketID();
	map<unsigned int, MASTERSERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapPacketProcessProcManager.find(unPacketID);
	if (_ito == m_mapPacketProcessProcManager.end())
	{
		//	error
		return;
	}
	_ito->second(this, u64SessionID, pPacket);
}


