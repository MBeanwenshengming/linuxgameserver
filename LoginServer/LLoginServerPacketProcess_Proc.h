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

class LPacketSingle;
class LLoginServerPacketProcess_Proc;
class LLoginServerMainLogic;

typedef enum
{
	E_LoginServer_Packet_From_Client,
	E_LoginServer_Packet_From_MasterServer,
	E_LoginServer_Packet_From_AccountDBServer,
	E_LoginServer_Packet_From_Servers,
}E_LoginServer_Packet_From_Type;

typedef void (*LOGINSERVER_PACKET_PROCESS_PROC)(LLoginServerPacketProcess_Proc* pLoginServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_LoginServer_Packet_From_Type eFromType);

#define REGISTER_LOGINSERVER_PACKET_PROCESS_PROC(X) Register(X, (LOGINSERVER_PACKET_PROCESS_PROC)LLoginServerPacketProcess_Proc::Proc_##X);

#define DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(X) static void Proc_##X(LLoginServerPacketProcess_Proc* pLoginServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_LoginServer_Packet_From_Type eFromType)

#define DEFINE_LOGINSERVER_PACKET_PROCESS_PROC(X) void LLoginServerPacketProcess_Proc::Proc_##X(LLoginServerPacketProcess_Proc* pLoginServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_LoginServer_Packet_From_Type eFromType)

class LLoginServerPacketProcess_Proc
{
public:
	LLoginServerPacketProcess_Proc();
	~LLoginServerPacketProcess_Proc();
public: 
	bool Initialize();
	//	注册函数
	bool Register(unsigned int unPacketID, LOGINSERVER_PACKET_PROCESS_PROC pProc);
	//	派遣处理函数
	void DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket, E_LoginServer_Packet_From_Type eFromType);
private:
	//	逻辑包处理函数管理器
	map<unsigned int, LOGINSERVER_PACKET_PROCESS_PROC> m_mapLoginPacketProcessProcManager;
public:
	void SetLoginServerMainLogic(LLoginServerMainLogic* plsml);
	LLoginServerMainLogic* GetLoginServerMainLogic();
private:
	LLoginServerMainLogic* m_pLoginServerMainLogic;
public:
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Register_Broadcast1);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Res);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Can_ConnectTo_Server_Infos);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Current_Serve_Count_BroadCast);

	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_CS_START_REQ);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_CS_2LS_USERID_AND_PASSWORD);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_ADB2L_USER_VERIFY);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_M2L_USER_ONLINE_VERIFY);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(PACKET_C2LS_CREATE_USER);
	DECLARE_LOGINSERVER_PACKET_PROCESS_PROC(Packet_ADB2L_ADD_USER);
};


