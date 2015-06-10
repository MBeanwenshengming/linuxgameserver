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

typedef enum
{
	E_GateServer_None = 0,
	E_GateServer_NewUp,				//	新连接上来
	E_GateServer_ServerID_Set,		//	GateServer已通报上ServerID
	E_GateServer_Communicate_Start,	//	与GateServer正常通训开始
}E_Gate_Server_State;

class LLBGateServer : public LServerID
{
public:
	LLBGateServer();
	~LLBGateServer();

public:
	void SetSessionID(uint64_t u64SessionID);
	uint64_t GetSessionID();

	void SetServerState(E_Gate_Server_State eState);
	E_Gate_Server_State GetServerState();

	int GetSendThreadID();
	void SetSendThreadID(int nSendThreadID);
	int GetRecvThreadID();
	void SetRecvThreadID(int nRecvThreadID);
private:
	E_Gate_Server_State m_eGateServerState; 
	uint64_t m_u64SessionID;
	int m_nSendThreadID;
	int m_nRecvThreadID;
};

