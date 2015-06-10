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

#include "../include/Server_Define.h"
#include "../NetWork/LConnectorWorkManager.h"
#include<map>
using namespace std;

#define MAX_CONNECT_WORK_QUEUE 200

class LServer;
class LGateServerMainLogic;

class LServerManager
{
public:
	LServerManager();
	~LServerManager();
public:
	//	初始化连接线程
	bool Initialize(int nMaxConnectWorkQueueSize, int nNetWorkModelType, LSelectServer* pss, LNetWorkServices* pes);
public:
	//	添加，查找，删除服务器
	bool AddNewServer(uint64_t n64ServerSessionID, char* pExtData, unsigned short usExtDataLen, int nRecvThreadID, int nSendThreadID, char* pszIp, unsigned short usPort);
	//	根据ServerID查找服务器
	LServer* FindServerByServerID(uint64_t n64ServerID);
	void RemoveServerByServerID(uint64_t n64ServerID);
	bool ExistServerByServerID(uint64_t n64ServerID);

	//	根据SessionID查找服务器信息
	LServer* FindServerBySessionID(uint64_t n64SessionID);
	void RemoveServerBySessionID(uint64_t n64SessionID);

	//	停止所有的连接线程
	bool StopAllConnectThread();
	//	释放服务器管理器资源
	void ReleaseServerManagerResource();
public:
protected:
	
private:
	//	服务器分组管理
	map<uint64_t, LServer*> m_mapServerIDToServer;
	//	SessionID映射到Server
	map<uint64_t, LServer*> m_mapSessionIDToServer;
	//	接收到服务器信息的时候，开起连接线程
public: 
	//	连接任务
	bool AddConnectWork(char* pszIP, unsigned short usPort, char* pExtData, unsigned short usExtDataLen);

private:
	LConnectorWorkManager m_ConnectorWorkManager;

public:
	LGateServerMainLogic* GetGateServerMainLogic();
	void SetGateServerMainLogic(LGateServerMainLogic* pgsml);
private:
	LGateServerMainLogic* m_pGateServerMainLogic;
};

