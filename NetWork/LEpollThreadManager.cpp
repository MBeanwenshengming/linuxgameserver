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

#include "LEpollThreadManager.h"
#include "LRecvThread.h"
#include "LEpollThread.h"
#include "LSession.h"
#include "LNetWorkServices.h"
#include "LErrorWriter.h"


extern bool g_bEpollETEnabled;
extern LErrorWriter g_ErrorWriter;

LEpollThreadManager::LEpollThreadManager()
{
	m_parrEpollThreadManager 	= NULL;	
	m_pNetWorkServices			= NULL;
	m_unEpollThreadCount		= 0;
}
LEpollThreadManager::~LEpollThreadManager()
{
//	if (m_parrEpollThreadManager != NULL)
//	{
//		for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
//		{
//			m_parrEpollThreadManager[unIndex].pEpollThread->Stop();
//		} 
//		for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
//		{
//			pthread_join(m_parrEpollThreadManager[unIndex].pEpollThread->GetEpollHandle(), NULL);
//			delete m_parrEpollThreadManager[unIndex].pEpollThread;
//		}
//	}
//	delete[] m_parrEpollThreadManager; 
//	m_parrEpollThreadManager = NULL;
}	
//	unEpollThreadCount EPOLL线程数量
//	unRecvThreadCount 接收线程的数量
//	unRecvThreadWorkItemCount 可以放置的最大EPOLLIN事件数量,本地缓存的EPOLLIN事件数量，一次提交给接收线程
//	unSendThreadCount 发送线程的数量
//	unSendThreadWorkItemCount 每个发送线程本地缓存EPOLLOUT事件的数量，一次性提交给发送线程
//	unWaitClientSizePerEpoll 每个EPOLL上监听的套接字数量, 创建epoll时使用
bool LEpollThreadManager::Initialize(unsigned int unEpollThreadCount, unsigned int unRecvThreadCount, unsigned int unRecvThreadWorkItemCount, unsigned int unSendThreadCount, unsigned int unSendThreadWorkItemCount, unsigned int unWaitClientSizePerEpoll)
{
	if (m_pNetWorkServices == NULL)
	{
		char szError[512];
		sprintf(szError, "LEpollThreadManager::Initialize, m_pNetWorkServices == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (unEpollThreadCount == 0)
	{
		unEpollThreadCount = 1; 
	}
	m_unEpollThreadCount = unEpollThreadCount;
	m_parrEpollThreadManager = new t_Epoll_Thread_Desc[unEpollThreadCount];
	if (m_parrEpollThreadManager == NULL)
	{
		char szError[512];
		sprintf(szError, "LEpollThreadManager::Initialize, m_parrEpollThreadManager == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < unEpollThreadCount; ++unIndex)
	{
		m_parrEpollThreadManager[unIndex].pEpollThread = new LEpollThread;
		if (m_parrEpollThreadManager[unIndex].pEpollThread == NULL)
		{
			char szError[512];
			sprintf(szError, "LEpollThreadManager::Initialize, m_parrEpollThreadManager == NULL\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		m_parrEpollThreadManager[unIndex].pEpollThread->SetNetWorkServices(m_pNetWorkServices); 
		m_parrEpollThreadManager[unIndex].pEpollThread->m_nThreadID = unIndex;
		if (!m_parrEpollThreadManager[unIndex].pEpollThread->Initialize(unRecvThreadCount, unRecvThreadWorkItemCount, unSendThreadCount, unSendThreadWorkItemCount, unWaitClientSizePerEpoll))
		{
			char szError[512];
			sprintf(szError, "LEpollThreadManager::Initialize, m_parrEpollThreadManager[unIndex].pEpollThread->Initialize\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
	}

	return true;
}
bool LEpollThreadManager::StartAllEpollThread()
{
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		if (!m_parrEpollThreadManager[unIndex].pEpollThread->Start())
		{
			char szError[512];
			sprintf(szError, "LEpollThreadManager::StartAllEpollThread, pEpollThread->Start() Failed\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		} 
	}
	return true;
}
void LEpollThreadManager::StopAllEpollThread()
{
	if (m_parrEpollThreadManager != NULL)
	{
		for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
		{
			//	关闭线程
			if (m_parrEpollThreadManager[unIndex].pEpollThread != NULL)
			{
				m_parrEpollThreadManager[unIndex].pEpollThread->Stop();
			}
		} 
		for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
		{
			if (m_parrEpollThreadManager[unIndex].pEpollThread != NULL)
			{
				//	等待线程关闭
				pthread_t pID = m_parrEpollThreadManager[unIndex].pEpollThread->GetThreadHandle(); 
				if (pID != 0)
				{
					int nJoinResult = pthread_join(pID, NULL);
					if (nJoinResult != 0)
					{ 
						char szError[512];
						sprintf(szError, "LEpollThreadManager::StopAllEpollThread, join Failed.errorID:%d\n", errno);
						g_ErrorWriter.WriteError(szError, strlen(szError));
					}
				}
			}
		}
	}
}

bool LEpollThreadManager::PostEpollReadEvent(LSession* pSession)
{
	if (pSession == NULL)
	{
		return false;
	}
	int nThreadID = pSession->GetEpollThreadID();
	struct epoll_event epollEvent;
	memset(&epollEvent, 0, sizeof(epollEvent));

	t_Epoll_Bind_Param* pEpollBindParam = pSession->GetEpollBindParam();
	if (g_bEpollETEnabled)
	{ 
		epollEvent.events = EPOLLIN | EPOLLET;
		//epollEvent.data.ptr = pEpollBindParam;
		epollEvent.data.u64 = pEpollBindParam->u64SessionID;
	}
	else
	{
		epollEvent.events = EPOLLIN | EPOLLONESHOT;
		//epollEvent.data.ptr = pEpollBindParam;
		epollEvent.data.u64 = pEpollBindParam->u64SessionID;
	}
	int nBindEpollSuccess = epoll_ctl(m_parrEpollThreadManager[nThreadID].pEpollThread->GetEpollHandle(), EPOLL_CTL_ADD, pEpollBindParam->nSocket, &epollEvent);  
	if (nBindEpollSuccess == -1)
	{
		char szError[512];
		sprintf(szError, "LEpollThreadManager::PostEpollReadEvent, epoll_ctl Failed, Session:%lld\n", pSession->GetSessionID());
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}

bool LEpollThreadManager::BindEpollThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		return false;
	}
	int nThreadID = SelectEpollThread();
	if (nThreadID < 0 || nThreadID >= m_unEpollThreadCount)
	{
		char szError[512];
		sprintf(szError, "LEpollThreadManager::BindEpollThread, SelectEpollThread Failed, Session:%lld\n", pSession->GetSessionID());
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	uint64_t uSessionID = pSession->GetSessionID();
	int nSessionSocket = pSession->GetSocket(); 
	pSession->SetEpollBindParam(uSessionID, nSessionSocket);

	pSession->SetEpollThreadID(nThreadID); 
	__sync_add_and_fetch(&m_parrEpollThreadManager[nThreadID].nRefCount, 1);
	return true;
}
void LEpollThreadManager::UnBindEpollThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}
	//	struct epoll_event epollEvent;
	//	memset(&epollEvent, 0, sizeof(epollEvent));

	int nEpollThreadID = pSession->GetEpollThreadID();
	if (nEpollThreadID < 0 || nEpollThreadID >= m_unEpollThreadCount)
	{
		char szError[512];
		sprintf(szError, "LEpollThreadManager::UnBindEpollThread, unEpollThreadID >= g_unEpollThreadCount, Session:%lld\n", pSession->GetSessionID());
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return ;
	}
	//	t_Epoll_Bind_Param* pEpollBindParam = pSession->GetEpollBindParam();

	//if (g_bEpollETEnabled)
	//{ 
	//	epollEvent.events = EPOLLIN | EPOLLET;
	//	epollEvent.data.ptr = pEpollBindParam; 
	//}
	//else
	//{
	//	epollEvent.events = EPOLLIN | EPOLLONESHOT;
	//	epollEvent.data.ptr = pEpollBindParam; 
	//}
	__sync_sub_and_fetch(&m_parrEpollThreadManager[nEpollThreadID].nRefCount, 1);
	//int nBindEpollSuccess = epoll_ctl(m_parrEpollThreadManager[unEpollThreadID].pEpollThread->GetEpollHandle(), EPOLL_CTL_DEL, pEpollBindParam->nSocket, &epollEvent);  
	//if (nBindEpollSuccess == -1)
	//{
	//	return ;
	//} 
	pSession->SetEpollThreadID(-1);
}

int LEpollThreadManager::SelectEpollThread()
{
	int nMinEpollRefCount = 0x0fffffff;
	int nSelectted = -1;
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		int nRefCount = __sync_add_and_fetch(&m_parrEpollThreadManager[unIndex].nRefCount, 0);
		if (nRefCount < nMinEpollRefCount)
		{
			nMinEpollRefCount = nRefCount;
			nSelectted = unIndex;
		}
	}
	return nSelectted;
}
void LEpollThreadManager::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{ 
	m_pNetWorkServices	= pNetWorkServices;
}

LEpollThread* LEpollThreadManager::GetEpollThread(unsigned int unThreadID)
{
	if (unThreadID >= m_unEpollThreadCount)
	{
		char szError[512];
		sprintf(szError, "LEpollThreadManager::GetEpollThread, unThreadID >= g_unEpollThreadCount, ThreadID:%d\n", unThreadID); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return NULL;
	}
	return m_parrEpollThreadManager[unThreadID].pEpollThread;
}

#ifdef _DEBUG
void LEpollThreadManager::DumpEpollThreadManagerInfo()
{
	char szInfo[] = "LEpollThreadManager::Dump";
	printf("%s\n", szInfo);
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		char szInfoForEach[512];
		int nRefCount = atomic_read(&m_parrEpollThreadManager[unThreadID].atomicRefCount);
		sprintf(szInfoForEach, "EpollThreadID:%d, RefCount:%d", unIndex, nRefCount);
		printf("\t%s\n", szInfoForEach);
	}
}
#endif


void LEpollThreadManager::PrintEpollThreadStatus()
{
	char szInfo[] = "\tLEpollThreadManager Epoll Thread RefCount";
	printf("%s\n", szInfo);
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		char szInfoForEach[512];
		int nRefCount = __sync_add_and_fetch(&m_parrEpollThreadManager[unIndex].nRefCount, 0);

		sprintf(szInfoForEach, "EpollThreadID:%d, RefCount:%d", unIndex, nRefCount);
		printf("\t\t%s\n", szInfoForEach);
	}
}

//	释放EpollThread使用的所有资源
void LEpollThreadManager::ReleaseEpollThreadManagerResource()
{ 
	if (m_parrEpollThreadManager == NULL)
	{
		return ;
	}
	for (unsigned int unIndex = 0; unIndex < m_unEpollThreadCount; ++unIndex)
	{
		if (m_parrEpollThreadManager[unIndex].pEpollThread != NULL)
		{
			m_parrEpollThreadManager[unIndex].pEpollThread->ReleaseEpollThreadResource();
			delete m_parrEpollThreadManager[unIndex].pEpollThread;
			m_parrEpollThreadManager[unIndex].pEpollThread = NULL;
		}
	}
	delete[] m_parrEpollThreadManager;
}

