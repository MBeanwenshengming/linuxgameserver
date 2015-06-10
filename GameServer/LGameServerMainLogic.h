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
#include "../include/Server_Define.h"
#include "LGameServerConnectToMasterServer.h"
#include "LGameServerConnectToServerNetWork.h"
#include "LGSClientManager.h"
#include "LGSClientServerManager.h"
#include "LGSPacketProcessManager.h"
#include "LGSServerManager.h"

class LGameServerMainLogic : public LServerBaseNetWork, public LThreadBase, public LServerID
{
public:
	LGameServerMainLogic();
	~LGameServerMainLogic();
public:
	bool InitializeGameServer(char* pGameServerConfigFile, unsigned int unGameServerID);
protected:
	bool ReadServerIDConfig(char* pGameServerConfigFile, char* pSection);
public:			//	线程虚函数 
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();

public:			//	对公网络虚函数 
	virtual bool OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa);
	virtual void OnRemoveSession(uint64_t u64SessionID);
	virtual void OnRecvedPacket(t_Recv_Packet& tRecvedPacket);
public:
	bool StopGameServerMainLogic();
	void ReleaseGameServerMainLogicResource();

public:
	LGameServerConnectToMasterServer* GetConToMasterServer()
	{
		return &m_GSConToMasterServer;
	}
	LGameServerConnectToServerNetWork* GetGSConToServerNetWork()
	{
		return &m_GSConToServerNetWork;
	}
	LGSClientManager* GetGSClientManager()
	{
		return &m_GSClientManager;
	}
	LGSClientServerManager* GetGSClientServerManager()
	{
		return &m_GSClientServerManager;
	}
	LGSPacketProcessManager* GetGSPacketProcessManager()
	{
		return &m_GSPacketProcessManager;
	}
	LGSServerManager* GetGSServerManager()
	{
		return &m_GSServerManager;
	}
private:
	//	连接到MasterServer
	LGameServerConnectToMasterServer m_GSConToMasterServer;
	//	连接到其它服务器
	LGameServerConnectToServerNetWork m_GSConToServerNetWork;
	//	玩家管理器
	LGSClientManager m_GSClientManager;
	//	连接上来的服务器管理器
	LGSClientServerManager m_GSClientServerManager;
	//	协议包处理
	LGSPacketProcessManager m_GSPacketProcessManager;
	//	连接到其它服务器的管理
	LGSServerManager m_GSServerManager;
};


