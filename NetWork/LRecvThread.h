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

#ifndef __RECV_THREAD_HEADER_DEFINED__
#define __RECV_THREAD_HEADER_DEFINED__
#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"
#include "LPacketPoolManager.h"
#include "LPacketSingle.h"

//	
extern size_t g_sRecvCircleBufItemCount;
class LNetWorkServices;

typedef struct _Recv_WorkItem
{
	uint64_t u64SessionID;		//	xu yao jie shou de session de id
}t_Recv_WorkItem;
typedef struct _Recv_Packet
{
	uint64_t u64SessionID;
	LPacketSingle* pPacket;
}t_Recv_Packet;

class LSession;

class LRecvThread :
	public LThreadBase
{
public:
	LRecvThread(void);
	virtual ~LRecvThread(void);
public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop(); 
public:

	//	unRecvWorkItemCount epollin事件环形队列的最大队列数，大于该数量，那么EPOLLIN事件无法放入接收线程处理
	//ppdForLocalPool 本地接收数据包缓冲的大小，初始化本地缓存，减少锁冲突
	//unRecvppdForLocalPoolCount 数组的数量
	//unRecvLocalPacketPoolSize本地接收到的数据包缓冲的数量，达到一定数量提交，或者本地的线程完成一次循环，就提交
	bool Initialize(unsigned int unRecvWorkItemCount, t_Packet_Pool_Desc ppdForLocalPool[], unsigned int unRecvppdForLocalPoolCount, unsigned int unRecvLocalPacketPoolSize);
	bool AddWorkItems(char* pbuf, size_t sWorkItemCount);
	bool GetOneItem(t_Recv_WorkItem& workItem);
	
private:
	//	bao zheng dan ge xian cheng xie huan xing huan chong qu 
	pthread_mutex_t m_MutexForWriteCircleBuf;
	//	 gu ding yuan su de huan xing huan chong qu
	LFixLenCircleBuf m_FixLenBuf;

	t_Packet_Pool_Desc* m_pArrayppdForLocalPool;
	unsigned int m_unppdForLocalPoolDescCount;	


public:
	void SetNetServices(LNetWorkServices* pNetWorkServices);
	LNetWorkServices* GetNetServices()
	{
		return m_pNetWorkServices;
	}
	int m_nPacketRecved;
	int m_nThreadID;
private:
	LNetWorkServices* m_pNetWorkServices;

public:		// For Test
	void PrintRecvThreadStatus();
private:
	time_t m_tLastWriteBufDescTime;
	LErrorWriter m_ErrorWriterForRecvThreadStatus;

public:		// packet for recv
	LPacketSingle* GetOneFreePacket(unsigned short usPacketLen);
	bool AddPacketToLocal(uint64_t u64SessionID, LPacketSingle* pPacket);
	bool CommitLocalPacketToGlobalPacketPool();
private:
	LPacketPoolManager<LPacketSingle> m_PacketPoolManagerForRecv;
	LFixLenCircleBuf m_FixBufRecvedPacket;

public:
	void ReleaseRecvThreadResource();

	//	即将关闭的连接，需要处理这个消息，设置已经处理为true
public:
	//	加入一个需要处理的即将关闭的连接
	void AddWillCloseSession(LSession* pSession);
	//	取一个出来处理
	bool GetOneWillCloseSession(LSession** pSession);

	//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
	void ProcessRecvDataErrorToCloseSession(LSession* pSession);
private:
	LFixLenCircleBuf m_FixBufWillCloseSessionToProcess;
};
#endif

