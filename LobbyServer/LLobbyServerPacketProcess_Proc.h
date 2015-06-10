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

#pragma once

#include "../include/Server_To_Server_Packet_Define.h"
#include <map>
using namespace std;
class LPacketSingle;

typedef enum
{
	E_LobbyServer_Packet_From_GateServer,
	E_LobbyServer_Packet_From_MasterServer,
	E_LobbyServer_Packet_From_GameDBServer,
	E_LobbyServer_Packet_From_ConnectToServer,
}E_LobbyServer_Packet_From_Type;

class LLobbyServerPacketProcess_Proc;

typedef void (*LOBBYSERVER_PACKET_PROCESS_PROC)(LLobbyServerPacketProcess_Proc* pLobbyServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_LobbyServer_Packet_From_Type eFromType);

#define REGISTER_LOBBYSERVER_PACKET_PROCESS_PROC(X) Register(X, (LOBBYSERVER_PACKET_PROCESS_PROC)LLobbyServerPacketProcess_Proc::Proc_##X);

#define DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(X) static void Proc_##X(LLobbyServerPacketProcess_Proc* pLobbyServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_LobbyServer_Packet_From_Type eFromType)

#define DEFINE_LOBBYSERVER_PACKET_PROCESS_PROC(X) void LLobbyServerPacketProcess_Proc::Proc_##X(LLobbyServerPacketProcess_Proc* pLobbyServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_LobbyServer_Packet_From_Type eFromType)

class LLobbyServerMainLogic;

class LLobbyServerPacketProcess_Proc
{
public:
	LLobbyServerPacketProcess_Proc();
	~LLobbyServerPacketProcess_Proc();

public: 
	bool Initialize();
	//	注册函数
	bool Register(unsigned int unPacketID, LOBBYSERVER_PACKET_PROCESS_PROC pProc);
	//	派遣处理函数
	void DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket, E_LobbyServer_Packet_From_Type eFromType);

private:
	map<unsigned int, LOBBYSERVER_PACKET_PROCESS_PROC> m_mapPacketProcessProcManager;
public:
	void SetLobbyServerMainLogic(LLobbyServerMainLogic* plsml);
	LLobbyServerMainLogic* GetLobbyServerMainLogic();
private:
	LLobbyServerMainLogic* m_plsml;
	
public:
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Res);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos);

	//	逻辑处理协议
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_M2LB_NOTIFY_USER_WILL_ONLINE);
	DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_GA2LB_USER_ONLINE);
	//DECLARE_LOBBYSERVER_PACKET_PROCESS_PROC(Packet_GA2LB_USER_ONLINE);
};

