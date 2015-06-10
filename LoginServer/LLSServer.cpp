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

#include "LLSServer.h"

LLSServer::LLSServer()
{
	m_u64SessionID = 0;
	m_nRecvThreadID = 0;		//	接收线程ID
	m_nSendThreadID = 0;		//	发送线程ID
	memset(m_szIP, 0, sizeof(m_szIP));			//	服务器的IP
	m_usPort 		= 0;		//	服务器的监听端口
	m_unServeCount	= 0;
}
LLSServer::~LLSServer()
{
}

void LLSServer::SetSessionID(uint64_t u64SessionID)
{
	m_u64SessionID = u64SessionID;
}

uint64_t LLSServer::GetSessionID()
{
	return m_u64SessionID;
}

int LLSServer::GetSendThreadID()
{
	return m_nSendThreadID;
}

void LLSServer::SetSendThreadID(int nSendThreadID)
{
	m_nSendThreadID = nSendThreadID;
}
int LLSServer::GetRecvThreadID()
{
	return m_nRecvThreadID;
}
void LLSServer::SetRecvThreadID(int nRecvThreadID)
{
	m_nRecvThreadID = nRecvThreadID;
}
void LLSServer::SetServerIp(char* pServerIp, unsigned int unSize)
{ 
	strncpy(m_szIP, pServerIp, sizeof(m_szIP) - 1);
}
void LLSServer::GetServerIp(char* pbuf, unsigned int unbufSize)
{
	strncpy(pbuf, m_szIP, unbufSize);
}
void LLSServer::SetServerPort(unsigned short usServerPort)
{
	m_usPort = usServerPort;
}
unsigned short LLSServer::GetServerPort()
{
	return m_usPort;
}



void LLSServer::SetServeCount(unsigned int unServeCount)
{
	m_unServeCount = unServeCount;
}
unsigned int LLSServer::GetServeCount()
{
	return m_unServeCount;
}
