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

#pragma once

#include "IncludeHeader.h"
#include "LThreadBase.h"
#include "LPacketPoolManager.h"
#include "LPacketBroadCast.h"

class LNetWorkServices;
#ifdef __ADD_SEND_BUF_CHAIN__
//	回收请求但未被使用发送数据包，送回内存池中
class LNetWorkServicesIdleSendPacketRecycle : public LThreadBase
{
public:
	LNetWorkServicesIdleSendPacketRecycle();
	~LNetWorkServicesIdleSendPacketRecycle();
public:
	//	初始化工作队列和本地缓存
	//	spd 指定缓冲池的类型
	//	usspdCount 缓冲池类型的数量
	//	unMaxLocalCountForppd 本地缓冲池放置的最大数目
	bool InitializeIdleSendPacketRecycle(unsigned int unWorkItemMaxCount, t_Packet_Pool_Desc spd[], unsigned short usspdCount, unsigned int unMaxLocalCountForppd);

public:		//	线程相关
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
public:		//	线程停止，资源释放相关
	void SetNetWorkServices(LNetWorkServices* pNetWorkServices);
	bool StopNetWorkServicesIdleSendPacketRecycleThread();
	void ReleaseNetWorkServicesIdleSendPacketRecycleThreadResouces();

public:
	//	将主线程的idlepacket提交到本处理线程
	bool PushIdleSendPacket(LFixLenCircleBuf* pFixCircleBuf);
	int GetPacketFreeItemCount()
	{
		return m_LocalPoolForSendPacket.GetCurrentContentCount();
	}
private: 
	bool AddPacketToFreePool(LPacketBroadCast* pPacket);

	LNetWorkServices* m_pNetWorkServices;

	//	本线程的工作队列
	LFixLenCircleBuf m_FixCircleBufForWorkQueue;
	//	本地临时缓存的发送数据包的数量，一次提交多个数据包，各线程之间减少锁冲突
	LPacketPoolManager<LPacketBroadCast> m_LocalPoolForSendPacket;
	int m_nFreePacketCount;
};
#endif

