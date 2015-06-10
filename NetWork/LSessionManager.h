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

#ifndef __LINUX_SESSION_MANAGER_HEADER_DEFINED__
#define __LINUX_SESSION_MANAGER_HEADER_DEFINED__

#include "IncludeHeader.h"
#include "LSession.h"
#include <map>
#include <queue>

using namespace std;
class LNetWorkServices;

class LSessionManager
{
public:
	LSessionManager();
	~LSessionManager();
public:
	bool AddSession(LSession* pSession);	
	LSession* FindSession(unsigned long long u64SessionID);
	void ReMoveSession(LSession* pSession);
	void ReMoveSession(unsigned long long u64SessionID);	
	bool GetSessionIPAndPort(unsigned long long u64SessionID, char* pszBuf, unsigned short usbufLen);
public: 
	void SetKickOutIdleSession(time_t tValue);
	void KickOutIdleSession(time_t tNow);
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices)
	{
		m_pNetWorkServices = pNetWorkServices;
	}
public:
	void ReleaseSessionManagerResource();
private: 
	pthread_rwlock_t m_rwLock; 
	map<unsigned long long, LSession*> m_mapSessionManager;

	// zui jin yi ci de mei you tong xun de lian jie 
	time_t m_tLastTimeIdleSessionCheck; 
	LNetWorkServices* m_pNetWorkServices;

public:
	void IncrementSessionManageredNum()
	{
		__sync_add_and_fetch(&m_nCurrentSesssionManagered, 1);
	}
	void DecrementSessionManageredNum()
	{
		__sync_sub_and_fetch(&m_nCurrentSesssionManagered, 1);
	}
	int GetSessionManageredNum()
	{
		return __sync_add_and_fetch(&m_nCurrentSesssionManagered, 0);
	}
private:
	volatile int m_nCurrentSesssionManagered;
};

class LMasterSessionManager
{
public:
	LMasterSessionManager();
	~LMasterSessionManager();

	void FreeSessionToPool(LSession* pSession);
protected:

	unsigned short SelectSessionManager();
private: 
	queue<LSession*> m_queueDirtySessionManager;
	pthread_mutex_t m_mutexForDirty;
	unsigned long long m_ullSessionID;
	//	mei ge guan li qi guan li de lian jie shu liang 
	//	zai fen pei lian jie guan li qi de shi hou shi yong
	unsigned short m_usSessionManagerCount;

public:
	bool InitializeSessionPool(unsigned short ucSessionManagerNum, unsigned int unMaxSessionCountPerManager, unsigned int unSendBufChainSize);
	LSession* AllocSession();
	LSession* FindSession(unsigned long long u64SessionID);
	void FreeSession(LSession* pSession);
	void Release();
	bool GetSessionIPAndPort(unsigned long long u64SessionID, char* pszBuf, unsigned short usbufLen);

	void PrintAllSessionInfos();
private:
	LSessionManager* m_parrMasterSessionManager;
	unsigned int m_unMaxSessionCountPerManager;
	unsigned int m_unMaxSessionCount;

public:
	//	shan chu gu ding shi jian mei you tong xun de lian jie 
	void KickOutIdleSession(); 
public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices)
	{
		m_pNetWorkServices = pNetWorkServices;
	}
private:
	LNetWorkServices* m_pNetWorkServices;
public:
	void SetTimeForKickOutIdleSession(unsigned short usTime)
	{
		if (usTime != 0)
		{
			m_usTime = usTime;
		}
	}
	unsigned short GetTimeForKickOutIdleSession()
	{
		return m_usTime;
	}
private:
	unsigned short m_usTime; 	// 踢出未通信连接的时间
public:
	//	释放所有的Session和SessionManager，以及它们占用的资源
	void ReleaseMasterSessionManagerResource();

public:
	void AddToWillReuseManager(LSession* pSession);
//	void ReMoveFromWillReuseManager(LSession* pSession);
	size_t GetWillReuseSessionCount();
	void MoveWillCloseSessionToSessionPool(LSession* pSession);
private:
	//	准备释放重用的连接管理器，Session从Manager里面移除之后，放在这个管理器中
	//	等Epoll线程recv线程send线程和主线程(如果会操作Session的话)告知全部不使用了，那么这里的Session才放入SessionPool里面
	pthread_mutex_t m_MutexProtectedForWillReuseManager;		//	manager的保护器，操作这个的是多线程
	map<LSession*, LSession*> m_mapSessionWillReuseManager;
};

#endif

