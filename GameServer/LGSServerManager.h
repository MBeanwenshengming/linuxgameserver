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


class LGSServer;
class LGameServerMainLogic;

class LGSServerManager
{
public: 
	LGSServerManager();
	~LGSServerManager();
public: 
	//	初始化连接线程
	bool Initialize(int nMaxConnectWorkQueueSize, int nNetWorkModelType, LSelectServer* pss, LNetWorkServices* pes);

public: 
	bool AddNewServer(uint64_t n64ServerSessionID, char* pExtData, unsigned short usExtDataLen, int nRecvThreadID, int nSendThreadID, char* pszIp, unsigned short usPort);

	LGSServer* FindGSServerBySessionID(uint64_t u64SessionID);
	LGSServer* FineGSServerByServerID(int64_t u64ServerID);

	void RemoveGSServerBySessionID(uint64_t u64SessionID);
	void RemoveGSServerByServerID(uint64_t u64ServerID);


private:
	map<uint64_t, LGSServer*> m_mapSessionIDToServer;
	map<uint64_t, LGSServer*> m_mapServerIDToServer; 

public:
	void SetGameServerMainLogic(LGameServerMainLogic* pgsml);
	LGameServerMainLogic* GetGameServerMainLogic();
private:
	LGameServerMainLogic* m_pGSMainLogic;

public:
	bool StopGSServerManagerConnectThread();
	void ReleaseGSServerManagerConnectThreadResource();
public: 
	//	连接任务
	bool AddConnectWork(char* pszIP, unsigned short usPort, char* pExtData, unsigned short usExtDataLen);

private:
	LConnectorWorkManager m_ConnectorWorkManager;
};


