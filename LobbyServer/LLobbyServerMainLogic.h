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
#include "LLBGateServerManager.h"
#include "LLBConnectToMasterServer.h"
#include "LLBConnectToGameDBServer.h" 
#include "LLobbyServerPacketProcess_Proc.h"
#include "LLBServerManager.h"
#include "LLBConnectToServersNetWork.h"
#include "LUserManager.h"

class LLobbyServerMainLogic : public LServerBaseNetWork, public LThreadBase, public LServerID
{
public:
	LLobbyServerMainLogic();
	~LLobbyServerMainLogic(); 

public:
	bool InitializeLobbyServerMainLogic(char* pConfigFileForNetWork, char* pConfigFileForMasterServer, char* pConfigFileForGameDBServer, char* pConfigFileForLobbyServer);

public:			//	线程虚函数 
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();

public:			//	对公网络虚函数 
	virtual bool OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa);
	virtual void OnRemoveSession(uint64_t u64SessionID);
	virtual void OnRecvedPacket(t_Recv_Packet& tRecvedPacket);

public:
	bool StopLobbyServerMainLogic();
	void ReleaseLobbyServerMainLogicResource();
protected:
	bool ReadConfigForLobbyServer(char* pConfigFileForLobbyServer);
public:
	LLBGateServerManager* GetGateServerManager()
	{
		return &m_GateServerManager;
	}
	LLBConnectToMasterServer* GetConnectToMasterServer()
	{
		return &m_ConnectToMasterServer;
	}
//	LLBConnectToGameDBServer* GetConnectToGameDBServer()
//	{
//		return &m_ConnectToGameDBServer;
//	}

private:
	LLBGateServerManager m_GateServerManager;
	LLBConnectToMasterServer m_ConnectToMasterServer;
	//LLBConnectToGameDBServer m_ConnectToGameDBServer;

public: 
	LLobbyServerPacketProcess_Proc* GetPacketProcessProcManager()
	{
		return &m_PacketProcessProcManager;
	}
private:
	LLobbyServerPacketProcess_Proc m_PacketProcessProcManager;
	

public:	//	连接到其它服务器
	LLBServerManager* GetConnectToServerManager();
private:
	LLBServerManager m_LBConnectToServerManager;
public:
	LLBConnectToServersNetWork* GetConnectToServerNetWork();
private:
	LLBConnectToServersNetWork m_ConnectToServerNetWork;

public:
	LUserManager* GetUserManager()
	{
		return &m_UserManager;
	}
private:
	LUserManager m_UserManager;
};


 
