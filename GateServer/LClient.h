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
#include "../include/Common_Define.h"

typedef enum
{
	E_Client_State_UnKnown = 0,	//	玩家状态未知
	E_Client_State_Just_Connect,	//	玩家刚刚连接上来
	E_Client_State_Initialize,	//	玩家在GateServer服务器上初始化完毕
	E_Client_State_Login_Waiting_Master_Confirm,	//	等待MasterServer登录确认
	E_Client_State_Logined,	//	MasterServer登录确认完成
	E_Client_State_Request_Lobby,	//	玩家请求登录Lobby
	E_Client_State_Loginning_Lobby,		//	玩家正在登录Lobby
	E_Client_State_In_Lobby,			//	玩家在Lobby服务器上 
	E_Client_State_Request_GameServer,	//	玩家请求登录GameServer
	E_Client_State_Loginning_GameServer,	//	玩家正在登录GameServer
	E_Client_State_In_GameServer,	//	玩家正在GameServer 服务器上
}E_Client_State;


class LClient
{
public:
	LClient();
	~LClient();
public:
	void Reset();
public:
	void SetUniqueIDInDB(uint64_t u64UniqueIDInDB);
	uint64_t GetUniqueIDInDB();
	void SetClientState(E_Client_State eClientState);
	E_Client_State GetClientState();
	void SetCurrentServerUniqueID(uint64_t u64ServerID);
	uint64_t GetCurrentServerUniqueID();
	void SetUserID(char* pszUserID);
	void GetUserID(char* pbuf, unsigned int unbufLen);
private:
	uint64_t m_u64UniqueIDInDB;					//	玩家的唯一帐号ID
	char m_szUserID[MAX_USER_ID_LEN + 1];		//	玩家帐户名
	E_Client_State m_eClientState;				//	玩家当前的状态
	uint64_t m_u64CurrentServerUniqueID;		//	当前正在连接的服务器唯一ID
	//	网络相关
public:
	void SetSessionID(uint64_t u64SessionID);
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
	int m_nRecvThreadID;		//	接收线程ID
	int m_nSendThreadID;		//	发送线程ID
	uint64_t m_u64SessionID;
	char m_szIP[20];			//	服务器的IP
	unsigned short m_usPort;	//	服务器的监听端口 
};


