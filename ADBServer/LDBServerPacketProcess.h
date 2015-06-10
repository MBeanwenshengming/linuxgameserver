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

class LPacketSingle;

typedef enum 
{
	E_DBServer_Packet_From_Client = 1,
	E_DBServer_Packet_From_Master,							  
}E_DBServer_Packet_From_Type;

class LDBServerPacketProcess;


typedef void (*DBSERVER_PACKET_PROCESS_PROC)(LDBServerPacketProcess* pDBServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_DBServer_Packet_From_Type eFromType);

#define REGISTER_DBSERVER_PACKET_PROCESS_PROC(X) Register(X, (DBSERVER_PACKET_PROCESS_PROC)LDBServerPacketProcess::Proc_##X);

#define DECLARE_DBSERVER_PACKET_PROCESS_PROC(X) static void Proc_##X(LDBServerPacketProcess* pDBServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_DBServer_Packet_From_Type eFromType)

#define DEFINE_DBSERVER_PACKET_PROCESS_PROC(X) void LDBServerPacketProcess::Proc_##X(LDBServerPacketProcess* pDBServerPacketProcessProc, uint64_t u64SessionID, LPacketSingle* pPacket, E_DBServer_Packet_From_Type eFromType)

class LDBServerMainLogicThread;
class LDBServerPacketProcess
{
public:
	LDBServerPacketProcess();
	~LDBServerPacketProcess();
public:
	bool Initialize();
	//	注册函数
	bool Register(unsigned int unPacketID, DBSERVER_PACKET_PROCESS_PROC pProc);
	//	派遣处理函数
	void DispatchMessageProcess(uint64_t u64SessionID, LPacketSingle* pPacket, E_DBServer_Packet_From_Type eFromType);
private: 
	map<unsigned int, DBSERVER_PACKET_PROCESS_PROC> m_mapPacketProcessProcManager;

public:
	void SetDBServerMainLogicThread(LDBServerMainLogicThread* pdbsmlt);
	LDBServerMainLogicThread* GetDBServerMainLogicThread();
private:
	LDBServerMainLogicThread* m_pDBServerMainLogic;

public:
	DECLARE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Req); 
	DECLARE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Start_Res); 
	DECLARE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Res);
	DECLARE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Server_Will_Connect_Req);
	DECLARE_DBSERVER_PACKET_PROCESS_PROC(Packet_SS_Register_Server_Req1);

	//	处理逻辑数据包
	DECLARE_DBSERVER_PACKET_PROCESS_PROC(Packet_L2ADB_USER_VERIFY);
	DECLARE_DBSERVER_PACKET_PROCESS_PROC(Packet_L2ADB_ADD_USER);
};


