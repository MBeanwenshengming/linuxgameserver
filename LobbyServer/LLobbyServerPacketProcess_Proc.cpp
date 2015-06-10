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

#include "LLobbyServerPacketProcess_Proc.h"
#include "../NetWork/LPacketSingle.h"
#include "LLobbyServerMainLogic.h"



LLobbyServerPacketProcess_Proc::LLobbyServerPacketProcess_Proc()
{
	m_plsml = NULL;
}
LLobbyServerPacketProcess_Proc::~LLobbyServerPacketProcess_Proc()
{
}

void LLobbyServerPacketProcess_Proc::SetLobbyServerMainLogic(LLobbyServerMainLogic* plsml)
{
	m_plsml = plsml;
}
LLobbyServerMainLogic* LLobbyServerPacketProcess_Proc::GetLobbyServerMainLogic()
{
	return m_plsml;
}

bool LLobbyServerPacketProcess_Proc::Initialize()
{
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Res);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos);

	//	逻辑处理协议
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_M2LB_NOTIFY_USER_WILL_ONLINE);
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_GA2LB_USER_ONLINE); 
	REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_GA2LB_USER_ONLINE);
	return true;
}

//	注册函数
bool LLobbyServerPacketProcess_Proc::Register(unsigned int unPacketID, LOBBYSERVER_PACKET_PROCESS_PROC pProc)
{ 
	if (pProc == NULL)
	{
		return false;
	}
	map<unsigned int, LOBBYSERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapPacketProcessProcManager.find(unPacketID);
	if (_ito != m_mapPacketProcessProcManager.end())
	{
		return false;
	}
	m_mapPacketProcessProcManager[unPacketID] = pProc;
	return true;
}

//	派遣处理函数
void LLobbyServerPacketProcess_Proc::DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket, E_LobbyServer_Packet_From_Type eFromType)
{
	if (pPacket == NULL)
	{
		return ;
	}
	unsigned int unPacketID = pPacket->GetPacketID();

	map<unsigned int, LOBBYSERVER_PACKET_PROCESS_PROC>::iterator _ito = m_mapPacketProcessProcManager.find(unPacketID);
	if (_ito == m_mapPacketProcessProcManager.end())
	{
		return ;
	}
	_ito->second(this, u64SessionID, pPacket, eFromType);
}
