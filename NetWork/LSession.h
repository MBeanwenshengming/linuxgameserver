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

#ifndef __LINUX_SESSION_HEADER_DEFINED__
#define __LINUX_SESSION_HEADER_DEFINED__

#include "IncludeHeader.h"
#include "LSocket.h"
#include "LFixLenCircleBuf.h"

class LRecvThread;
#define MAX_PACKET_SIZE 8 * 1024
typedef struct _Epoll_Bind_Param
{
	uint64_t u64SessionID;
	int nSocket;
}t_Epoll_Bind_Param;

class LPacketBroadCast;
class LSendThread;
class LCloseSocketThread;
class LPacketSingle;

class LSession : public LSocket
{
public:
	LSession();
	~LSession();
public:
	//	client ID sessionManager分配
	void SetSessionID(unsigned long long u64SessionID);
	unsigned long long GetSessionID();

	// 设置接收线程ID
	void SetRecvThreadID(int unRecvThreadID);
	int GetRecvThreadID();

	//	设置发送线程ID
	void SetSendThreadID(int unSendThreadID);
	int GetSendThreadID();

	//	设置Epoll线程ID
	void SetEpollThreadID(int unEpollThreadID);
	int GetEpollThreadID();

	//	Session重用时，需要重置Session的变量	
	void Reset();
private:
	unsigned long long m_u64SessionID;
	//	绑定的发送线程的ID	
	volatile int m_nRecvThreadID;
	//	绑定的发送线程的ID
	volatile int m_nSendThreadID;
	//	绑定的Epoll线程的ID
	volatile int m_nEpollThreadID;

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	//	session安全关闭相关的数据
	//	在__USE_SESSION_BUF_TO_SEND_DATA__方式下(即连接使用发送缓存情况下)
		//	只有在接收线程，发送线程，主线程退出处理的情况下，才可以回收session,重新分配给新连接上来的连接
		//	因为这个几个线程会操作session的buf，所以需要完全安全的情况下，才能重用session
public:
	void GetStopProcessInfos(int& nSendThreadStopProcessInfo, int& nRecvThreadStopProcessInfo,
			int& nMainLogicThreadStopProcessInfo, int& nEpollThreadSopProcessInfo);
	void SetSendThreadStopProcess(int nStop);
	int GetSendThreadStopProcess();
	void SetRecvThreadStopProcess(int nStop);
	int GetRecvThreadStopProcess();
	void SetMainLogicThreadStopProcess(int nStop);
	int GetMainLogicThreadStopProcess();
	void SetEpollThreadStopProcess(int nStop);
	int GetEpollThreadSopProcess();
	void SetCloseWorkSendedToCloseThread(int nSended);
	int GetCloseWorkSendedToCloseThread();
private:
	volatile int m_nSendThreadStopProcessSession;		//	发送线程停止处理Session
	volatile int m_nRecvThreadStopProcessSession;		//	接收线程停止处理Session
	volatile int m_nMainLogicThreadStopProcessSession;	//	主线程停止处理Session
	volatile int m_nEpollThreadStopProcessSession;		//	Epoll线程停止处理Session
	volatile int m_nCloseWorkSendedToCloseThread; 		//	关闭工作已经发往closethread，不用再发送了
#endif

public:
	void RecvData(LRecvThread* pRecvThread);
	void ResetRecv();
private:
	char m_szRecvedContent[MAX_PACKET_SIZE + 4];
	unsigned int m_unCurrentContentLen;

public:
	t_Epoll_Bind_Param* GetEpollBindParam()
	{
		return &m_EpollBindParam;
	}
	void SetEpollBindParam(uint64_t u64SessionID, int nSocket)
	{
		m_EpollBindParam.u64SessionID 	= u64SessionID;
		m_EpollBindParam.nSocket		= nSocket;
	}
	bool AddToEpollWaitThread(LRecvThread* pRecvThread);
private:
	t_Epoll_Bind_Param m_EpollBindParam;

public:			//	 fa song xiang guan
	void SetCanSend(bool bValue)
	{ 
		m_bCanSend = bValue;
	}
	bool GetCanSend()
	{
		return m_bCanSend;
	}
private:
	bool m_bCanSend;

#ifdef __ADD_SEND_BUF_CHAIN__
public:
	//	发送数据，首先判断发送缓冲区是否有数据，如果有那么，先加入队列，然后从队列里面
	//	取出来发送；如果没有，那么直接发送，如果发送返回缓冲区满，那么将该数据包添加到队列
	//	等待下次发送
	//	如果函数返回成功，那么在发送线程就不需要释放该数据包 
	int SendPacket(LPacketBroadCast* pSendPacket, LSendThread* pSendThread);
	//	初始化发送缓冲区
	//	unSendBufChainSize 发送缓冲区最大的缓冲包的数量  
	bool InitializeSendBufChain(unsigned int unSendBufChainSize);
	//	如果连接要关闭，那么需要释放尚未发送的数据包
	void ReleaseSendPacketBufChain(LCloseSocketThread* pCloseThread);
	//	发送发送缓冲区的数据,当EPOLL线程返回该连接可以发送数据时，发送在发送缓存中的数据
	int SendPacketInSendBufChain(LSendThread* pSendThread);
	//	如果套接字发送缓存满了，那么返回true
	bool GetSendBufIsFull();
private:
	//	发送队列，在套接字缓冲区满时，缓冲发送的数据
	LFixLenCircleBuf m_SendBufChain;
	//	队列保护器,防止发送线程和接收线程同时访问发送队列,两者的冲突很小,对性能的影响应该不大
	pthread_mutex_t m_MutexForSendBufChain;
	//	标志目前发送区满，那么在接收线程注册Epoll事件时，需要将EPOLLOUT一起注册
	atomic_t m_SendBufIsFull;
	//	如果一段时间没有收到数据包，那么，需要断开连接	
#endif

public:
	time_t GetSessionConnecttedTime()
	{
		return m_tSessionConnectted;
	}
	void SetSessionConnecttedTime()
	{
		m_tSessionConnectted = time(NULL);
	}
	int GetLastRecvTime()
	{
		return __sync_add_and_fetch(&m_nLastRecvTime, 0);
	}
	void UpdateLastRecvTime()
	{
		time_t tNow = time(NULL);
		int nLastRecvTime = tNow - m_tSessionConnectted;
		__sync_lock_test_and_set(&m_nLastRecvTime, nLastRecvTime);
	}
	int GetLastSendTime()
	{
		return __sync_add_and_fetch(&m_nLastSendTime, 0);
	}
	void UpdateLastSendTime()
	{ 
		time_t tNow = time(NULL);
		int nLastSendTime = tNow - m_tSessionConnectted;
		__sync_lock_test_and_set(&m_nLastSendTime, nLastSendTime);
	}
	bool GetIpAndPort(char* pszBuf, unsigned short usbufLen);
private:
	time_t m_tSessionConnectted;	// 连接建立的时间
	volatile int m_nLastRecvTime;		//	最后一次接收数据的时间
	volatile int m_nLastSendTime;		//	最后一次发送数据的时间

public:
	//	释放Session占用的资源
	void ReleaseSessionResource();
#ifdef __ADD_SEND_BUF_CHAIN__
public:
	int PushSendPacketToSessionSendChain(LPacketBroadCast* pSendPacket, LSendThread* pSendThread);
	int SendPacketInSendBufChainV2(LSendThread* pSendThread);
	int SendPacketInSendBufChainUseSendThreadBuf(LSendThread* pSendThread);
	inline int GetIsClosing()
	{
		return m_nIsClosing;
	}
	inline void SetIsClosing(int nClosingValue)
	{
		 __sync_lock_test_and_set(&m_nIsClosing, nClosingValue);
	}
private:
	//	连接是否处于关闭中，主要涉及连接发送队列的释放问题
	volatile int m_nIsClosing;
#endif

#ifdef	__USE_SESSION_BUF_TO_SEND_DATA__
public:
	bool AppendDataToSend(LPacketSingle* pPacket);
	int SendDataInFixCircleSendBuf(LSendThread* pSendThread, bool bFromEpollOutEvent);
	bool GetSendBufIsFull();
private:
	LFixLenCircleBuf m_FixLenCircleBufForSendData;
	volatile int m_nSendBufIsFull;
#endif
};

#endif

