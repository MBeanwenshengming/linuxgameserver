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

#include "LSession.h"
#include "LRecvThread.h"
#include "LNetWorkServices.h"
#include "LEpollThreadManager.h"
#include "LRecvThreadManager.h"
#include "LEpollThread.h"
#include "LCloseSocketThread.h" 
#include "LErrorWriter.h"
#include "LSendThread.h"
#include "LPacketBroadCast.h"

extern bool g_bEpollETEnabled;


extern LErrorWriter g_ErrorWriter;
extern int errno;

extern int g_globalCount;
extern int g_narrLen[];

LSession::LSession()
{
#ifdef __ADD_SEND_BUF_CHAIN__
	pthread_mutex_init(&m_MutexForSendBufChain, NULL);
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	if (!m_FixLenCircleBufForSendData.Initialize(1, 128 * 1024))
	{
		char szError[512];
		sprintf(szError, "LSession::LSession, m_FixLenCircleBufForSendData.Initialize\n");
		g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
	}
	__sync_lock_test_and_set(&m_nSendBufIsFull, 0);
#endif

	Reset();
}
LSession::~LSession()
{
#ifdef __ADD_SEND_BUF_CHAIN__ 
	pthread_mutex_destroy(&m_MutexForSendBufChain);
#endif
}

//	client ID sessionManager分配
void LSession::SetSessionID(unsigned long long u64SessionID)
{
	m_u64SessionID = u64SessionID;
}
unsigned long long LSession::GetSessionID()
{
	return m_u64SessionID;
}


void LSession::SetRecvThreadID(int nRecvThreadID)
{
	__sync_lock_test_and_set(&m_nRecvThreadID, nRecvThreadID);
}
int LSession::GetRecvThreadID()
{
	return __sync_add_and_fetch(&m_nRecvThreadID, 0);
}


void LSession::SetSendThreadID(int nSendThreadID)
{
	__sync_lock_test_and_set(&m_nSendThreadID, nSendThreadID);
}
int LSession::GetSendThreadID()
{
	return __sync_add_and_fetch(&m_nSendThreadID, 0);
}

void LSession::SetEpollThreadID(int nEpollThreadID)
{
	__sync_lock_test_and_set(&m_nEpollThreadID, nEpollThreadID);
}
int LSession::GetEpollThreadID()
{
	return __sync_add_and_fetch(&m_nEpollThreadID, 0);
}

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
void LSession::GetStopProcessInfos(int& nSendThreadStopProcessInfo, int& nRecvThreadStopProcessInfo,
		int& nMainLogicThreadStopProcessInfo, int& nEpollThreadSopProcessInfo)
{
	nSendThreadStopProcessInfo 		= __sync_add_and_fetch(&m_nSendThreadStopProcessSession, 0);
	nRecvThreadStopProcessInfo 		= __sync_add_and_fetch(&m_nRecvThreadStopProcessSession, 0);
	nMainLogicThreadStopProcessInfo 	= __sync_add_and_fetch(&m_nMainLogicThreadStopProcessSession, 0);
	nEpollThreadSopProcessInfo 		= __sync_add_and_fetch(&m_nEpollThreadStopProcessSession, 0);
}
void LSession::SetSendThreadStopProcess(int nStop)
{
	__sync_lock_test_and_set(&m_nSendThreadStopProcessSession, nStop);
}
int LSession::GetSendThreadStopProcess()
{
	return __sync_add_and_fetch(&m_nSendThreadStopProcessSession, 0);
}
void LSession::SetRecvThreadStopProcess(int nStop)
{
	__sync_lock_test_and_set(&m_nRecvThreadStopProcessSession, nStop);
}
int LSession::GetRecvThreadStopProcess()
{
	return __sync_add_and_fetch(&m_nRecvThreadStopProcessSession, 0);
}
void LSession::SetMainLogicThreadStopProcess(int nStop)
{
	__sync_lock_test_and_set(&m_nMainLogicThreadStopProcessSession, nStop);
}
int LSession::GetMainLogicThreadStopProcess()
{
	return __sync_add_and_fetch(&m_nMainLogicThreadStopProcessSession, 0);
}
void LSession::SetEpollThreadStopProcess(int nStop)
{
	__sync_lock_test_and_set(&m_nEpollThreadStopProcessSession, nStop);
}
int LSession::GetEpollThreadSopProcess()
{
	return __sync_add_and_fetch(&m_nEpollThreadStopProcessSession, 0);
}
void LSession::SetCloseWorkSendedToCloseThread(int nSended)
{
	__sync_lock_test_and_set(&m_nCloseWorkSendedToCloseThread, nSended);
}
int LSession::GetCloseWorkSendedToCloseThread()
{
	return __sync_add_and_fetch(&m_nCloseWorkSendedToCloseThread, 0);
}
#endif

void LSession::Reset()
{
	m_u64SessionID 		= 0ll; 
	__sync_lock_test_and_set(&m_nRecvThreadID, -1);
	__sync_lock_test_and_set(&m_nSendThreadID, -1);
	__sync_lock_test_and_set(&m_nEpollThreadID, -1);
	ResetRecv();
	m_bCanSend = true;
	__sync_lock_test_and_set(&m_nLastRecvTime, 0);
	__sync_lock_test_and_set(&m_nLastSendTime, 0);

	//	这里不重置这个值 ，在分配session再重置 ，因为发送和关闭线程可以在连接释放时候还会使用到这个值
	//__sync_lock_release(&m_nIsClosing);
#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	m_FixLenCircleBufForSendData.ClearContent();
	__sync_lock_test_and_set(&m_nSendBufIsFull, 0);
#endif
#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	__sync_lock_test_and_set(&m_nSendThreadStopProcessSession, 0);
	__sync_lock_test_and_set(&m_nRecvThreadStopProcessSession, 0);
	__sync_lock_test_and_set(&m_nMainLogicThreadStopProcessSession, 0);
	__sync_lock_test_and_set(&m_nEpollThreadStopProcessSession, 0);
	__sync_lock_test_and_set(&m_nCloseWorkSendedToCloseThread, 0);
#endif
}

void LSession::RecvData(LRecvThread* pRecvThread)
{
	LNetWorkServices* pNetWorkServices = pRecvThread->GetNetServices();
	t_Client_Need_To_Close cntc;
	cntc.u64SessionID = m_EpollBindParam.u64SessionID;
	LCloseSocketThread* pCloseThread = &pNetWorkServices->GetCloseSocketThread();
	if (g_bEpollETEnabled)
	{
	}
	else
	{
		int	nerrorCode = Recv(m_szRecvedContent + m_unCurrentContentLen, sizeof(m_szRecvedContent) - m_unCurrentContentLen);
		if (nerrorCode == E_Socket_No_Recv_Data)
		{
			if (!AddToEpollWaitThread(pRecvThread))
			{
				char szError[512];
				sprintf(szError, "LSession::RecvData, AddToEpollWaitThread Failed pos 1, SessionId:%lld\n", m_EpollBindParam.u64SessionID); 
				g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);

				// close client
				//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
				if (GetCloseWorkSendedToCloseThread() == 0)
				{
					pCloseThread->AppendToClose(cntc);
					SetCloseWorkSendedToCloseThread(1);
				}
				return ;
			}
			return ;
		}
		else if (nerrorCode <= 0)
		{
			char szError[512];
			sprintf(szError, "LSession::RecvData, Recv Error, SessionId:%lld, errCode:%d\n", m_EpollBindParam.u64SessionID, errno); 
			g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
			//	guan bi client
			//	jiang yi ge guan bi qing qiu ti jiao dao guan bi xian cheng
			//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
			if (GetCloseWorkSendedToCloseThread() == 0)
			{
				pCloseThread->AppendToClose(cntc);
				SetCloseWorkSendedToCloseThread(1);
			}
			return ;
		}
		else
		{
			m_unCurrentContentLen += nerrorCode;
			if (m_unCurrentContentLen < sizeof(unsigned short))		//	收到的字节少于2字节，那么等待后续字节到来
			{
				return ;
			}
			else
			{
				unsigned short usPacketLen = *((unsigned short*)m_szRecvedContent);
				if (usPacketLen > MAX_PACKET_SIZE)
				{
					char szError[512];
					sprintf(szError, "LSession::RecvData, Packet Len Error, SessionId:%lld, packeLen:%hd, RemainDataLen:%d\n", m_EpollBindParam.u64SessionID, usPacketLen, m_unCurrentContentLen); 
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);


					//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
					if (GetCloseWorkSendedToCloseThread() == 0)
					{
						pCloseThread->AppendToClose(cntc);
						SetCloseWorkSendedToCloseThread(1);
					}
					return ; 
				}
				if (usPacketLen < sizeof(unsigned short))
				{

					char szError[512];
					sprintf(szError, "LSession::RecvData, usPacketLen < sizeof(unsigned short), Packet Len Error, SessionId:%lld, packeLen:%hd, RemainDataLen:%d\n", m_EpollBindParam.u64SessionID, usPacketLen, m_unCurrentContentLen); 
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);


					//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
					if (GetCloseWorkSendedToCloseThread() == 0)
					{
						pCloseThread->AppendToClose(cntc);
						SetCloseWorkSendedToCloseThread(1);
					}
					return ; 
				}
				while (m_unCurrentContentLen >= usPacketLen)
				{
//					bool bFinded = false;
//					for (int i = 0; i < g_globalCount; ++i)
//					{
//						if (usPacketLen - sizeof(int) - sizeof(unsigned short) == g_narrLen[i])
//						{
//
//							bFinded = true;
//						}
//					}
//					if (!bFinded)
//					{
//						printf("Recved PacketError, Len:%d\n", usPacketLen - sizeof(int));
//					}

					//	 ti jiao yi ge xiao xi bao 
					LPacketSingle* pPacket = pRecvThread->GetOneFreePacket(usPacketLen);
					if (pPacket == NULL)
					{
						char szError[512];
						sprintf(szError, "LSession::RecvData, pRecvThread->GetOneFreePacket Failed, SessionId:%lld\n", m_EpollBindParam.u64SessionID); 
						g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
						//	guan bi socket 
						//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
						if (GetCloseWorkSendedToCloseThread() == 0)
						{
							pCloseThread->AppendToClose(cntc);
							SetCloseWorkSendedToCloseThread(1);
						}
						return ;
					}

					UpdateLastRecvTime();
					//	4  wei bao chang du xin xi 
					//pPacket->AddData(m_szRecvedContent + sizeof(unsigned short), usPacketLen - sizeof(unsigned short));
					//	直接设置数据
					pPacket->DirectSetData(m_szRecvedContent + sizeof(unsigned short),  usPacketLen - sizeof(unsigned short));

					if (!pRecvThread->AddPacketToLocal(m_EpollBindParam.u64SessionID, pPacket))
					{
						char szError[512];
						sprintf(szError, "LSession::RecvData, pRecvThread->AddPacketToLocal Failed, SessionId:%lld\n", m_EpollBindParam.u64SessionID); 
						g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);


						//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
						if (GetCloseWorkSendedToCloseThread() == 0)
						{
							pCloseThread->AppendToClose(cntc);
							SetCloseWorkSendedToCloseThread(1);
						}
						delete pPacket;
						return ;
					}
					//pRecvThread->m_nPacketRecved++;
					//	gai bian chang du 
					memcpy(m_szRecvedContent, m_szRecvedContent + usPacketLen, m_unCurrentContentLen - usPacketLen);
					m_unCurrentContentLen -= usPacketLen;
					if (m_unCurrentContentLen >= sizeof(unsigned short))
					{
						usPacketLen = *((unsigned short*)m_szRecvedContent);
						//	这里需要判断接收数据包的长度，如果长度非法，那么，关闭连接	
						if (usPacketLen > MAX_PACKET_SIZE)
						{
							char szError[512];
							sprintf(szError, "LSession::RecvData2, Packet Len Error, SessionId:%lld, packeLen:%hd, RemainDataLen:%d\n", m_EpollBindParam.u64SessionID, usPacketLen, m_unCurrentContentLen); 
							g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);


							//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
							if (GetCloseWorkSendedToCloseThread() == 0)
							{
								pCloseThread->AppendToClose(cntc);
								SetCloseWorkSendedToCloseThread(1);
							}
							return ; 
						}
						if (usPacketLen < sizeof(unsigned short))
						{
							char szError[512];
							sprintf(szError, "LSession::RecvData2, usPacketLen < sizeof(unsigned short), Packet Len Error, SessionId:%lld, packeLen:%hd, RemainDataLen:%d\n", m_EpollBindParam.u64SessionID, usPacketLen, m_unCurrentContentLen); 
							g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);


							//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
							if (GetCloseWorkSendedToCloseThread() == 0)
							{
								pCloseThread->AppendToClose(cntc);
								SetCloseWorkSendedToCloseThread(1);
							}
							return ; 
						}
					}
					else
					{
						break;
					}
				}
				if (!AddToEpollWaitThread(pRecvThread))
				{
					char szError[512];
					sprintf(szError, "LSession::RecvData, AddToEpollWaitThread Failed pos 3, SessionId:%lld\n", m_EpollBindParam.u64SessionID); 
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);

					//pNetWorkServices->GetRecvThreadManager().UnBindRecvThread(this);
					if (GetCloseWorkSendedToCloseThread() == 0)
					{
						pCloseThread->AppendToClose(cntc);
						SetCloseWorkSendedToCloseThread(1);
					}
					return ;
				}
			}
		}
	}
}
void LSession::ResetRecv()
{
	memset(m_szRecvedContent, 0, sizeof(m_szRecvedContent));
	m_unCurrentContentLen = 0;
}

bool LSession::AddToEpollWaitThread(LRecvThread* pRecvThread)
{
	if (pRecvThread == NULL)
	{
		char szError[512];
		sprintf(szError, "LSession::AddToEpollWaitThread, pRecvThread == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	//	 tian jia gen zong shi jian 
	struct epoll_event epollEvent;
	memset(&epollEvent, 0, sizeof(epollEvent));
#ifdef __ADD_SEND_BUF_CHAIN__
	if (GetSendBufIsFull())
	{
		epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
	}
	else
	{ 
		epollEvent.events = EPOLLIN | EPOLLONESHOT;
	}
	epollEvent.data.ptr = &m_EpollBindParam;
#else
//	epollEvent.events = EPOLLIN | EPOLLONESHOT;
//	epollEvent.data.ptr = &m_EpollBindParam;
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	if (GetSendBufIsFull())
	{
		epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
	}
	else
	{
		epollEvent.events = EPOLLIN | EPOLLONESHOT;
	}
	//epollEvent.data.ptr = &m_EpollBindParam;
	epollEvent.data.u64 = m_EpollBindParam.u64SessionID;
#else
//	epollEvent.events = EPOLLIN | EPOLLONESHOT;
//	epollEvent.data.ptr = &m_EpollBindParam;
#endif

	int nEpollThreadID = GetEpollThreadID();
	if (nEpollThreadID < 0)
	{
		return false;
	}
	LEpollThread* pEpollThread = pRecvThread->GetNetServices()->GetEpollThreadManager().GetEpollThread(nEpollThreadID);
	if (pEpollThread == NULL)
	{
		char szError[512];
		sprintf(szError, "LSession::AddToEpollWaitThread, pEpollThread == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	int nEpollHandle = pEpollThread->GetEpollHandle();

	if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
	{
		char szError[512];
		sprintf(szError, "LSession::AddToEpollWaitThread, epoll_ctl Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}

bool LSession::GetIpAndPort(char* pszBuf, unsigned short usbufLen)
{
	return true;
}
#ifdef __ADD_SEND_BUF_CHAIN__ 
bool LSession::InitializeSendBufChain(unsigned int unSendBufChainSize)
{
	if (unSendBufChainSize == 0)
	{
		unSendBufChainSize = 50;
	}
	if (!m_SendBufChain.Initialize(sizeof(LPacketBroadCast*), unSendBufChainSize))
	{
		return false;
	}
	return true;
}

//	发送数据，首先判断发送缓冲区是否有数据，如果有那么，先加入队列，然后从队列里面
//	取出来发送；如果没有，那么直接发送，如果发送返回缓冲区满，那么将该数据包添加到队列
//	等待下次发送
//	如果函数返回0，表示函数没有发生错误,如果返回-1,表示需要关闭连接
int LSession::SendPacket(LPacketBroadCast* pSendPacket, LSendThread* pSendThread)
{
	pthread_mutex_lock(&m_MutexForSendBufChain);
	int nCount = m_SendBufChain.GetCurrentExistCount();
	if (nCount == 0)	//	检查是否存在上次没有发送的数据，如果存在那么先添加到队列，然后从队列中取出数据来发送
	{
		int nResult = Send(pSendPacket->GetPacketBuf(), pSendPacket->GetPacketDataAndHeaderLen());
		if (nResult > 0)
		{
			//	发送成功，那么直接返回,释放数据包到本地缓存
			if (pSendPacket->DecrementRefCountAndResultIsTrue())
			{
				pSendThread->AddPacketToFreePool(pSendPacket);
			}
			pthread_mutex_unlock(&m_MutexForSendBufChain);
			return 0;
		}
		else
		{
			if (nResult == E_Socket_Send_System_Buf_Full)
			{
				E_Circle_Error ErrorID = m_SendBufChain.AddItems((char*)&pSendPacket, 1);
				if (ErrorID == E_Circle_Buf_No_Error)
				{
					//	注册1个事件，EPOLLOUT事件
					LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
					int nBindEpollThreadID = GetEpollThreadID();
					if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
					{ 
						char szError[512];
						sprintf(szError, "LSession::SendPacket, nBindEpollThreadID < 0\n"); 
						g_ErrorWriter.WriteError(szError, strlen(szError));
						pthread_mutex_unlock(&m_MutexForSendBufChain);
						return -1;
					}
				
					struct epoll_event epollEvent;
					memset(&epollEvent, 0, sizeof(epollEvent));
					epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
					epollEvent.data.ptr = &m_EpollBindParam;

					LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
					if (pEpollThread == NULL)
					{
						char szError[512];
						sprintf(szError, "LSession::SendPacket, pEpollThread == NULL\n"); 
						g_ErrorWriter.WriteError(szError, strlen(szError));
						pthread_mutex_unlock(&m_MutexForSendBufChain);
						return -1;
					}
					//	设置发送缓存满标志位
					atomic_set(&m_SendBufIsFull, 1);

					int nEpollHandle = pEpollThread->GetEpollHandle();

					if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
					{
						char szError[512];
						sprintf(szError, "LSession::SendPacket, epoll_ctl Failed\n"); 
						g_ErrorWriter.WriteError(szError, strlen(szError));

						pthread_mutex_unlock(&m_MutexForSendBufChain);
						return -1;
					}

					pthread_mutex_unlock(&m_MutexForSendBufChain);
					return 0;
				}
				else 	//	出现添加到缓冲区错误，那么需要关闭连接	
				{ 
					char szError[512];
					sprintf(szError, "LSession::SendPacket, m_SendBufChain.AddItems, Error:%d\n", nResult);
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);

					//	首先，回收数据包
					if (pSendPacket->DecrementRefCountAndResultIsTrue())
					{
						pSendThread->AddPacketToFreePool(pSendPacket);
					} 
					pthread_mutex_unlock(&m_MutexForSendBufChain);
					return -1;
				}
			}
			else		//	这里时套接字发生其它错误，需要关闭
			{ 
				char szError[512];
				sprintf(szError, "LSession::SendPacket, nResult <= 0, Error:%d\n", nResult);
				g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);

				if (pSendPacket->DecrementRefCountAndResultIsTrue())
				{
					pSendThread->AddPacketToFreePool(pSendPacket);
				}
				pthread_mutex_unlock(&m_MutexForSendBufChain);
				return -1;
			}
		}
	}
	else		//	存在数据，那么先将数据包加入队列，然后取数据出来发送
	{
		//	首先，将发送数据添加到队列中 
		E_Circle_Error ErrorID = m_SendBufChain.AddItems((char*)&pSendPacket, 1);
		if (ErrorID == E_Circle_Buf_No_Error)
		{
			//	从队列中取出数据来发送,没次最多发送100个数据包,如果还没有发送完，那么等下一次再发送
			LPacketBroadCast* pPacket = NULL;
			E_Circle_Error LookUpErrorID = m_SendBufChain.LookUpOneItem((char*)&pPacket, sizeof(LPacketBroadCast*));
			while (LookUpErrorID == E_Circle_Buf_No_Error)
			{ 
				int nResult = Send(pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen());
				if (nResult > 0)
				{
					//	发送成功，回收数据包
					if (pPacket->DecrementRefCountAndResultIsTrue())
					{
						pSendThread->AddPacketToFreePool(pPacket);
					}
				}
				else
				{
					if (nResult == E_Socket_Send_System_Buf_Full)
					{
						//	发送缓存满了，那么需要投递一个EPOLLOUT事件 
						LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
						int nBindEpollThreadID = GetEpollThreadID();
						if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
						{ 
							char szError[512];
							sprintf(szError, "LSession::SendPacket, nBindEpollThreadID < 0\n"); 
							g_ErrorWriter.WriteError(szError, strlen(szError));

							pthread_mutex_unlock(&m_MutexForSendBufChain);
							return -1;
						}
					
						struct epoll_event epollEvent;
						memset(&epollEvent, 0, sizeof(epollEvent));
						epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
						epollEvent.data.ptr = &m_EpollBindParam;

						LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
						if (pEpollThread == NULL)
						{
							char szError[512];
							sprintf(szError, "LSession::SendPacket2, pEpollThread == NULL\n"); 
							g_ErrorWriter.WriteError(szError, strlen(szError));
							pthread_mutex_unlock(&m_MutexForSendBufChain);
							return -1;
						}
						//	设置发送缓存满标志位
						atomic_set(&m_SendBufIsFull, 1);

						int nEpollHandle = pEpollThread->GetEpollHandle();

						if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
						{
							char szError[512];
							sprintf(szError, "LSession::SendPacket2, epoll_ctl Failed\n"); 
							g_ErrorWriter.WriteError(szError, strlen(szError));

							pthread_mutex_unlock(&m_MutexForSendBufChain);
							return -1;
						}
						pthread_mutex_unlock(&m_MutexForSendBufChain);
						return 0;
					}
					else		//	套接字发生错误，那么直接关闭连接
					{
						pthread_mutex_unlock(&m_MutexForSendBufChain);
						char szError[512];
						sprintf(szError, "LSession::SendPacket, nResult <= 0, Error:%d\n", nResult);
						g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);

						return -1;
					}
				}

				//	删除发送缓存区中的一个数据
				m_SendBufChain.DeleteOneItemAtHead();
				LookUpErrorID = m_SendBufChain.LookUpOneItem((char*)&pPacket, sizeof(LPacketBroadCast*)); 
			}
			//	取消发送缓冲区满的标志，设置EPOLLIN事件，让套接字继续接收数据
			//atomic_set(&m_SendBufIsFull, 0);

			//LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
			//int nBindEpollThreadID = GetEpollThreadID();
			//if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
			//{ 
			//	char szError[512];
			//	sprintf(szError, "LSession::SendPacket, nBindEpollThreadID < 0\n"); 
			//	g_ErrorWriter.WriteError(szError, strlen(szError));

			//	pthread_mutex_unlock(&m_MutexForSendBufChain);
			//	return -1;
			//}
		
			//struct epoll_event epollEvent;
			//memset(&epollEvent, 0, sizeof(epollEvent));
			//epollEvent.events = EPOLLIN | EPOLLONESHOT;
			//epollEvent.data.ptr = &m_EpollBindParam;

			//LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
			//if (pEpollThread == NULL)
			//{
			//	char szError[512];
			//	sprintf(szError, "LSession::SendPacket2, pEpollThread == NULL\n"); 
			//	g_ErrorWriter.WriteError(szError, strlen(szError));
			//	pthread_mutex_unlock(&m_MutexForSendBufChain);
			//	return -1;
			//}

			//int nEpollHandle = pEpollThread->GetEpollHandle();

			//if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
			//{
			//	char szError[512];
			//	sprintf(szError, "LSession::SendPacket2, epoll_ctl Failed\n"); 
			//	g_ErrorWriter.WriteError(szError, strlen(szError));

			//	pthread_mutex_unlock(&m_MutexForSendBufChain);
			//	return -1;
			//}
		}
		else	//	添加失败，那么缓冲区满了，直接关闭连接
		{
			if (pSendPacket->DecrementRefCountAndResultIsTrue())
			{
				pSendThread->AddPacketToFreePool(pSendPacket);
			}

			pthread_mutex_unlock(&m_MutexForSendBufChain);

			char szError[512];
			sprintf(szError, "LSession::SendPacket, m_SendBufChain.AddItems, Error:%d\n", ErrorID);
			g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
	}

	pthread_mutex_unlock(&m_MutexForSendBufChain);
	return 0;
}
//	如果连接要关闭，那么需要释放尚未发送的数据包
void LSession::ReleaseSendPacketBufChain(LCloseSocketThread* pCloseThread)
{
	pthread_mutex_lock(&m_MutexForSendBufChain);
	//	设置连接已经处于关闭状态，这个时候，不能再向连接的发送队列中添加数据包
	SetIsClosing(1);

	LPacketBroadCast* pPacket = NULL;
	E_Circle_Error LookUpErrorID = m_SendBufChain.LookUpOneItem((char*)&pPacket, sizeof(LPacketBroadCast*));
	while (LookUpErrorID == E_Circle_Buf_No_Error)
	{
		if (pPacket->DecrementRefCountAndResultIsTrue())
		{
			pCloseThread->AddPacketToLocalPool(pPacket);
		}

		m_SendBufChain.DeleteOneItemAtHead();
		LookUpErrorID = m_SendBufChain.LookUpOneItem((char*)&pPacket, sizeof(LPacketBroadCast*)); 
	}
	pthread_mutex_unlock(&m_MutexForSendBufChain);
}

//	发送发送缓冲区的数据,当EPOLL线程返回该连接可以发送数据时，发送在发送缓存中的数据
int LSession::SendPacketInSendBufChain(LSendThread* pSendThread)
{
	pthread_mutex_lock(&m_MutexForSendBufChain);

	LPacketBroadCast* pPacket = NULL;
	E_Circle_Error LookUpErrorID = m_SendBufChain.LookUpOneItem((char*)&pPacket, sizeof(LPacketBroadCast*));
	while (LookUpErrorID == E_Circle_Buf_No_Error)
	{ 
		int nResult = Send(pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen());
		if (nResult > 0)
		{
			//	发送成功，回收数据包
			if (pPacket->DecrementRefCountAndResultIsTrue())
			{
				pSendThread->AddPacketToFreePool(pPacket);
			}
		}
		else
		{
			if (nResult == E_Socket_Send_System_Buf_Full)
			{
				//	发送缓存满了，那么需要投递一个EPOLLOUT事件 
				LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
				int nBindEpollThreadID = GetEpollThreadID();
				if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
				{ 
					pthread_mutex_unlock(&m_MutexForSendBufChain);
					return -1;
				}
			
				struct epoll_event epollEvent;
				memset(&epollEvent, 0, sizeof(epollEvent));
				epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
				epollEvent.data.ptr = &m_EpollBindParam;

				LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
				if (pEpollThread == NULL)
				{
					char szError[512];
					sprintf(szError, "LSession::SendPacketInSendBufChain, pEpollThread == NULL\n"); 
					g_ErrorWriter.WriteError(szError, strlen(szError));
					pthread_mutex_unlock(&m_MutexForSendBufChain);
					return -1;
				}
				//	设置发送缓存满标志位
				atomic_set(&m_SendBufIsFull, 1);

				int nEpollHandle = pEpollThread->GetEpollHandle();

				if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
				{
					char szError[512];
					sprintf(szError, "LSession::SendPacketInSendBufChain, epoll_ctl Failed\n"); 
					g_ErrorWriter.WriteError(szError, strlen(szError));

					pthread_mutex_unlock(&m_MutexForSendBufChain);
					return -1;
				}
				pthread_mutex_unlock(&m_MutexForSendBufChain);
				return 0;
			}
			else		//	套接字发生错误，那么直接关闭连接
			{ 
				pthread_mutex_unlock(&m_MutexForSendBufChain);
				return -1;
			}
		}

		//	删除发送缓存区中的一个数据
		m_SendBufChain.DeleteOneItemAtHead();
		LookUpErrorID = m_SendBufChain.LookUpOneItem((char*)&pPacket, sizeof(LPacketBroadCast*)); 
	}

	//	取消发送缓冲区满的标志，设置EPOLLIN事件，让套接字继续接收数据
	atomic_set(&m_SendBufIsFull, 0);

	LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
	int nBindEpollThreadID = GetEpollThreadID();
	if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
	{ 
		char szError[512];
		sprintf(szError, "LSession::SendPacketInSendBufChain, nBindEpollThreadID < 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		pthread_mutex_unlock(&m_MutexForSendBufChain);
		return -1;
	}

	struct epoll_event epollEvent;
	memset(&epollEvent, 0, sizeof(epollEvent));
	epollEvent.events = EPOLLIN | EPOLLONESHOT;
	epollEvent.data.ptr = &m_EpollBindParam;

	LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
	if (pEpollThread == NULL)
	{
		char szError[512];
		sprintf(szError, "LSession::SendPacketInSendBufChain, pEpollThread == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		pthread_mutex_unlock(&m_MutexForSendBufChain);
		return -1;
	}

	int nEpollHandle = pEpollThread->GetEpollHandle();

	if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
	{
		char szError[512];
		sprintf(szError, "LSession::SendPacketInSendBufChain, epoll_ctl Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		pthread_mutex_unlock(&m_MutexForSendBufChain);
		return -1;
	}
	pthread_mutex_unlock(&m_MutexForSendBufChain);
	return 0;
}
//	如果套接字发送缓存满了，那么返回true
bool LSession::GetSendBufIsFull()
{
	int nIsFull = atomic_read(&m_SendBufIsFull);
	if (nIsFull == 1)
	{
		return true;
	}
	return false;
}
#endif


//	释放Session占用的资源
void LSession::ReleaseSessionResource()
{ 
#ifdef __ADD_SEND_BUF_CHAIN__
	if (m_SendBufChain.GetCurrentExistCount() > 0)
	{
		LPacketBroadCast* pSendPacket = NULL;

		while (1)
		{
			pSendPacket = NULL;
			E_Circle_Error error = m_SendBufChain.GetOneItem((char*)&pSendPacket, sizeof(LPacketBroadCast*));
				
			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				if (pSendPacket->DecrementRefCountAndResultIsTrue())
				{
					delete pSendPacket;
					pSendPacket = NULL;
				}
			}
		}
	}
#endif
}
#ifdef __ADD_SEND_BUF_CHAIN__
int LSession::PushSendPacketToSessionSendChain(LPacketBroadCast* pSendPacket, LSendThread* pSendThread)
{
	pthread_mutex_lock(&m_MutexForSendBufChain);
	if (GetIsClosing() == 1)	//	如果连接已经在关闭，那么不能再把数据包加入发送队列了，如果加入，那么没有机会得到释放
	{
		pthread_mutex_unlock(&m_MutexForSendBufChain);
		return -2;		//	返回结果表示正在关闭，发送线程不用再投递关闭连接的工作到关闭线程
	}
	E_Circle_Error ErrorID = m_SendBufChain.AddItems((char*)&pSendPacket, 1);
	if (ErrorID != E_Circle_Buf_No_Error)	// 如果加入成功，那么直接返回，如果加入失败，那么判断是否
	{
		//	发送队列满了，那么先将数据发送一遍，然后再添加到队列，如果再添加出错，那么说明socket的缓冲已满并且发送队列也满了，那么关闭连接
		if (ErrorID == E_Circle_Buf_Can_Not_Contain_Data)
		{
			//	发送数据
//			int nSendResult = SendPacketInSendBufChainV2(pSendThread);
//			if (nSendResult == 0)	//	发送成功，那么再添加到发送队列，等待发送线程发送
//			{
//				ErrorID = m_SendBufChain.AddItems((char*)&pSendPacket, 1);
//				if (ErrorID != E_Circle_Buf_No_Error)
//				{
//					pthread_mutex_unlock(&m_MutexForSendBufChain);
//					return -1;
//				}
//				else
//				{
//					//	添加成功
//					pthread_mutex_unlock(&m_MutexForSendBufChain);
//					return 0;
//				}
//			}
//			else		//	发送函数出现失败
//			{
				pthread_mutex_unlock(&m_MutexForSendBufChain);
				return -1;
//			}
		}
		else
		{
			//	其他情况，直接让发送线程投递关闭连接的工作到关闭线程
			pthread_mutex_unlock(&m_MutexForSendBufChain);
			return -1;
		}
	}
	pthread_mutex_unlock(&m_MutexForSendBufChain);
	return 0;
}

int LSession::SendPacketInSendBufChainV2(LSendThread* pSendThread)
{
	pSendThread->m_unCurrentSendThreadBufDataLen = 0;

	int nItemToRelease = 0;
	LPacketBroadCast* pPacket = NULL;
	int nExistItemCount = m_SendBufChain.GetCurrentExistCount();
	if (nExistItemCount <= 0)
	{
		return 0;
	}

	for (int i = 0; i < nExistItemCount; ++i)
	{
		E_Circle_Error LookUpErrorID = m_SendBufChain.LookUpNItem((char*)&pPacket, sizeof(LPacketBroadCast*), i);
		if (LookUpErrorID != E_Circle_Buf_No_Error)
		{
			break;
		}

		unsigned short usContentLen = pPacket->GetPacketDataAndHeaderLen();
		if (pSendThread->m_unCurrentSendThreadBufDataLen + usContentLen > SEND_THREAD_SEND_BUF_LEN)
		{
			break;
		}

		nItemToRelease++;
		//	拷贝数据到发送的缓冲区
		memcpy(pSendThread->m_szSendThreadBuf + pSendThread->m_unCurrentSendThreadBufDataLen, pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen());
		pSendThread->m_unCurrentSendThreadBufDataLen += pPacket->GetPacketDataAndHeaderLen();
	}
	if (pSendThread->m_unCurrentSendThreadBufDataLen == 0)
	{
		char szError[512];
		sprintf(szError, "LSession::SendPacketInSendBufChainV2, pSendThread->m_unCurrentSendThreadBufDataLen == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
	//	发送数据
	int nResult = Send(pSendThread->m_szSendThreadBuf, pSendThread->m_unCurrentSendThreadBufDataLen);
	if (nResult > 0)	//	发送成功
	{
		//	释放发送的数据包，删除在发送队列的信息
		for (int i = 0; i < nItemToRelease; ++i)
		{
			E_Circle_Error ErrorID = m_SendBufChain.GetOneItem((char*)&pPacket, sizeof(LPacketBroadCast*));
			if (ErrorID != E_Circle_Buf_No_Error)
			{
				char szError[512];
				sprintf(szError, "LSession::SendPacketInSendBufChainV2, ErrorID != E_Circle_Buf_No_Error\n");
				g_ErrorWriter.WriteError(szError, strlen(szError));
				break;
			}
			else
			{
				//	释放数据包
				if (pPacket->DecrementRefCountAndResultIsTrue())
				{
					pSendThread->AddPacketToFreePool(pPacket);
				}
			}
		}
	}
	else		//	发送不成功，那么返回去，直接关闭连接,说明系统发送缓存已满并且发送队列也满或者连接已经断开
	{
		return -1;
	}
	return 0;
}

int LSession::SendPacketInSendBufChainUseSendThreadBuf(LSendThread* pSendThread)
{
	pthread_mutex_lock(&m_MutexForSendBufChain);
	if (GetIsClosing())		//	如果是这个状态，那么表示连接已经断开了,closethread刚刚关闭连接，信息包已经释放，不需要再发送
	{
		pthread_mutex_unlock(&m_MutexForSendBufChain);
		return 0;
	}
	pSendThread->m_unCurrentSendThreadBufDataLen = 0;

	int nItemToRelease = 0;
	LPacketBroadCast* pPacket = NULL;
	int nExistItemCount = m_SendBufChain.GetCurrentExistCount();
	if (nExistItemCount <= 0)
	{
		pthread_mutex_unlock(&m_MutexForSendBufChain);
		return 0;
	}

	for (int i = 0; i < nExistItemCount; ++i)
	{
		E_Circle_Error LookUpErrorID = m_SendBufChain.LookUpNItem((char*)&pPacket, sizeof(LPacketBroadCast*), i);
		if (LookUpErrorID != E_Circle_Buf_No_Error)
		{
			break;
		}

		unsigned short usContentLen = pPacket->GetPacketDataAndHeaderLen();
		if (pSendThread->m_unCurrentSendThreadBufDataLen + usContentLen > SEND_THREAD_SEND_BUF_LEN)
		{
			break;
		}

		nItemToRelease++;
		//	拷贝数据到发送的缓冲区
		memcpy(pSendThread->m_szSendThreadBuf + pSendThread->m_unCurrentSendThreadBufDataLen, pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen());
		pSendThread->m_unCurrentSendThreadBufDataLen += pPacket->GetPacketDataAndHeaderLen();
	}
	if (pSendThread->m_unCurrentSendThreadBufDataLen == 0)
	{
		char szError[512];
		sprintf(szError, "LSession::SendPacketInSendBufChainUseSendThreadBuf, pSendThread->m_unCurrentSendThreadBufDataLen == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
	//	发送数据
	int nResult = Send(pSendThread->m_szSendThreadBuf, pSendThread->m_unCurrentSendThreadBufDataLen);
	if (nResult > 0)	//	发送成功
	{
		//	释放发送的数据包，删除在发送队列的信息
		for (int i = 0; i < nItemToRelease; ++i)
		{
			E_Circle_Error ErrorID = m_SendBufChain.GetOneItem((char*)&pPacket, sizeof(LPacketBroadCast*));
			if (ErrorID != E_Circle_Buf_No_Error)
			{
				char szError[512];
				sprintf(szError, "LSession::SendPacketInSendBufChainUseSendThreadBuf, ErrorID != E_Circle_Buf_No_Error\n");
				g_ErrorWriter.WriteError(szError, strlen(szError));
				break;
			}
			else
			{
				//	释放数据包
				if (pPacket->DecrementRefCountAndResultIsTrue())
				{
					pSendThread->AddPacketToFreePool(pPacket);
				}
			}
		}
	}
	else		//	发送不成功，那么返回去，直接关闭连接,说明系统发送缓存已满并且发送队列也满或者连接已经断开
	{
		if (E_Socket_Send_System_Buf_Full == nResult)		//	缓冲满，那么添加一个epollout事件
		{
			LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
			int nBindEpollThreadID = GetEpollThreadID();
			if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
			{
				char szError[512];
				sprintf(szError, "LSession::SendPacketInSendBufChainUseSendThreadBuf, nBindEpollThreadID < 0\n");
				g_ErrorWriter.WriteError(szError, strlen(szError));
				pthread_mutex_unlock(&m_MutexForSendBufChain);
				return -1;
			}

			struct epoll_event epollEvent;
			memset(&epollEvent, 0, sizeof(epollEvent));
			epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
			epollEvent.data.ptr = &m_EpollBindParam;

			LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
			if (pEpollThread == NULL)
			{
				char szError[512];
				sprintf(szError, "LSession::SendPacketInSendBufChainUseSendThreadBuf, pEpollThread == NULL\n");
				g_ErrorWriter.WriteError(szError, strlen(szError));
				pthread_mutex_unlock(&m_MutexForSendBufChain);
				return -1;
			}

			int nEpollHandle = pEpollThread->GetEpollHandle();

			if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
			{
				char szError[512];
				sprintf(szError, "LSession::SendPacketInSendBufChainUseSendThreadBuf, epoll_ctl Failed\n");
				g_ErrorWriter.WriteError(szError, strlen(szError));

				pthread_mutex_unlock(&m_MutexForSendBufChain);
				return -1;
			}
		}
		else
		{
			pthread_mutex_unlock(&m_MutexForSendBufChain);
			return -1;
		}
	}
	pthread_mutex_unlock(&m_MutexForSendBufChain);
	return 0;
}
#endif

#ifdef	__USE_SESSION_BUF_TO_SEND_DATA__
bool LSession::AppendDataToSend(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}
//	if (pPacket->GetDataLen() <= 2)
//	{
//		char szError[512];
//		sprintf(szError, "LNetWorkServices::SendPacket, pPacket->GetDataLen() <= 2\n");
//		g_ErrorWriter.WriteError(szError, strlen(szError));
//		return false;
//	}
//	if (pPacket->GetPacketDataAndHeaderLen() <= 2)
//	{
//		char szError[512];
//		sprintf(szError, "LNetWorkServices::SendPacket, pPacket->GetPacketDataAndHeaderLen() <= 2\n");
//		g_ErrorWriter.WriteError(szError, strlen(szError));
//		return false;
//	}
	if (m_FixLenCircleBufForSendData.AddItems(pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen()) != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LSession::AppendDataToSend, ErrorID != E_Circle_Buf_No_Error, SessionId:%llu\n", this->GetSessionID());
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
bool  LSession::GetSendBufIsFull()
{
	return m_nSendBufIsFull == 1 ? true : false;
}
int LSession::SendDataInFixCircleSendBuf(LSendThread* pSendThread, bool bFromEpollOutEvent)
{
	if (bFromEpollOutEvent)
	{
		__sync_lock_test_and_set(&m_nSendBufIsFull, 0);
	}
	int nContentLen = m_FixLenCircleBufForSendData.GetCurrentExistCount();
	if (nContentLen == 0)
	{
		return 0;
	}
	int nReadIndex = 0;
	int nWriteIndex = 0;
	m_FixLenCircleBufForSendData.GetCurrentReadAndWriteIndex(nReadIndex, nWriteIndex);
	if (nReadIndex == nWriteIndex)
	{
		return 0;
	}
	char* pBufStart = m_FixLenCircleBufForSendData.GetBuf();

	if (nReadIndex < nWriteIndex)
	{
		int nResult = Send(pBufStart + nReadIndex, nWriteIndex - nReadIndex);
		if (nResult > 0)
		{
			//	全部发送成功，那么将数据读索引设置成写索引，表示数据已经读取
			m_FixLenCircleBufForSendData.SetReadIndex(nWriteIndex);
			return nResult;
		}
		else		//	发送不成功，检查发送返回值
		{
			if (E_Socket_Send_System_Buf_Full == nResult)		//	缓冲满了
			{
				__sync_lock_test_and_set(&m_nSendBufIsFull, 1);
				LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
				int nBindEpollThreadID = GetEpollThreadID();
				if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, nBindEpollThreadID < 0\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}

				struct epoll_event epollEvent;
				memset(&epollEvent, 0, sizeof(epollEvent));
				epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
				//epollEvent.data.ptr = &m_EpollBindParam;
				epollEvent.data.u64 = m_EpollBindParam.u64SessionID;

				LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
				if (pEpollThread == NULL)
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, pEpollThread == NULL\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}

				int nEpollHandle = pEpollThread->GetEpollHandle();

				if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, epoll_ctl Failed\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}
				return 0;
			}
			else
			{
				char szError[512];
				sprintf(szError, "LSession::SendDataInFixCircleSendBuf, Send Error, ErrorCode:%d\n", nResult);
				g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}
	else
	{
		//	先发送到缓冲结尾的地方，然后发送从头开始的地方
		//	第一步，发送到结尾的数据
		int nSendCount = 0;

		int nResult = Send(pBufStart + nReadIndex, m_FixLenCircleBufForSendData.GetMaxItemCount() - nReadIndex);
		if (nResult > 0)
		{
			m_FixLenCircleBufForSendData.SetReadIndex(0);
			nSendCount += nResult;
		}
		else
		{
			if (nResult == E_Socket_Send_System_Buf_Full)
			{
				__sync_lock_test_and_set(&m_nSendBufIsFull, 1);
				LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
				int nBindEpollThreadID = GetEpollThreadID();
				if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, nBindEpollThreadID < 0\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}

				struct epoll_event epollEvent;
				memset(&epollEvent, 0, sizeof(epollEvent));
				epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
				//epollEvent.data.ptr = &m_EpollBindParam;
				epollEvent.data.u64 = m_EpollBindParam.u64SessionID;

				LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
				if (pEpollThread == NULL)
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, pEpollThread == NULL\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}

				int nEpollHandle = pEpollThread->GetEpollHandle();

				if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, epoll_ctl Failed\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}
				return nSendCount;
			}
			else
			{
				char szError[512];
				sprintf(szError, "LSession::SendDataInFixCircleSendBuf, Send Error, ErrorCode:%d\n", nResult);
				g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
		}
		if (nWriteIndex == 0)	//	说明没有数据
		{
			m_FixLenCircleBufForSendData.SetReadIndex(nWriteIndex);
			return nSendCount;
		}
		//	第二步， 发送到写位置的数据
		nResult = Send(pBufStart, nWriteIndex);
		if (nResult > 0)
		{
			m_FixLenCircleBufForSendData.SetReadIndex(nWriteIndex);
			nSendCount += nResult;
			return nSendCount;
		}
		else
		{
			if (nResult == E_Socket_Send_System_Buf_Full)
			{
				__sync_lock_test_and_set(&m_nSendBufIsFull, 1);
				LNetWorkServices* pNetWorkServices = pSendThread->GetNetWorkServices();
				int nBindEpollThreadID = GetEpollThreadID();
				if (nBindEpollThreadID < 0)    // 绑定的EpollThreadID小于0,那么需要关闭连接
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, nBindEpollThreadID < 0\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}

				struct epoll_event epollEvent;
				memset(&epollEvent, 0, sizeof(epollEvent));
				epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
				//epollEvent.data.ptr = &m_EpollBindParam;
				epollEvent.data.u64 = m_EpollBindParam.u64SessionID;

				LEpollThread* pEpollThread = pNetWorkServices->GetEpollThreadManager().GetEpollThread(nBindEpollThreadID);
				if (pEpollThread == NULL)
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, pEpollThread == NULL\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}

				int nEpollHandle = pEpollThread->GetEpollHandle();

				if (epoll_ctl(nEpollHandle, EPOLL_CTL_MOD, m_EpollBindParam.nSocket, &epollEvent) == -1)
				{
					char szError[512];
					sprintf(szError, "LSession::SendDataInFixCircleSendBuf, epoll_ctl Failed\n");
					g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
					return -1;
				}
				return nSendCount;
			}
			else
			{
				char szError[512];
				sprintf(szError, "LSession::SendDataInFixCircleSendBuf, Send Error, ErrorCode:%d\n", nResult);
				g_ErrorWriter.WriteError(szError, strlen(szError), __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}
}
#endif
