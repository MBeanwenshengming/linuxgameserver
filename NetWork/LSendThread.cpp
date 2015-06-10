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

#include "LSendThread.h"
#include "LSessionManager.h"
#include "LSession.h"
#include "LNetWorkServices.h"
#include "LPacketBroadCast.h"
#include "LCloseSocketThread.h" 
#include "LErrorWriter.h"
#include <sys/prctl.h>

extern LErrorWriter g_ErrorWriter;

//extern int errno;

#ifdef __EPOLL_TEST_STATISTIC__
extern volatile int g_nSendPacketFreeCount;
#endif
LSendThread::LSendThread(void)
{
	m_pNetWorkServices = NULL;
	m_nThreadID = -1;
	m_nRealSendCount = 0;
	m_tLastWriteErrorTime = 0;
	m_nFreePacketCount = 0;

#ifdef __ADD_SEND_BUF_CHAIN__
	m_pArrayForLocalFreePoolDesc = NULL;
	m_unppdForLocalFreePoolCount = 0;	

	pthread_mutex_init(&m_MutexForEpollOutCircleBuf, NULL);
#endif
}

LSendThread::~LSendThread(void)
{ 
#ifdef __ADD_SEND_BUF_CHAIN__
	pthread_mutex_destroy(&m_MutexForEpollOutCircleBuf);
#endif
}

//	unSendWorkItemCount 发送队列的长度，这里描述了需要发送的连接和数据包
//	spd 发送数据包释放时，本地的缓存，
//	usspdCount 缓存描述的数量
//	unEpollOutEventMaxCount 发送线程接收的EPOLLOUT事件的最大数量
bool LSendThread::Initialize(unsigned int unSendWorkItemCount, t_Packet_Pool_Desc spd[], unsigned short usspdCount, unsigned int unEpollOutEventMaxCount)
{
	if (m_pNetWorkServices == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThread::Initialize, m_pNetWorkServices == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	if (unSendWorkItemCount == 0)
	{
		char szError[512];
		sprintf(szError, "LSendThread::Initialize, m_pNetWorkServices == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_SendCircleBuf.Initialize(sizeof(t_Send_Content_Desc), unSendWorkItemCount))
	{
		char szError[512];
		sprintf(szError, "LSendThread::Initialize, m_SendCircleBuf.Initialize Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#ifndef __USE_SESSION_BUF_TO_SEND_DATA__
	if (!m_LocalBroadCastPacketForFree.Initialize(spd, usspdCount))
	{
		char szError[512];
		sprintf(szError, "LSendThread::Initialize, m_LocalBroadCastPacketForFree.Initialize Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_unppdForLocalFreePoolCount = usspdCount;
	m_pArrayForLocalFreePoolDesc = new t_Packet_Pool_Desc[m_unppdForLocalFreePoolCount];
	if (m_pArrayForLocalFreePoolDesc == NULL)
	{
		return false;
	}
	memcpy(m_pArrayForLocalFreePoolDesc, spd, sizeof(t_Packet_Pool_Desc) * m_unppdForLocalFreePoolCount);


	m_LocalBroadCastPacketForFree.SetReuqestPoolFromGlobalManager(m_pNetWorkServices->GetSendGlobalPool());
#endif

	if (unEpollOutEventMaxCount == 0)
	{
		char szError[512];
		sprintf(szError, "LSendThread::Initialize, unEpollOutEventMaxCount == 0, Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_SendEpollOutCircleBuf.Initialize(sizeof(t_Send_Epoll_Out_Event), unEpollOutEventMaxCount))
	{
		char szError[512];
		sprintf(szError, "LSendThread::Initialize, m_SendEpollOutCircleBuf.Initialize, Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_FixCircleBufWillSessionCloseToProcessSendThread.Initialize(sizeof(LSession*), unSendWorkItemCount))
	{
		char szError[512];
		sprintf(szError, "LSendThread::Initialize, m_FixCircleBufWillSessionCloseToProcessSendThread.Initialize, Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	//	for Test
	char szFileName[256];
	sprintf(szFileName, "SendThreadStatus%d.txt", m_nThreadID);
	if (!m_ErrorWriterForSendThreadStatus.Initialize(szFileName))
	{
		return false;
	}
	return true;
}
bool LSendThread::CommitSendWorkItems(t_Send_Content_Desc* parrSendContentDesc, unsigned int unArrSize)
{
	if (parrSendContentDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThread::CommitSendWorkItems, parrSendContentDesc == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (unArrSize == 0)
	{
		char szError[512];
		sprintf(szError, "LSendThread::CommitSendWorkItems, unArrSize == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (m_SendCircleBuf.AddItems((char*)parrSendContentDesc, unArrSize) != 0)
	{
		char szError[512];
		sprintf(szError, "LSendThread::CommitSendWorkItems, AddItems Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
bool LSendThread::GetSendWorkItem(t_Send_Content_Desc* pSendContentDesc)
{
	if (pSendContentDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThread::GetSendWorkItem, pSendContentDesc == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	E_Circle_Error errorCode = m_SendCircleBuf.GetOneItem((char*)pSendContentDesc, sizeof(t_Send_Content_Desc));
	if (errorCode != E_Circle_Buf_No_Error && errorCode != E_Circle_Buf_Is_Empty)
	{
		char szError[512];
		sprintf(szError, "LSendThread::GetSendWorkItem, m_SendCircleBuf.GetOneItem Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (errorCode == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}

bool LSendThread::CommitAllSendWorkItems(LFixLenCircleBuf* pFixLenSendPool)
{
	if (pFixLenSendPool == NULL)
	{
		char szError[512];
		sprintf(szError, "LSendThread::CommitAllSendWorkItems, pFixLenSendPool == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!pFixLenSendPool->CopyAllItemsToOtherFixLenCircleBuf(&m_SendCircleBuf))
	{
		char szError[512];
		sprintf(szError, "LSendThread::CommitAllSendWorkItems, pFixLenSendPool->CopyAllItemsToOtherFixLenCircleBuf Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
int LSendThread::ThreadDoing(void* pParam)
{
#ifndef __ADD_SEND_BUF_CHAIN__
//	t_Client_Need_To_Close cntc;
//
//	t_Send_Content_Desc SendWorkItem;
//	while(true)
//	{
//		memset(&SendWorkItem, 0, sizeof(SendWorkItem));
//		if (GetSendWorkItem(&SendWorkItem))
//		{
//			LPacketBroadCast* pSendPacket = SendWorkItem.pPacket;
//			if (pSendPacket == NULL)
//			{
//				char szError[512];
//				sprintf(szError, "LSendThread::ThreadDoing, Packet Is Null, SessionID:%lld\n", SendWorkItem.u64SessionID);
//				g_ErrorWriter.WriteError(szError, strlen(szError));
//				continue;
//			}
//			LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();
//			LSession* pSession = pMasterSessionManager->FindSession(SendWorkItem.u64SessionID);
//
//			if (pSession != NULL)
//			{
//				if (pSession->GetCanSend())
//				{
//					//	fa song shu ju bao
//					if (pSendPacket->GetBuf() != NULL)
//					{
//						if (pSession->Send(pSendPacket->GetBufBase(), pSendPacket->GetPacketDataAndHeaderLen()) < 0)
//						{
//							char szError[512];
//							sprintf(szError, "LSendThread::ThreadDoing, Send Failed, SessionID:%lld, ErrCode:%d\n", SendWorkItem.u64SessionID, errno);
//							g_ErrorWriter.WriteError(szError, strlen(szError));
//							pSession->SetCanSend(false);
//							//	guan bi client
//							cntc.u64SessionID = pSession->GetEpollBindParam()->u64SessionID;
//							m_pNetWorkServices->GetSendThreadManager().UnBindSendThread(pSession);
//							m_pNetWorkServices->GetCloseSocketThread().AppendToClose(cntc);
//							//	ti jiao guan bi shi jian
//						}
//						//m_nRealSendCount++;
//						//printf("SendThreadID:%d, RealSendCount:%d\n",m_nThreadID, m_nRealSendCount);
//					}
//					else
//					{
//						char szError[512];
//						sprintf(szError, "LSendThread::ThreadDoing, pSendPacket->GetBuf() == NULL, SessionID:%lld\n", SendWorkItem.u64SessionID);
//						g_ErrorWriter.WriteError(szError, strlen(szError));
//					}
//				}
//				pSession->UpdateLastSendTime();
//			}
//			else
//			{
//				char szError[512];
//				sprintf(szError, "LSendThread::ThreadDoing, SessionID NOt Found, SessionID:%lld\n", SendWorkItem.u64SessionID);
//				g_ErrorWriter.WriteError(szError, strlen(szError));
//			}
//			//	shi fang gai shu ju bao
//			if (pSendPacket->DecrementRefCountAndResultIsTrue())
//			{
//				//	shi fang fa song shu ju bao
//				AddPacketToFreePool(pSendPacket);
//			}
//		}
//		else
//		{
//			sched_yield();
//		}
//		if (CheckForStop())
//		{
//			break;
//		}
//		time_t timeNow = time(NULL);
//		if (timeNow - m_tLastWriteErrorTime > 30)  // 10  miao ji lu yi ci huan cun zhuang tai, ce shi shi yong
//		{
//			PrintSendThreadLocalBufStatus();
//			m_tLastWriteErrorTime = timeNow;
//		}
//	}
#else

	while(true)
	{
		bool bHaveEpollOutEvent = false;
		//	发送需要发送的数据包
		t_Client_Need_To_Close cntc;
		//	首先检查发送事件EPOLLOUT事件
		t_Send_Epoll_Out_Event seoe;
		int nEpollOutEventProcessed = 0;	//	处理固定数目的EPOLLOUT事件后，退出，
		for (;;)
		{
			if (GetOneEpollOutEvent(seoe))
			{
				bHaveEpollOutEvent = true;
				LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager(); 
				LSession* pSession = pMasterSessionManager->FindSession(seoe.u64SessionID);

				if (pSession != NULL)
				{
					//if (pSession->SendPacketInSendBufChain(this) < 0)
					if (pSession->SendPacketInSendBufChainUseSendThreadBuf(this) < 0)
					{
						char szError[512];
						sprintf(szError, "LSendThread::ThreadDoing, pSession->SendPacketInSendBufChain(this), SessionID:%lld\n", seoe.u64SessionID); 
						g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
						//	这里需要关闭套接字 
						cntc.u64SessionID = pSession->GetEpollBindParam()->u64SessionID;
						m_pNetWorkServices->GetSendThreadManager().UnBindSendThread(pSession);
						m_pNetWorkServices->GetCloseSocketThread().AppendToClose(cntc);
					}
					else
					{
						pSession->UpdateLastSendTime();
					}
				} 
				nEpollOutEventProcessed++;
				if (nEpollOutEventProcessed >= 300)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}

		LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();

		m_mapWillSendPacketSessionID.clear();
		bool bHaveSendWork = false;
		int nSendWorkProcessed = 0;
		t_Send_Content_Desc SendWorkItem;
		for (;;)
		{
			memset(&SendWorkItem, 0, sizeof(SendWorkItem));
			if (GetSendWorkItem(&SendWorkItem))
			{
//				nSendWorkProcessed++;
//				bHaveSendWork = true;
//				LPacketBroadCast* pSendPacket = SendWorkItem.pPacket;
//				if (pSendPacket == NULL)
//				{
//					char szError[512];
//					sprintf(szError, "LSendThread::ThreadDoing, Packet Is Null, SessionID:%lld\n", SendWorkItem.u64SessionID);
//					g_ErrorWriter.WriteError(szError, strlen(szError));
//					continue;
//				}
//				LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();
//				LSession* pSession = pMasterSessionManager->FindSession(SendWorkItem.u64SessionID);
//
//				if (pSession != NULL)
//				{
//					if (pSession->GetCanSend())
//					{
//						if (pSession->SendPacket(pSendPacket, this) == -1)
//						{
//							char szError[512];
//							sprintf(szError, "LSendThread::ThreadDoing, Send Failed, SessionID:%lld, ErrCode:%d\n", SendWorkItem.u64SessionID, errno);
//							g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
//							pSession->SetCanSend(false);
//							//	guan bi client
//							cntc.u64SessionID = pSession->GetEpollBindParam()->u64SessionID;
//							m_pNetWorkServices->GetSendThreadManager().UnBindSendThread(pSession);
//							m_pNetWorkServices->GetCloseSocketThread().AppendToClose(cntc);
//							//	ti jiao guan bi shi jian
//						}
//						//m_nRealSendCount++;
//						//printf("SendThreadID:%d, RealSendCount:%d\n",m_nThreadID, m_nRealSendCount);
//					}
//					else
//					{
//						//	检查引用计数，如果为0,那么收回数据包
//						if (pSendPacket->DecrementRefCountAndResultIsTrue())
//						{
//							AddPacketToFreePool(pSendPacket);
//						}
//					}
//					pSession->UpdateLastSendTime();
//				}
//				else
//				{
//					char szError[512];
//					sprintf(szError, "LSendThread::ThreadDoing, SessionID NOt Found, SessionID:%lld\n", SendWorkItem.u64SessionID);
//					g_ErrorWriter.WriteError(szError, strlen(szError));
//					//	检查引用计数，如果为0,那么收回数据包
//					if (pSendPacket->DecrementRefCountAndResultIsTrue())
//					{
//						AddPacketToFreePool(pSendPacket);
//					}
//				}
//				if (nSendWorkProcessed >= 500)
//				{
//					break;
//				}
				nSendWorkProcessed++;
				bHaveSendWork = true;
				LPacketBroadCast* pSendPacket = SendWorkItem.pPacket;
				if (pSendPacket == NULL)
				{ 
					char szError[512];
					sprintf(szError, "LSendThread::ThreadDoing, Packet Is Null, SessionID:%lld\n", SendWorkItem.u64SessionID); 
					g_ErrorWriter.WriteError(szError, strlen(szError));
					continue;
				}

				LSession* pSession = pMasterSessionManager->FindSession(SendWorkItem.u64SessionID);

				//	先将要发送的数据放入到连接的发送队列，然后将队列的数据拷贝到m_szSendThreadBuf中，然后用send发送
				//	这个改动的目的是，大量广播数据时 ，可以将数据拷贝到一起，节省send系统调用，原来逻辑是一个数据包使用
				//	一个send系统调用，测试后发现效率太低，大量广播数据存在时，连接经常因为数据无法及时发送出去而被关闭连接
				if (pSession != NULL)
				{
					if (pSession->GetIsClosing() != 1)
					{
						//	不在连接关闭状态中，那么将数据包放入连接的发送队列
						int nPushResult = pSession->PushSendPacketToSessionSendChain(pSendPacket, this);
						if (nPushResult == 0)	//	加入成功，那么将sessionID添加到m_mapWillSendPacketSessionID中
						{
							m_mapWillSendPacketSessionID[SendWorkItem.u64SessionID] = SendWorkItem.u64SessionID;
						}
						else
						{
							if (nPushResult == -1)
							{
								//	投递关闭连接的工作到关闭连接线程
								char szError[512];
								sprintf(szError, "LSendThread::ThreadDoing, nPushResult == -1, SessionID:%lld\n", SendWorkItem.u64SessionID);
								g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
								//	这里需要关闭套接字
								cntc.u64SessionID = pSession->GetEpollBindParam()->u64SessionID;
								m_pNetWorkServices->GetSendThreadManager().UnBindSendThread(pSession);
								m_pNetWorkServices->GetCloseSocketThread().AppendToClose(cntc);
							}
							//	检查引用计数，如果为0,那么收回数据包
							if (pSendPacket->DecrementRefCountAndResultIsTrue())
							{
								AddPacketToFreePool(pSendPacket);
							}
						}
					}
					else		//	连接关闭中，那么直接释放这个数据包，不用再发送了
					{ 
						//	检查引用计数，如果为0,那么收回数据包
						if (pSendPacket->DecrementRefCountAndResultIsTrue())
						{
							AddPacketToFreePool(pSendPacket);
						}
					}
					pSession->UpdateLastSendTime();
				}
				else 
				{
					char szError[512];
					sprintf(szError, "LSendThread::ThreadDoing, SessionID NOt Found, SessionID:%lld\n", SendWorkItem.u64SessionID); 
					g_ErrorWriter.WriteError(szError, strlen(szError));
					//	检查引用计数，如果为0,那么收回数据包
					if (pSendPacket->DecrementRefCountAndResultIsTrue())
					{
						AddPacketToFreePool(pSendPacket);
					}
				}
				if (nSendWorkProcessed >= 5000)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		map<uint64_t, uint64_t>::iterator _ito = m_mapWillSendPacketSessionID.begin();
		while (_ito != m_mapWillSendPacketSessionID.end())
		{
			LSession* pSession = pMasterSessionManager->FindSession(_ito->first);
			if (pSession != NULL)	//	如果此时为NULL，表示连接已经关闭，不需要再发送
			{
				if (pSession->GetIsClosing() == 0)	//	不等于0表示连接正在关闭，不需要再发送数据
				{
					int nSendResult = pSession->SendPacketInSendBufChainUseSendThreadBuf(this);
					if (nSendResult != 0)
					{
						//	投递关闭连接的工作到关闭连接线程
						char szError[512];
						sprintf(szError, "LSendThread::ThreadDoing, nSendResult == -1, SessionID:%lld\n", _ito->first);
						g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
						//	这里需要关闭套接字
						cntc.u64SessionID = pSession->GetEpollBindParam()->u64SessionID;
						m_pNetWorkServices->GetSendThreadManager().UnBindSendThread(pSession);
						m_pNetWorkServices->GetCloseSocketThread().AppendToClose(cntc);
					}
				}
			}
			_ito++;
		}
		if (!bHaveEpollOutEvent && !bHaveSendWork)	
		{
			//	sched_yield();
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		if (CheckForStop())
		{
			break;
		} 
		time_t timeNow = time(NULL);
		if (timeNow - m_tLastWriteErrorTime > 30)  // 	
		{
			PrintSendThreadLocalBufStatus();
			m_tLastWriteErrorTime = timeNow;
		}
	}
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	char szThreadName[128];
	sprintf(szThreadName, "SendThread_%d", m_nThreadID);
	prctl(PR_SET_NAME, szThreadName);

	while(true)
	{
		int nWillCloseSessionProcessed = 0;
		LSession* pWillCloseSession = NULL;
		while (GetOneWillCloseSessionInSendThread(&pWillCloseSession))
		{
			nWillCloseSessionProcessed++;
			if (pWillCloseSession != NULL)
			{
				ProcessSendDataErrorToCloseSession(pWillCloseSession);
			}
			if (nWillCloseSessionProcessed >= 100)
			{
				break;
			}
		}
		bool bHaveEpollOutEvent = false;
		//	发送需要发送的数据包
		t_Client_Need_To_Close cntc;
		//	首先检查发送事件EPOLLOUT事件
		t_Send_Epoll_Out_Event seoe;
		int nEpollOutEventProcessed = 0;	//	处理固定数目的EPOLLOUT事件后，退出，
		for (;;)
		{
			if (GetOneEpollOutEvent(seoe))
			{
				bHaveEpollOutEvent = true;
				LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();
				LSession* pSession = pMasterSessionManager->FindSession(seoe.u64SessionID);

				if (pSession != NULL)
				{
					if (pSession->GetCloseWorkSendedToCloseThread() == 0)
					{
						if (pSession->SendDataInFixCircleSendBuf(this, true) < 0)
						{
							char szError[512];
							sprintf(szError, "LSendThread::ThreadDoing, pSession->SendDataInFixCircleSendBuf(this), SessionID:%lld\n", seoe.u64SessionID);
							g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
							//	这里需要关闭套接字
							cntc.u64SessionID = pSession->GetEpollBindParam()->u64SessionID;
							m_pNetWorkServices->GetCloseSocketThread().AppendToClose(cntc);
						}
						else
						{
							pSession->UpdateLastSendTime();
						}
					}
				}
				nEpollOutEventProcessed++;
				if (nEpollOutEventProcessed >= 300)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		LMasterSessionManager* pMasterSessionManager = &m_pNetWorkServices->GetSessionManager();

		bool bHaveSendWork = false;
		int nSendWorkProcessed = 0;
		t_Send_Content_Desc SendWorkItem;
		for (;;)
		{
			memset(&SendWorkItem, 0, sizeof(SendWorkItem));
			if (GetSendWorkItem(&SendWorkItem))
			{
				nSendWorkProcessed++;
				bHaveSendWork = true;


				LSession* pSession = pMasterSessionManager->FindSession(SendWorkItem.u64SessionID);

				//	先将要发送的数据放入到连接的发送队列，然后将队列的数据拷贝到m_szSendThreadBuf中，然后用send发送
				//	这个改动的目的是，大量广播数据时 ，可以将数据拷贝到一起，节省send系统调用，原来逻辑是一个数据包使用
				//	一个send系统调用，测试后发现效率太低，大量广播数据存在时，连接经常因为数据无法及时发送出去而被关闭连接
				if (pSession != NULL)
				{
					if (pSession->GetCloseWorkSendedToCloseThread() == 0)
					{
						if (pSession->SendDataInFixCircleSendBuf(this, false) < 0)
						{
							char szError[512];
							sprintf(szError, "LSendThread::ThreadDoing 2, pSession->SendDataInFixCircleSendBuf(this), SessionID:%lld\n", seoe.u64SessionID);
							g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
							//	这里需要关闭套接字
							cntc.u64SessionID = pSession->GetEpollBindParam()->u64SessionID;
							m_pNetWorkServices->GetCloseSocketThread().AppendToClose(cntc);
						}
						else
						{
							pSession->UpdateLastSendTime();
						}
					}
				}

				if (nSendWorkProcessed >= 500)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (!bHaveEpollOutEvent && !bHaveSendWork)
		{
			//	sched_yield();
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		if (CheckForStop())
		{
			break;
		}
	}
#endif
	char szError[512];
	sprintf(szError, "LSendThread::ThreadDoing, Exit\n");
	g_ErrorWriter.WriteError(szError, strlen(szError));
	return 0;
}
bool LSendThread::OnStart()
{
	return true;
}
void LSendThread::OnStop()
{
}

void LSendThread::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices =  pNetWorkServices;
}
LNetWorkServices* LSendThread::GetNetWorkServices()	
{
	return m_pNetWorkServices;
}

#ifdef __ADD_SEND_BUF_CHAIN__
bool LSendThread::AddPacketToFreePool(LPacketBroadCast* pPacket)
{
	pPacket->Reset();
#ifdef __EPOLL_TEST_STATISTIC__
//	atomic_inc(&g_nSendPacketFreeCount);
	__sync_add_and_fetch(&g_nSendPacketFreeCount, 1);
	pPacket->FillPacketForTest();
#endif
	if (!m_LocalBroadCastPacketForFree.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen()))
	{ 
		//char szError1[512];
		//sprintf(szError1, "LSendThread::AddPacketToFreePool, FreeOneItemToPool FirstFailed\n"); 
		//g_ErrorWriter.WriteError(szError1, strlen(szError1));


		if (!m_pNetWorkServices->CommitFreePacketToGlobalSendPool(&m_LocalBroadCastPacketForFree, pPacket->GetPacketBufLen()))
		{
			char szError[512];
			sprintf(szError, "LSendThread::AddPacketToFreePool, m_pNetWorkServices->CommitFreePacketToGlobalSendPool Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			
			//	删除数据包
			m_nFreePacketCount++;
			delete pPacket;
			return true;
		}
		//	再提交一次，不成功就删除
		if (!m_LocalBroadCastPacketForFree.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen()))
		{
			char szError[512];
			sprintf(szError, "LSendThread::AddPacketToFreePool, m_LocalBroadCastPacketForFree.FreeOneItemToPool Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));

			m_nFreePacketCount++;
			delete pPacket;
			return true;
		}
	}
	return true;
} 
#endif

bool LSendThread::PushOneEpollOutEvent(t_Send_Epoll_Out_Event& sdd)
{
	E_Circle_Error error = E_Circle_Buf_No_Error;
	pthread_mutex_lock(&m_MutexForEpollOutCircleBuf);
	error = m_SendEpollOutCircleBuf.AddItems((char*)&sdd, 1);
	pthread_mutex_unlock(&m_MutexForEpollOutCircleBuf);
	if (error != E_Circle_Buf_No_Error)
	{
		return false;
	}
	return true;
}

//	一次性提交多个EPOLLOUT事件
bool LSendThread::PushEpollOutEvent(char* pbuf, unsigned int unEventCount)
{
	E_Circle_Error error = E_Circle_Buf_No_Error;
	pthread_mutex_lock(&m_MutexForEpollOutCircleBuf);
	error = m_SendEpollOutCircleBuf.AddItems(pbuf, unEventCount); 
	pthread_mutex_unlock(&m_MutexForEpollOutCircleBuf);
	if (error == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}

bool LSendThread::GetOneEpollOutEvent(t_Send_Epoll_Out_Event& sdd)
{
	E_Circle_Error error = E_Circle_Buf_No_Error;
	error = m_SendEpollOutCircleBuf.GetOneItem((char*)&sdd, sizeof(t_Send_Content_Desc));
	if (error == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}



void LSendThread::PrintSendThreadLocalBufStatus()
{
#ifdef __ADD_SEND_BUF_CHAIN__
	char szTempString[512];

	//	fa song gong zuo dui lie de chang du 
	sprintf(szTempString, "Current Send Work Item Count:%d, FreePacketCount:%d\n", m_SendCircleBuf.GetCurrentExistCount(), m_nFreePacketCount);
	m_ErrorWriterForSendThreadStatus.WriteError(szTempString, strlen(szTempString));

	//	ben di de xu yao shi fang hui quan ju packet huan cun de bao shu liang 

	sprintf(szTempString, "Current Send Thread Local Free Packet Pool Desc\n"); 
	m_ErrorWriterForSendThreadStatus.WriteError(szTempString, strlen(szTempString));

	for (unsigned int unIndex = 0; unIndex < m_unppdForLocalFreePoolCount; ++unIndex)
	{
		unsigned short usPacketLen = m_pArrayForLocalFreePoolDesc[unIndex].usPacketLen;
		unsigned int unMaxAdd = 0;
		LFixLenCircleBuf* pFixLenCircleBuf = m_LocalBroadCastPacketForFree.GetFixLenCircleBuf(usPacketLen, unMaxAdd);
		sprintf(szTempString, "\tBufLen:%hd, ItemCount:%d\n", usPacketLen, pFixLenCircleBuf->GetCurrentExistCount()); 
		m_ErrorWriterForSendThreadStatus.WriteError(szTempString, strlen(szTempString)); 
	} 
#endif
}

void LSendThread::ReleaseSendThreadResource()
{
#ifdef __ADD_SEND_BUF_CHAIN__
	if (m_SendCircleBuf.GetCurrentExistCount() != 0)
	{
		t_Send_Content_Desc tSendContentDesc;
		while (1)
		{
			memset(&tSendContentDesc, 0, sizeof(tSendContentDesc));

			E_Circle_Error error = m_SendCircleBuf.GetOneItem((char*)&tSendContentDesc, sizeof(tSendContentDesc));
				
			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{

				if (tSendContentDesc.pPacket->DecrementRefCountAndResultIsTrue())
				{
					//	引用计数减1,防止多次释放
					delete tSendContentDesc.pPacket; 
				}

			}
		}

	}
	//	释放本地缓存的数据包
	m_LocalBroadCastPacketForFree.ReleasePacketPoolManagerResource();

	delete[] m_pArrayForLocalFreePoolDesc;
#endif
}

//	加入一个需要处理的即将关闭的连接
void LSendThread::AddWillCloseSessionInSendThread(LSession* pSession)
{
	E_Circle_Error eError = m_FixCircleBufWillSessionCloseToProcessSendThread.AddItems((char*)&pSession, 1);
	if (eError != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LSendThread::AddWillCloseSessionInSendThread, Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
}
//	取一个出来处理
bool LSendThread::GetOneWillCloseSessionInSendThread(LSession** pSession)
{
	E_Circle_Error eError = m_FixCircleBufWillSessionCloseToProcessSendThread.GetOneItem((char*)(&(*pSession)), sizeof(LSession*));
	if (eError != E_Circle_Buf_No_Error && eError != E_Circle_Buf_Is_Empty)
	{
		char szError[512];
		sprintf(szError, "LSendThread::GetOneWillCloseSessionInSendThread, errorCode != E_Circle_Buf_No_Error\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (eError == E_Circle_Buf_Is_Empty)
	{
		return false;
	}
	return true;
}

//	Session的RecvData出错，需要关闭连接，那么设置本线程不再对该Session进行处理，并且广播给发送线程和主逻辑线程
void LSendThread::ProcessSendDataErrorToCloseSession(LSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	}

	//	设置本接收线程不再对该连接进行操作
	pSession->SetSendThreadStopProcess(1);

	//	检查是否需要关闭连接
	int nSendThreadStopProcessInfo 		= 0;
	int nRecvThreadStopProcessInfo		= 0;
	int nMainLogicThreadStopProcessInfo	= 0;
	int nEpollThreadStopProcessInfo		= 0;
	pSession->GetStopProcessInfos(nSendThreadStopProcessInfo, nRecvThreadStopProcessInfo
			, nMainLogicThreadStopProcessInfo, nEpollThreadStopProcessInfo);
	if (nSendThreadStopProcessInfo == 1 && nRecvThreadStopProcessInfo == 1
			&& nMainLogicThreadStopProcessInfo == 1 && nEpollThreadStopProcessInfo == 1)
	{
		m_pNetWorkServices->GetSessionManager().MoveWillCloseSessionToSessionPool(pSession);
	}
}

