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

#include "LDBServerPacketProcess.h"
#include "LDBServerMainLogicThread.h"
#include "../NetWork/LPacketSingle.h"

LDBServerPacketProcess::LDBServerPacketProcess()
{
	m_pDBServerMainLogic = NULL;
}

LDBServerPacketProcess::~LDBServerPacketProcess()
{
}

void LDBServerPacketProcess::SetDBServerMainLogicThread(LDBServerMainLogicThread* pdbsmlt)
{
	m_pDBServerMainLogic = pdbsmlt;
}
LDBServerMainLogicThread* LDBServerPacketProcess::GetDBServerMainLogicThread()
{
	return m_pDBServerMainLogic;
}

bool LDBServerPacketProcess::Initialize()
{
	if (m_pDBServerMainLogic == NULL)
	{
		return false;
	}

	REGISTER_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req); 
	REGISTER_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res); 
	REGISTER_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Res);
	REGISTER_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req);
	REGISTER_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1);
	return true;
}
//	注册函数
bool LDBServerPacketProcess::Register(unsigned int unPacketID, DBSERVER_PACKET_PROCESS_PROC pProc)
{ 
	if (pProc == NULL)
	{
		return false;
	}
	map<unsigned int, DBSERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapPacketProcessProcManager.find(unPacketID);
	if (_ito != m_mapPacketProcessProcManager.end())
	{
		if (_ito->second == pProc)
		{
			return true;
		}
		return false;
	}
	m_mapPacketProcessProcManager[unPacketID] = pProc;
	return true;
}
//	派遣处理函数
void LDBServerPacketProcess::DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket, E_DBServer_Packet_From_Type eFromType)
{
	if (pPacket == NULL)
	{
		return ;
	}
	unsigned int unPacketID = pPacket->GetPacketID();

	map<unsigned int, DBSERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapPacketProcessProcManager.find(unPacketID);
	if (_ito == m_mapPacketProcessProcManager.end())
	{
		return;
	}
	_ito->second(this, u64SessionID, pPacket, eFromType);
}
