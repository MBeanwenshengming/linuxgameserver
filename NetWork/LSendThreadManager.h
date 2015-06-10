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

#ifndef __LINUX_SEND_THREAD_MANAGER_HEADER_DEFINED__
#define __LINUX_SEND_THREAD_MANAGER_HEADER_DEFINED__
#include "IncludeHeader.h"
#include "LPacketPoolManager.h"

class LSendThread;
class LSession;
class LNetWorkServices;

typedef struct _Send_Thread_Desc
{
	volatile int nRefCount;
	LSendThread* pSendThread;
	_Send_Thread_Desc()
	{
		__sync_lock_test_and_set(&nRefCount, 0);
		pSendThread = NULL;
	} 
}t_Send_Thread_Desc;

class LSendThreadManager
{
public:
	LSendThreadManager();
	~LSendThreadManager();
public:

	//	unSendThreadCount 发送线程的数量
	//	unSendWorkItemCount 发送队列的长度，这里描述了需要发送的连接和数据包
	//	spd 发送数据包释放时，本地的缓存，
	//	usspdCount 缓存描述的数量
	//	unEpollOutEventMaxCount 发送线程接收的EPOLLOUT事件的最大数量
	bool Initialize(unsigned int unSendThreadCount, unsigned int unSendWorkItemCount, t_Packet_Pool_Desc spd[], unsigned short usspdCount, unsigned int unEpollOutEventMaxCount);
	LSendThread* GetSendThread(unsigned int unThreadID);

	bool StartAllSendThread();
	void StopAllSendThread();
public:
	bool BindSendThread(LSession* pSession);
	void UnBindSendThread(LSession* pSession);
	int GetCurrentPacketFreePoolItemCount();
protected:
	int SelectSendThread();
private:
	t_Send_Thread_Desc* m_parrSendThreadDesc;
	unsigned int m_unSendThreadCount;
public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices); 
	void ReleaseSendThreadManagerResource();
private:
	LNetWorkServices* m_pNetWorkServices;

public: // for Test
	void PrintSendThreadRefStatus(); 
};

#endif

