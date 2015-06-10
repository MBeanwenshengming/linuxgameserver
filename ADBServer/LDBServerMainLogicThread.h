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
#include "../NetWork/LWorkQueueManager.h"
#include "LServerManager.h"
#include "LDBOpThread.h"
#include "LDBServerPacketProcess.h"
#include "LDBServerConnectToMasterServer.h"
#include "LUserInfoManager.h"
#include "LADBServerDBMessageProcess.h"

class LDBServerMainLogicThread : public LThreadBase, public LServerBaseNetWork, public LServerID
{
public:
	LDBServerMainLogicThread();
	~LDBServerMainLogicThread(); 
public:
	bool InitializeDBServer(char* pConfigFileForDBServer);
protected:
	bool ReadDBServerConfigFile(char* pConfigFileForDBServer, char* pSectionName);
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
	bool StopDBServerMainLogicThread();
	//	释放主线程资源
	bool ReleaseDBServerMainLogicThreadResource();

public:
	LServerManager* GetServerManager();
	LWorkQueueManager* GetWorkQueueManager();
public:
	LWorkItem* GetOneFreeWorkItemFromPoolInMainLogic(unsigned short usNeedBufLen);
	bool AddOneWorkItemToMainLogic(LWorkItem* pWorkItem);
private:	//	当前的工作队列
	LWorkQueueManager m_LocalWorkQueueManager;
	
	//	服务器管理
	LServerManager m_ServerManager;

public:
	LDBOpThread* GetDBOpThread()
	{
		return &m_DBOpThreadManager;
	}
protected:
	bool InitializeDBOpThread(char* pConfigFile, char* pSectionHeader);
private:
	LDBOpThread m_DBOpThreadManager;


public: 
	LDBServerPacketProcess* GetPacketProcessManager();
private:
	LDBServerPacketProcess m_PacketProcessManager;
public:
	LDBServerConnectToMasterServer* GetDBServerConnectToMasterServer();
private:
	LDBServerConnectToMasterServer m_DBConToMasterServer;


public:		//	玩家信息管理器
	LUserInfoManager* GetUserInfoManager()
	{
		return &m_UserInfoManager;
	}
private:
	LUserInfoManager m_UserInfoManager;
	unsigned int m_unDBMessageFromDBThreadProcessed;
	LADBServerDBMessageProcess m_DBServerDBMessageProcessManager;

public:
	void ProcessDBMessageFromDTThread(); 
};


