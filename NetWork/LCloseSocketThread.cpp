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

#include "LCloseSocketThread.h"
#include "LNetWorkServices.h"
#include "LRecvThreadManager.h"
#include "LSendThreadManager.h"
#include "LEpollThreadManager.h"
#include "LSessionManager.h"
#include "LSession.h"
#include "LErrorWriter.h"
#include "IncludeHeader.h"
#include "LEpollThread.h"
#include "LSendThread.h"
#include "LRecvThread.h"
#include <sys/prctl.h>

extern size_t g_sCloseSessionMaxCount; 
extern LErrorWriter g_ErrorWriter;

#ifdef __EPOLL_TEST_STATISTIC__
extern volatile int g_nSendPacketCloseThreadFreeCount;
#endif

LCloseSocketThread::LCloseSocketThread()
{
	m_unCloseWorkItemCount = 0;
	pthread_mutex_init(&m_MutexToProtectedWriteCloseInfo, NULL);
	m_pNetWorkServices = NULL;

#ifdef __ADD_SEND_BUF_CHAIN__
	m_nFreePacketCount	= 0;
#endif
}

LCloseSocketThread::~LCloseSocketThread()
{	
	pthread_mutex_destroy(&m_MutexToProtectedWriteCloseInfo);
}


//	unCloseWorkItemCount 最大可以提交的关闭事件数量
//	ppdForLocalPool 连接关闭时，释放该连接下没有发送的发送数据包到本地缓存池，达到一定数量时，提交到全局缓冲池
//	unppdForLocalPoolCount 描述信息数组的长度
bool LCloseSocketThread::Initialize(unsigned int unCloseWorkItemCount, t_Packet_Pool_Desc ppdForLocalPool[], unsigned int unppdForLocalPoolCount)
{
	if (m_pNetWorkServices == NULL)
	{
		char szError[512];
		sprintf(szError, "LCloseSocketThread::Initialize, m_pNetWorkServices == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (unCloseWorkItemCount == 0)
	{
		return false;
	}
	m_unCloseWorkItemCount = unCloseWorkItemCount;
	if (!m_BufSessionNeedToClose.Initialize(sizeof(t_Client_Need_To_Close), m_unCloseWorkItemCount))
	{
		char szError[512];
		sprintf(szError, "LCloseSocketThread::Initialize, m_BufSessionNeedToClose.Initialize Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_FixBufToCommitSessionDisconnect.Initialize(sizeof(t_Recv_Packet), 10))
	{
		char szError[512];
		sprintf(szError, "LCloseSocketThread::Initialize, m_FixBufToCommitSessionDisconnect.Initialize Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	} 
#ifdef __ADD_SEND_BUF_CHAIN__
	if (!m_LocalBroadCastPacketForFree.Initialize(ppdForLocalPool, unppdForLocalPoolCount))
	{ 
		char szError[512];
		sprintf(szError, "LCloseSocketThread::Initialize, m_LocalBroadCastPacketForFree.Initialize Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_LocalBroadCastPacketForFree.SetReuqestPoolFromGlobalManager(m_pNetWorkServices->GetSendGlobalPool());
#endif
	return true;
}
bool LCloseSocketThread::AppendToClose(t_Client_Need_To_Close ClientToClose)
{
	E_Circle_Error nError = E_Circle_Buf_No_Error;
	pthread_mutex_lock(&m_MutexToProtectedWriteCloseInfo);
	nError = m_BufSessionNeedToClose.AddItems((char*)&ClientToClose, 1);
	pthread_mutex_unlock(&m_MutexToProtectedWriteCloseInfo);
	if (nError != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LCloseSocketThread::AppendToClose, AddItems Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}

int LCloseSocketThread::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "CloseThread");
	prctl(PR_SET_NAME, szThreadName);

	t_Client_Need_To_Close OneWorkItem;
	while (1)
	{
		if (m_BufSessionNeedToClose.GetOneItem((char*)&OneWorkItem, sizeof(OneWorkItem)) == E_Circle_Buf_No_Error)
		{
			//	guan bi client
			uint64_t u64SessionID = OneWorkItem.u64SessionID;
			LSession* pSession = m_pNetWorkServices->GetSessionManager().FindSession(u64SessionID);
			if (pSession == NULL)
			{
				char szError[512];
				sprintf(szError, "LCloseSocketThread::ThreadDoing, Can not Find Session:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				continue;
			}

#ifdef __ADD_SEND_BUF_CHAIN__
			pSession->ReleaseSendPacketBufChain(this);	
#endif
			m_pNetWorkServices->GetSessionManager().FreeSession(pSession);

			//	将该线程添加到连接管理器里面的即将重用的管理器里面去
			m_pNetWorkServices->GetSessionManager().AddToWillReuseManager(pSession);

			//	通知所有操作Session的线程，停止处理该Session(原理是：session从session管理器里面移除了，那么其他线程无法再获取到Session，只要处理了该信息，那么对应的线程就不会再处理了)
			m_pNetWorkServices->AddWillCloseSessionInMainLogic(pSession);

			//	获取不到线程，直接崩溃，说明整个程序初始化不正确
			int nEpollThreadID = pSession->GetEpollThreadID();
			m_pNetWorkServices->GetEpollThreadManager().GetEpollThread(nEpollThreadID)->AddWillCloseSessionInEpollThread(pSession);
			int nRecvThreadID = pSession->GetRecvThreadID();
			m_pNetWorkServices->GetRecvThreadManager().GetRecvThread(nRecvThreadID)->AddWillCloseSession(pSession);
			int nSendThreadID = pSession->GetSendThreadID();
			m_pNetWorkServices->GetSendThreadManager().GetSendThread(nSendThreadID)->AddWillCloseSessionInSendThread(pSession);

//			m_pNetWorkServices->GetEpollThreadManager().UnBindEpollThread(pSession);
//			m_pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(pSession);
//			m_pNetWorkServices->GetSendThreadManager().UnBindSendThread(pSession);

			t_Recv_Packet tRecvPacket;
			tRecvPacket.u64SessionID = u64SessionID;
			tRecvPacket.pPacket = (LPacketSingle*)0;
			if (m_FixBufToCommitSessionDisconnect.AddItems((char*)&tRecvPacket, 1) != E_Circle_Buf_No_Error)
			{ 
				char szError[512];
				sprintf(szError, "LCloseSocketThread::ThreadDoing, AddItems Failed, Session:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				continue;
			}
			if (!m_pNetWorkServices->CommitPackets(&m_FixBufToCommitSessionDisconnect))
			{ 
				char szError[512];
				sprintf(szError, "LCloseSocketThread::ThreadDoing, CommitPackets Failed, Session:%lld\n", u64SessionID);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				continue;
			}
		}
		else
		{
			//	sched_yield();
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		if (CheckForStop())
		{
			break;
		}
	}
	return 0;
}
bool LCloseSocketThread::OnStart()
{
	return true;
}
void LCloseSocketThread::OnStop()
{
}


#ifdef __ADD_SEND_BUF_CHAIN__
//	释放连接的数据包
bool LCloseSocketThread::AddPacketToLocalPool(LPacketBroadCast* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}

#ifdef __EPOLL_TEST_STATISTIC__
//	atomic_inc(&g_nSendPacketCloseThreadFreeCount);
	__sync_add_and_fetch(&g_nSendPacketCloseThreadFreeCount, 1);
	pPacket->FillPacketForTest();
#endif

	if (!m_LocalBroadCastPacketForFree.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen()))
	{ 
		if (!m_pNetWorkServices->CommitFreePacketToGlobalSendPool(&m_LocalBroadCastPacketForFree, pPacket->GetPacketBufLen()))
		{
			char szError[512];
			sprintf(szError, "LCloseSocketThread::AddPacketToFreePool, m_pNetWorkServices->CommitFreePacketToGlobalSendPool Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			
			//	shan chu yi ge shu ju bao,
			m_nFreePacketCount++;
			delete pPacket;
			return true;
		}
		//		再提交一次，不成功，那么删除数据包
		if (!m_LocalBroadCastPacketForFree.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen()))
		{
			char szError[512];
			sprintf(szError, "LCloseSocketThread::AddPacketToFreePool, m_LocalBroadCastPacketForFree.FreeOneItemToPool Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));

			m_nFreePacketCount++;
			delete pPacket;
			return true;
		}
	}
	return true;
}
#endif


void LCloseSocketThread::ReleaseCloseSocketThreadResource()
{
#ifdef __ADD_SEND_BUF_CHAIN__
	//	释放内存
	m_LocalBroadCastPacketForFree.ReleasePacketPoolManagerResource();
#endif
}


void LCloseSocketThread::StopCloseSocketThread()
{
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		Stop();

		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes != 0)
		{ 
			char szError[512];
			sprintf(szError, "LCloseSocketThread::StopCloseSocketThread, Failed, ErrorId:%d\n", errno); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
		}
	}
}


