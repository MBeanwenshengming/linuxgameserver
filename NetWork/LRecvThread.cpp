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

#include "LRecvThread.h"
#include "LNetWorkServices.h"
#include "LSessionManager.h"
#include "LSession.h"
#include "LErrorWriter.h"
#include "LSendThread.h"
#include <sys/prctl.h>

extern LErrorWriter g_ErrorWriter;

#ifdef __EPOLL_TEST_STATISTIC__
extern atomic_t g_nRecvPacketAllocCount;
#endif

// ben di huan cun de jie shou dao de shu ju bao de shu liang,yi ci ti jiao
extern size_t g_sRecvedPacketLocalKeepSize;
LRecvThread::LRecvThread()
{
	pthread_mutex_init(&m_MutexForWriteCircleBuf, NULL);
	m_pNetWorkServices = NULL; 
	m_nPacketRecved = 0;
	m_nThreadID = -1;
	m_tLastWriteBufDescTime = 0;

	m_pArrayppdForLocalPool 		= NULL;
	m_unppdForLocalPoolDescCount 	= 0;	
}

LRecvThread::~LRecvThread()
{
	pthread_mutex_destroy(&m_MutexForWriteCircleBuf);
}


//	unRecvWorkItemCount epollin事件环形队列的最大队列数，大于该数量，那么EPOLLIN事件无法放入接收线程处理
//ppdForLocalPool 本地接收数据包缓冲的大小，初始化本地缓存，减少锁冲突
//unRecvppdForLocalPoolCount 数组的数量
//unRecvLocalPacketPoolSize本地接收到的数据包缓冲的数量，达到一定数量提交，或者本地的线程完成一次循环，就提交
bool LRecvThread::Initialize(unsigned int unRecvWorkItemCount, t_Packet_Pool_Desc ppdForLocalPool[], unsigned int unRecvppdForLocalPoolCount, unsigned int unRecvLocalPacketPoolSize)
{
	if (m_pNetWorkServices == NULL)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::Initialize, m_pNetWorkServices == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (unRecvWorkItemCount == 0)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::Initialize, g_sRecvCircleBufItemCount == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_FixLenBuf.Initialize(sizeof(t_Recv_WorkItem), unRecvWorkItemCount))
	{
		char szError[512];
		sprintf(szError, "LRecvThread::Initialize, m_FixLenBuf.Initialize Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	m_PacketPoolManagerForRecv.SetReuqestPoolFromGlobalManager(m_pNetWorkServices->GetRecvGlobalPool());
	if (!m_PacketPoolManagerForRecv.Initialize(ppdForLocalPool, unRecvppdForLocalPoolCount))
	{
		char szError[512];
		sprintf(szError, "LRecvThread::Initialize, m_PacketPoolManagerForRecv.Initialize Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_unppdForLocalPoolDescCount = unRecvppdForLocalPoolCount;
	m_pArrayppdForLocalPool = new t_Packet_Pool_Desc[m_unppdForLocalPoolDescCount];
	if (m_pArrayppdForLocalPool == NULL)
	{
		return false;
	}
	memcpy(m_pArrayppdForLocalPool, ppdForLocalPool, sizeof(t_Packet_Pool_Desc) * m_unppdForLocalPoolDescCount);


	if (!m_FixBufRecvedPacket.Initialize(sizeof(t_Recv_Packet), unRecvLocalPacketPoolSize))
	{
		char szError[512];
		sprintf(szError, "LRecvThread::Initialize, m_FixBufRecvedPacket.Initialize Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_FixBufWillCloseSessionToProcess.Initialize(sizeof(LSession*), unRecvWorkItemCount))
	{
		char szError[512];
		sprintf(szError, "LRecvThread::Initialize, m_FixBufWillCloseSessionToProcess.Initialize Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	//	for Test
	char szFileName[256];
	sprintf(szFileName, "RecvThreadStatus%d.txt", m_nThreadID);
	if (!m_ErrorWriterForRecvThreadStatus.Initialize(szFileName))
	{
		return false;
	}
	return true;
}
bool LRecvThread::AddWorkItems(char* pbuf, size_t sWorkItemCount)
{
	if (pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::AddWorkItems, pbuf == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (sWorkItemCount == 0)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::AddWorkItems, sWorkItemCount == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	pthread_mutex_lock(&m_MutexForWriteCircleBuf);
	E_Circle_Error errorCode = m_FixLenBuf.AddItems(pbuf, sWorkItemCount); 
	pthread_mutex_unlock(&m_MutexForWriteCircleBuf);
	if (errorCode != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::AddWorkItems, errorCode != E_Circle_Buf_No_Error\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
bool LRecvThread::GetOneItem(t_Recv_WorkItem& workItem)
{
	E_Circle_Error errorCode = m_FixLenBuf.GetOneItem((char*)&workItem, sizeof(t_Recv_WorkItem));
	if (errorCode != E_Circle_Buf_No_Error && errorCode != E_Circle_Buf_Is_Empty)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::GetOneItem, errorCode != E_Circle_Buf_No_Error\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (errorCode == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}

int LRecvThread::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "RecvThread_%d", m_nThreadID);
	prctl(PR_SET_NAME, szThreadName);

	t_Recv_WorkItem tWorkItem;
	uint64_t u64SessionWillToClose = 0;
	while (true)
	{
		int nSessionWillToCloseProcessedCount = 0;
		LSession* pSession = NULL;
		while (GetOneWillCloseSession(&pSession))
		{
			nSessionWillToCloseProcessedCount++;

			if (pSession != NULL)
			{
				ProcessRecvDataErrorToCloseSession(pSession);
			}
			if (nSessionWillToCloseProcessedCount > 100)
			{
				break;
			}
		}
		tWorkItem.u64SessionID = 0;
		bool bPacketProcessed = false;
		int nPacketProcess = 0;
		int nPacketAllProcessedCount = 0;
		while (GetOneItem(tWorkItem))
		{
			//	接收数据
			LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();
			LSession* pSession = pMasterSessionManager->FindSession(tWorkItem.u64SessionID);
			if (pSession != NULL)
			{
				//	没有人要关闭连接，才处理，否则就不处理了，等待关闭
				if (pSession->GetCloseWorkSendedToCloseThread() == 0)
				{
					pSession->RecvData(this);
				}

				nPacketProcess++;
				nPacketAllProcessedCount++;
				bPacketProcessed = true;
				//printf("ThreadID:%d, Thread Recv:%d\n", m_nThreadID, m_nPacketRecved);
			}
			else
			{ 
				char szError[512];
				sprintf(szError, "LRecvThread::ThreadDoing,Session Not Find:%lld\n", tWorkItem.u64SessionID); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
			}
			if (nPacketProcess >= 30)
			{
				if (!CommitLocalPacketToGlobalPacketPool())
				{
					char szError[512];
					sprintf(szError, "LRecvThread::ThreadDoing, pos 1 !CommitLocalPacketToGlobalPacketPool\n"); 
					g_ErrorWriter.WriteError(szError, strlen(szError));
				}
				else
				{
					nPacketProcess = 0;
				}
			}
			if (nPacketAllProcessedCount >= 300)
			{
				break;
			}
		}
		if (nPacketAllProcessedCount == 0 && !bPacketProcessed)
		{
			//	sched_yield();
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		
		if (nPacketProcess > 0)
		{
			if (!CommitLocalPacketToGlobalPacketPool())
			{
				char szError[512];
				sprintf(szError, "LRecvThread::ThreadDoing, !CommitLocalPacketToGlobalPacketPool\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
			}
			else
			{
				nPacketProcess = 0;
			}
		}
		if (CheckForStop())
		{
			break;
		}
		time_t timeNow = time(NULL);
		if (timeNow - m_tLastWriteBufDescTime > 30)  // 30秒记录一次缓存状态，测试使用
		{
			PrintRecvThreadStatus();
			m_tLastWriteBufDescTime = timeNow;
		} 
	}
	return 0;
}

bool LRecvThread::OnStart()
{
	return true;
}
void LRecvThread::OnStop()
{
	
}

void LRecvThread::SetNetServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices = pNetWorkServices;
}
LPacketSingle* LRecvThread::GetOneFreePacket(unsigned short usPacketLen)
{
	LPacketSingle* pPacket = NULL;
	
	for (unsigned int unIndex = 0; unIndex < m_unppdForLocalPoolDescCount; ++unIndex)
	{
		unsigned short usCurrentSelecttedLen = m_pArrayppdForLocalPool[unIndex].usPacketLen;
		if (usCurrentSelecttedLen < usPacketLen)
		{
			continue;
		}
		pPacket	= m_PacketPoolManagerForRecv.RequestOnePacket(usCurrentSelecttedLen);

		if (pPacket == NULL)
		{
			if (m_pNetWorkServices->RequestFreePacketFromGlobalRecvPacketPool(&m_PacketPoolManagerForRecv, usCurrentSelecttedLen))
			{
				pPacket = m_PacketPoolManagerForRecv.RequestOnePacket(usCurrentSelecttedLen);	
				if (pPacket == NULL)
				{
					continue;
				} 
				else
				{
#ifdef __EPOLL_TEST_STATISTIC__
					atomic_inc(&g_nRecvPacketAllocCount);
#endif
					return pPacket;
				}
			}
			else
			{
				continue;
			}
		}
		else
		{
#ifdef __EPOLL_TEST_STATISTIC__
			atomic_inc(&g_nRecvPacketAllocCount);
#endif
			return pPacket;
		} 
	}
	if (pPacket != NULL)
	{
		pPacket->Reset();
	}
	return pPacket;
}
bool LRecvThread::AddPacketToLocal(uint64_t u64SessionID, LPacketSingle* pPacket)
{
	t_Recv_Packet RecvedPacket;
	RecvedPacket.u64SessionID = u64SessionID;
	RecvedPacket.pPacket = pPacket;
	if (m_FixBufRecvedPacket.AddItems((char*)&RecvedPacket, 1) != E_Circle_Buf_No_Error)
	{
		//	ti jiao ben di de shu shu ju dao zhu xian cheng guan li de bao dui lie
		if (m_pNetWorkServices->CommitPackets(&m_FixBufRecvedPacket) == false)
		{
			char szError[512];
			sprintf(szError, "LRecvThread::AddPacketToLocal, m_pNetWorkServices->CommitPackets Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		else
		{
			if (m_FixBufRecvedPacket.AddItems((char*)&RecvedPacket, 1) != E_Circle_Buf_No_Error)
			{
				char szError[512];
				sprintf(szError, "LRecvThread::AddPacketToLocal, m_FixBufRecvedPacket.AddItems Failed\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return false;
			}
		}
	}
	return true;
}

bool LRecvThread::CommitLocalPacketToGlobalPacketPool()
{
	if (m_pNetWorkServices == NULL)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::CommitLocalPacketToGlobalPacketPool, m_pNetWorkServices == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_pNetWorkServices->CommitPackets(&m_FixBufRecvedPacket))
	{
		char szError[512];
		sprintf(szError, "LRecvThread::CommitLocalPacketToGlobalPacketPool, CommitPackets Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}


void LRecvThread::PrintRecvThreadStatus()
{
	char szTempString[512];
	//	dang qian jie shou gong zuo dui lie chang du
	sprintf(szTempString, "ThreadID:%d, Recv Work Item Count:%d\n", m_nThreadID, m_FixLenBuf.GetCurrentExistCount()); 
	m_ErrorWriterForRecvThreadStatus.WriteError(szTempString, strlen(szTempString));
	// dang qian de ben di packet huan cun de zhuang kuang
	//   
	sprintf(szTempString, "Recv Thread Local Packet Pool Desc\n");
	m_ErrorWriterForRecvThreadStatus.WriteError(szTempString, strlen(szTempString));

	for (unsigned int unIndex = 0; unIndex < m_unppdForLocalPoolDescCount; ++unIndex)
	{
		unsigned int unMaxAddToPoolCount = 0;
		LFixLenCircleBuf* pFixCircleBuf = m_PacketPoolManagerForRecv.GetFixLenCircleBuf(m_pArrayppdForLocalPool[unIndex].usPacketLen, unMaxAddToPoolCount);

		sprintf(szTempString, "\tBufLen:%hd, Count:%d\n", m_pArrayppdForLocalPool[unIndex].usPacketLen, pFixCircleBuf->GetCurrentExistCount());
		m_ErrorWriterForRecvThreadStatus.WriteError(szTempString, strlen(szTempString));
	}
	
	// dang qian de mei you ti jiao de yi jie shou shu ju bao de shu liang
	int nUnCommitRecvedPacketCount = m_FixBufRecvedPacket.GetCurrentExistCount();
	sprintf(szTempString, "Current UnCommit Recved Packet Count:%d\n", nUnCommitRecvedPacketCount);	
	m_ErrorWriterForRecvThreadStatus.WriteError(szTempString, strlen(szTempString));
}

void LRecvThread::ReleaseRecvThreadResource()
{
	if (m_pArrayppdForLocalPool != NULL)
	{
		delete[] m_pArrayppdForLocalPool;
	}
	//	释放缓冲用的数据包
	m_PacketPoolManagerForRecv.ReleasePacketPoolManagerResource();

	if (m_FixBufRecvedPacket.GetCurrentExistCount() != 0)
	{
		while (1)
		{
			t_Recv_Packet tRecvPacket;
			memset(&tRecvPacket, 0, sizeof(tRecvPacket));
			E_Circle_Error error = m_FixBufRecvedPacket.GetOneItem((char*)&tRecvPacket, sizeof(tRecvPacket));

			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				delete tRecvPacket.pPacket;
				tRecvPacket.pPacket = NULL;
			}
		}
	}
}

//	加入一个需要处理的即将关闭的连接
void LRecvThread::AddWillCloseSession(LSession* pSession)
{
	E_Circle_Error eError = m_FixBufWillCloseSessionToProcess.AddItems((char*)&pSession, 1);
	if (eError != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::AddWillCloseSession, Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
}
//	取一个出来处理
bool LRecvThread::GetOneWillCloseSession(LSession** pSession)
{
	E_Circle_Error eError = m_FixBufWillCloseSessionToProcess.GetOneItem((char*)(&(*pSession)), sizeof(LSession*));
	if (eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty)
	{
		char szError[512];
		sprintf(szError, "LRecvThread::GetOneWillCloseSession, errorCode != E_Circle_Buf_No_Error\n");
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
void LRecvThread::ProcessRecvDataErrorToCloseSession(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}

	//	设置本接收线程不再对该连接进行操作
	pSession->SetRecvThreadStopProcess(1);

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
