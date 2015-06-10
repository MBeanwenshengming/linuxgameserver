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

#include "LEpollThread.h" 
#include "LRecvThreadManager.h"
#include "LRecvThread.h"
#include "LSessionManager.h"
#include "LSession.h" 
#include "LErrorWriter.h"
#include <sys/prctl.h>

extern LErrorWriter g_ErrorWriter;

LEpollThread::LEpollThread(void)
{
	m_parrLocalWorkItemDesc 	= NULL;
	m_pEvents 					= NULL;
	m_nEpollThreadHandle 		= -1; 
	m_nMaxEvents 				= 0;
	m_pNetWorkServices 			= NULL;
	m_unRecvThreadCount 		= 0;
	m_unRecvThreadWorkItemCount = 0;
#ifdef __ADD_SEND_BUF_CHAIN__
	m_parrLocalSendWorkItemDesc = NULL;
	m_unSendThreadCount 		= 0;
	m_unSendThreadWorkItemCount = 0;
#endif
#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	m_parrLocalSendWorkItemDesc = NULL;
	m_unSendThreadCount 		= 0;
	m_unSendThreadWorkItemCount = 0;
#endif
	m_nThreadID = -1;
}

LEpollThread::~LEpollThread(void)
{
}

int LEpollThread::ThreadDoing(void* pParam)
{
#ifndef __ADD_SEND_BUF_CHAIN__ 
//	while (1)
//	{
//		int nLocalAllSum = 0;
//
//		if (CheckForStop())
//		{
//			break;
//		}
//		int nEpollEventCount = epoll_wait(m_nEpollThreadHandle, m_pEvents, m_nMaxEvents, 1);
//		if (nEpollEventCount < 0)
//		{
//			if (errno == 4)
//			{
//				continue;
//			}
//
//			char szError[512];
//			sprintf(szError, "LEpollThread::ThreadDoing, epoll_wait errorcode:%d\n", errno);
//			g_ErrorWriter.WriteError(szError, strlen(szError));
//			return -1;
//		}
//
//		for (int nIndex = 0; nIndex < nEpollEventCount; ++nIndex)
//		{
//			t_Epoll_Bind_Param* pEpollBindParam = (t_Epoll_Bind_Param*)m_pEvents[nIndex].data.ptr;
//			uint64_t u64SessionID = pEpollBindParam->u64SessionID;
//			LMasterSessionManager* pSessionManager = &m_pNetWorkServices->GetSessionManager();
//			LSession* pSession = pSessionManager->FindSession(u64SessionID);
//			if (pSession == NULL)
//			{
//				//	should write error
//				char szError[512];
//				sprintf(szError, "LEpollThread::ThreadDoing, Can not Find Session:%lld\n", u64SessionID);
//				g_ErrorWriter.WriteError(szError, strlen(szError));
//				continue;
//			}
//			int unRecvThreadID = pSession->GetRecvThreadID();
//			if (unRecvThreadID == -1)
//			{
//				char szError[512];
//				sprintf(szError, "LEpollThread::ThreadDoing, RecvThreadID Error, SessionID:%lld\n", u64SessionID);
//				g_ErrorWriter.WriteError(szError, strlen(szError));
//				continue;
//			}
//			PushRecvWorkItem(unRecvThreadID, u64SessionID);
//			nLocalAllSum++;
//			if (nLocalAllSum >= m_unRecvThreadWorkItemCount)
//			{
//				if (!CommitLocalWorkItem())
//				{
//					char szError[512];
//					sprintf(szError, "LEpollThread::ThreadDoing, CommitLocalWorkItem Error, SessionID:%lld\n", u64SessionID);
//					g_ErrorWriter.WriteError(szError, strlen(szError));
//					//	write error
//				}
//				nLocalAllSum = 0;
//			}
//		}
//		if (!CommitLocalWorkItem())
//		{
//			//	error write
//			//
//			char szError[512];
//			sprintf(szError, "LEpollThread::ThreadDoing, CommitLocalWorkItem Error\n");
//			g_ErrorWriter.WriteError(szError, strlen(szError));
//		}
//	}
//	return 0;
#else 
	while (1)
	{
		if (CheckForStop())
		{
			break;
		}

		int nLocalAllSum = 0;
		int nLocalSendAllSum = 0;
		int nEpollEventCount = epoll_wait(m_nEpollThreadHandle, m_pEvents, m_nMaxEvents, 1);
		if (nEpollEventCount < 0)
		{
			if (errno == 4)
			{
				continue;
			}
			char szError[512];
			sprintf(szError, "LEpollThread::ThreadDoing, epoll_wait errorcode:%d\n", errno);
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return -1;
		}
		else if (nEpollEventCount == 0)
		{
			continue;
		}
		else
		{
		}
		
		for (int nIndex = 0; nIndex < nEpollEventCount; ++nIndex)
		{
			t_Epoll_Bind_Param* pEpollBindParam = (t_Epoll_Bind_Param*)m_pEvents[nIndex].data.ptr;
			uint64_t u64SessionID = pEpollBindParam->u64SessionID;
			LMasterSessionManager* pSessionManager = &(m_pNetWorkServices->GetSessionManager());
			LSession* pSession = pSessionManager->FindSession(u64SessionID);
			if (pSession == NULL)
			{
				//	should write error
				char szError[512];
				sprintf(szError, "LEpollThread::ThreadDoing, Can not Find Session2:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				continue;
			}
			int unRecvThreadID = pSession->GetRecvThreadID();
			if (unRecvThreadID == -1)
			{
				char szError[512];
				sprintf(szError, "LEpollThread::ThreadDoing, RecvThreadID Error, SessionID:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				//continue;
			} 
			else
			{
				if (m_pEvents[nIndex].events & EPOLLIN)
				{
					PushRecvWorkItem(unRecvThreadID, u64SessionID);
					nLocalAllSum++;
					if (nLocalAllSum >= m_unRecvThreadWorkItemCount)
					{
						if (!CommitLocalWorkItem())
						{
							char szError[512];
							sprintf(szError, "LEpollThread::ThreadDoing, CommitLocalWorkItem Error, SessionID:%lld\n", u64SessionID);
							g_ErrorWriter.WriteError(szError, strlen(szError));
							//	write error
						}
						nLocalAllSum = 0;
					}
				}
			}

			int nSendThreadID = pSession->GetSendThreadID();
			if (nSendThreadID == -1)
			{

				char szError[512];
				sprintf(szError, "LEpollThread::ThreadDoing, SendThreadID Error, SessionID:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				//continue;
			}
			else
			{
				if (m_pEvents[nIndex].events & EPOLLOUT)
				{
					PushSendWorkItem(nSendThreadID, u64SessionID);
					nLocalSendAllSum++;
					if (nLocalSendAllSum >= m_unSendThreadWorkItemCount)
					{
						if (!CommitAllSendWorkItem())
						{
							char szError[512];
							sprintf(szError, "LEpollThread::ThreadDoing, CommitAllSendWorkItem Error, SessionID:%lld\n", u64SessionID);
							g_ErrorWriter.WriteError(szError, strlen(szError));
							//	write error
						}
						nLocalSendAllSum = 0;
					}
				}
			}
		}
		if (!CommitLocalWorkItem())
		{
			char szError[512];
			sprintf(szError, "LEpollThread::ThreadDoing, CommitLocalWorkItem Error\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
		}
		if (!CommitAllSendWorkItem())
		{
			char szError[512];
			sprintf(szError, "LEpollThread::ThreadDoing, CommitAllSendWorkItem Error\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
		}
	}
	return 0;
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	char szThreadName[128];
	sprintf(szThreadName, "EPollThread_%d", m_nThreadID);
	prctl(PR_SET_NAME, szThreadName);

	while (1)
	{
		if (CheckForStop())
		{
			break;
		}

		int nWillCloseSessionProcessed = 0;
		LSession* pSession = NULL;
		while (GetOneWillCloseSessionInEpollThread(&pSession))
		{
			nWillCloseSessionProcessed++;
			if (pSession != NULL)
			{
				ProcessCloseSessionInEpollThread(pSession);
			}
			if (nWillCloseSessionProcessed > 100)
			{
				break;
			}
		}

		int nLocalAllSum = 0;
		int nLocalSendAllSum = 0;
		int nEpollEventCount = epoll_wait(m_nEpollThreadHandle, m_pEvents, m_nMaxEvents, 1);
		if (nEpollEventCount < 0)
		{
			if (errno == 4)
			{
				continue;
			}
			char szError[512];
			sprintf(szError, "LEpollThread::ThreadDoing, epoll_wait errorcode:%d\n", errno);
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return -1;
		}
		else if (nEpollEventCount == 0)
		{
			continue;
		}
		else
		{
		}

		for (int nIndex = 0; nIndex < nEpollEventCount; ++nIndex)
		{
//			t_Epoll_Bind_Param* pEpollBindParam = (t_Epoll_Bind_Param*)m_pEvents[nIndex].data.ptr;
//			uint64_t u64SessionID = pEpollBindParam->u64SessionID;
			uint64_t u64SessionID = m_pEvents[nIndex].data.u64;
			LMasterSessionManager* pSessionManager = &(m_pNetWorkServices->GetSessionManager());
			LSession* pSession = pSessionManager->FindSession(u64SessionID);
			if (pSession == NULL)
			{
				//	should write error
				char szError[512];
				sprintf(szError, "LEpollThread::ThreadDoing, Can not Find Session2:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				continue;
			}
			//	已经在关闭了，就不处理了
			if (pSession->GetCloseWorkSendedToCloseThread() != 0)
			{
				continue;
			}

			int unRecvThreadID = pSession->GetRecvThreadID();
			if (unRecvThreadID == -1)
			{
				char szError[512];
				sprintf(szError, "LEpollThread::ThreadDoing, RecvThreadID Error, SessionID:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				//continue;
			}
			else
			{
				if (m_pEvents[nIndex].events & EPOLLIN)
				{
					PushRecvWorkItem(unRecvThreadID, u64SessionID);
					nLocalAllSum++;
					if (nLocalAllSum >= m_unRecvThreadWorkItemCount)
					{
						if (!CommitLocalWorkItem())
						{
							char szError[512];
							sprintf(szError, "LEpollThread::ThreadDoing, CommitLocalWorkItem Error, SessionID:%lld\n", u64SessionID);
							g_ErrorWriter.WriteError(szError, strlen(szError));
							//	write error
						}
						nLocalAllSum = 0;
					}
				}
			}

			int nSendThreadID = pSession->GetSendThreadID();
			if (nSendThreadID == -1)
			{

				char szError[512];
				sprintf(szError, "LEpollThread::ThreadDoing, SendThreadID Error, SessionID:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				//continue;
			}
			else
			{
				if (m_pEvents[nIndex].events & EPOLLOUT)
				{
					PushSendWorkItem(nSendThreadID, u64SessionID);
					nLocalSendAllSum++;
					if (nLocalSendAllSum >= m_unSendThreadWorkItemCount)
					{
						if (!CommitAllSendWorkItem())
						{
							char szError[512];
							sprintf(szError, "LEpollThread::ThreadDoing, CommitAllSendWorkItem Error, SessionID:%lld\n", u64SessionID);
							g_ErrorWriter.WriteError(szError, strlen(szError));
							//	write error
						}
						nLocalSendAllSum = 0;
					}
				}
			}
		}
		if (!CommitLocalWorkItem())
		{
			char szError[512];
			sprintf(szError, "LEpollThread::ThreadDoing, CommitLocalWorkItem Error\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
		}
		if (!CommitAllSendWorkItem())
		{
			char szError[512];
			sprintf(szError, "LEpollThread::ThreadDoing, CommitAllSendWorkItem Error\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
		}
	}
#endif
	return 0;
}

void LEpollThread::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices = pNetWorkServices;
}

//	unRecvThreadCount 接收线程的数量
//	unRecvThreadWorkItemCount 可以放置的最大EPOLLIN事件数量,本地缓存的EPOLLIN事件数量，一次提交给接收线程
//	unSendThreadCount 发送线程的数量
//	unSendThreadWorkItemCount 每个发送线程本地缓存EPOLLOUT事件的数量，一次性提交给发送线程
//	unWaitClientSizePerEpoll 每个EPOLL上监听的套接字数量, 创建epoll时使用
bool LEpollThread::Initialize(unsigned int unRecvThreadCount, unsigned int unRecvThreadWorkItemCount, unsigned int unSendThreadCount, unsigned int unSendThreadWorkItemCount, unsigned int unWaitClientSizePerEpoll)
{
	if (unRecvThreadWorkItemCount == 0)
	{
		unRecvThreadWorkItemCount = 200;
	} 
	m_unRecvThreadWorkItemCount = unRecvThreadWorkItemCount;

	if (unRecvThreadCount == 0)
	{
		return false;
	}
	m_unRecvThreadCount = unRecvThreadCount;

	m_parrLocalWorkItemDesc = new t_Local_Work_Item_Desc[unRecvThreadCount];
	if (m_parrLocalWorkItemDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, m_parrLocalWorkItemDesc == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	for (unsigned int unIndex = 0; unIndex < unRecvThreadCount; ++unIndex)
	{
		m_parrLocalWorkItemDesc[unIndex].unWorkItemCount = 0;
		m_parrLocalWorkItemDesc[unIndex].parrWorkItem = new t_Recv_WorkItem[unRecvThreadWorkItemCount];
		if (m_parrLocalWorkItemDesc[unIndex].parrWorkItem == NULL)
		{
			char szError[512];
			sprintf(szError, "LEpollThread::Initialize, m_parrLocalWorkItemDesc[unIndex].parrWorkItem == NULL\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
	}
	if (unWaitClientSizePerEpoll == 0)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, g_unWaitClientPerEpoll == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_nMaxEvents = unWaitClientSizePerEpoll + 100;
	m_pEvents = new epoll_event[m_nMaxEvents];
	if (m_pEvents == NULL)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, m_pEvents == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}


#ifdef  __ADD_SEND_BUF_CHAIN__
	if (unSendThreadCount == 0)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, unSendThreadCount == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_unSendThreadCount = unSendThreadCount;

	if (unSendThreadWorkItemCount == 0)
	{
		unSendThreadWorkItemCount = 200;
	}
	m_unSendThreadWorkItemCount = unSendThreadWorkItemCount;

	m_parrLocalSendWorkItemDesc = new t_Local_Work_Item_For_Send_Desc[unSendThreadCount];
	if (m_parrLocalSendWorkItemDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, m_parrLocalSendWorkItemDesc == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < unSendThreadCount; ++unIndex)
	{
		m_parrLocalSendWorkItemDesc[unIndex].unItemCount = 0;
		m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem = new t_Send_Epoll_Out_Event[unSendThreadWorkItemCount];
		if (m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem == NULL)
		{
			char szError[512];
			sprintf(szError, "LEpollThread::Initialize, m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem == NULL\n");
			g_ErrorWriter.WriteError(szError, strlen(szError)); 
			return false;
		}
	}
#endif
#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	if (unSendThreadCount == 0)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, unSendThreadCount == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_unSendThreadCount = unSendThreadCount;

	if (unSendThreadWorkItemCount == 0)
	{
		unSendThreadWorkItemCount = 200;
	}
	m_unSendThreadWorkItemCount = unSendThreadWorkItemCount;

	m_parrLocalSendWorkItemDesc = new t_Local_Work_Item_For_Send_Desc[unSendThreadCount];
	if (m_parrLocalSendWorkItemDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, m_parrLocalSendWorkItemDesc == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < unSendThreadCount; ++unIndex)
	{
		m_parrLocalSendWorkItemDesc[unIndex].unItemCount = 0;
		m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem = new t_Send_Epoll_Out_Event[unSendThreadWorkItemCount];
		if (m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem == NULL)
		{
			char szError[512];
			sprintf(szError, "LEpollThread::Initialize, m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem == NULL\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
	}

	if (!m_FixBufWillCloseSessionToProcessInEpollThread.Initialize(sizeof(LSession*), unRecvThreadWorkItemCount))
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, m_FixBufWillCloseSessionToProcessInEpollThread.Initialize Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#endif
	m_nEpollThreadHandle = epoll_create(unWaitClientSizePerEpoll);
	if (m_nEpollThreadHandle == -1)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::Initialize, m_nEpollThreadHandle == -1\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
bool LEpollThread::CommitLocalWorkItem()
{
	bool bAllSucessCommit = true;
	for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
	{
		if (m_parrLocalWorkItemDesc[unIndex].unWorkItemCount != 0)
		{
			if (!CommitSingleLocalWorkItem(unIndex))
			{
				char szError[512];
				sprintf(szError, "LEpollThread::CommitLocalWorkItem, Error, RecvThreadId:%d\n", unIndex);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				//	write error
				bAllSucessCommit = false;
				continue;
			}
		}
	}
	return bAllSucessCommit;
}

bool LEpollThread::CommitSingleLocalWorkItem(unsigned int unRecvThreadID)
{
	if (m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount == 0)
	{
		return true;
	}
	LRecvThreadManager* pRecvThreadManager = &m_pNetWorkServices->GetRecvThreadManager();
	LRecvThread* pRecvThread = pRecvThreadManager->GetRecvThread(unRecvThreadID);
	if (pRecvThread == NULL)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::CommitSingleLocalWorkItem, Can not Find Recv Thread, RecvThreadId:%d\n", unRecvThreadID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!pRecvThread->AddWorkItems((char*)m_parrLocalWorkItemDesc[unRecvThreadID].parrWorkItem, m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount))
	{
		char szError[512];
		sprintf(szError, "LEpollThread::CommitSingleLocalWorkItem, AddWorkItems Failed, RecvThreadId:%d\n", unRecvThreadID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount = 0;
	return true;
}

bool LEpollThread::PushRecvWorkItem(unsigned int unRecvThreadID, uint64_t u64SessionID)
{
	if (unRecvThreadID >= m_unRecvThreadCount)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::PushRecvWorkItem Failed, unRecvThreadID >= g_unRecvThreadCount, RecvThreadId:%d, SessionID:%lld\n", unRecvThreadID, u64SessionID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount + 1 == 	m_unRecvThreadWorkItemCount)
	{
		m_parrLocalWorkItemDesc[unRecvThreadID].parrWorkItem[m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount].u64SessionID = u64SessionID;
		m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount++;
		if (!CommitSingleLocalWorkItem(unRecvThreadID))
		{
			//	should write Error
			//
			char szError[512];
			sprintf(szError, "LEpollThread::PushRecvWorkItem, CommitSingleLocalWorkItem Failed, RecvThreadId:%d, SessionID:%lld\n", unRecvThreadID, u64SessionID);
			g_ErrorWriter.WriteError(szError, strlen(szError));

			m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount = 0;
		}
	}
	else
	{
		m_parrLocalWorkItemDesc[unRecvThreadID].parrWorkItem[m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount].u64SessionID = u64SessionID;
		m_parrLocalWorkItemDesc[unRecvThreadID].unWorkItemCount++; 
	}
	return true;
}

int LEpollThread::GetEpollHandle()
{
	return m_nEpollThreadHandle;
}

#ifdef _DEBUG
void LEpollThread::DumpEpollTheadInfo()
{
	char szInfo[] = "LEpollThread::DumpInfo";
	printf("%s\n", szInfo);
	for (unsigned int unIndex = 0; unIndex < g_unRecvThreadCount; ++unIndex)
	{
		char szEachInfo[512];
		sprintf(szEachInfo, "RecvThreadID:%d, EpollEventRecved:%d", unIndex, m_parrLocalWorkItemDesc[unIndex].unWorkItemCount);
		printf("\t%s\n", szEachInfo);
	}
} 
#endif

#ifdef  __ADD_SEND_BUF_CHAIN__
bool LEpollThread::PushSendWorkItem(unsigned int unSendThreadID, uint64_t u64SessionID)
{
	if (unSendThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::PushSendWorkItem, unSendThreadID >= m_unSendThreadCount, SendThreadId:%d, SessionID:%lld\n", unSendThreadID, u64SessionID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount + 1 == m_unSendThreadWorkItemCount)
	{
		m_parrLocalSendWorkItemDesc[unSendThreadID].parrWorkItem[m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount].u64SessionID = u64SessionID;
		m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount++;
		if (!CommitSingleLocalSendItem(unSendThreadID))
		{
			char szError[512];
			sprintf(szError, "LEpollThread::PushSendWorkItem, CommitSingleLocalSendItem Failed, SendThreadId:%d, SessionID:%lld\n", unSendThreadID, u64SessionID);
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		return true;
	}
	else
	{ 
		m_parrLocalSendWorkItemDesc[unSendThreadID].parrWorkItem[m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount].u64SessionID = u64SessionID;
		m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount++;
		return true;
	}
}
bool LEpollThread::CommitSingleLocalSendItem(unsigned int unSendThreadID)
{
	if (unSendThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::CommitSingleLocalSendItem, unSendThreadID >= m_unSendThreadCount,%d\n", unSendThreadID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	LSendThread* pSendThread = m_pNetWorkServices->GetSendThreadManager().GetSendThread(unSendThreadID);
	if (pSendThread == NULL)
	{ 
		char szError[512];
		sprintf(szError, "LEpollThread::CommitSingleLocalSendItem, pSendThread == NULL,%d\n", unSendThreadID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!pSendThread->PushEpollOutEvent((char*)(m_parrLocalSendWorkItemDesc[unSendThreadID].parrWorkItem), m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount))
	{
		char szError[512];
		sprintf(szError, "LEpollThread::CommitSingleLocalSendItem, PushEpollOutEvent Failed,%d\n", unSendThreadID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount = 0;
	return true;
}
bool LEpollThread::CommitAllSendWorkItem()
{
	for (unsigned int i = 0; i < m_unSendThreadCount; ++i)
	{
		if (!CommitSingleLocalSendItem(i))
		{ 
			char szError[512];
			sprintf(szError, "LEpollThread::CommitAllSendWorkItem, CommitSingleLocalSendItem Failed,%d\n", i);
			g_ErrorWriter.WriteError(szError, strlen(szError));
			continue;
		}
	}
	return true;
}
#endif

#ifdef  __USE_SESSION_BUF_TO_SEND_DATA__
bool LEpollThread::PushSendWorkItem(unsigned int unSendThreadID, uint64_t u64SessionID)
{
	if (unSendThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::PushSendWorkItem, unSendThreadID >= m_unSendThreadCount, SendThreadId:%d, SessionID:%lld\n", unSendThreadID, u64SessionID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount + 1 == m_unSendThreadWorkItemCount)
	{
		m_parrLocalSendWorkItemDesc[unSendThreadID].parrWorkItem[m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount].u64SessionID = u64SessionID;
		m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount++;
		if (!CommitSingleLocalSendItem(unSendThreadID))
		{
			char szError[512];
			sprintf(szError, "LEpollThread::PushSendWorkItem, CommitSingleLocalSendItem Failed, SendThreadId:%d, SessionID:%lld\n", unSendThreadID, u64SessionID);
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		return true;
	}
	else
	{
		m_parrLocalSendWorkItemDesc[unSendThreadID].parrWorkItem[m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount].u64SessionID = u64SessionID;
		m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount++;
		return true;
	}
}
bool LEpollThread::CommitSingleLocalSendItem(unsigned int unSendThreadID)
{
	if (unSendThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::CommitSingleLocalSendItem, unSendThreadID >= m_unSendThreadCount,%d\n", unSendThreadID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	LSendThread* pSendThread = m_pNetWorkServices->GetSendThreadManager().GetSendThread(unSendThreadID);
	if (pSendThread == NULL)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::CommitSingleLocalSendItem, pSendThread == NULL,%d\n", unSendThreadID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!pSendThread->PushEpollOutEvent((char*)(m_parrLocalSendWorkItemDesc[unSendThreadID].parrWorkItem), m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount))
	{
		char szError[512];
		sprintf(szError, "LEpollThread::CommitSingleLocalSendItem, PushEpollOutEvent Failed,%d\n", unSendThreadID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_parrLocalSendWorkItemDesc[unSendThreadID].unItemCount = 0;
	return true;
}
bool LEpollThread::CommitAllSendWorkItem()
{
	for (unsigned int i = 0; i < m_unSendThreadCount; ++i)
	{
		if (!CommitSingleLocalSendItem(i))
		{
			char szError[512];
			sprintf(szError, "LEpollThread::CommitAllSendWorkItem, CommitSingleLocalSendItem Failed,%d\n", i);
			g_ErrorWriter.WriteError(szError, strlen(szError));
			continue;
		}
	}
	return true;
}
//	加入一个需要处理的即将关闭的连接
void LEpollThread::AddWillCloseSessionInEpollThread(LSession* pSession)
{
	E_Circle_Error eError = m_FixBufWillCloseSessionToProcessInEpollThread.AddItems((char*)&pSession, 1);
	if (eError != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::AddWillCloseSessionInEpollThread, Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
}
//	取一个出来处理
bool LEpollThread::GetOneWillCloseSessionInEpollThread(LSession** pSession)
{
	E_Circle_Error eError = m_FixBufWillCloseSessionToProcessInEpollThread.GetOneItem((char*)(&(*pSession)), sizeof(LSession*));
	if (eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty)
	{
		char szError[512];
		sprintf(szError, "LEpollThread::GetOneWillCloseSessionInEpollThread, errorCode != E_Circle_Buf_No_Error\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (eError == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}

//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
void LEpollThread::ProcessCloseSessionInEpollThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}

	//	设置本接收线程不再对该连接进行操作
	pSession->SetEpollThreadStopProcess(1);

	//	检查是否需要关闭连接
	int nSendThreadStopProcessInfo 		= 0;
	int nRecvThreadStopProcessInfo		= 0;
	int nMainLogicThreadStopProcessInfo	= 0;
	int nEpollThreadStopProcessInfo		= 0;
	pSession->GetStopProcessInfos(nSendThreadStopProcessInfo, nRecvThreadStopProcessInfo, nMainLogicThreadStopProcessInfo, nEpollThreadStopProcessInfo);
	if (nSendThreadStopProcessInfo == 1 && nRecvThreadStopProcessInfo == 1
			&& nMainLogicThreadStopProcessInfo == 1 && nEpollThreadStopProcessInfo == 1)
	{
		//	把该session放入可重用的Session内存池中
		m_pNetWorkServices->GetSessionManager().MoveWillCloseSessionToSessionPool(pSession);
	}
}
#endif

void LEpollThread::ReleaseEpollThreadResource()
{ 
	if (m_nEpollThreadHandle != -1)
	{
		close(m_nEpollThreadHandle);
	}
	if (m_pEvents != NULL)
	{
		delete[] m_pEvents;
	}
	if (m_parrLocalWorkItemDesc != NULL)
	{
		for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
		{
			if (m_parrLocalWorkItemDesc[unIndex].parrWorkItem != NULL)
			{
				delete[] m_parrLocalWorkItemDesc[unIndex].parrWorkItem;
			}
		}
		delete[] m_parrLocalWorkItemDesc;
	}
#ifdef  __ADD_SEND_BUF_CHAIN__
	if (m_parrLocalSendWorkItemDesc != NULL)
	{ 
		for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
		{
			if (m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem != NULL)
			{
				delete[] m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem;
			}
		}
		delete[] m_parrLocalSendWorkItemDesc;
	}
#endif
#ifdef  __USE_SESSION_BUF_TO_SEND_DATA__
	if (m_parrLocalSendWorkItemDesc != NULL)
	{
		for (unsigned int unIndex = 0; unIndex < m_unRecvThreadCount; ++unIndex)
		{
			if (m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem != NULL)
			{
				delete[] m_parrLocalSendWorkItemDesc[unIndex].parrWorkItem;
			}
		}
		delete[] m_parrLocalSendWorkItemDesc;
	}
#endif
}

