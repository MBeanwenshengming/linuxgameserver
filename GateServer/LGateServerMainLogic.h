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

#include "../NetWork/LServerBaseNetWork.h"
#include "../NetWork/LThreadBase.h"
#include "LConnectToMasterServer.h"
#include "LConnectToServerNetWork.h"
#include "LServerManager.h"
#include "../include/Server_Define.h"
#include "LGateServerPacketProcess_Proc.h"
#include "LClientManager.h"
#include "LGateServerLogicProcess.h"

#include <map>
using namespace std;

typedef struct _Session_Kick_Out
{
	uint64_t u64SessionID;	//	将要删除的连接的ID
	time_t tTimeToKickOut;	//	删除事件，没有到达删除事件，那么就不删除
}t_Session_Kick_Out;

class LGateServerMainLogic : public LServerBaseNetWork, public LThreadBase , public LServerID
{
public:
	LGateServerMainLogic(); 
	~LGateServerMainLogic();
public:
	//	初始化对公网网络层和连接到MasterServer连接
	bool Initialize(char* pNetWorkConfigFileName, char* pConnectToMasterServerConfigFileName, char* pConnnectToServerConfigFileName, char* pGateServerConfigFileName);
private:
	bool ReadGateServerConfigFile(char* pConfigFile);
public:			//	对公网连接的处理函数
	virtual bool OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa);
	virtual void OnRemoveSession(uint64_t u64SessionID);
	virtual void OnRecvedPacket(t_Recv_Packet& tRecvedPacket);

public: 		// 线程虚函数
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
public:
	bool StopGateServerMainLogicThread();
	void ReleaseGateServerMainLogicThreadResource();

	//	获取服务器管理器
	LServerManager* GetServerManager()
	{
		return &m_ServerManager;
	}
	LConnectToServerNetWork* GetConnectToServerNetWork()
	{
		return &m_ConnectToServerNetWork;
	}
	LConnectToMasterServer* GetConnectToMasterServer()
	{
		return &m_ConnectToMasterServer;
	}
	LGateServerPacketProcess_Proc* GetPacketProcessManager()
	{
		return &m_PacketProcessManager;
	}
	LClientManager* GetClientManager()
	{
		return &m_ClientManager;
	}
	LGateServerLogicProcess* GetGateServerLogicProcess()
	{
		return &m_GateServerLogicProcess;
	}
private:
	//	连接到MasterServer
	LConnectToMasterServer m_ConnectToMasterServer;
	LConnectToServerNetWork m_ConnectToServerNetWork;
	LServerManager m_ServerManager;
	//	消息包处理类
	LGateServerPacketProcess_Proc m_PacketProcessManager;

	//	客户端管理器
	LClientManager m_ClientManager;	

	LGateServerLogicProcess m_GateServerLogicProcess;

public:
	void AddSessionIDToKickOutQueue(uint64_t u64SessionID, unsigned int unTimeToKeep);
private:
	map<uint64_t, t_Session_Kick_Out> m_mapSessionIDToKickOut;

};
							 
