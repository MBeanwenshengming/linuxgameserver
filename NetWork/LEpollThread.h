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

#ifndef __EPOLL_THREAD_HEADER_DEFINED__
#define __EPOLL_THREAD_HEADER_DEFINED__

#include "LThreadBase.h"
#include "LRecvThread.h"
#include "LNetWorkServices.h"
#include "IncludeHeader.h"
#include "LSendThread.h"

extern size_t g_sEpollWorkItemCount;
extern unsigned int g_unRecvThreadCount;
extern unsigned int g_unWaitClientPerEpoll;

typedef struct _Local_Work_Item_Desc
{
	unsigned int unWorkItemCount;
	t_Recv_WorkItem* parrWorkItem; 
	_Local_Work_Item_Desc()
	{
		unWorkItemCount = 0;
		parrWorkItem = NULL;
	} 
}t_Local_Work_Item_Desc;


typedef struct _Local_Work_Item_For_Send_Desc
{
	unsigned int unItemCount;
	t_Send_Epoll_Out_Event* parrWorkItem;
	_Local_Work_Item_For_Send_Desc()
	{
		unItemCount = 0;
		parrWorkItem = NULL;
	}
}t_Local_Work_Item_For_Send_Desc;

class LEpollThread :
	public LThreadBase
{
public:
	LEpollThread(void);
	virtual ~LEpollThread(void);
public:

	//	unRecvThreadCount 接收线程的数量
	//	unRecvThreadWorkItemCount 可以放置的最大EPOLLIN事件数量,本地缓存的EPOLLIN事件数量，一次提交给接收线程
	//	unSendThreadCount 发送线程的数量
	//	unSendThreadWorkItemCount 每个发送线程本地缓存EPOLLOUT事件的数量，一次性提交给发送线程
	//	unWaitClientSizePerEpoll 每个EPOLL上监听的套接字数量, 创建epoll时使用
	bool Initialize(unsigned int unRecvThreadCount, unsigned int unRecvThreadWorkItemCount, unsigned int unSendThreadCount, unsigned int unSendThreadWorkItemCount, unsigned int unWaitClientSizePerEpoll);
public:
	virtual int ThreadDoing(void* pParam);
	int GetEpollHandle();

	void ReleaseEpollThreadResource();

	int m_nThreadID;
public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices);
	bool CommitSingleLocalWorkItem(unsigned int unRecvThreadID);
	bool CommitLocalWorkItem();
#ifdef _DEBUG
	void DumpEpollTheadInfo();
#endif
protected:
	bool PushRecvWorkItem(unsigned int unRecvThreadID, uint64_t u64SessionID);
private:
	//	shu zu shu liang wei jie shou xian cheng de shu liang
	t_Local_Work_Item_Desc* m_parrLocalWorkItemDesc;

	int m_nEpollThreadHandle;
	struct epoll_event* m_pEvents;
	int m_nMaxEvents;
	unsigned int m_unRecvThreadCount;		//	接收线程的数量
	unsigned int m_unRecvThreadWorkItemCount;	//	对应每个接收线程本地存放的workItem数量
	LNetWorkServices* m_pNetWorkServices;

#ifdef  __ADD_SEND_BUF_CHAIN__
public:
	bool PushSendWorkItem(unsigned int unSendThreadID, uint64_t u64SessionID);
	bool CommitSingleLocalSendItem(unsigned int unSendThreadID);
	bool CommitAllSendWorkItem();
private:
	unsigned int m_unSendThreadCount;	//	发送线程的数量
	unsigned int m_unSendThreadWorkItemCount;		//	对应每个发送线程本地存放的workItem数量
	t_Local_Work_Item_For_Send_Desc* m_parrLocalSendWorkItemDesc;	//	发送任务的本地缓存
#endif
#ifdef  __USE_SESSION_BUF_TO_SEND_DATA__
public:
	bool PushSendWorkItem(unsigned int unSendThreadID, uint64_t u64SessionID);
	bool CommitSingleLocalSendItem(unsigned int unSendThreadID);
	bool CommitAllSendWorkItem();
private:
	unsigned int m_unSendThreadCount;	//	发送线程的数量
	unsigned int m_unSendThreadWorkItemCount;		//	对应每个发送线程本地存放的workItem数量
	t_Local_Work_Item_For_Send_Desc* m_parrLocalSendWorkItemDesc;	//	发送任务的本地缓存

public:
	//	加入一个需要处理的即将关闭的连接
	void AddWillCloseSessionInEpollThread(LSession* pSession);
	//	取一个出来处理
	bool GetOneWillCloseSessionInEpollThread(LSession** pSession);

	//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
	void ProcessCloseSessionInEpollThread(LSession* pSession);
private:
	LFixLenCircleBuf m_FixBufWillCloseSessionToProcessInEpollThread;
#endif
};
#endif


