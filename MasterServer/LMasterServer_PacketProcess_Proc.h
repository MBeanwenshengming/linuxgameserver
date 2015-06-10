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
#include "stdint.h"

class LMasterServerPacketProcessProc;
class LPacketSingle;
class LMainLogicThread;

typedef void (*MASTERSERVER_PACKET_PROCESS_PROC)(LMasterServerPacketProcessProc* pMasterServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket);

#define REGISTER_MASTERSERVER_PACKET_PROCESS_PROC(X) Register(X, (MASTERSERVER_PACKET_PROCESS_PROC)LMasterServerPacketProcessProc::Proc_##X);

#define DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(X) static void Proc_##X(LMasterServerPacketProcessProc* pMasterServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket)

#define DEFINE_MASTERSERVER_PACKET_PROCESS_PROC(X) void LMasterServerPacketProcessProc::Proc_##X(LMasterServerPacketProcessProc* pMasterServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket)

class LMasterServerPacketProcessProc
{
public:
	LMasterServerPacketProcessProc();
	~LMasterServerPacketProcessProc();
public:
	bool Initialize();
public: 
	//	注册函数
	bool Register(unsigned int unPacketID, MASTERSERVER_PACKET_PROCESS_PROC pProc);
	//	派遣处理函数
	void DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket);

private:
	map<unsigned int, MASTERSERVER_PACKET_PROCESS_PROC> m_mapPacketProcessProcManager;
public:
	void SetMainLogicThread(LMainLogicThread* pmlt); 
	LMainLogicThread* GetMainLogicThread();
private:
	LMainLogicThread* m_pMainLogicThread;
	

public:
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Current_Serve_Count);

	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_L2M_USER_ONLINE);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_WILL_LOGIN);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_L2M_USER_OFFLINE);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_GATESERVER_OFFLINE);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_USER_GATESERVER_ONLINE);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_GA2M_SELECT_LOBBYSERVER);
	DECLARE_MASTERSERVER_PACKET_PROCESS_PROC(Packet_LB2M_NOTIFY_USER_WILL_ONLINE);

};


