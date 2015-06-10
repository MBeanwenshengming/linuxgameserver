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

#include "LNetWorkServicesIdleSendPacket_Recycle.h"
#include "LNetWorkServices.h"

#ifdef __EPOLL_TEST_STATISTIC__
extern volatile int g_nSendPacketIdleThreadFreeCount;
#endif
#ifdef __ADD_SEND_BUF_CHAIN__
LNetWorkServicesIdleSendPacketRecycle::LNetWorkServicesIdleSendPacketRecycle()
{
	m_pNetWorkServices = NULL;
	m_nFreePacketCount = 0;
}
LNetWorkServicesIdleSendPacketRecycle::~LNetWorkServicesIdleSendPacketRecycle()
{
}
//	初始化工作队列和本地缓存
//	spd 指定缓冲池的类型
//	usspdCount 缓冲池类型的数量
//	unMaxLocalCountForppd 本地缓冲池放置的最大数目
bool LNetWorkServicesIdleSendPacketRecycle::InitializeIdleSendPacketRecycle(unsigned int unWorkItemMaxCount, t_Packet_Pool_Desc spd[], unsigned short usspdCount, unsigned int unMaxLocalCountForppd)
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	if (unWorkItemMaxCount == 0)
	{
		unWorkItemMaxCount = 10000;
	}
	if (usspdCount == 0)
	{
		return false;
	}
	if (unMaxLocalCountForppd == 0 || unMaxLocalCountForppd > 500)
	{
		unMaxLocalCountForppd = 20;
	}

	//	 初始化本地缓冲池
	t_Packet_Pool_Desc* pLocalppd = new t_Packet_Pool_Desc[usspdCount];
	if (pLocalppd == NULL)
	{
		return false;
	}
	memcpy(pLocalppd, spd, sizeof(t_Packet_Pool_Desc) * usspdCount);
	//	改变分配大小
	for (unsigned int unIndex = 0; unIndex < usspdCount; ++unIndex)
	{
		pLocalppd[unIndex].unInitSize 		= 0;
		pLocalppd[unIndex].unMaxAllocSize 	= unMaxLocalCountForppd;
	}
	if (!m_LocalPoolForSendPacket.Initialize(pLocalppd, usspdCount))
	{
		return false;
	}
	delete[] pLocalppd;

	//	初始化工作队列
	if (!m_FixCircleBufForWorkQueue.Initialize(sizeof(LPacketBroadCast*), unWorkItemMaxCount))
	{
		return false;
	}
	return true;
}

int LNetWorkServicesIdleSendPacketRecycle::ThreadDoing(void* pParam)
{
	while (1)
	{
		if (CheckForStop())
		{
			break;
		}
		
		LPacketBroadCast* pPacketForFree = NULL;
		E_Circle_Error error = m_FixCircleBufForWorkQueue.GetOneItem((char*)&pPacketForFree, sizeof(LPacketBroadCast*));
		if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
		{
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		else
		{
			if (pPacketForFree->DecrementRefCountAndResultIsTrue())
			{
				//	释放到本地缓存
				AddPacketToFreePool(pPacketForFree);
			}
		}
	}
	return 0;
}

bool LNetWorkServicesIdleSendPacketRecycle::AddPacketToFreePool(LPacketBroadCast* pPacket)
{
	pPacket->Reset();

#ifdef __EPOLL_TEST_STATISTIC__
//	atomic_inc(&g_nSendPacketIdleThreadFreeCount);
	__sync_add_and_fetch(&g_nSendPacketIdleThreadFreeCount, 1);
	pPacket->FillPacketForTest();
#endif
	if (!m_LocalPoolForSendPacket.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen()))
	{ 
		if (!m_pNetWorkServices->CommitFreePacketToGlobalSendPool(&m_LocalPoolForSendPacket, pPacket->GetPacketBufLen()))
		{
			char szError[512];
			sprintf(szError, "LNetWorkServicesIdleSendPacketRecycle::AddPacketToFreePool, m_pNetWorkServices->CommitFreePacketToGlobalSendPool Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			
			//	删除数据包
			m_nFreePacketCount++;
			delete pPacket;
			return true;
		}
		//	再提交一次，不成功就删除
		if (!m_LocalPoolForSendPacket.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen()))
		{
			char szError[512];
			sprintf(szError, "LNetWorkServicesIdleSendPacketRecycle::AddPacketToFreePool, m_LocalBroadCastPacketForFree.FreeOneItemToPool Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));

			m_nFreePacketCount++;
			delete pPacket;
			return true;
		}
	}
	return true;
} 

bool LNetWorkServicesIdleSendPacketRecycle::OnStart()
{
	return true;
}
void LNetWorkServicesIdleSendPacketRecycle::OnStop()
{
	return;
}


void LNetWorkServicesIdleSendPacketRecycle::SetNetWorkServices(LNetWorkServices* pNetWorkServices) 
{
	m_pNetWorkServices = pNetWorkServices;
}

bool LNetWorkServicesIdleSendPacketRecycle::StopNetWorkServicesIdleSendPacketRecycleThread()
{
	Stop();

	pthread_t pID = GetThreadHandle();
	if (pID == 0)
	{
		return false;
	}
	int nJoinRes = pthread_join(pID, NULL);
	if (nJoinRes != 0)
	{
		return false;
	}
	return true;
}

void LNetWorkServicesIdleSendPacketRecycle::ReleaseNetWorkServicesIdleSendPacketRecycleThreadResouces()
{
	if (m_FixCircleBufForWorkQueue.GetCurrentExistCount() != 0)
	{
		while (1)
		{
			LPacketBroadCast* pPacketForFree = NULL;
			E_Circle_Error error = m_FixCircleBufForWorkQueue.GetOneItem((char*)&pPacketForFree, sizeof(LPacketBroadCast*));
			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				if (pPacketForFree->DecrementRefCountAndResultIsTrue())
				{
					//	引用计数减1,防止多次释放
					delete pPacketForFree; 
				}
			}
		}
	}

	m_LocalPoolForSendPacket.ReleasePacketPoolManagerResource();
}

//	将主线程的idlepacket提交到本处理线程
bool LNetWorkServicesIdleSendPacketRecycle::PushIdleSendPacket(LFixLenCircleBuf* pFixCircleBuf)
{
	if (pFixCircleBuf == NULL)
	{
		return false;
	}
	return pFixCircleBuf->CopyAllItemsToOtherFixLenCircleBuf(&m_FixCircleBufForWorkQueue);
}
#endif
