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

#include "LLBGateServer.h"

LLBGateServer::LLBGateServer()
{
	m_eGateServerState = E_GateServer_None;
	m_u64SessionID = 0;
	m_nSendThreadID = 0;
	m_nRecvThreadID = 0;
}
LLBGateServer::~LLBGateServer()
{
}

void LLBGateServer::SetSessionID(uint64_t u64SessionID)
{
	m_u64SessionID = u64SessionID;
}
uint64_t LLBGateServer::GetSessionID()
{
	return m_u64SessionID;
}

void LLBGateServer::SetServerState(E_Gate_Server_State eState)
{
	m_eGateServerState = eState;
}

E_Gate_Server_State LLBGateServer::GetServerState()
{
	return m_eGateServerState;
}

int LLBGateServer::GetSendThreadID()
{
	return m_nSendThreadID;
}
void LLBGateServer::SetSendThreadID(int nSendThreadID)
{
	m_nSendThreadID = nSendThreadID;
}
int LLBGateServer::GetRecvThreadID()
{
	return m_nRecvThreadID;
}
void LLBGateServer::SetRecvThreadID(int nRecvThreadID)
{
	m_nRecvThreadID = nRecvThreadID;
}

