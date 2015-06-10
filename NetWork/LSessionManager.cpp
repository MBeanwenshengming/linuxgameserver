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

#include "LSessionManager.h"

#include "LErrorWriter.h" 
#include "LCloseSocketThread.h"
#include "LNetWorkServices.h"
#include "LAutoReleaseMutex.h"

extern LErrorWriter g_ErrorWriter;

//	=================================SessionManager==================Start
LSessionManager::LSessionManager()
{
	m_pNetWorkServices = NULL;
	pthread_rwlock_init(&m_rwLock, NULL);
	m_tLastTimeIdleSessionCheck = 0;
	__sync_lock_test_and_set(&m_nCurrentSesssionManagered, 0);
}
LSessionManager::~LSessionManager()
{
	pthread_rwlock_destroy(&m_rwLock);
}

bool LSessionManager::AddSession(LSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LSessionManager::AddSession, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	unsigned long long u64SessionID = pSession->GetSessionID();
	if (u64SessionID == 0)
	{
		char szError[512];
		sprintf(szError, "LSessionManager::AddSession, u64SessionID == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	pthread_rwlock_wrlock(&m_rwLock);
	map<unsigned long long, LSession*>::iterator _ito =  m_mapSessionManager.find(u64SessionID);
	if (_ito != m_mapSessionManager.end())
	{
		pthread_rwlock_unlock(&m_rwLock);

		char szError[512];
		sprintf(szError, "LSessionManager::AddSession, _ito != m_mapSessionManager.end()\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		return false;
	}
	m_mapSessionManager[u64SessionID] = pSession;
	pthread_rwlock_unlock(&m_rwLock);
	return true;
}
LSession* LSessionManager::FindSession(unsigned long long u64SessionID)
{
	LSession* pSession = NULL;
	pthread_rwlock_rdlock(&m_rwLock);
	map<unsigned long long, LSession*>::iterator _ito =  m_mapSessionManager.find(u64SessionID);
	if (_ito != m_mapSessionManager.end())
	{
		pSession = _ito->second; 
	}
	pthread_rwlock_unlock(&m_rwLock);
	return pSession;
}
bool LSessionManager::GetSessionIPAndPort(unsigned long long u64SessionID, char* pszBuf, unsigned short usbufLen)
{ 
	LSession* pSession = NULL;
	pthread_rwlock_rdlock(&m_rwLock);
	map<unsigned long long, LSession*>::iterator _ito =  m_mapSessionManager.find(u64SessionID);
	if (_ito != m_mapSessionManager.end())
	{
		pSession = _ito->second; 
		bool bResult = pSession->GetIpAndPort(pszBuf, usbufLen); 
		pthread_rwlock_unlock(&m_rwLock);
		return bResult;
	}
	else
	{
		pthread_rwlock_unlock(&m_rwLock);
		return false;
	}
	pthread_rwlock_unlock(&m_rwLock);
	return true;
}
void LSessionManager::ReMoveSession(LSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LSessionManager::ReMoveSession, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return ;
	}
	unsigned long long u64SessionID = pSession->GetSessionID();
	pthread_rwlock_wrlock(&m_rwLock);
	map<unsigned long long, LSession*>::iterator _ito =  m_mapSessionManager.find(u64SessionID);
	if (_ito != m_mapSessionManager.end())
	{
		m_mapSessionManager.erase(_ito);	
	}
	pthread_rwlock_unlock(&m_rwLock); 
}
void LSessionManager::ReMoveSession(unsigned long long u64SessionID)
{ 
	pthread_rwlock_wrlock(&m_rwLock);
	map<unsigned long long, LSession*>::iterator _ito =  m_mapSessionManager.find(u64SessionID);
	if (_ito != m_mapSessionManager.end())
	{
		m_mapSessionManager.erase(_ito);	
	}
	pthread_rwlock_unlock(&m_rwLock); 
}
void LSessionManager::SetKickOutIdleSession(time_t tValue)
{
	m_tLastTimeIdleSessionCheck = tValue;
}
void LSessionManager::KickOutIdleSession(time_t tNow)
{
	unsigned short usKickOutTime = m_pNetWorkServices->GetSessionManager().GetTimeForKickOutIdleSession();
	if (tNow > m_tLastTimeIdleSessionCheck && tNow - m_tLastTimeIdleSessionCheck > 60)
	{
		pthread_rwlock_rdlock(&m_rwLock);
		map<unsigned long long, LSession*>::iterator _ito =  m_mapSessionManager.begin();
		while (_ito != m_mapSessionManager.end())
		{
			LSession* pSession = _ito->second;
			if (pSession == NULL)
			{
				++_ito;
				continue;
			}

			time_t tConnectTime =  pSession->GetSessionConnecttedTime();
			int nDifTime = tNow - tConnectTime;
			int nDifSendTime = pSession->GetLastSendTime();
			int nDifRecvTime = pSession->GetLastRecvTime();
			//if (nDifTime >= nDifSendTime + 5 * 60 && nDifTime >= nDifRecvTime + 5 * 60)
			if (nDifTime > nDifRecvTime + usKickOutTime)
			{
				char szError[512];
				sprintf(szError, "LSessionManager::KickOutIdleSession, Close Session!\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
				//	tian jia shan chu shi jian
//				t_Client_Need_To_Close cntc;
//				cntc.u64SessionID = pSession->GetSessionID();
//				LCloseSocketThread* pCloseThread = &(m_pNetWorkServices->GetCloseSocketThread());
//				pCloseThread->AppendToClose(cntc);
				m_pNetWorkServices->ProcessKickOutSession(pSession->GetSessionID());
			}
			++_ito;
		}
		pthread_rwlock_unlock(&m_rwLock);
		m_tLastTimeIdleSessionCheck = tNow;
	}
}
void LSessionManager::ReleaseSessionManagerResource()
{ 
	map<unsigned long long, LSession*>::iterator _ito = m_mapSessionManager.begin();
	while (_ito != m_mapSessionManager.end())
	{
		LSession* pSession = _ito->second;
		//	释放Session占用的资源
		pSession->ReleaseSessionResource();
		//	删除Session自身占用的资源
		delete pSession;
		_ito++;
	}
}

// ================================SessionManager=======================ended


//	================================MasterSessionManager=============started
LMasterSessionManager::LMasterSessionManager()
{
	pthread_mutex_init(&m_mutexForDirty, NULL);
	m_ullSessionID 					= 0;
	m_unMaxSessionCount 			= 0;
	m_unMaxSessionCountPerManager	= 0;
	m_usSessionManagerCount			= 0;
	m_parrMasterSessionManager		= NULL;
	m_pNetWorkServices				= NULL;
	m_usTime 						= 30;		//	20秒未通信就踢出连接，默认

	pthread_mutex_init(&m_MutexProtectedForWillReuseManager, NULL);
}
LMasterSessionManager::~LMasterSessionManager()
{
	pthread_mutex_destroy(&m_mutexForDirty);
	pthread_mutex_destroy(&m_MutexProtectedForWillReuseManager);
}

//	public member
bool LMasterSessionManager::InitializeSessionPool(unsigned short usSessionManagerNum, unsigned int unMaxSessionCountPerManager, unsigned int unSendBufChainSize)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	if (usSessionManagerNum == 0)
	{
		usSessionManagerNum = 1;
	}
	if (usSessionManagerNum >= 255)
	{
		usSessionManagerNum = 255;
	}
	if (unMaxSessionCountPerManager == 0)
	{
		unMaxSessionCountPerManager = 1;
	}
	m_usSessionManagerCount = usSessionManagerNum;
	m_unMaxSessionCountPerManager = unMaxSessionCountPerManager;
	m_unMaxSessionCount = unMaxSessionCountPerManager * usSessionManagerNum;

	unsigned short usSessionManagerID = 0;
	

	m_parrMasterSessionManager = new LSessionManager[m_usSessionManagerCount];
	if (m_parrMasterSessionManager == NULL)
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::InitializeSessionPool, m_parrMasterSessionManager == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned short usIndex = 0; usIndex < usSessionManagerNum; ++usIndex)
	{ 
		m_parrMasterSessionManager[usIndex].SetNetWorkServices(m_pNetWorkServices);
		m_parrMasterSessionManager[usIndex].SetKickOutIdleSession(time(NULL) + 5 * usIndex);
	}
	for (unsigned int uIndex = 0; uIndex < m_unMaxSessionCount; ++uIndex)
	{
		LSession* pTemp = new LSession; 
		if (pTemp == NULL)
		{
			char szError[512];
			sprintf(szError, "LMasterSessionManager::InitializeSessionPool, pTemp == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		pTemp->Reset();
#ifdef __ADD_SEND_BUF_CHAIN__
		if (!pTemp->InitializeSendBufChain(unSendBufChainSize))
		{
			delete pTemp;
			return false;
		}
#endif
		m_queueDirtySessionManager.push(pTemp);
	}
	return true;
}
unsigned short LMasterSessionManager::SelectSessionManager()
{
	unsigned short usSessionManagerID = 0;
	int unMinSessionCount = 0x0fffffff;
	for (unsigned short usIndex = 0; usIndex < m_usSessionManagerCount; ++usIndex)
	{
		int nSessionCount = m_parrMasterSessionManager[usIndex].GetSessionManageredNum();
		if (nSessionCount < unMinSessionCount)
		{
			unMinSessionCount = nSessionCount;
			usSessionManagerID = usIndex;
		}
	}
	return usSessionManagerID;
}
LSession* LMasterSessionManager::AllocSession()
{
	unsigned short usSessionManagerSelected = SelectSessionManager();

	LSession* pSession = NULL;
	pthread_mutex_lock(&m_mutexForDirty);
	if (m_queueDirtySessionManager.empty())
	{
		pthread_mutex_unlock(&m_mutexForDirty);

		char szError[512];
		sprintf(szError, "LMasterSessionManager::AllocSession, m_queueDirtySessionManager.empty()\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));


		return NULL;
	}
	pSession = m_queueDirtySessionManager.front();
	m_queueDirtySessionManager.pop();

	pSession->Reset();

	m_ullSessionID++;
	unsigned long long ullMaxID = 0x0000ffffffffffffll;
	if (m_ullSessionID >= ullMaxID)	//  da yu 48 wei hui gun
	{
		m_ullSessionID = 0;
	}
	unsigned long long u64SessionID = usSessionManagerSelected;
    u64SessionID = (u64SessionID << 48) | m_ullSessionID;
	pSession->SetSessionID(u64SessionID);
#ifdef __ADD_SEND_BUF_CHAIN__
	//	设置为非关闭状态
	pSession->SetIsClosing(0);
#endif
	//	zeng jia yi ge guan li qi de yin yong ji shu
	m_parrMasterSessionManager[usSessionManagerSelected].IncrementSessionManageredNum();
	pthread_mutex_unlock(&m_mutexForDirty);

	m_parrMasterSessionManager[usSessionManagerSelected].AddSession(pSession);

	return pSession;
}
LSession* LMasterSessionManager::FindSession(unsigned long long u64SessionID)
{ 
	if (u64SessionID == 0ll)
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::FindSession, u64SessionID == 0ll\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return NULL;
	}
	unsigned long long u64SessionManagerID = u64SessionID >> 48;

	unsigned short usSessionManagerID = (unsigned short)u64SessionManagerID;
	if (usSessionManagerID > m_usSessionManagerCount)
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::FindSession, usSessionManagerID > m_usSessionManagerCount\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return NULL;
	}
	return m_parrMasterSessionManager[usSessionManagerID].FindSession(u64SessionID);
	
}
void LMasterSessionManager::FreeSession(LSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::FreeSession, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return ;
	}
	unsigned long long u64SessionID = pSession->GetSessionID();

	unsigned long long u64SessionManagerID = u64SessionID >> 48; 
	unsigned short usSessionManagerID = (unsigned short)u64SessionManagerID;
	if (usSessionManagerID >= m_usSessionManagerCount)
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::FreeSession, usSessionManagerID >= m_usSessionManagerCount\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return ;
	}
	m_parrMasterSessionManager[usSessionManagerID].ReMoveSession(u64SessionID);
	m_parrMasterSessionManager[usSessionManagerID].DecrementSessionManageredNum();
	if (pSession->GetSocket() != -1)
	{
		close(pSession->GetSocket()); 
	}
//	pSession->Reset();
//	FreeSessionToPool(pSession);
}

void LMasterSessionManager::Release()
{

}
// protected member
void LMasterSessionManager::FreeSessionToPool(LSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::FreeSessionToPool, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return ;
	}

	pthread_mutex_lock(&m_mutexForDirty);
	m_queueDirtySessionManager.push(pSession);	
	pthread_mutex_unlock(&m_mutexForDirty);
}


//	shan chu gu ding shi jian mei you tong xun de lian jie 
void LMasterSessionManager::KickOutIdleSession()
{ 
	time_t tNow = time(NULL);
	for (unsigned short usIndex = 0; usIndex < m_usSessionManagerCount; ++usIndex)
	{
		m_parrMasterSessionManager[usIndex].KickOutIdleSession(tNow);
		
	}
}

bool LMasterSessionManager::GetSessionIPAndPort(unsigned long long u64SessionID, char* pszBuf, unsigned short usbufLen)
{ 
	if (u64SessionID == 0ll)
	{
		return false;
	}
	unsigned long long u64SessionManagerID = u64SessionID >> 48;

	unsigned short usSessionManagerID = (unsigned short)u64SessionManagerID;
	if (usSessionManagerID > m_usSessionManagerCount)
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::GetSessionIPAndPort, usSessionManagerID > m_usSessionManagerCount\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return m_parrMasterSessionManager[usSessionManagerID].GetSessionIPAndPort(u64SessionID, pszBuf, usbufLen);
}

//	释放所有的Session和SessionManager，以及它们占用的资源
void LMasterSessionManager::ReleaseMasterSessionManagerResource()
{
	if (m_parrMasterSessionManager != NULL)
	{ 
		for (unsigned short usIndex = 0; usIndex < m_usSessionManagerCount; ++usIndex)
		{
			m_parrMasterSessionManager[usIndex].ReleaseSessionManagerResource();
		}
		delete[] m_parrMasterSessionManager;
	}

	while(!m_queueDirtySessionManager.empty())
	{
		LSession* pSession = m_queueDirtySessionManager.front();
		m_queueDirtySessionManager.pop();
		pSession->ReleaseSessionResource();
		delete pSession;
	}
}

void LMasterSessionManager::AddToWillReuseManager(LSession* pSession)
{
	LAutoReleaseMutex AutoReleaseMutex(&m_MutexProtectedForWillReuseManager);
	map<LSession*, LSession*>::iterator _ito = m_mapSessionWillReuseManager.find(pSession);
	if (_ito == m_mapSessionWillReuseManager.end())
	{
		m_mapSessionWillReuseManager[pSession] = pSession;
	}
}
//void LMasterSessionManager::ReMoveFromWillReuseManager(LSession* pSession)
//{
//	LAutoReleaseMutex AutoReleaseMutex(&m_MutexProtectedForWillReuseManager);
//	map<LSession*, LSession*>::iterator _ito = m_mapSessionWillReuseManager.find(pSession);
//	if (_ito != m_mapSessionWillReuseManager.end())
//	{
//		m_mapSessionWillReuseManager.erase(_ito);
//	}
//}
size_t LMasterSessionManager::GetWillReuseSessionCount()
{
	LAutoReleaseMutex AutoReleaseMutex(&m_MutexProtectedForWillReuseManager);
	return m_mapSessionWillReuseManager.size();
}
void LMasterSessionManager::MoveWillCloseSessionToSessionPool(LSession* pSession)
{
	LAutoReleaseMutex AutoReleaseMutex(&m_MutexProtectedForWillReuseManager);
	map<LSession*, LSession*>::iterator _ito = m_mapSessionWillReuseManager.find(pSession);
	if (_ito != m_mapSessionWillReuseManager.end())
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::MoveWillCloseSessionToSessionPool, SessionID:%llu, EpollThreaID:%d, RecvThreadID:%d, SendThreadID:%d\n",
				pSession->GetSessionID(), pSession->GetEpollThreadID(), pSession->GetRecvThreadID(), pSession->GetSendThreadID());
		g_ErrorWriter.WriteError(szError, strlen(szError));

		m_pNetWorkServices->GetEpollThreadManager().UnBindEpollThread(pSession);
		m_pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(pSession);
		m_pNetWorkServices->GetSendThreadManager().UnBindSendThread(pSession);



		FreeSessionToPool(pSession);
		m_mapSessionWillReuseManager.erase(_ito);
	}
	else
	{
		char szError[512];
		sprintf(szError, "LMasterSessionManager::MoveWillCloseSessionToSessionPool, Double Free\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
}

void LMasterSessionManager::PrintAllSessionInfos()
{
	pthread_mutex_lock(&m_mutexForDirty);
	int nDirtySessionCount = (int)m_queueDirtySessionManager.size();
	pthread_mutex_unlock(&m_mutexForDirty);

	int nAllSessionCount = 0;
	for (unsigned short usIndex = 0; usIndex < m_usSessionManagerCount; ++usIndex)
	{
		int nSessionCount = m_parrMasterSessionManager[usIndex].GetSessionManageredNum();
		printf("\t\t\tSessionManagerID:%hd, SessionCount:%d\n", usIndex, nSessionCount);
		nAllSessionCount += nSessionCount;
	}
	printf("SessionCount:%d, DirtyCount:%d\n", nAllSessionCount, nDirtySessionCount);
}

