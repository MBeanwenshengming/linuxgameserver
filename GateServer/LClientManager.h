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
#include <queue> 
using namespace std;
class LClient;
class LGateServerMainLogic;
#include "../include/Common_Define.h"
#include "../NetWork/LServerBaseNetWork.h"

typedef struct _Client_Will_Connect
{
	char szLoginKey[128];
	char szUserID[MAX_USER_ID_LEN + 1];
}t_Client_Will_Connect;

typedef struct _Client_Will_Connect_TimeOutInfo
{
	uint64_t u64UserUniqueIDInDB;
	time_t tMoveTime;
}t_Client_Will_Connect_TimeOutInfo;

class LClientManager
{
public:
	LClientManager();
	~LClientManager();
public:
	//	玩家新连接上来
	bool AddNewUpClient(uint64_t u64SesssionID, t_Session_Accepted& tas);	

	//	检查玩家发送的的登录信息是否存在
	bool CheckClientInfo(uint64_t u64SessionID, uint64_t u64UserUniqueIDInDB, char* pszLoginKey);

	//	查找玩家
	LClient* FindClientBySessionID(uint64_t u64SessionID);
	LClient* FineClientByUserUniqueIDInDB(uint64_t u64UserUniqueIDInDB);

	//	移除玩家
	void RemoveClientBySessionID(uint64_t u64SessionID);
	void RemoveClientByUserUniqueIDInDB(uint64_t u64UserUniqueIDInDB);
private:
	map<uint64_t, LClient*> m_mapSessionIDToClient;
	map<uint64_t, LClient*> m_mapUniqueIDInDBToClient;

public:
	bool InitializeClientPool(unsigned int unPoolSize);
	LClient* AllocClient();
	void FreeClient(LClient* pClient);
	void ReleaseClientManagerResoutce();
private:
	queue<LClient*> m_ClientPool;


public:		//	处理玩家预登录信息
	bool AddWillConnectClientInfo(uint64_t u64UserUniqueIDInDB, char* pszLoginKey, char* pszUserID);
	bool FindWillConnectClientInfo(uint64_t u64UserUniqueIDInDB,char* pszLoginKeyBuf, unsigned int unBufLen, char* pszUserIDBuf, unsigned int unUIDBufLen);
	void ProcessTimeOutWillConnect();

	bool BuildLoginKey(char* pbuf, unsigned int unbufLen);
	unsigned int GetServeCount();
private:
	map<uint64_t, t_Client_Will_Connect> m_mapClientWillConnectInfos;
	queue<t_Client_Will_Connect_TimeOutInfo> m_queueTimeOutQueue;
	unsigned int m_unLoginKeyWillConnect;
public:
	void SetGateServerMainLogic(LGateServerMainLogic* pgsml);
	LGateServerMainLogic* GetGateServerMainLogic();
private:
	LGateServerMainLogic* m_pGateServerMainLogic;
};


