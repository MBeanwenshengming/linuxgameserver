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

#ifndef __LINUX_NET_WORK_SERVICES_HEADER_DEFINED__
#define __LINUX_NET_WORK_SERVICES_HEADER_DEFINED__

#include "LRecvThreadManager.h"
#include "LSendThreadManager.h"
#include "LEpollThreadManager.h"
#include "LAcceptThread.h"
#include "LSessionManager.h"

#include "LFixLenCircleBuf.h"
#include "LPacketPoolManager.h"
#include "LPacketSingle.h"
#include "LRecvThread.h"
#include "LPacketBroadCast.h"
#include "LCloseSocketThread.h" 
#include "LNetWorkServicesIdleSendPacket_Recycle.h"

typedef struct _NetWorkServices_Params_Base
{
	char* pListenIP;					//	 监听IP
	unsigned short	usListenPort;		// 监听端口
	unsigned int	unListenListSize;	// 监听套接字上允许的排队最大数量
}t_NetWorkServices_Params_Base;

typedef struct _NetWorkServices_Params_Session
{
	unsigned short usSessionManagerCount;				//	连接管理数量
	unsigned int unMaxSessionCountPerSessionManager;	//	每个连接管理器管理的连接数量
	unsigned int unSendBufChainSize;					//	每个连接分配的发送队列的容量
	unsigned short usKickOutSessionTime;				//	踢出不通信连接的超时时间，默认为30秒
}t_NetWorkServices_Params_Session;

typedef struct _NetWorkServices_Params_Recv_Thread
{
	unsigned short usThreadCount;						//	接收线程的数量
	unsigned int unRecvWorkItemCount;					// 	epollin事件环形队列的最大队列数，大于该数量，那么EPOLLIN事件无法放入接收线程处理tNetWorkServicesParams
	t_Packet_Pool_Desc*	pArrppdForRecvLocalPool;		//	本地接收数据包缓冲的大小，初始化本地缓存，减少锁冲突
	unsigned int unRecvppdForRecvLocalPoolCount;		//	数组的数量
	unsigned int unRecvLocalPacketPoolSize;				//	本地接收到的数据包缓冲的数量，达到一定数量提交，或者本地的线程完成一次循环，就提交
}t_NetWorkServices_Params_Recv_Thread;

typedef struct _NetWorkServices_Params_Send_Thread
{
	unsigned int unSendThreadCount;						//	发送线程的数量
	unsigned int unSendWorkItemCount;					//	发送队列的长度，这里描述了需要发送的连接和数据包
	t_Packet_Pool_Desc*	pArrspdForSend;					//	发送数据包释放时，本地的缓存，
	unsigned int usspdSendCount;						//	缓存描述的数量
	unsigned int unEpollOutEventMaxCount;				//	发送线程接收的EPOLLOUT事件的最大数量
}t_NetWorkServices_Params_Send_Thread;

typedef struct _NetWorkServices_Params_Epoll_Thread
{
	unsigned int unEpollThreadCount;					//	EPOLL线程数量
	unsigned int unRecvThreadCount;						//	接收线程的数量
	unsigned int unRecvThreadWorkItemCount;				//	可以放置的最大EPOLLIN事件数量,本地缓存的EPOLLIN事件数量，一次提交给接收线程
	unsigned int unSendThreadCount;						//	发送线程的数量
	unsigned int unSendThreadWorkItemCount; 			//	每个发送线程本地缓存EPOLLOUT事件的数量，一次性提交给发送线程
	unsigned int unWaitClientSizePerEpoll; 				//	每个EPOLL上监听的套接字数量, 创建epoll时使用
}t_NetWorkServices_Params_Epoll_Thread;

typedef struct _NetWorkServices_Params_Close_Thread
{
	unsigned int unCloseWorkItemCount;					//	最大可以提交的关闭事件数量
	t_Packet_Pool_Desc*	pArrppdForCloseLocalPool;		//	连接关闭时，释放该连接下没有发送的发送数据包到本地缓存池，达到一定数量时，提交到全局缓冲池
	unsigned int unppdForCloseLocalPoolCount;			//	描述信息数组的长度
}t_NetWorkServices_Params_Close_Thread;

typedef struct _NetWorkServices_Params_Global_Pool
{
	unsigned int unRecvedPacketPoolSize;				//  全局的接收到的数据包的缓冲池，缓冲池最大数量
	t_Packet_Pool_Desc* pArrppdForGlobalRecvPacketPool;	//	全局的接收数据包的缓冲池
	unsigned int unppdForGlobalRecvPacketPoolCount;		//	全局的接收数据包的缓冲池数量
	t_Packet_Pool_Desc* pArrppdForGlobalSendPacketPool;	//	全局的发送数据包的缓冲池
	unsigned int unppdForGlobalSendPacketPoolCount;		//	全局的发送数据包的缓冲池数量
	unsigned int unSendLocalPoolSizeForNetWorkServices;	//	在NetWorkServices里面缓存的发送数据包的数量
}t_NetWorkServices_Params_Global_Pool;

typedef struct _NetWorkServices_Params_Idle_SendPacket
{
	unsigned int unIdleThreadWorkQueueSize;				//	空闲发送数据包回收线程工作队列大小
	unsigned int unLocalPoolMaxSize;					//	回收线程本地缓存池大小
}t_NetWorkServices_Params_Idle_SendPacket;

typedef struct _NetWorkServices_Params
{ 
	t_NetWorkServices_Params_Base nwspBase;
	t_NetWorkServices_Params_Session nwspSession;
	t_NetWorkServices_Params_Recv_Thread nwspRecvThread;
	t_NetWorkServices_Params_Send_Thread nwspSendThread;
	t_NetWorkServices_Params_Epoll_Thread nwspEpollThread;
	t_NetWorkServices_Params_Close_Thread nwspCloseThread;
	t_NetWorkServices_Params_Global_Pool nwspGlobalPool;
	t_NetWorkServices_Params_Idle_SendPacket nwspis;
}t_NetWorkServices_Params;

class LNetWorkServices
{
public:
	LNetWorkServices();
	~LNetWorkServices();

public:
	LRecvThreadManager& GetRecvThreadManager();
	LSendThreadManager& GetSendThreadManager();
	LEpollThreadManager& GetEpollThreadManager();
	LAcceptThread& GetAcceptThread();
	LMasterSessionManager& GetSessionManager();
	LCloseSocketThread& GetCloseSocketThread();
#ifdef __ADD_SEND_BUF_CHAIN__
	LNetWorkServicesIdleSendPacketRecycle& GetIdleSendPacketRecycle();
#endif
private:
	LRecvThreadManager m_RecvThreadManager;
	LSendThreadManager m_SendThreadManager;
	LEpollThreadManager m_EpollThreadManager;
	LAcceptThread m_AcceptThread;
	LMasterSessionManager m_SessionManager;
	LCloseSocketThread m_CloseSocketThread;
#ifdef __ADD_SEND_BUF_CHAIN__
	LNetWorkServicesIdleSendPacketRecycle m_nwsIdleSendPacketRecycle;
#endif
public:

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


//	char* pListenIP, unsigned short usListenPort, unsigned int unListenListSize,
//		unsigned int unRecvedPacketPoolSize,
//		t_Packet_Pool_Desc ppdForGlobalRecvPacketPool, unsigned int unppdForGlobalRecvPacketPoolCount,
//		t_Packet_Pool_Desc ppdForGlobalSendPacketPool, unsigned int unppdForGlobalSendPacketPoolCount,
//		unsigned int unSendLocalPoolSizeForNetWorkServices,
//		unsigned short usSessionManagerCount, unsigned int unMaxSessionCountPerSessionManager, unsigned int unSendBufChainSize,
//		unsigned int unRecvThreadCount, unsigned int unRecvWorkItemCount, t_Packet_Pool_Desc ppdForRecvLocalPool[], unsigned int unRecvppdForLocalPoolCount, unsigned int unRecvLocalPacketPoolSize,
//		unsigned int unSendThreadCount, unsigned int unSendWorkItemCount, t_Packet_Pool_Desc spdForSend[], unsigned short usspdSendCount, unsigned int unEpollOutEventMaxCount,
//		unsigned int unEpollThreadCount, unsigned int unRecvThreadWorkItemCount, unsigned int unSendThreadWorkItemCount, unsigned int unWaitClientSizePerEpoll,
//		unsigned int unCloseWorkItemCount, t_Packet_Pool_Desc ppdForCloseLocalPool[], unsigned int unppdForCloseLocalPoolCount

//bInitialAccept  是否初始化监听线程，如果是用来连接的，那么不需要初始化监听线程
	bool Initialize(t_NetWorkServices_Params& nwsp, bool bInitialAccept = true);
	bool Start();
	void Stop();
	//	jie shou shu ju bao xiang guan nei rong
public:
	LPacketPoolManager<LPacketSingle>* GetRecvGlobalPool();
	bool CommitPackets(LFixLenCircleBuf* pLocalFixPacketBuf);
	bool FreeRecvedPacket(LPacketSingle* pPacket);
	bool RequestFreePacketFromGlobalRecvPacketPool(LPacketPoolManager<LPacketSingle>* pRecvLocalPacketPool, unsigned short usPacketBufLen);
private:
	pthread_mutex_t m_tMutexForRequestRecvPacketPoolFromGlobalRecvPool;
	LPacketPoolManager<LPacketSingle> m_RecvGlobalPacketPool;
	t_Packet_Pool_Desc* m_pArrayRecvGlobalPacketPoolDesc;
	unsigned int m_unRecvGlobalPacketArrayCount;
	pthread_mutex_t m_tMutexForProtectedCommitFromRecvThread;
	LFixLenCircleBuf m_RecvedPacketQueue;

	//	用作连接消息包处理时，用来添加已经连接的连接
	pthread_mutex_t m_tMutexForAddSession;
	bool m_bInitialAccept;
	//	fa song xiang guan
public:
#ifdef __ADD_SEND_BUF_CHAIN__
	LPacketPoolManager<LPacketBroadCast>* GetSendGlobalPool();
	LPacketBroadCast* RequestOnePacket(unsigned short usPacketLen);
	bool CommitFreePacketToGlobalSendPool(LPacketPoolManager<LPacketBroadCast>* pFreePacketPool, unsigned short usPacketLen);
	bool SendPacket(uint64_t u64SessionID, int nSendThreadID, LPacketBroadCast* pPacket);
#endif
#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	bool SendPacket(uint64_t u64SessionID, int nSendThreadID, LPacketSingle* pPacket);
#endif
	bool FlushSendPacketToSend(int nSendThreadID);
	void FlushAllPacketToSend();

#ifdef __ADD_SEND_BUF_CHAIN__
	int GetAllFreeSendPacket()
	{
		return m_SendGlobalPacketPool.GetCurrentContentCount() + m_SendThreadManager.GetCurrentPacketFreePoolItemCount() + m_CloseSocketThread.GetPacketFreeItemCount() + m_nwsIdleSendPacketRecycle.GetPacketFreeItemCount();
	}
#endif

private:
	unsigned int m_unSendThreadCount;
#ifdef __ADD_SEND_BUF_CHAIN__
	pthread_mutex_t m_tMutexForProtectedCommitFromSendThread;
	LPacketPoolManager<LPacketBroadCast> m_SendGlobalPacketPool;
	t_Packet_Pool_Desc* m_pArraySendGlobalPacketPoolDesc;
	unsigned int m_unSendGlobalPacketArrayCount;
#endif
	LFixLenCircleBuf* m_parrLocalSendPacketPoolCache;


//	for main Logic
public:
	bool GetOneRecvedPacket(t_Recv_Packet* pRecvPacket);

public:	//	for Test
	void PrintNetWorkServiceBufStatus(LErrorWriter* pRecvWriter, LErrorWriter* pSendWriter);
	void PrintRefCountInfoForAll();

public:
	//	shan chu gu ding shi jian ,mei you tong xun de lian jie
	void KickOutIdleSession();

	bool GetSessionIPAndPort(unsigned long long u64SessionID, char* pszBuf, unsigned short usbufLen);
public:	//	For AcceptThread
	void FreeAcceptThreadPacket(LPacketSingle* pPacket);
public:	// For Close Socket
	void KickOutSession(uint64_t u64SessionID);
public:	//	释放占用的资源
	void ReleaseNetWorkServicesResource();

public:		//	添加一个已经连接的连接到管理器中，进行消息包的处理
	bool AddConnecttedSocketToSessionManager(int newClient, char* pDataAttachedToPacket, unsigned short usDataLen);

#ifdef __ADD_SEND_BUF_CHAIN__
public:		//	本地保存一个队列，该队列放置了请求的数据包的一个引用，在主线程没有使用的请况下，也可以回收到内存池中
	bool AddOneSendPacketToLocalQueue(LPacketBroadCast* pPacket);
private:
	LFixLenCircleBuf m_LocalFixLenCircleBufForIdleSendPacket;
#endif

public: 
	void GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort);
#ifdef  __EPOLL_TEST_STATISTIC__
	void PrintfAllocAndFreeCount();
#endif

public:
	//	主线程必须调用这个函数，来这却处理关闭连接
	void ProcessWillCloseSessionInMainLogic();
	void AddWillCloseSessionInMainLogic(LSession* pSession);
	bool GetOneWillCloseSessionInMainLogic(LSession** pSession);
	void ProcessKickOutSession(uint64_t u64SessionID);
private:
	LFixLenCircleBuf m_FixCircleBufWillCloseSessionInMainLogic;
};
#endif

