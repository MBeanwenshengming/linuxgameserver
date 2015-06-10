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
#include "LServerInfo.h"

class LServer : public LServerInfo
{
public:
	LServer();
	~LServer();
public:
	void SetSessionID(int64_t u64SessionID);
	uint64_t GetSessionID();
	int GetSendThreadID();
	void SetSendThreadID(int nSendThreadID);
	int GetRecvThreadID();
	void SetRecvThreadID(int nRecvThreadID);
	void SetServerIp(char* pServerIp, unsigned int unSize);
	void GetServerIp(char* pbuf, unsigned int unbufSize);
	void SetServerPort(unsigned short usServerPort);
	unsigned short GetServerPort();
private:
	uint64_t m_u64SessionID;	//	服务器连接的SessionID
	int m_nRecvThreadID;		//	接收线程ID
	int m_nSendThreadID;		//	发送线程ID
	char m_szIP[20];			//	服务器的IP
	unsigned short m_usPort;	//	服务器的监听端口
};

