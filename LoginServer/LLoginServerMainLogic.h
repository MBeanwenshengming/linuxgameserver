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

#include "LServerBaseNetWork.h"
#include "LThreadBase.h"
#include "LConnectToMaster.h"
#include "LConnectToAccountDBServer.h"
#include "../include/Server_Define.h"
#include "LLoginServerPacketProcess_Proc.h"
#include "LLSServerManager.h"
#include "LLoginServerConnectToServerNetWork.h"
#include "LClientManager.h"

#include <map>
using namespace std;

typedef struct _Session_Kick_Out
{
	uint64_t u64SessionID;	//	将要删除的连接的ID
	time_t tTimeToKickOut;	//	删除事件，没有到达删除事件，那么就不删除
}t_Session_Kick_Out;

class LLoginServerMainLogic : public LServerBaseNetWork, public LThreadBase, public LServerID
{
public:
	LLoginServerMainLogic();
	~LLoginServerMainLogic();
public:
	bool Initialize(char* pLoginServerConfigFileName, char* pNetWorkConfigFile, char* pMasterServerConfigFileName, char* pAccountDBServerConfigFileName);
public:			//	线程虚函数 
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
public:			//	对公网络虚函数 
	virtual bool OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa);
	virtual void OnRemoveSession(uint64_t u64SessionID);
	virtual void OnRecvedPacket(t_Recv_Packet& tRecvedPacket);
public:
	//	停止主线程
	bool StopLoginServerMainLogicThread();
	//	释放主线程资源
	bool ReleaseLoginServerMainLogicThreadResource();

public:		//	处理连接到masterserver的连接上的数据包
	void ProcessPacketFromMasterServer();
public:		//	处理从AccountDBServer上来的数据包
	void ProcessPacketFromAccountDBServer();
public:
	//	获取指向MasterServer连接
	LConnectToMaster* GetConnectToMaster();
	//	获取指向AccountDBServer连接
	LConnectToAccountDBServer* GetConnectToAccountDBServer();
	LLoginServerPacketProcess_Proc* GetPacketProcessProcManager();
protected:
	bool ReadLoginServerConfigFile(char* pFileName);

private:
	LConnectToMaster m_ConnectToMasterServer;
//	LConnectToAccountDBServer m_ConnectToAccountDBServer;
	LLoginServerPacketProcess_Proc m_PacketProcessManager; 
private:
	unsigned int m_unPacketProcessedFromMasterServer;
	unsigned int m_unPacketProcessedFromAccountDBServer;

public:
	LLSServerManager* GetConnectToServerManager();
	LLoginServerConnectToServerNetWork* GetLoginServerConToServerNetWork();
private: 
	LLSServerManager m_LSServerManager;
	LLoginServerConnectToServerNetWork m_LSConToServerNetWork;

	//	客户端管理
public:
	LClientManager* GetClientManager()
	{
		return &m_ClientManager;
	} 
private:
	LClientManager m_ClientManager;
	unsigned int m_unMaxClientServ;

public:
	void AddSessionIDToKickOutQueue(uint64_t u64SessionID, unsigned int unTimeToKeep);
private:
	map<uint64_t, t_Session_Kick_Out> m_mapSessionIDToKickOut;
};

