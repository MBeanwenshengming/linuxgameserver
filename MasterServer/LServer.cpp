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

#include "LServer.h"



LServer::LServer()
{
	memset(&m_ServerBaseInfo, 0, sizeof(m_ServerBaseInfo));
	m_eServerState = E_Server_State_Unknow;
	m_u64SessionID = 0;
	m_unCurrentServeCount = 0;
}

LServer::~LServer()
{

} 

void LServer::Reset()
{
	memset(&m_ServerBaseInfo, 0, sizeof(m_ServerBaseInfo));
	m_eServerState = E_Server_State_Unknow;
	m_u64SessionID = 0;
}

void LServer::SetServerBaseInfo(t_Session_Accepted& tSa)
{
	memcpy(&m_ServerBaseInfo, &tSa, sizeof(t_Session_Accepted));
}

int LServer::GetIP(char* pszBuf, size_t sBufLen)
{
	if (sBufLen <= 16)
	{
		return -1;
	}
	memcpy(pszBuf, m_ServerBaseInfo.szIp, 15);
	return 0;
}
unsigned short LServer::GetPort()
{
	return m_ServerBaseInfo.usPort;
}
int LServer::GetRecvThreadID()
{
	return m_ServerBaseInfo.nRecvThreadID;
}
int LServer::GetSendThreadID()
{
	return m_ServerBaseInfo.nSendThreadID;
}

void LServer::GetServerSessionBaseInfo(t_Session_Accepted& tsa)
{
	memcpy(&tsa, &m_ServerBaseInfo, sizeof(t_Session_Accepted));
}

void LServer::SetCurrentServeCount(unsigned int unCount)
{
	m_unCurrentServeCount = unCount;
}
unsigned int LServer::GetCurrentServeCount()
{
	return m_unCurrentServeCount;
}

LLoginServer::LLoginServer()
{
}
LLoginServer::~LLoginServer()
{
}

LGateServer::LGateServer()
{
}
LGateServer::~LGateServer()
{
}

LLobbyServer::LLobbyServer()
{
}
LLobbyServer::~LLobbyServer()
{
}


LGameServer::LGameServer()
{
}
LGameServer::~LGameServer()
{
}

LDBServer::LDBServer()
{
}
LDBServer::~LDBServer()
{
}


LAccountDBServer::LAccountDBServer()
{
}
LAccountDBServer::~LAccountDBServer()
{
}
