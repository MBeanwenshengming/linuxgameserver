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

#include "LSendThreadManager.h"
#include "LSendThread.h"
#include "LSession.h"
#include "LNetWorkServices.h" 
#include "LErrorWriter.h"

extern LErrorWriter g_ErrorWriter;

LSendThreadManager::LSendThreadManager()
{
	m_parrSendThreadDesc 	= NULL;
	m_pNetWorkServices 		= NULL;
	m_unSendThreadCount		= 0;
}
LSendThreadManager::~LSendThreadManager()
{
}

//	unSendThreadCount 发送线程的数量
//	unSendWorkItemCount 发送队列的长度，这里描述了需要发送的连接和数据包
//	spd 发送数据包释放时，本地的缓存，
//	usspdCount 缓存描述的数量
//	unEpollOutEventMaxCount 发送线程接收的EPOLLOUT事件的最大数量
bool LSendThreadManager::Initialize(unsigned int unSendThreadCount, unsigned int unSendWorkItemCount, t_Packet_Pool_Desc spd[], unsigned short usspdCount, unsigned int unEpollOutEventMaxCount)
{
	if (m_pNetWorkServices == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::Initialize, m_pNetWorkServices == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (unSendThreadCount == 0)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::Initialize, g_unSendThreadCount == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_unSendThreadCount = unSendThreadCount;

	m_parrSendThreadDesc = new t_Send_Thread_Desc[m_unSendThreadCount];
	if (m_parrSendThreadDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::Initialize, m_parrSendThreadDesc == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		m_parrSendThreadDesc[unIndex].pSendThread = new LSendThread;
		if (m_parrSendThreadDesc[unIndex].pSendThread == NULL)
		{
			char szError[512];
			sprintf(szError, "LSendThreadManager::Initialize, m_parrSendThreadDesc[unIndex].pSendThread == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		m_parrSendThreadDesc[unIndex].pSendThread->m_nThreadID = unIndex;
		m_parrSendThreadDesc[unIndex].pSendThread->SetNetWorkServices(m_pNetWorkServices); 
		if (!m_parrSendThreadDesc[unIndex].pSendThread->Initialize(unSendWorkItemCount, spd,  usspdCount, unEpollOutEventMaxCount))
		{
			char szError[512];
			sprintf(szError, "LSendThreadManager::Initialize, m_parrSendThreadDesc[unIndex].pSendThread->Initialize Failed.\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));

			return false;
		} 
	}
	return true;
}
int LSendThreadManager::GetCurrentPacketFreePoolItemCount()
{
	int nCount = 0;
#ifdef __ADD_SEND_BUF_CHAIN__
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		nCount += m_parrSendThreadDesc[unIndex].pSendThread->GetCurrentFreePacketPoolItemCount();
	}
#endif
	return nCount;
}

int LSendThreadManager::SelectSendThread()
{
	int nSelectedThreadID = -1;
	int nMinSessionCount = 0x0fffffff;
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		int nCurrentRefCount = __sync_add_and_fetch(&m_parrSendThreadDesc[unIndex].nRefCount, 0);
		if (nCurrentRefCount < nMinSessionCount)
		{
			nMinSessionCount = nCurrentRefCount;
			nSelectedThreadID = unIndex;
		}
	}
	return nSelectedThreadID;
} 
bool LSendThreadManager::BindSendThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::BindSendThread, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	int nSelectedThreadID = SelectSendThread();
	if (nSelectedThreadID == -1)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::BindSendThread, nSelectedThreadID == -1\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	pSession->SetSendThreadID(nSelectedThreadID);

	__sync_add_and_fetch(&m_parrSendThreadDesc[nSelectedThreadID].nRefCount, 1);
	return true;
}
void LSendThreadManager::UnBindSendThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::UnBindSendThread, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return ;
	}
	int nSendThreadID = pSession->GetSendThreadID();
	if (nSendThreadID < 0 || nSendThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::UnBindSendThread, nSendThreadID < 0 || nSendThreadID >= g_unSendThreadCount\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return ;
	}
	pSession->SetSendThreadID(-1);

	__sync_sub_and_fetch(&(m_parrSendThreadDesc[nSendThreadID].nRefCount), 1);
}

void LSendThreadManager::SetNetWorkServices(LNetWorkServices* pNetWorkServices) 
{
	m_pNetWorkServices = pNetWorkServices;
}

	
LSendThread* LSendThreadManager::GetSendThread(unsigned int unThreadID)
{
	if (unThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::UnBindSendThread, unThreadID >= g_unSendThreadCount\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return NULL;
	}
	return m_parrSendThreadDesc[unThreadID].pSendThread;
}


bool LSendThreadManager::StartAllSendThread()
{
	if (m_parrSendThreadDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThreadManager::StartAllSendThread, m_parrSendThreadDesc == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (!m_parrSendThreadDesc[unIndex].pSendThread->Start())
		{
			char szError[512];
			sprintf(szError, "LSendThreadManager::StartAllSendThread, !m_parrSendThreadDesc[unIndex].pSendThread->Start Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
	}
	return true;
}

void LSendThreadManager::StopAllSendThread()
{
	if (m_parrSendThreadDesc == NULL)
	{
		return ;
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (m_parrSendThreadDesc[unIndex].pSendThread != NULL)
		{ 
			m_parrSendThreadDesc[unIndex].pSendThread->Stop();
		}
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (m_parrSendThreadDesc[unIndex].pSendThread != NULL)
		{
			pthread_t pID = m_parrSendThreadDesc[unIndex].pSendThread->GetThreadHandle();
			if (pID == 0)
			{
				continue;
			}
			int nJoinRes = pthread_join(pID, NULL);
			if (nJoinRes != 0)
			{ 
				char szError[512];
				sprintf(szError, "LSendThreadManager::StopAllSendThread, Join Failed, ErrorID:%d\n", errno); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
			}
		}
	}
}


void LSendThreadManager::PrintSendThreadRefStatus()
{
	printf("\tSendThreadManager, Send Thread RefCount\n");
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		int nRefCount = __sync_add_and_fetch(&m_parrSendThreadDesc[unIndex].nRefCount, 0);
		printf("\t\tSendThreadID:%d, RefCount:%d\n", unIndex, nRefCount);
	}
}

void LSendThreadManager::ReleaseSendThreadManagerResource()
{
	if (m_parrSendThreadDesc == NULL)
	{
		return ;
	}
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (m_parrSendThreadDesc[unIndex].pSendThread != NULL)
		{
			m_parrSendThreadDesc[unIndex].pSendThread->ReleaseSendThreadResource();
			delete m_parrSendThreadDesc[unIndex].pSendThread;
			m_parrSendThreadDesc[unIndex].pSendThread = NULL;
		}
	}
	delete[] m_parrSendThreadDesc;
	m_parrSendThreadDesc = NULL;
}
