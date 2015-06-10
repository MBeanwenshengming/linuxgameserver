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

#ifndef __SEND_THREAD_HEADER_DEFINED__
#define __SEND_THREAD_HEADER_DEFINED__
#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"
#include "LPacketBroadCast.h"
#include "LPacketPoolManager.h"
#include <map>
using namespace std;

class LPacketBase;
class LNetWorkServices;
class LPacketBroadCast;
class LSession;

#define SEND_THREAD_SEND_BUF_LEN 128 * 1024

#ifndef __USE_SESSION_BUF_TO_SEND_DATA__
typedef struct _Send_Content_Desc
{
	uint64_t u64SessionID;
	LPacketBroadCast* pPacket;
}t_Send_Content_Desc;
#else
typedef struct _Send_Content_Desc
{
	uint64_t u64SessionID;
}t_Send_Content_Desc;
#endif

typedef struct _Send_Epoll_Out_Event
{
	uint64_t u64SessionID;
}t_Send_Epoll_Out_Event;


class LSendThread :
	public LThreadBase
{
public:
	LSendThread(void);
	virtual ~LSendThread(void);
public:

	//	unSendWorkItemCount 发送队列的长度，这里描述了需要发送的连接和数据包
	//	spd 发送数据包释放时，本地的缓存，
	//	usspdCount 缓存描述的数量
	//	unEpollOutEventMaxCount 发送线程接收的EPOLLOUT事件的最大数量
	bool Initialize(unsigned int unSendWorkItemCount, t_Packet_Pool_Desc spd[], unsigned short usspdCount, unsigned int unEpollOutEventMaxCount = 0);
	bool CommitSendWorkItems(t_Send_Content_Desc* parrSendContentDesc, unsigned int unArrSize);
	bool GetSendWorkItem(t_Send_Content_Desc* pSendContentDesc);

	bool CommitAllSendWorkItems(LFixLenCircleBuf* pFixLenSendPool);
public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop(); 
private:
	LFixLenCircleBuf m_SendCircleBuf;
public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices);
	LNetWorkServices* GetNetWorkServices();	
	int m_nThreadID;
private:
	LNetWorkServices* m_pNetWorkServices;

#ifdef __ADD_SEND_BUF_CHAIN__
public:
	bool AddPacketToFreePool(LPacketBroadCast* pPacket);
private:
	LPacketPoolManager<LPacketBroadCast> m_LocalBroadCastPacketForFree;
	t_Packet_Pool_Desc* m_pArrayForLocalFreePoolDesc;
	unsigned int m_unppdForLocalFreePoolCount;
#endif

public:
#ifdef __ADD_SEND_BUF_CHAIN__
	int GetCurrentFreePacketPoolItemCount()
	{
		return m_LocalBroadCastPacketForFree.GetCurrentContentCount();
	}
#endif

	int m_nRealSendCount;
public:
	void PrintSendThreadLocalBufStatus();
private:
	time_t m_tLastWriteErrorTime; 
	LErrorWriter m_ErrorWriterForSendThreadStatus;

	int m_nFreePacketCount;

public:
	bool PushOneEpollOutEvent(t_Send_Epoll_Out_Event& sdd);
	bool GetOneEpollOutEvent(t_Send_Epoll_Out_Event& sdd);
	//	一次性提交多个EPOLLOUT事件
	bool PushEpollOutEvent(char* pbuf, unsigned int unEventCount);
private:
	pthread_mutex_t m_MutexForEpollOutCircleBuf;
	LFixLenCircleBuf m_SendEpollOutCircleBuf;	//	EPOLL发送事件的环形缓冲区，各个epoll线程添加事件，发送线程取出事件并且发送数据

public:
	void ReleaseSendThreadResource();
#ifdef __ADD_SEND_BUF_CHAIN__
public:
	//	每个套接字用这个缓冲拷贝数据包数据，然后用这个缓冲发送数据
	char m_szSendThreadBuf[SEND_THREAD_SEND_BUF_LEN];
	//	m_szSendThreadBuf中已经存在的数据长度
	unsigned int m_unCurrentSendThreadBufDataLen;
private:
	//	本次Process处理过程中，需要发送数据的连接
	map<uint64_t, uint64_t> m_mapWillSendPacketSessionID;
#endif

public:
	//	加入一个需要处理的即将关闭的连接
	void AddWillCloseSessionInSendThread(LSession* pSession);
	//	取一个出来处理
	bool GetOneWillCloseSessionInSendThread(LSession** pSession);

	//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
	void ProcessSendDataErrorToCloseSession(LSession* pSession);
private:
	LFixLenCircleBuf m_FixCircleBufWillSessionCloseToProcessSendThread;
};
#endif


