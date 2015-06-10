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

#include "LNetWorkServices.h"
#include "LSendThread.h"
#include "LErrorWriter.h"
#include "LAutoReleaseMutex.h"

extern LErrorWriter g_ErrorWriter;

#ifdef __EPOLL_TEST_STATISTIC__
//	接收数据包总共的分配次数
volatile int g_nRecvPacketAllocCount;
//	接收数据包总共的释放次数
volatile int g_nRecvPacketFreeCount;
//	发送数据包总共的分配次数
volatile int g_nSendPacketAllocCount;
//	发送数据包总共的释放次数
volatile int g_nSendPacketFreeCount;
//	发送数据包关闭线程释放次数
volatile int g_nSendPacketCloseThreadFreeCount;
//	发送数据包空闲线程释放次数
volatile int g_nSendPacketIdleThreadFreeCount;
#endif

LNetWorkServices::LNetWorkServices()
{
	m_bInitialAccept = true;
	pthread_mutex_init(&m_tMutexForRequestRecvPacketPoolFromGlobalRecvPool, NULL);
	pthread_mutex_init(&m_tMutexForProtectedCommitFromRecvThread, NULL); 

#ifdef __ADD_SEND_BUF_CHAIN__
	pthread_mutex_init(&m_tMutexForProtectedCommitFromSendThread, NULL);
#endif
	pthread_mutex_init(&m_tMutexForAddSession, NULL);

	m_parrLocalSendPacketPoolCache 	= NULL;
	m_unRecvGlobalPacketArrayCount	= 0;

	m_unSendThreadCount = 0 ;
#ifdef __ADD_SEND_BUF_CHAIN__
	m_unSendGlobalPacketArrayCount 	= 0;
	m_pArraySendGlobalPacketPoolDesc = NULL;
#endif
	m_pArrayRecvGlobalPacketPoolDesc = NULL;

#ifdef __EPOLL_TEST_STATISTIC__
//	atomic_set(&g_nRecvPacketAllocCount, 0);
//	atomic_set(&g_nRecvPacketFreeCount, 0);
//	atomic_set(&g_nSendPacketAllocCount, 0);
//	atomic_set(&g_nSendPacketFreeCount, 0);
//	atomic_set(&g_nSendPacketCloseThreadFreeCount, 0);
//	atomic_set(&g_nSendPacketIdleThreadFreeCount, 0);

	__sync_lock_test_and_set(&g_nRecvPacketAllocCount, 0);
	__sync_lock_test_and_set(&g_nRecvPacketFreeCount, 0);
	__sync_lock_test_and_set(&g_nSendPacketAllocCount, 0);
	__sync_lock_test_and_set(&g_nSendPacketFreeCount, 0);
	__sync_lock_test_and_set(&g_nSendPacketCloseThreadFreeCount, 0);
	__sync_lock_test_and_set(&g_nSendPacketIdleThreadFreeCount, 0);
#endif
}

LNetWorkServices::~LNetWorkServices()
{
	pthread_mutex_destroy(&m_tMutexForRequestRecvPacketPoolFromGlobalRecvPool);
	pthread_mutex_destroy(&m_tMutexForProtectedCommitFromRecvThread);
#ifdef __ADD_SEND_BUF_CHAIN__
	pthread_mutex_destroy(&m_tMutexForProtectedCommitFromSendThread);
#endif
	pthread_mutex_destroy(&m_tMutexForAddSession);
}

LRecvThreadManager& LNetWorkServices::GetRecvThreadManager()
{
	return m_RecvThreadManager;
}
LSendThreadManager& LNetWorkServices::GetSendThreadManager()
{
	return m_SendThreadManager;
}
LEpollThreadManager& LNetWorkServices::GetEpollThreadManager()
{
	return m_EpollThreadManager;
}
LAcceptThread& LNetWorkServices::GetAcceptThread()
{
	return m_AcceptThread;
}

LMasterSessionManager& LNetWorkServices::GetSessionManager()
{
	return m_SessionManager;
}

LCloseSocketThread& LNetWorkServices::GetCloseSocketThread()
{
	return m_CloseSocketThread;
}
//	监听线程相关参数描述
	//	pListenIP 监听IP
	//	usListenPort 监听端口
	//	unListenListSize 监听套接字上允许的排队最大数量
//	连接管理器相关参数
	//	usSessionManagerCount	连接管理数量
	//	unMaxSessionCountPerSessionManager 每个连接管理器管理的连接数量
	//	unSendBufChainSize 每个连接分配的发送队列的容量
//	接收相关的参数描述
	//	usThreadCount 接收线程的数量
	//	unRecvWorkItemCount epollin事件环形队列的最大队列数，大于该数量，那么EPOLLIN事件无法放入接收线程处理
	//	ppdForRecvLocalPool 本地接收数据包缓冲的大小，初始化本地缓存，减少锁冲突
	//	unRecvppdForRecvLocalPoolCount 数组的数量
	//	unRecvLocalPacketPoolSize本地接收到的数据包缓冲的数量，达到一定数量提交，或者本地的线程完成一次循环，就提交
	
//	发送相关的参数描述 
	//	unSendThreadCount 发送线程的数量
	//	unSendWorkItemCount 发送队列的长度，这里描述了需要发送的连接和数据包
	//	spdForSend 发送数据包释放时，本地的缓存，
	//	usspdSendCount 缓存描述的数量
	//	unEpollOutEventMaxCount 发送线程接收的EPOLLOUT事件的最大数量
	
//	EPOLL线程相关参数描述 
	//	unEpollThreadCount EPOLL线程数量
	//	unRecvThreadCount 接收线程的数量
	//	unRecvThreadWorkItemCount 可以放置的最大EPOLLIN事件数量,本地缓存的EPOLLIN事件数量，一次提交给接收线程
	//	unSendThreadCount 发送线程的数量
	//	unSendThreadWorkItemCount 每个发送线程本地缓存EPOLLOUT事件的数量，一次性提交给发送线程
	//	unWaitClientSizePerEpoll 每个EPOLL上监听的套接字数量, 创建epoll时使用
//	关闭线程相关参数描述
	//	unCloseWorkItemCount 最大可以提交的关闭事件数量
	//	ppdForCloseLocalPool 连接关闭时，释放该连接下没有发送的发送数据包到本地缓存池，达到一定数量时，提交到全局缓冲池
	//	unppdForCloseLocalPoolCount 描述信息数组的长度

//	unRecvedPacketPoolSize  全局的接收到的数据包的缓冲池，缓冲池最大数量
//	ppdForGlobalRecvPacketPool 全局的接收数据包的缓冲池
//	unppdForGlobalRecvPacketPoolCount 全局的接收数据包的缓冲池数量
//	ppdForGlobalSendPacketPool 全局的发送数据包的缓冲池
//	unppdForGlobalSendPacketPoolCount 全局的发送数据包的缓冲池数量
//	unSendLocalPoolSizeForNetWorkServices 在NetWorkServices里面缓存的发送数据包的数量

bool LNetWorkServices::Initialize(t_NetWorkServices_Params& nwsp, bool bInitialAccept)
{ 
	m_RecvThreadManager.SetNetWorkServices(this);
	if (!m_RecvThreadManager.Init(nwsp.nwspRecvThread.usThreadCount, nwsp.nwspRecvThread.unRecvWorkItemCount, nwsp.nwspRecvThread.pArrppdForRecvLocalPool, nwsp.nwspRecvThread.unRecvppdForRecvLocalPoolCount, nwsp.nwspRecvThread.unRecvLocalPacketPoolSize))
	{
		return false;
	}
	m_SendThreadManager.SetNetWorkServices(this);
	if (!m_SendThreadManager.Initialize(nwsp.nwspSendThread.unSendThreadCount, nwsp.nwspSendThread.unSendWorkItemCount, nwsp.nwspSendThread.pArrspdForSend, nwsp.nwspSendThread.usspdSendCount, nwsp.nwspSendThread.unEpollOutEventMaxCount)) 
	{
		return false;
	}
	m_unSendThreadCount = nwsp.nwspSendThread.unSendThreadCount;

	m_EpollThreadManager.SetNetWorkServices(this);
	if (!m_EpollThreadManager.Initialize(nwsp.nwspEpollThread.unEpollThreadCount, nwsp.nwspEpollThread.unRecvThreadCount, nwsp.nwspEpollThread.unRecvThreadWorkItemCount, nwsp.nwspEpollThread.unSendThreadCount, nwsp.nwspEpollThread.unSendThreadWorkItemCount, nwsp.nwspEpollThread.unWaitClientSizePerEpoll))
	{
		return false;
	}

	//bInitialAccept  是否初始化监听线程，如果是用来连接的，那么不需要初始化监听线程
	m_bInitialAccept = bInitialAccept;

	m_AcceptThread.SetNetWorkServices(this);
	if (!m_AcceptThread.Initialize(nwsp.nwspBase.pListenIP, nwsp.nwspBase.usListenPort, nwsp.nwspBase.unListenListSize, nwsp.nwspSession.unMaxSessionCountPerSessionManager * nwsp.nwspSession.usSessionManagerCount, m_bInitialAccept))
	{
		return false;
	}

	m_SessionManager.SetNetWorkServices(this);
	if (!m_SessionManager.InitializeSessionPool(nwsp.nwspSession.usSessionManagerCount, nwsp.nwspSession.unMaxSessionCountPerSessionManager, nwsp.nwspSession.unSendBufChainSize))
	{
		return false;
	}
	m_SessionManager.SetTimeForKickOutIdleSession(nwsp.nwspSession.usKickOutSessionTime);

	m_CloseSocketThread.SetNetWorkServices(this);
	if (!m_CloseSocketThread.Initialize(nwsp.nwspCloseThread.unCloseWorkItemCount, nwsp.nwspCloseThread.pArrppdForCloseLocalPool, nwsp.nwspCloseThread.unppdForCloseLocalPoolCount))
	{
		return false;
	}


	if (!m_RecvedPacketQueue.Initialize(sizeof(t_Recv_Packet), nwsp.nwspGlobalPool.unRecvedPacketPoolSize))
	{
		return false;
	}
	if (!m_RecvGlobalPacketPool.Initialize(nwsp.nwspGlobalPool.pArrppdForGlobalRecvPacketPool, nwsp.nwspGlobalPool.unppdForGlobalRecvPacketPoolCount))
	{
		return false;
	} 
	m_unRecvGlobalPacketArrayCount = nwsp.nwspGlobalPool.unppdForGlobalRecvPacketPoolCount;
	m_pArrayRecvGlobalPacketPoolDesc = new t_Packet_Pool_Desc[m_unRecvGlobalPacketArrayCount];
	if (m_pArrayRecvGlobalPacketPoolDesc == NULL)
	{
		return false;
	}
	memcpy(m_pArrayRecvGlobalPacketPoolDesc, nwsp.nwspGlobalPool.pArrppdForGlobalRecvPacketPool, sizeof(t_Packet_Pool_Desc) * m_unRecvGlobalPacketArrayCount);

#ifdef __ADD_SEND_BUF_CHAIN__
	m_SendGlobalPacketPool.SetEnableAlloc(true);
	if (!m_SendGlobalPacketPool.Initialize(nwsp.nwspGlobalPool.pArrppdForGlobalSendPacketPool, nwsp.nwspGlobalPool.unppdForGlobalSendPacketPoolCount))
	{
		return false;
	}

	m_unSendGlobalPacketArrayCount = nwsp.nwspGlobalPool.unppdForGlobalSendPacketPoolCount;
	m_pArraySendGlobalPacketPoolDesc = new t_Packet_Pool_Desc[nwsp.nwspGlobalPool.unppdForGlobalSendPacketPoolCount];
	if (m_pArraySendGlobalPacketPoolDesc == NULL)
	{
		return false;
	}
	memcpy(m_pArraySendGlobalPacketPoolDesc, nwsp.nwspGlobalPool.pArrppdForGlobalSendPacketPool, sizeof(t_Packet_Pool_Desc) * m_unSendGlobalPacketArrayCount);
#endif

	m_parrLocalSendPacketPoolCache = new LFixLenCircleBuf[nwsp.nwspSendThread.unSendThreadCount];
	if (m_parrLocalSendPacketPoolCache == NULL)
	{
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < nwsp.nwspSendThread.unSendThreadCount; ++unIndex)
	{
		if (!m_parrLocalSendPacketPoolCache[unIndex].Initialize(sizeof(t_Send_Content_Desc), nwsp.nwspGlobalPool.unSendLocalPoolSizeForNetWorkServices))
		{
			return false;
		}
	}

#ifdef __ADD_SEND_BUF_CHAIN__
	m_nwsIdleSendPacketRecycle.SetNetWorkServices(this);
	if (!m_LocalFixLenCircleBufForIdleSendPacket.Initialize(sizeof(LPacketBroadCast*), nwsp.nwspis.unIdleThreadWorkQueueSize))
	{
		return false;
	}
	if (!m_nwsIdleSendPacketRecycle.InitializeIdleSendPacketRecycle(nwsp.nwspis.unIdleThreadWorkQueueSize, m_pArraySendGlobalPacketPoolDesc, m_unSendGlobalPacketArrayCount, nwsp.nwspis.unLocalPoolMaxSize))
	{
		return false;
	}
#endif
	if (!m_FixCircleBufWillCloseSessionInMainLogic.Initialize(sizeof(LSession*), nwsp.nwspRecvThread.unRecvWorkItemCount))
	{
		return false;
	}

	return true;
}
bool LNetWorkServices::Start()
{
#ifdef __ADD_SEND_BUF_CHAIN__
	if (!m_nwsIdleSendPacketRecycle.Start())
	{
		return false;
	}
#endif
	if (!m_RecvThreadManager.StartAllRecvThread())
	{
		return false;
	}
	if (!m_SendThreadManager.StartAllSendThread())
	{
		return false;
	}
	if (!m_EpollThreadManager.StartAllEpollThread())
	{
		return false;
	}
	if (!m_CloseSocketThread.Start())
	{
		return false;
	}
	if (m_bInitialAccept)
	{
		if (!m_AcceptThread.Start())
		{
			return false;
		}
	}
	return true;
}

void LNetWorkServices::Stop()
{
	if (m_bInitialAccept)
	{
		m_AcceptThread.StopAcceptThread();
	}
#ifdef __ADD_SEND_BUF_CHAIN__
	m_nwsIdleSendPacketRecycle.Stop();
#endif
	m_EpollThreadManager.StopAllEpollThread();
	m_SendThreadManager.StopAllSendThread();
	m_RecvThreadManager.StopAllRecvThread();
	m_CloseSocketThread.StopCloseSocketThread();
}

LPacketPoolManager<LPacketSingle>* LNetWorkServices::GetRecvGlobalPool()
{
	return &m_RecvGlobalPacketPool;
}
bool LNetWorkServices::CommitPackets(LFixLenCircleBuf* pLocalFixPacketBuf)
{
	if (pLocalFixPacketBuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::CommitPackets, pLocalFixPacketBuf == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	pthread_mutex_lock(&m_tMutexForProtectedCommitFromRecvThread);
	if (!pLocalFixPacketBuf->CopyAllItemsToOtherFixLenCircleBuf(&m_RecvedPacketQueue))
	{
		pthread_mutex_unlock(&m_tMutexForProtectedCommitFromRecvThread);
		char szError[512];
		sprintf(szError, "LNetWorkServices::CommitPackets, CopyAllItemsToOtherFixLenCircleBuf\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	} 
	pthread_mutex_unlock(&m_tMutexForProtectedCommitFromRecvThread);
	return true;
}

bool LNetWorkServices::FreeRecvedPacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::FreeRecvedPacket, pPacket == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	pPacket->Reset();
#ifdef  __EPOLL_TEST_STATISTIC__
//	atomic_inc(&g_nRecvPacketFreeCount);
	__sync_add_and_fetch(&g_nRecvPacketFreeCount, 1);
	pPacket->FillPacketForTest();
#endif
	bool bFreeSuccess = m_RecvGlobalPacketPool.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen());

	if (!bFreeSuccess)
	{ 
		char szError[512];
		sprintf(szError, "LNetWorkServices::FreeRecvedPacket, FreeOneItemToPool Failed, packetLen:%hd\n", pPacket->GetPacketBufLen()); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
	return bFreeSuccess;
}

bool LNetWorkServices::RequestFreePacketFromGlobalRecvPacketPool(LPacketPoolManager<LPacketSingle>* pRecvLocalPacketPool, unsigned short usPacketBufLen)
{
	if (pRecvLocalPacketPool == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::RequestFreePacketFromGlobalRecvPacketPool, pRecvLocalPacketPool == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (usPacketBufLen == 0)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::RequestFreePacketFromGlobalRecvPacketPool, usPacketBufLen == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	unsigned int unMaxCanAddToPoolCount = 0;
	LFixLenCircleBuf* pFixLenCircleBuf = pRecvLocalPacketPool->GetFixLenCircleBuf(usPacketBufLen, unMaxCanAddToPoolCount);
	if (pFixLenCircleBuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::RequestFreePacketFromGlobalRecvPacketPool, pFixLenCircleBuf == NULL, PacketBufLen:%hd\n", usPacketBufLen); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	int nGettedCount = 0;
	pthread_mutex_lock(&m_tMutexForRequestRecvPacketPoolFromGlobalRecvPool);
	nGettedCount = m_RecvGlobalPacketPool.Request(pFixLenCircleBuf, unMaxCanAddToPoolCount, usPacketBufLen);
	pthread_mutex_unlock(&m_tMutexForRequestRecvPacketPoolFromGlobalRecvPool);
	if (nGettedCount > 0)
	{
		return true;
	}
	return false; 
}

#ifdef __ADD_SEND_BUF_CHAIN__
LPacketPoolManager<LPacketBroadCast>* LNetWorkServices::GetSendGlobalPool()
{
	return &m_SendGlobalPacketPool;
}
LPacketBroadCast* LNetWorkServices::RequestOnePacket(unsigned short usPacketLen)
{
	int nFirstSelectedIndex = -1;
	LPacketBroadCast* pPacket = NULL;

	for (unsigned int unIndex = 0; unIndex < m_unSendGlobalPacketArrayCount; ++unIndex)
	{
		if (m_pArraySendGlobalPacketPoolDesc[unIndex].usPacketLen < usPacketLen)
		{
			continue;
		}
		unsigned short usCurrentSelectPacketLen = m_pArraySendGlobalPacketPoolDesc[unIndex].usPacketLen;
		if (nFirstSelectedIndex == -1)
		{
			nFirstSelectedIndex = unIndex;
		}
		pPacket = m_SendGlobalPacketPool.RequestOnePacket(usCurrentSelectPacketLen);
		if (pPacket != NULL)
		{
			pPacket->ResetRefCount();
			break;
		}
		else
		{
			continue;
		}
	}
	if (pPacket == NULL)
	{
		if (nFirstSelectedIndex == -1)
		{
			return NULL;
		}
		//  chuan jian yige xin de bao
		pPacket = new LPacketBroadCast(m_pArraySendGlobalPacketPoolDesc[nFirstSelectedIndex].usPacketLen);
		if (pPacket != NULL)
		{
			pPacket->ResetRefCount();
		}
	}
	if (!AddOneSendPacketToLocalQueue(pPacket))
	{
		//	error write
		char szError[512];
		sprintf(szError, "LNetWorkServices::RequestOnePacket, AddOneSendPacketToLocalQueue Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
#ifdef __EPOLL_TEST_STATISTIC__
	if (pPacket != NULL)
	{
//		atomic_inc(&g_nSendPacketAllocCount);
		__sync_add_and_fetch(&g_nSendPacketAllocCount, 1);
	}
#endif
	return pPacket;
}

bool LNetWorkServices::CommitFreePacketToGlobalSendPool(LPacketPoolManager<LPacketBroadCast>* pFreePacketPool, unsigned short usPacketBufLen)
{
	if (pFreePacketPool == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::CommitFreePacketToGlobalSendPool, pFreePacketPool == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	bool bSuccess = false;
	pthread_mutex_lock(&m_tMutexForProtectedCommitFromSendThread);
	bSuccess = pFreePacketPool->CommitToAnother(&m_SendGlobalPacketPool, usPacketBufLen);
	pthread_mutex_unlock(&m_tMutexForProtectedCommitFromSendThread);
	return bSuccess;
}


bool LNetWorkServices::SendPacket(uint64_t u64SessionID, int nSendThreadID, LPacketBroadCast* pPacket)
{
	if (pPacket == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::SendPacket, pPacket == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
//	LSession* pSession = m_SessionManager.FindSession(u64SessionID);
//	if (pSession == NULL)
//	{
//		char szError[512];
//		sprintf(szError, "LNetWorkServices::SendPacket, pSession == NULL\n"); 
//		g_ErrorWriter.WriteError(szError, strlen(szError));
//
//		//	need free packet
//		//	delete pPacket;
//		//if (pPacket->GetCurRefCount() == 0)
//		//{
//		//	delete pPacket;
//		//}
//
//		return false;
//	}
//	int nSendThreadID = pSession->GetSendThreadID();
//	if (nSendThreadID < 0)
//	{
//		char szError[512];
//		sprintf(szError, "LNetWorkServices::SendPacket, nSendThreadID < 0\n"); 
//		g_ErrorWriter.WriteError(szError, strlen(szError));
//
//		//	delete pPacket;
//		return false;
//	}
	if (nSendThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::SendPacket, nSendThreadID >= g_unSendThreadCount\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		//	delete pPacket;

		return false;
	}
	t_Send_Content_Desc tScd;
	tScd.u64SessionID 	= u64SessionID;
	tScd.pPacket		= pPacket;
	if (m_parrLocalSendPacketPoolCache[nSendThreadID].AddItems((char*)&tScd, 1) != E_Circle_Buf_No_Error)
	{ 
		char szError[512];
		sprintf(szError, "LNetWorkServices::SendPacket, AddItems Pos1 Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		//	bu neng zai zhe li flush,yin wei , ru guo shu ju bao hai meiyou fa tianjia wan cheng ,zhe ge shi hou,ying yong ji shu hai bu zheng que ,jiu fa song ,na me hui chan sheng bug,hai mei you  fa song wan cheng jiu shi fang le ,na me jiu hui chan sheng cuo wu
		//	error will raise
		//	FlushSendPacketToSend(nSendThreadID);

		//if (m_parrLocalSendPacketPoolCache[nSendThreadID].AddItems((char*)&tScd, 1) != E_Circle_Buf_No_Error)
		//{
		//	char szError[512];
		//	sprintf(szError, "LNetWorkServices::SendPacket, AddItems Failed\n"); 
		//	g_ErrorWriter.WriteError(szError, strlen(szError));
			//	delete pPacket;

		//	return false;
		//}
		//else
		//{ 
		//	pPacket->IncrementRefCount();
		//}
		return false;
	}
	else
	{
		pPacket->IncrementRefCount();
	}
	return true;
}
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
bool LNetWorkServices::SendPacket(uint64_t u64SessionID, int nSendThreadID, LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::SendPacket, pPacket == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	if (nSendThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::SendPacket, nSendThreadID >= g_unSendThreadCount\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	LSession* pSession = GetSessionManager().FindSession(u64SessionID);
	if (pSession == NULL)
	{
		return false;
	}
	//	关闭连接的工作已经向关闭线程发送，那么这里就不再发送数据了，等待连接的关闭
	if (pSession->GetCloseWorkSendedToCloseThread() != 0)
	{
		return false;
	}
	if (!pSession->AppendDataToSend(pPacket))
	{
		KickOutSession(u64SessionID);
		char szError[512];
		sprintf(szError, "LNetWorkServices::SendPacket, pSession->AppendDataToSend, Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	t_Send_Content_Desc tScd;
	tScd.u64SessionID 	= u64SessionID;
	if (m_parrLocalSendPacketPoolCache[nSendThreadID].AddItems((char*)&tScd, 1) != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::SendPacket, AddItems Pos1 Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	return true;
}
#endif

bool LNetWorkServices::FlushSendPacketToSend(int nSendThreadID)
{
	if (nSendThreadID < 0 || nSendThreadID >= m_unSendThreadCount)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::FlushSendPacketToSend, nSendThreadID < 0 || nSendThreadID >= g_unSendThreadCount\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	LSendThread* pSendThread = m_SendThreadManager.GetSendThread(nSendThreadID);
	if (pSendThread == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::FlushSendPacketToSend, pSendThread == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return pSendThread->CommitAllSendWorkItems(&m_parrLocalSendPacketPoolCache[nSendThreadID]);
}

void LNetWorkServices::FlushAllPacketToSend()
{
	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{
		if (!FlushSendPacketToSend(unIndex))
		{ 
			char szError[512];
			sprintf(szError, "LNetWorkServices::FlushAllPacketToSend, FlushSendPacketToSendFailed, index is:%d\n", unIndex); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
		}
	}
#ifdef __ADD_SEND_BUF_CHAIN__
	if (!m_nwsIdleSendPacketRecycle.PushIdleSendPacket(&m_LocalFixLenCircleBufForIdleSendPacket))
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::FlushAllPacketToSend, PushIdleSendPacket\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
#endif
}

bool LNetWorkServices::GetOneRecvedPacket(t_Recv_Packet* pRecvPacket)
{
	if (pRecvPacket == NULL)
	{
		return false;
	}

	E_Circle_Error errorCode = m_RecvedPacketQueue.GetOneItem((char*)pRecvPacket, sizeof(t_Recv_Packet)); 
	if (errorCode != E_Circle_Buf_No_Error && errorCode != E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	if (errorCode == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}


void LNetWorkServices::PrintNetWorkServiceBufStatus(LErrorWriter* pRecvWriter, LErrorWriter* pSendWriter)
{
	if (pRecvWriter == NULL || pSendWriter == NULL)
	{
		return ;
	}
	char szTempString[512];

	//	xie jie shou zhuang tai 
	sprintf(szTempString, "Recv buf Status Info\n");
	pRecvWriter->WriteError(szTempString, strlen(szTempString));
	
	//  deng dai chu li de xiao xi bao	
	sprintf(szTempString, "\tUnProcessed Recved Packet:%d\n", m_RecvedPacketQueue.GetCurrentExistCount()); 
	pRecvWriter->WriteError(szTempString, strlen(szTempString));
	
	//	jie shou packet pool zhuang tai 
	sprintf(szTempString, "\tRecv Packet Pool Status\n"); 
	pRecvWriter->WriteError(szTempString, strlen(szTempString));

	//	g_RecvGlobalPacketPool, g_sRecvGlobalPacketPoolCount
	//	m_RecvGlobalPacketPool
	for (unsigned int unIndex = 0; unIndex < m_unRecvGlobalPacketArrayCount; ++unIndex)
	{
		unsigned short usPacketLen = m_pArrayRecvGlobalPacketPoolDesc[unIndex].usPacketLen;
		unsigned int unMaxAdd = 0;
		LFixLenCircleBuf* pFixBuf = m_RecvGlobalPacketPool.GetFixLenCircleBuf(usPacketLen, unMaxAdd);
		sprintf(szTempString, "\t\tGlobal Recv Packet Len:%hd, Pool Item Count:%d\n", usPacketLen, pFixBuf->GetCurrentExistCount()); 
		pRecvWriter->WriteError(szTempString, strlen(szTempString));
	}
	//	xie fa song zhuang tai 
	sprintf(szTempString, "\tSend Global Pool Desc Info\n");
	pSendWriter->WriteError(szTempString, strlen(szTempString));
#ifdef __ADD_SEND_BUF_CHAIN__
	//
	for (unsigned int unIndex = 0; unIndex < m_unSendGlobalPacketArrayCount; ++unIndex)
	{
		unsigned short usPacketLen = m_pArraySendGlobalPacketPoolDesc[unIndex].usPacketLen;
		unsigned int unMaxAlloc = 0;
		LFixLenCircleBuf* pFixBuf = m_SendGlobalPacketPool.GetFixLenCircleBuf(usPacketLen, unMaxAlloc);
		sprintf(szTempString, "\t\tSend Global Packet Pool:%hd, ItemCount:%d\n", usPacketLen,pFixBuf->GetCurrentExistCount()); 
		pSendWriter->WriteError(szTempString, strlen(szTempString));
	}

	//	ben di fa song huan cun zhong de shu ju 
	sprintf(szTempString, "\tLocal Send Packet Pool\n"); 
	pSendWriter->WriteError(szTempString, strlen(szTempString));

	for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
	{ 
		int nCurrentNeedSendPacket = m_parrLocalSendPacketPoolCache[unIndex].GetCurrentExistCount();
		sprintf(szTempString, "\t\tSendThreadID:%d, Will Send Packet Count:%d\n", unIndex, nCurrentNeedSendPacket); 
		pSendWriter->WriteError(szTempString, strlen(szTempString));
	}
#endif	//	__ADD_SEND_BUF_CHAIN__
}


void LNetWorkServices::PrintRefCountInfoForAll()
{
	m_RecvThreadManager.PrintSendThreadRefStatus();
	m_SendThreadManager.PrintSendThreadRefStatus();
	m_EpollThreadManager.PrintEpollThreadStatus();
}


//	shan chu gu ding shi jian ,mei you tong xun de lian jie
void LNetWorkServices::KickOutIdleSession()
{ 
	m_SessionManager.KickOutIdleSession();
}

bool LNetWorkServices::GetSessionIPAndPort(unsigned long long u64SessionID, char* pszBuf, unsigned short usbufLen)
{
	return m_SessionManager.GetSessionIPAndPort(u64SessionID, pszBuf, usbufLen);
}


//	For AcceptThread
void LNetWorkServices::FreeAcceptThreadPacket(LPacketSingle* pPacket)
{
	m_AcceptThread.FreeAcceptedPacket(pPacket);
}

void LNetWorkServices::KickOutSession(uint64_t u64SessionID)
{
//	t_Client_Need_To_Close tcntc;
//	tcntc.u64SessionID = u64SessionID;
//	m_CloseSocketThread.AppendToClose(tcntc);
	ProcessKickOutSession(u64SessionID);
}

//	释放占用的资源
void LNetWorkServices::ReleaseNetWorkServicesResource()
{
	m_RecvThreadManager.ReleaseRecvThreadManagerResource();
	m_SendThreadManager.ReleaseSendThreadManagerResource();
	m_EpollThreadManager.ReleaseEpollThreadManagerResource();
	m_AcceptThread.ReleaseAcceptThreadResource();
	m_CloseSocketThread.ReleaseCloseSocketThreadResource();

	m_SessionManager.ReleaseMasterSessionManagerResource();

	if (m_RecvedPacketQueue.GetCurrentExistCount() != 0)
	{
		while (1)
		{
			t_Recv_Packet tTemp; memset(&tTemp, 0, sizeof(tTemp)); 

			E_Circle_Error error = m_RecvedPacketQueue.GetOneItem((char*)&tTemp, sizeof(tTemp)); 
			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				delete tTemp.pPacket;
			}
		}
	}
	m_RecvGlobalPacketPool.ReleasePacketPoolManagerResource();
	if (m_pArrayRecvGlobalPacketPoolDesc != NULL)
	{
		delete[] m_pArrayRecvGlobalPacketPoolDesc;
	}
	if (m_parrLocalSendPacketPoolCache != NULL)
	{
		delete[] m_parrLocalSendPacketPoolCache;
	}
#ifdef __ADD_SEND_BUF_CHAIN__
	m_SendGlobalPacketPool.ReleasePacketPoolManagerResource();
	if (m_pArraySendGlobalPacketPoolDesc != NULL)
	{
		delete[] m_pArraySendGlobalPacketPoolDesc;
	}

	if (m_parrLocalSendPacketPoolCache != NULL)
	{
		for (unsigned int unIndex = 0; unIndex < m_unSendThreadCount; ++unIndex)
		{
			while (1)
			{
				t_Send_Content_Desc tscd; memset(&tscd, 0, sizeof(tscd));
				E_Circle_Error error = m_parrLocalSendPacketPoolCache[unIndex].GetOneItem((char*)&tscd, sizeof(tscd));

				if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
				{
					break;
				}
				else
				{
					if (tscd.pPacket->DecrementRefCountAndResultIsTrue())
					{
						delete tscd.pPacket;
					}
				}
			}
		}
		delete[] m_parrLocalSendPacketPoolCache;
	}
	m_nwsIdleSendPacketRecycle.ReleaseNetWorkServicesIdleSendPacketRecycleThreadResouces();
	//	释放在本地还没有提交的数据包
	if (m_LocalFixLenCircleBufForIdleSendPacket.GetCurrentExistCount() != 0)
	{
		while (1)
		{
			LPacketBroadCast* pPacketToSend = NULL;
			E_Circle_Error error = m_LocalFixLenCircleBufForIdleSendPacket.GetOneItem((char*)&pPacketToSend, sizeof(LPacketBroadCast*)); 

			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				delete pPacketToSend;
			}
		}
	}
#endif
}

//	添加一个已经连接的连接到管理器中，进行消息包的处理
bool LNetWorkServices::AddConnecttedSocketToSessionManager(int newClient, char* pDataAttachedToPacket, unsigned short usDataLen)
{
	if (newClient <= 0)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::AddConnecttedSocketToSessionManager, newSocket <= 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	LAutoReleaseMutex AutoReleaseMutexForAddSession(&m_tMutexForAddSession);

	LMasterSessionManager& pMasterSessManager = GetSessionManager();
	LSession* pSession = pMasterSessManager.AllocSession();
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::AddConnecttedSocketToSessionManager, AllocSession Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	pSession->SetSocket(newClient);
	pSession->SetSessionConnecttedTime();

	LRecvThreadManager& pRecvThreadManager = GetRecvThreadManager();
	if (!pRecvThreadManager.BindRecvThread(pSession))
	{ 
		char szError[512];
		sprintf(szError, "LNetWorkServices::AddConnecttedSocketToSessionManager, BindRecvThread Failed, SessionID:%lld\n", pSession->GetSessionID());
		g_ErrorWriter.WriteError(szError, strlen(szError));

		pMasterSessManager.FreeSession(pSession); 
		return false;
	}

	LSendThreadManager& pSendThreadManager = GetSendThreadManager();
	if (!pSendThreadManager.BindSendThread(pSession))
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::AddConnecttedSocketToSessionManager, BindSendThread Failed, SessionID:%lld\n", pSession->GetSessionID());
		g_ErrorWriter.WriteError(szError, strlen(szError));

		pRecvThreadManager.UnBindRecvThread(pSession);
		pMasterSessManager.FreeSession(pSession);
		return false;
	}

	LEpollThreadManager& pEpollThreadManager = GetEpollThreadManager();
	if (!pEpollThreadManager.BindEpollThread(pSession))
	{ 
		char szError[512];
		sprintf(szError, "LNetWorkServices::AddConnecttedSocketToSessionManager, BindEpollThread Failed, SessionID:%lld\n", pSession->GetSessionID());
		g_ErrorWriter.WriteError(szError, strlen(szError));

		pSendThreadManager.UnBindSendThread(pSession);
		pRecvThreadManager.UnBindRecvThread(pSession);
		pMasterSessManager.FreeSession(pSession); 
		return false;
	}

	t_Recv_Packet tRecvPacket;		// tong zhi luo ji xian cheng ,ke hu  duan lian jie dao da
	LPacketSingle* pPacket = GetAcceptThread().GetOneAcceptedPacket();
	//	将必要的信息填入数据包中
	GetAcceptThread().BuildAcceptedPacket(pSession, pPacket);
	pPacket->AddUShort(usDataLen);
	pPacket->AddData(pDataAttachedToPacket, usDataLen);

	tRecvPacket.u64SessionID = pSession->GetSessionID();
	tRecvPacket.pPacket = pPacket;

	if (m_RecvedPacketQueue.AddItems((char*)&tRecvPacket, 1) != E_Circle_Buf_No_Error)
	{

		char szError[512];
		sprintf(szError, "LNetWorkServices::AddConnecttedSocketToSessionManager, AddItems Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		pEpollThreadManager.UnBindEpollThread(pSession);
		pSendThreadManager.UnBindSendThread(pSession);
		pRecvThreadManager.UnBindRecvThread(pSession);
		pMasterSessManager.FreeSession(pSession); 

		return false;
	} 

	if (!pEpollThreadManager.PostEpollReadEvent(pSession))
	{
		//	session closed
		tRecvPacket.pPacket = (LPacketSingle*)0;

		if (m_RecvedPacketQueue.AddItems((char*)&tRecvPacket, 1) != E_Circle_Buf_No_Error)
		{

			char szError[512];
			sprintf(szError, "LNetWorkServices::AddConnecttedSocketToSessionManager, AddItems close Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));

			pEpollThreadManager.UnBindEpollThread(pSession);
			pSendThreadManager.UnBindSendThread(pSession);
			pRecvThreadManager.UnBindRecvThread(pSession);
			pMasterSessManager.FreeSession(pSession); 
			return false;
		} 

		char szError[512];
		sprintf(szError, "LNetWorkServices::AddConnecttedSocketToSessionManager, PostEpollReadEvent Failed, SessionID:%lld\n", pSession->GetSessionID());
		g_ErrorWriter.WriteError(szError, strlen(szError));

		pEpollThreadManager.UnBindEpollThread(pSession);
		pSendThreadManager.UnBindSendThread(pSession);
		pRecvThreadManager.UnBindRecvThread(pSession);
		pMasterSessManager.FreeSession(pSession); 
		return false;
	}
	return true;
}
#ifdef __ADD_SEND_BUF_CHAIN__
bool LNetWorkServices::AddOneSendPacketToLocalQueue(LPacketBroadCast* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}
	pPacket->IncrementRefCount();
	E_Circle_Error error = m_LocalFixLenCircleBufForIdleSendPacket.AddItems((char*)&pPacket, 1);
	if (error != E_Circle_Buf_No_Error)
	{
		return false;
	}
	return true;
}
#endif
void LNetWorkServices::GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort)
{
	m_AcceptThread.GetListenIpAndPort(pBuf, unBufSize, usPort);
}

#ifdef  __EPOLL_TEST_STATISTIC__
void LNetWorkServices::PrintfAllocAndFreeCount()
{
//	int nRecvPacketAllocCount = atomic_read(&g_nRecvPacketAllocCount);
//	int nRecvPacketFreeCount = atomic_read(&g_nRecvPacketFreeCount);
//	int nSendPacketAllocCount = atomic_read(&g_nSendPacketAllocCount);
//	int nSendPacketFreeCount = atomic_read(&g_nSendPacketFreeCount);
//	int nSendPacketCloseThreadFreeCount = atomic_read(&g_nSendPacketCloseThreadFreeCount);
//	int nSendPacketIdleThreadFreeCount = atomic_read(&g_nSendPacketIdleThreadFreeCount);

	int nRecvPacketAllocCount = g_nRecvPacketAllocCount;
	int nRecvPacketFreeCount = g_nRecvPacketFreeCount;
	int nSendPacketAllocCount = g_nSendPacketAllocCount;
	int nSendPacketFreeCount = g_nSendPacketFreeCount;
	int nSendPacketCloseThreadFreeCount = g_nSendPacketCloseThreadFreeCount;
	int nSendPacketIdleThreadFreeCount = g_nSendPacketIdleThreadFreeCount;
#ifdef __ADD_SEND_BUF_CHAIN__
	printf("RecvPacket Alloc:%d, Free:%d;  SendPacket Alloc:%d, SendThreadFree:%d, CloseThreadFree:%d, IdleThreadFree:%d, TotalFreeSendPacket:%d\n", nRecvPacketAllocCount, nRecvPacketFreeCount, nSendPacketAllocCount, nSendPacketFreeCount, nSendPacketCloseThreadFreeCount, nSendPacketIdleThreadFreeCount, GetAllFreeSendPacket());
#else
	printf("RecvPacket Alloc:%d, Free:%d;  SendPacket Alloc:%d, SendThreadFree:%d, CloseThreadFree:%d, IdleThreadFree:%d\n", nRecvPacketAllocCount, nRecvPacketFreeCount, nSendPacketAllocCount, nSendPacketFreeCount, nSendPacketCloseThreadFreeCount, nSendPacketIdleThreadFreeCount);
#endif
}
#endif


void LNetWorkServices::AddWillCloseSessionInMainLogic(LSession* pSession)
{
	E_Circle_Error eError = m_FixCircleBufWillCloseSessionInMainLogic.AddItems((char*)&pSession, 1);
	if (eError != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::AddWillCloseSessionInMainLogic, Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
}
bool LNetWorkServices::GetOneWillCloseSessionInMainLogic(LSession** pSession)
{
	E_Circle_Error eError = m_FixCircleBufWillCloseSessionInMainLogic.GetOneItem((char*)(&(*pSession)), sizeof(LSession*));
	if (eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty)
	{
		char szError[512];
		sprintf(szError, "LNetWorkServices::GetOneWillCloseSessionInMainLogic, errorCode != E_Circle_Buf_No_Error\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (eError == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}
void LNetWorkServices::ProcessKickOutSession(uint64_t u64SessionID)
{
	LSession* pSession = GetSessionManager().FindSession(u64SessionID);
	if (pSession == NULL)
	{
		return ;
	}
	if (pSession->GetCloseWorkSendedToCloseThread() == 0)
	{
		//	发送一个关闭工作给关闭线程,所有连接相关的线程都不再操作连接了，那么就可以关闭该连接了
		t_Client_Need_To_Close cntc;
		cntc.u64SessionID 						= u64SessionID;
		LCloseSocketThread* pCloseThread 	= &(GetCloseSocketThread());
		pCloseThread->AppendToClose(cntc);

		pSession->SetCloseWorkSendedToCloseThread(1);
	}

}
//	主线程必须调用这个函数，来这却处理关闭连接
void LNetWorkServices::ProcessWillCloseSessionInMainLogic()
{
	int nProcessedCount = 0;
	LSession* pSession = NULL;
	while (GetOneWillCloseSessionInMainLogic(&pSession))
	{
		if (pSession != NULL)
		{
			pSession->SetMainLogicThreadStopProcess(1);

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
				this->GetSessionManager().MoveWillCloseSessionToSessionPool(pSession);
			}
		}
		nProcessedCount++;
		if (nProcessedCount >= 100)
		{
			break;
		}
	}
}
