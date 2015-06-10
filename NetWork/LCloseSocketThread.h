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

#ifndef __LINUX_CLOSE_THREAD_HEADER_INCLUDED_DEFINED__
#define __LINUX_CLOSE_THREAD_HEADER_INCLUDED_DEFINED__

#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"
#include "LPacketBroadCast.h"
#include "LPacketPoolManager.h"

class LNetWorkServices;

typedef struct _Client_Need_To_Close
{
	uint64_t u64SessionID;
}t_Client_Need_To_Close;

class LCloseSocketThread : public LThreadBase
{
public:
	LCloseSocketThread();
	~LCloseSocketThread();
public:

	//	unCloseWorkItemCount 最大可以提交的关闭事件数量
	//	ppdForLocalPool 连接关闭时，释放该连接下没有发送的发送数据包到本地缓存池，达到一定数量时，提交到全局缓冲池
	//	unppdForLocalPoolCount 描述信息数组的长度
	bool Initialize(unsigned int unCloseWorkItemCount, t_Packet_Pool_Desc ppdForLocalPool[], unsigned int unppdForLocalPoolCount);
	bool AppendToClose(t_Client_Need_To_Close ClientToClose);
public: 
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop(); 
public:
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices)
	{
		m_pNetWorkServices = pNetWorkServices;
	}
private:
	pthread_mutex_t m_MutexToProtectedWriteCloseInfo;

	LFixLenCircleBuf m_BufSessionNeedToClose;

	LNetWorkServices* m_pNetWorkServices;
	unsigned int m_unCloseWorkItemCount;
public:
	void StopCloseSocketThread();
		
	void ReleaseCloseSocketThreadResource();

private:
	LFixLenCircleBuf m_FixBufToCommitSessionDisconnect;

#ifdef __ADD_SEND_BUF_CHAIN__
public:
	//	释放连接的数据包
	bool AddPacketToLocalPool(LPacketBroadCast* pPacket);
	int GetPacketFreeItemCount()
	{
		m_LocalBroadCastPacketForFree.GetCurrentContentCount();
	}
private: 
	//	再关闭连接时，释放该连接下，还没有释放的发送数据包
	LPacketPoolManager<LPacketBroadCast> m_LocalBroadCastPacketForFree;
	int m_nFreePacketCount; //	记录删除的数据包数量	
#endif
};

#endif

