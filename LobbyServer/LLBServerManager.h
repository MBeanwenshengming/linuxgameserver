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

#include "../NetWork/IncludeHeader.h" 
#include "LLBServer.h"
#include <map>
using namespace std;
#include "../NetWork/LConnectorWorkManager.h"

class LNetWorkServices;
class LSelectServer;
class LLobbyServerMainLogic;

class LLBServerManager
{
public:
	LLBServerManager();
	~LLBServerManager();
public:
	bool Initialize(int nMaxConnectWorkQueueSize, int nNetWorkModelType, LSelectServer* pss, LNetWorkServices* pes);

public:
	//	添加，查找，删除服务器
	bool AddNewServer(uint64_t n64ServerSessionID, char* pExtData, unsigned short usExtDataLen, int nRecvThreadID, int nSendThreadID, char* pszIp, unsigned short usPort);

	//	根据ServerID查找服务器
	LLBServer* FindServerByServerID(uint64_t n64ServerID);
	void RemoveServerByServerID(uint64_t n64ServerID);
	bool ExistServerByServerID(uint64_t n64ServerID);

	//	根据SessionID查找服务器信息
	LLBServer* FindServerBySessionID(uint64_t n64SessionID);
	void RemoveServerBySessionID(uint64_t n64SessionID);

private:
	map<uint64_t, LLBServer*> m_mapSessionIDToServer;
	map<uint64_t, LLBServer*> m_mapServerIDToServer;

public:
	//	停止所有的连接线程
	bool StopAllConnectThread();
	//	释放服务器管理器资源
	void ReleaseServerManagerResource();

public: 
	//	连接任务
	bool AddConnectWork(char* pszIP, unsigned short usPort, char* pExtData, unsigned short usExtDataLen);

private:
	LConnectorWorkManager m_ConnectorWorkManager;
public:
	void SetLobbyServerMainLogic(LLobbyServerMainLogic* plbsml);
	LLobbyServerMainLogic* GetLobbyServerMainLogic();
private:
	LLobbyServerMainLogic* m_pLBMainLogic;
};

