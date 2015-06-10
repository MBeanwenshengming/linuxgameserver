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

#include "../include/Common_Define.h"
#include "../include/Server_Define.h"
#include "../NetWork/LServerBaseNetWork.h"

class LServer : public LServerID
{
public:
	LServer();
	virtual ~LServer();
public:
	inline void SetServerState(E_Server_State eServerState)
	{
		m_eServerState = eServerState;
	}
	inline E_Server_State GetServerState()
	{
		return m_eServerState;
	}
	void SetServerBaseInfo(t_Session_Accepted& tSa);
		
public:
	void Reset();
public:
	int GetIP(char* pszBuf, size_t sBufLen);
	unsigned short GetPort();
	int GetRecvThreadID();
	int GetSendThreadID();
	inline uint64_t GetNetWorkSessionID()
	{
		return m_u64SessionID;
	}
	inline void SetNetWorkSessionID(uint64_t u64SessionID)
	{
		m_u64SessionID = u64SessionID;
	}
	void GetServerSessionBaseInfo(t_Session_Accepted& tsa);

	void SetCurrentServeCount(unsigned int unCount);
	unsigned int GetCurrentServeCount();
private:
	uint64_t m_u64SessionID;			//	该连接在网络层的唯一标志ID
	E_Server_State m_eServerState;
	t_Session_Accepted m_ServerBaseInfo;
	unsigned int m_unCurrentServeCount;		//	当前服务器服务器的人数
};

class LLoginServer : public LServer
{
public:
	LLoginServer();
	virtual ~LLoginServer();
private:
};

class LGateServer : public LServer
{
public:
	LGateServer();
	virtual ~LGateServer();
private: 
};

class LLobbyServer : public LServer
{
public:
	LLobbyServer();
	virtual ~LLobbyServer();
private:

};


class LGameServer : public LServer
{
public:
	LGameServer();
	virtual ~LGameServer();
private:
};

class LDBServer : public LServer
{
public:
	LDBServer();
	virtual ~LDBServer();
private:
};

class LAccountDBServer : public LServer
{
public:
	LAccountDBServer();
	~LAccountDBServer();
private:
};


