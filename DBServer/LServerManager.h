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
class LServer;

#include "time.h"
#include "stdint.h"

typedef struct _Will_Connect_To_Server
{
	_Will_Connect_To_Server()
	{
		u64ServerUniqueID = 0;
		tTimeStart = 0;
	}
	uint64_t u64ServerUniqueID;
	time_t tTimeStart;
}t_Will_Connect_To_Server;

class LServerManager
{
public:
	LServerManager();
	~LServerManager();
public:
	bool NewUpServer(uint64_t u64SessionID, int nSendThreadID, int nRecvThreadID, char* pszIp, unsigned short usPort);
	bool SetServerID(uint64_t u64SessionID, uint64_t u64ServerID);
public:
	LServer* FindServerBySessionID(uint64_t u64SessionID);
	LServer* FindServerByServerID(uint64_t u64ServerID);
	void RemoveServerBySessionID(uint64_t u64sessionID);
	void RemoveServerByServerID(uint64_t u64ServerID); 
private:
	//	服务器ID对应服务器
	map<uint64_t, LServer*> m_mapServerIDToServer;
	//	服务器SessionID对应服务器
	map<uint64_t, LServer*> m_mapServerSessionToServer;
public:
	bool AddWillConnectToServer(uint64_t u64ServerUniqueID);
private:
	map<uint64_t, t_Will_Connect_To_Server> m_mapWillConnectToServer;
};
