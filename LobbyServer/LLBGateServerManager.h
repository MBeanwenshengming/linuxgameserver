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
#include "stdint.h"
using namespace std;
class LLBGateServer;
#include "time.h"
typedef struct _Server_Will_Connect
{
	uint64_t u64ServerID;
	time_t unReqStartTime;
}t_Server_Will_Connect;

class LLBGateServerManager
{
public:
	LLBGateServerManager();
	~LLBGateServerManager();
public:
	bool Initialize();
public:
	bool AddNewUpServer(uint64_t uSessionID);
	bool UpdateServerID(uint64_t uSessionID, uint64_t uServerID);

	LLBGateServer* FindGateServerBySessionID(uint64_t u64SessionID);
	LLBGateServer* FindGateServerByServerID(uint64_t u64ServerID);

	void RemoveGateServerBySessionID(uint64_t u64SessionID);
	void RemoveGateServerByServerID(uint64_t u64ServerID);
private: 
	//	SessionID映射到GateServer
	map<uint64_t, LLBGateServer*> m_mapSessionIDToGateServer;
	//	ServerID映射到GateServer
	map<uint64_t, LLBGateServer*> m_mapServerIDToGateServer;


public:		//	即将连接的服务器信息，在规定的时间内没有连接，那么清除信息，
	void AddWillConnectServer(uint64_t u64ServerID);
	bool ExistWillConnectServer(uint64_t u64ServerID);
	void RemoveWillConnectServer(uint64_t u64ServerID);
private:
	map<uint64_t, t_Server_Will_Connect> m_mapWillConnectServer;
};

