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
#include "../NetWork/LPacketSingle.h"
#include "LLoginServerMainLogic.h"
#include "../include/Server_To_Server_Packet_Define.h" 
#include "../include/Client_To_Server_Packet_Define.h"

LLoginServerPacketProcess_Proc::LLoginServerPacketProcess_Proc()
{
	m_pLoginServerMainLogic = NULL;
}

LLoginServerPacketProcess_Proc::~LLoginServerPacketProcess_Proc()
{
}

void LLoginServerPacketProcess_Proc::SetLoginServerMainLogic(LLoginServerMainLogic* plsml)
{
	m_pLoginServerMainLogic = plsml;
}

LLoginServerMainLogic* LLoginServerPacketProcess_Proc::GetLoginServerMainLogic()
{
	return m_pLoginServerMainLogic;
}

bool LLoginServerPacketProcess_Proc::Initialize()
{
	if (m_pLoginServerMainLogic == NULL)
	{
		return false;
	}
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res); 
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1);
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res);
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos);
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Current_Serve_Count_BroadCast);

	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_CS_START_REQ);
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_CS_2LS_USERID_AND_PASSWORD); 
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(Packet_ADB2L_USER_VERIFY);
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(Packet_M2L_USER_ONLINE_VERIFY);

	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_C2LS_CREATE_USER);
	REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(Packet_ADB2L_ADD_USER);
	return true;
}

//	注册函数
bool LLoginServerPacketProcess_Proc::Register(unsigned int unPacketID, LOGINSERVER_PACKET_PROCESS_PROC pProc)
{
	if (pProc == NULL)
	{
		return false;
	}
	if (unPacketID == 0)
	{
		return false;
	}

	map<unsigned int, LOGINSERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapLoginPacketProcessProcManager.find(unPacketID);
	if (_ito != m_mapLoginPacketProcessProcManager.end())
	{
		return false;
	}
	m_mapLoginPacketProcessProcManager[unPacketID] = pProc;
	return true;
}

//	派遣处理函数
void LLoginServerPacketProcess_Proc::DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket, E_LoginServer_Packet_From_Type eFromType)
{
	if (pPacket == NULL)
	{
		return ;
	}

	unsigned int unPacketID = pPacket->GetPacketID();	
	map<unsigned int, LOGINSERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapLoginPacketProcessProcManager.find(unPacketID);
	if (_ito == m_mapLoginPacketProcessProcManager.end())
	{
		return ;
	}
	_ito->second(this, u64SessionID, pPacket, eFromType);
}

