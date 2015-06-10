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

#include <map>
using namespace std;

#include "../include/Server_To_Server_Packet_Define.h"
#include "../include/Client_To_Server_Packet_Define.h"

#include "IncludeHeader.h"
class LPacketSingle;
class LGateServerPacketProcess_Proc;

typedef enum
{
	E_Packet_From_MasterServer = 0,
	E_Packet_From_OtherServer,
	E_Packet_From_Client,
}E_Packet_From_Type;

typedef void (*GATESERVER_PACKET_PROCESS_PROC)(LGateServerPacketProcess_Proc* pGateServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_Packet_From_Type eFromType);

#define REGISTER_GATESERVER_PACKET_PROCESS_PROC(X) Register(X, (GATESERVER_PACKET_PROCESS_PROC)LGateServerPacketProcess_Proc::Proc_##X);

#define DECLARE_GATESERVER_PACKET_PROCESS_PROC(X) static void Proc_##X(LGateServerPacketProcess_Proc* pGateServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_Packet_From_Type eFromType)

#define DEFINE_GATESERVER_PACKET_PROCESS_PROC(X) void LGateServerPacketProcess_Proc::Proc_##X(LGateServerPacketProcess_Proc* pGateServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_Packet_From_Type eFromType)

class LGateServerMainLogic;

class LGateServerPacketProcess_Proc
{
public:
	LGateServerPacketProcess_Proc();
	~LGateServerPacketProcess_Proc();
public:
	//	初始化化处理函数
	bool Initialize();
	//	注册函数
	bool Register(unsigned int unPacketID, GATESERVER_PACKET_PROCESS_PROC pProc);
	//	派遣处理函数
	void DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket, E_Packet_From_Type eFromType);
public:
	//	服务器广播
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_USER_WILL_LOGIN);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_USER_GATESERVER_ONLINE);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_M2GA_SELECT_LOBBYSERVER);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(Packet_LB2GA_USER_ONLINE);
public:		//	客户端数据包
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_USER_LOGINKEY);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_ENTER_LOBBYSERVER);
	DECLARE_GATESERVER_PACKET_PROCESS_PROC(PACKET_C2GA_GET_ROLE_INFO);
private:
	map<unsigned int, GATESERVER_PACKET_PROCESS_PROC> m_mapPacketProcessProc;

public:
	void SetGateServerMainLogic(LGateServerMainLogic* pgsml);
	LGateServerMainLogic* GetGateServerMainLogic();
private:
	LGateServerMainLogic* m_pgsml;
};

