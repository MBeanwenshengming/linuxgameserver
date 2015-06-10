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

#include "LSelectServer.h"
#include "LIniFileReadAndWrite.h"
#include "limits.h"

#ifdef __EPOLL_TEST_STATISTIC__
atomic_t g_nSelectServerRecvPacketAllocCount;
atomic_t g_nSelectServerRecvPacketFreeCount;
atomic_t g_nSelectServerSendPacketAllocCount;
atomic_t g_nSelectServerSendPacketFreeCount;
#endif
//	====================================LSelectServerSession========================BEGIN
LSelectServerSession::LSelectServerSession()
{
	//	接收缓存
	//	memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));
	//	遗留没有解析的数据
	m_unRemainDataLen = 0;
	m_pszRecvBuf	= NULL;
	m_unRecvBufSize = 0;
	//	是否可以发送数据
	m_bSendable = true;
	//	服务设置为空
	m_pSelectServer = NULL;
	m_unSessionID = 0;
	m_bReconnect = false;
}

LSelectServerSession::~LSelectServerSession()
{
	if (m_pszRecvBuf == NULL)
	{
		delete[] m_pszRecvBuf;
	}
}

//	重置
void LSelectServerSession::Reset()
{
	m_unSessionID = 0; 
	m_bSendable = true;
	m_unRemainDataLen = 0;
}


bool LSelectServerSession::Initialize(t_Select_Server_Params& tssp)
{
	if (m_pSelectServer == NULL)
	{
		return false;
	}
	if (tssp.unMaxSendQueueSize == 0)
	{
		return false;
	}
	if (tssp.unRecvBufSize == 0)
	{
		return false;
	}
	if (!m_FixLenCircleBufForSendQueue.Initialize(sizeof(LPacketBroadCast*), tssp.unMaxSendQueueSize))
	{
		return false;
	}
	m_unRecvBufSize = tssp.unRecvBufSize;
	m_pszRecvBuf = new char[m_unRecvBufSize];
	if (m_pszRecvBuf == NULL)
	{
		return false;
	}
	return true;
}
//	有数据的时候，接收数据
int LSelectServerSession::SessionRecv()
{
	int nRecvedSize = Recv(m_pszRecvBuf + m_unRemainDataLen, m_unRecvBufSize - m_unRemainDataLen);
	if (nRecvedSize > 0)
	{
		m_unRemainDataLen += nRecvedSize;
		int nParsedLen = ParsePacket(m_pszRecvBuf, m_unRemainDataLen);
		if (nParsedLen < 0)
		{
			return -1;
		}
		else if (nParsedLen == 0)
		{
			return 0;
		}
		else
		{
			if (nParsedLen > m_unRemainDataLen)
			{
				return -1;
			}
			else if (nParsedLen == m_unRemainDataLen)
			{
				m_unRemainDataLen = 0;
			}
			else
			{
				m_unRemainDataLen -= nParsedLen;
				memcpy(m_pszRecvBuf, m_pszRecvBuf + nParsedLen, m_unRemainDataLen);
			}
			return 0;
		} 
	}
	else if (nRecvedSize == E_Socket_No_Recv_Data)
	{
		return 0;
	}
	else
	{
		return -1;
	}
	return 0;
}
//	发送数据, 返回<0那么需要关闭套接字，返回0,正常
int LSelectServerSession::SessionSend(LPacketBroadCast* pPacket)
{
	if (!m_bSendable)	//	当前不能发送数据，那么直接将数据放入队列中，等待下次发送
	{ 
		E_Circle_Error errorID = m_FixLenCircleBufForSendQueue.AddItems((char*)&pPacket, 1);
		if (errorID == E_Circle_Buf_No_Error)		//	添加成功，那么直接返回
		{
			return 0;
		}
		else	//	不成功，那么释放被引用的数据包
		{ 
			if (pPacket->DecrementRefCountAndResultIsTrue())
			{
				if (!m_pSelectServer->FreeOneSendPacket(pPacket))
				{
					delete pPacket;
				}
			}
			return -1;
		}
	}
	else
	{
		int nCurrentCount = m_FixLenCircleBufForSendQueue.GetCurrentExistCount();
		if (nCurrentCount == 0)	//	队列中没有数据，那么直接发送，如果不能发送再添加到队列中
		{
			int nSended = Send(pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen());	
			if (nSended > 0)
			{
				if (pPacket->DecrementRefCountAndResultIsTrue())
				{
					if (!m_pSelectServer->FreeOneSendPacket(pPacket))
					{
						delete pPacket;
					}
				}
				return 0;
			}
			else if (nSended == E_Socket_Send_System_Buf_Full)
			{ 
				E_Circle_Error errorID = m_FixLenCircleBufForSendQueue.AddItems((char*)&pPacket, 1);
				if (errorID == E_Circle_Buf_No_Error)
				{
					m_bSendable = false;
					return 0;
				}
				else
				{
					if (pPacket->DecrementRefCountAndResultIsTrue())
					{ 
						if (!m_pSelectServer->FreeOneSendPacket(pPacket))
						{
							delete pPacket;
						}
					}
					m_bSendable = false;
					return -1; 
				}
			}
			else
			{ 
				if (pPacket->DecrementRefCountAndResultIsTrue())
				{ 
					if (!m_pSelectServer->FreeOneSendPacket(pPacket))
					{
						delete pPacket;
					}
				}
				m_bSendable = false;
				return -1; 
			}
		}
		else		//	队列中存在数据，那么，先添加到队列，然后再发送
		{
			E_Circle_Error errorID = m_FixLenCircleBufForSendQueue.AddItems((char*)&pPacket, 1);
			if (errorID == E_Circle_Buf_No_Error)
			{
				//	从发送的队列中去处数据发送
				LPacketBroadCast* pPacketForSendTemp = NULL;
				E_Circle_Error errorIDTemp = m_FixLenCircleBufForSendQueue.LookUpOneItem((char*)&pPacketForSendTemp, sizeof(LPacketBroadCast*));
				while (errorIDTemp == E_Circle_Buf_No_Error)
				{
					//	发送数据
					int nSendedTemp = Send(pPacketForSendTemp->GetPacketBuf(), pPacketForSendTemp->GetPacketDataAndHeaderLen());
					if (nSendedTemp > 0)
					{ 
						if (pPacketForSendTemp->DecrementRefCountAndResultIsTrue())
						{
							if (!m_pSelectServer->FreeOneSendPacket(pPacket))
							{
								delete pPacketForSendTemp;
							}
						}
						m_FixLenCircleBufForSendQueue.DeleteOneItemAtHead();
						errorIDTemp = m_FixLenCircleBufForSendQueue.LookUpOneItem((char*)&pPacketForSendTemp, sizeof(LPacketBroadCast*)); 
					}
					else if (nSendedTemp == E_Socket_Send_System_Buf_Full)
					{
						m_bSendable = false;
						return 0;
					}
					else
					{
						m_bSendable = false;
						return -1;
					}
				}
				if (errorIDTemp == E_Circle_Buf_Is_Empty)
				{
					return 0;
				}
				else
				{
					m_bSendable = false;
					return -1;
				} 
			}
			else 		//	加入队列不成功，那么直接释放,关闭连接
			{ 
				if (pPacket->DecrementRefCountAndResultIsTrue())
				{ 
					if (!m_pSelectServer->FreeOneSendPacket(pPacket))
					{
						delete pPacket;
					}
				}	
				m_bSendable = false;
				return -1;
			}
		}
	}
}

//	发送在队列中的数据
int LSelectServerSession::SessionSendInSendQueue()
{ 
	m_bSendable = true;
	int nCurrentCount = m_FixLenCircleBufForSendQueue.GetCurrentExistCount();
	if (nCurrentCount == 0)
	{
		return 0;
	}
	else
	{ 
		LPacketBroadCast* pPacketForSendTemp = NULL;
		E_Circle_Error errorIDTemp = m_FixLenCircleBufForSendQueue.LookUpOneItem((char*)&pPacketForSendTemp, sizeof(LPacketBroadCast*));
		while (errorIDTemp == E_Circle_Buf_No_Error)
		{
			//	发送数据
			int nSendedTemp = Send(pPacketForSendTemp->GetPacketBuf(), pPacketForSendTemp->GetPacketDataAndHeaderLen());
			if (nSendedTemp > 0)
			{ 
				if (pPacketForSendTemp->DecrementRefCountAndResultIsTrue())
				{
					if (!m_pSelectServer->FreeOneSendPacket(pPacketForSendTemp))
					{
						delete pPacketForSendTemp;
					}
				}
				m_FixLenCircleBufForSendQueue.DeleteOneItemAtHead();
				errorIDTemp = m_FixLenCircleBufForSendQueue.LookUpOneItem((char*)&pPacketForSendTemp, sizeof(LPacketBroadCast*)); 
			} 
			else if (nSendedTemp == E_Socket_Send_System_Buf_Full)
			{
				m_bSendable = false;
				return 0;
			}
			else
			{
				return -1;
			}
		}
		return 0;
	}
}

//	释放没有发送的数据
void LSelectServerSession::ReleaseAllPacketInSendQueue()
{ 
	int nCurrentCount = m_FixLenCircleBufForSendQueue.GetCurrentExistCount();
	if (nCurrentCount == 0)
	{
		return ;
	}
	else
	{ 
		//	删除还没有发送的数据
		LPacketBroadCast* pPacketForSendTemp = NULL;
		E_Circle_Error errorIDTemp = m_FixLenCircleBufForSendQueue.LookUpOneItem((char*)&pPacketForSendTemp, sizeof(LPacketBroadCast*));
		while (errorIDTemp == E_Circle_Buf_No_Error)
		{
			if (pPacketForSendTemp->DecrementRefCountAndResultIsTrue())
			{
				if (!m_pSelectServer->FreeOneSendPacket(pPacketForSendTemp))
				{
					delete pPacketForSendTemp;
				}
			}
			m_FixLenCircleBufForSendQueue.DeleteOneItemAtHead();
			errorIDTemp = m_FixLenCircleBufForSendQueue.LookUpOneItem((char*)&pPacketForSendTemp, sizeof(LPacketBroadCast*)); 
		} 
	}
}

int LSelectServerSession::ParsePacket(char* pData, unsigned int unDataLen)
{
	unsigned int unParsedLen = 0;

	if (pData == NULL)
	{
		return -1;
	}
	if (unDataLen < 2)
	{
		return 0;
	}
	unsigned short usPacketLen = *((unsigned short*)pData);	//	判断数据包长度，如果非法，那么关闭连接
	if (usPacketLen > 8 * 1024)
	{
		return -1;
	}
	if (usPacketLen < sizeof(unsigned short)) 
	{
		return -2;
	}
	while (unDataLen >= usPacketLen)
	{
		LPacketSingle* pPacketSingle = m_pSelectServer->AllocOneRecvPacket(usPacketLen);
		if (pPacketSingle == NULL)
		{
			return -3;
		}
		pPacketSingle->DirectSetData(pData + sizeof(unsigned short), usPacketLen - sizeof(unsigned short));
		if (!m_pSelectServer->AddOneRecvedPacket(GetSessionID(), pPacketSingle))
		{
			return -4;
		}

		//	统计解析长度
		unParsedLen += usPacketLen;
		pData 		+= usPacketLen;
		unDataLen 	-= usPacketLen;

		if (unDataLen < 2)
		{
			break;
		}
		else
		{
			usPacketLen = *((unsigned short*)pData);

			if (usPacketLen > 8 * 1024)
			{
				return -1;
			}
			if (usPacketLen < sizeof(unsigned short)) 
			{
				return -2;
			}
		}
	}
	return unParsedLen;
} 
//	====================================LSelectServerSession========================END


//	==============================LSelectServerSessionManager=========================BEGIN
LSelectServerSessionManager::LSelectServerSessionManager()
{
	m_pSelectServer = NULL;
	m_unCurrentSessionID = 0;
	FD_ZERO(&m_SETRecvSocket);
	FD_ZERO(&m_SETSendSocket);
}
LSelectServerSessionManager::~LSelectServerSessionManager()
{
}
//	初始化SelectServer
bool LSelectServerSessionManager::Initialize(t_Select_Server_Params& tssp)
{
	if (m_pSelectServer == NULL)
	{
		return false;
	}
	if (tssp.unMaxSessionCount == 0)
	{
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < tssp.unMaxSessionCount; ++unIndex)
	{ 
		LSelectServerSession* pSession = new LSelectServerSession;
		if (pSession == NULL)
		{
			return false;
		}
		pSession->SetSelectServer(m_pSelectServer);
		if (!pSession->Initialize(tssp))
		{	
			delete pSession;
			return false;
		}
		m_queueSessionPool.push(pSession);
	}
	return true;
}
LSelectServerSession* LSelectServerSessionManager::AllocSession()
{
	LSelectServerSession* pSession = NULL;
	if (m_queueSessionPool.empty())
	{
		return pSession;
	}
	pSession = m_queueSessionPool.front();
	m_queueSessionPool.pop();
	if (m_unCurrentSessionID < UINT_MAX - 1) 
	{
		m_unCurrentSessionID++;
	}
	else
	{
		m_unCurrentSessionID = 1;
	}
	pSession->SetSessionID(m_unCurrentSessionID);
	return pSession;
}
void LSelectServerSessionManager::FreeSession(LSelectServerSession* pSession)
{
	if (pSession == NULL)
	{
		return ;
	} 
	pSession->Reset();
	m_queueSessionPool.push(pSession);
}

//	添加一个正在使用的连接到管理器
bool LSelectServerSessionManager::AddOneUsingSession(LSelectServerSession* pSession)
{
	if (pSession == NULL)
	{
		return false;
	}
	unsigned int unSessionID = pSession->GetSessionID();
	if (unSessionID == 0)
	{
		return false;
	}
	map<unsigned int, LSelectServerSession*>::iterator _ito = m_mapSessionManager.find(unSessionID);
	if (_ito != m_mapSessionManager.end())
	{
		return false;
	}
	m_mapSessionManager[unSessionID] = pSession;
	return true;
}

//	查找一个正在使用的连接
LSelectServerSession* LSelectServerSessionManager::GetOneUsingSession(unsigned int unSessionID)
{
	map<unsigned int, LSelectServerSession*>::iterator _ito = m_mapSessionManager.find(unSessionID);
	if (_ito != m_mapSessionManager.end())
	{
		return _ito->second;
	}
	return NULL; 
}

//	移除一个正在使用的连接
bool LSelectServerSessionManager::RemoveOneUsingSession(unsigned int unSessionID)
{
	map<unsigned int, LSelectServerSession*>::iterator _ito = m_mapSessionManager.find(unSessionID);
	if (_ito != m_mapSessionManager.end())
	{
		m_mapSessionManager.erase(_ito);
		return true;
	} 
	return false;
}

bool LSelectServerSessionManager::BuildSet()
{
	m_nMaxSocketPlus1 = 0;
	m_mapSocketMapToSession.clear();

	FD_ZERO(&m_SETRecvSocket);
	FD_ZERO(&m_SETSendSocket);
	
	map<unsigned int, LSelectServerSession*>::iterator _ito = m_mapSessionManager.begin();
	for (; _ito != m_mapSessionManager.end(); ++_ito)
	{
		LSelectServerSession* pSession = _ito->second;
		int nSocket = pSession->GetSocket();
 
		if (nSocket > m_nMaxSocketPlus1)
		{
			m_nMaxSocketPlus1 = nSocket;
		}
		m_mapSocketMapToSession[nSocket] = pSession;

		FD_SET(nSocket, &m_SETRecvSocket);

		if (!pSession->GetSendable())
		{
			FD_SET(nSocket, &m_SETSendSocket);
		}
	}
	//	加入监听套接字
	int nListenSocket = m_pSelectServer->GetSocket();
	FD_SET(nListenSocket, &m_SETRecvSocket);
	if (nListenSocket > m_nMaxSocketPlus1)
	{
		m_nMaxSocketPlus1 = nListenSocket;
	}

	if (m_nMaxSocketPlus1 > 0)
	{
		m_nMaxSocketPlus1++;
	} 
	return true;
}

fd_set LSelectServerSessionManager::GetRecvSet()
{
	return m_SETRecvSocket;
}
fd_set LSelectServerSessionManager::GetSendSet()
{
	return m_SETSendSocket;
}
LSelectServerSession* LSelectServerSessionManager::GetSessionFromSocketToSession(int nSocket)
{
	map<int, LSelectServerSession*>::iterator _ito = m_mapSocketMapToSession.find(nSocket);
	if (_ito == m_mapSocketMapToSession.end())
	{
		return NULL;
	}
	return _ito->second;
}
map<int, LSelectServerSession*>* LSelectServerSessionManager::GetSessionManagerForEvent()
{
	return &m_mapSocketMapToSession;
}
//	==============================LSelectServerSessionManager=========================END

//	===============================Select Server===============================
LSelectServer::LSelectServer()
{
	pthread_mutex_init(&m_mutexForCloseSessionQueue, NULL);
	pthread_mutex_init(&m_mutexForConnectToServer, NULL);
#ifdef __EPOLL_TEST_STATISTIC__
	atomic_set(&g_nSelectServerRecvPacketAllocCount, 0);
	atomic_set(&g_nSelectServerRecvPacketFreeCount, 0);
	atomic_set(&g_nSelectServerSendPacketAllocCount, 0);
	atomic_set(&g_nSelectServerSendPacketFreeCount, 0);
#endif
}
LSelectServer::~LSelectServer()
{
	pthread_mutex_destroy(&m_mutexForCloseSessionQueue);
	pthread_mutex_destroy(&m_mutexForConnectToServer);
}

bool LSelectServer::Initialize(t_Select_Server_Params& tssp)
{
	if (tssp.pszIp == NULL)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, tssp.pszIp == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (tssp.usPort == 0)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, tssp.usPort == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	//	初始化连接管理器和连接池
	m_SelectServerSessionManager.SetSelectServer(this);
	if (!m_SelectServerSessionManager.Initialize(tssp))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_SelectServerSessionManager.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	//	初始化接收缓存
	if (!m_FixLenCircleBufForRecvedPacket.Initialize(sizeof(t_Recved_Packet), tssp.unRecvedPacketQueueSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_FixLenCircleBufForRecvedPacket.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	//	初始化关闭队列的长度
	if (!m_FixLenCircleBufForCloseSession.Initialize(sizeof(unsigned int), tssp.unCloseWorkItemQueueSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_FixLenCircleBufForCloseSession.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	//	初始化接收数据包缓存池
	if (!m_RecvPacketPoolManager.Initialize(tssp.pppdForRecv, tssp.usppdForRecvSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_RecvPacketPoolManager.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	//	初始化发送缓存
	if (!m_FixLenCircleBufForSendPacketForMainLogic.Initialize(sizeof(t_Send_Packet), tssp.unSendPacketSizeForMainLogic))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_FixLenCircleBufForSendPacketForMainLogic.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_FixLenCircleBufForSendPacket.Initialize(sizeof(t_Send_Packet), tssp.unSendPacketSizeForSendThread))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_FixLenCircleBufForSendPacket.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_SendPacketPoolManager.Initialize(tssp.pppdForSend, tssp.usppdForSendSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_SendPacketPoolManager.Initialize\n"); 		 g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!InitializeConnectWorkManagerThreadManager(tssp.unConnectWorkItemCountForCircleBufLen))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, InitializeConnectWorkManagerThreadManager Failed \n"); 	
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!Initialized(tssp.pszIp, tssp.usPort))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !Initialized(tssp.pszIp, tssp.usPort)\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!Listen(tssp.usListenListSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !Listen(tssp.usListenListSize)\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
int LSelectServer::ThreadDoing(void* pParam)
{ 
	while (1)
	{
		//	首先，关闭需要关闭的连接
		unsigned short usProcessedCloseWorkCount = 0;
		unsigned int unSessionIDForClose = 0;
		while (GetOneCloseSessionWork(unSessionIDForClose))
		{
			LSelectServerSession* pSession = FindSession(unSessionIDForClose);	
			if (pSession != NULL)
			{
				bool bReconnect = pSession->GetReconnect();
				//	释放没有发送的数据
				pSession->ReleaseAllPacketInSendQueue();
				//	关闭连接
				close(pSession->GetSocket());
				//	从正在使用的连接管理器中删除连接
				RemoveSession(unSessionIDForClose);
				//	将连接放回连接池
				FreeSession(pSession);
				//	添加一个连接断开的数据包到消息队列
				AddOneRecvedPacket(unSessionIDForClose, NULL);
				if (bReconnect)
				{
					char szIP[32];
					unsigned short usPort = 0;
					pthread_mutex_lock(&m_mutexForConnectToServer);
					if (!m_ConnectorWorkManager.AddConnectWork(szIP, usPort, NULL, 0))
					{
						//	error print
					}
					pthread_mutex_unlock(&m_mutexForConnectToServer);
				}
			}
			usProcessedCloseWorkCount++;
			if (usProcessedCloseWorkCount >= 200)
			{
				break;
			}
		}
		t_Connector_Work_Result cwr;
		memset(&cwr, 0, sizeof(cwr));
		int nProcessConnecttedWorkCount = 0;
		while (GetOneConnectedSession(cwr))
		{ 
			LSelectServerSession* pSession = AllocSession();
			if (pSession == NULL)
			{
				close(cwr.nSocket);
				pthread_mutex_lock(&m_mutexForConnectToServer);
				if (!m_ConnectorWorkManager.AddConnectWork(cwr.szIP, cwr.usPort, NULL, 0))
				{
					//	error print
				}
				pthread_mutex_unlock(&m_mutexForConnectToServer);
				nProcessConnecttedWorkCount++;
				if (nProcessConnecttedWorkCount >= 200)
				{
					break;
				}
				continue;
			}
			else
			{
				pSession->SetSocket(cwr.nSocket);
				if (!AddSession(pSession))
				{
					close(cwr.nSocket);
					pthread_mutex_lock(&m_mutexForConnectToServer);
					if (!m_ConnectorWorkManager.AddConnectWork(cwr.szIP, cwr.usPort, NULL, 0))
					{
						//	error print
					}
					pthread_mutex_unlock(&m_mutexForConnectToServer);
					FreeSession(pSession);
				}
				else
				{ 
					pSession->SetReconnect(true);
					//	添加一个连接断开的数据包到消息队列
					AddOneRecvedPacket(pSession->GetSessionID(), (LPacketSingle*)0xFFFFFFFF);
				}
			}
			nProcessConnecttedWorkCount++;
			if (nProcessConnecttedWorkCount >= 200)
			{
				break;
			}
		}

		// 检查网络事件
		int nWorkCount = CheckForSocketEvent();
		if (nWorkCount < 0)
		{
			return 0;
		}
		// 发送在本地队列的数据		
		int nSendWorkCount = SendData();
		if (nWorkCount == 0 && nSendWorkCount == 0)
		{
			//	sched_yield();
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		if (CheckForStop())
		{
			return 0;
		} 
	}
	return 0; 
}
bool LSelectServer::OnStart()
{
	return true;
}
void LSelectServer::OnStop()
{
}
int LSelectServer::CheckForSocketEvent()
{
	if (!m_SelectServerSessionManager.BuildSet())
	{
		return -1;
	}

	fd_set rdset = m_SelectServerSessionManager.GetRecvSet();
	fd_set wrset = m_SelectServerSessionManager.GetSendSet();
	map<int, LSelectServerSession*>* pMapSocket = m_SelectServerSessionManager.GetSessionManagerForEvent();

	struct timeval val;
	val.tv_sec 		= 0;
	val.tv_usec 	= 0;
	int iReturn = select(m_SelectServerSessionManager.GetMaxSocketPlus1(), &rdset, &wrset, NULL, &val);	
	if (iReturn == -1)
	{
		return -1;
	}
	else if (iReturn == 0)
	{
		return 0;
	}
	int nListenSocket = GetSocket(); 
	if (FD_ISSET(nListenSocket, &rdset))	//	有连接上来，那么接受连接
	{
		int nAcceptedSocket = accept(nListenSocket, NULL, NULL);
		if (nAcceptedSocket != -1)
		{
			LSelectServerSession* pSession = AllocSession();
			if (pSession == NULL)
			{
				close(nAcceptedSocket);
			}
			else
			{
				pSession->SetSocket(nAcceptedSocket);
				if (!AddSession(pSession))
				{
					close(nAcceptedSocket);
					FreeSession(pSession);
				}
				else
				{ 
					//	添加一个连接断开的数据包到消息队列
					AddOneRecvedPacket(pSession->GetSessionID(), (LPacketSingle*)0xFFFFFFFF);
				}
			}
		}
	}
	map<int, LSelectServerSession*>::iterator _ito = pMapSocket->begin();
	while (_ito != pMapSocket->end())
	{
		int nTempSocket = _ito->first;
		LSelectServerSession* pSelectServerSession = _ito->second;
		if (pSelectServerSession == NULL)
		{
			_ito++;
			continue;
		}
		if (FD_ISSET(nTempSocket, &rdset))	//	接收数据
		{
			int nErrorID = pSelectServerSession->SessionRecv();
			if (nErrorID < 0)		//	需要关闭套接字
			{
				pSelectServerSession->SetSendable(false);
				AddOneCloseSessionWork(pSelectServerSession->GetSessionID());
				_ito++;
				continue;
			}
		}
		if (FD_ISSET(nTempSocket, &wrset)) 	//	可以发送数据了
		{
			pSelectServerSession->SetSendable(true);
		}
		_ito++;
	}

	return 1; 
}

int	LSelectServer::SendData()
{
	//	已经发送的数据包数量
	unsigned int unSendedPacket = 0;

	t_Send_Packet tSendPacket;
	memset(&tSendPacket, 0, sizeof(tSendPacket));

	while (GetOneSendPacket(tSendPacket))
	{
		//	取出数据包来发送
		LSelectServerSession* pSession = FindSession(tSendPacket.unSessionID);
		if (pSession != NULL)
		{
			int nSendResult = pSession->SessionSend(tSendPacket.pPacketForSend);
			if (nSendResult < 0)	//	出错，需要关闭连接
			{
				AddOneCloseSessionWork(tSendPacket.unSessionID);
			}
		}
		unSendedPacket++;
		if (unSendedPacket >= 2000)
		{
			break;
		}
		memset(&tSendPacket, 0, sizeof(tSendPacket));
	}
	return unSendedPacket;
}
#ifdef __EPOLL_TEST_STATISTIC__
void LSelectServer::PrintBufStatus()
{
	int nRcvAlloc = atomic_read(&g_nSelectServerRecvPacketAllocCount);
	int nRcvFree = atomic_read(&g_nSelectServerRecvPacketFreeCount);
	int nSndAlloc = atomic_read(&g_nSelectServerSendPacketAllocCount);
	int nSndFree = atomic_read(&g_nSelectServerSendPacketFreeCount);
	printf("RecvPacket Alloc:%d, Free:%d; SendPacket Alloc:%d, Free:%d\n", nRcvAlloc, nRcvFree, nSndAlloc, nSndFree);
}
#endif
//	接收相关	
bool LSelectServer::AddOneCloseSessionWork(unsigned int unSessionID)
{
	pthread_mutex_lock(&m_mutexForCloseSessionQueue);
	E_Circle_Error errorID = m_FixLenCircleBufForCloseSession.AddItems((char*)&unSessionID, 1);
	pthread_mutex_unlock(&m_mutexForCloseSessionQueue);
	if (errorID == E_Circle_Buf_No_Error)
	{
		return true;
	}
	char szError[512];
	sprintf(szError, "LSelectServer::AddOneCloseSessionWork, AddItems Error!\n");
	g_ErrorWriter.WriteError(szError, strlen(szError));
	return false;
}
bool LSelectServer::GetOneRecvedPacket(t_Recved_Packet& tRecvedPacket)
{
	E_Circle_Error errorID = m_FixLenCircleBufForRecvedPacket.GetOneItem((char*)&tRecvedPacket, sizeof(tRecvedPacket));
	if (errorID == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}
bool LSelectServer::FreeOneRecvedPacket(LPacketSingle* pRecvPacket)
{
#ifdef __EPOLL_TEST_STATISTIC__
	if (pRecvPacket != NULL)
	{
		atomic_inc(&g_nSelectServerRecvPacketFreeCount);
		pRecvPacket->FillPacketForTest();
	}
#endif
	if (!m_RecvPacketPoolManager.FreeOneItemToPool(pRecvPacket, pRecvPacket->GetPacketBufLen()))
	{ 
		char szError[512];
		sprintf(szError, "LSelectServer::FreeOneRecvedPacket, FreeOneItemToPool Failed!\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));

		delete pRecvPacket;
		return false;
	}
	return true;
}
LPacketSingle* LSelectServer::AllocOneRecvPacket(unsigned short usPacketLen)
{
	LPacketSingle* pPacket = NULL;
	pPacket = m_RecvPacketPoolManager.RequestOnePacket(usPacketLen);
	if (pPacket == NULL)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::AllocOneRecvPacket, RequestOnePacket Failed, Len:%hd\n", usPacketLen);
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
	else
	{
#ifdef __EPOLL_TEST_STATISTIC__
		atomic_inc(&g_nSelectServerRecvPacketAllocCount);
		pPacket->Reset();
#endif
	}
	return pPacket;
}
bool LSelectServer::AddOneRecvedPacket(unsigned int unSessionID, LPacketSingle* pRecvPacket)
{
	t_Recved_Packet trp;
	trp.unSessionID 	= unSessionID;
	trp.pPacketForRecv 	= pRecvPacket;

	E_Circle_Error errorID = m_FixLenCircleBufForRecvedPacket.AddItems((char*)&trp, 1);
	if (errorID != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::AddOneRecvedPacket, AddItems Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
bool LSelectServer::GetOneCloseSessionWork(unsigned int& unSessionID)
{
	E_Circle_Error errorID = m_FixLenCircleBufForCloseSession.GetOneItem((char*)&unSessionID, sizeof(unsigned int));
	if (errorID == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}



// 	发送相关
LPacketBroadCast* LSelectServer::AllocOneSendPacket(unsigned short usPacketLen)
{
	LPacketBroadCast* pSendPacket = m_SendPacketPoolManager.RequestOnePacket(usPacketLen);
	if (pSendPacket == NULL)
	{ 
		char szError[512];
		sprintf(szError, "LSelectServer::AllocOneSendPacket, pSendPacket == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
	else
	{
#ifdef __EPOLL_TEST_STATISTIC__
		atomic_inc(&g_nSelectServerSendPacketAllocCount);
		pSendPacket->Reset();
#endif
	}
	return pSendPacket;
}
bool LSelectServer::AddOneSendPacket(unsigned int unSessionID, LPacketBroadCast* pBroadCastPacket)
{
	//	每次发送，都增加一个引用计数
	pBroadCastPacket->IncrementRefCount();

	t_Send_Packet tsp;
	tsp.unSessionID 		= unSessionID;
	tsp.pPacketForSend 	= pBroadCastPacket;

	E_Circle_Error  errorID = m_FixLenCircleBufForSendPacketForMainLogic.AddItems((char*)&tsp, 1); 
	if (errorID != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::AddOneSendPacket, errorID != E_Circle_Buf_No_Error\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
void LSelectServer::PostAllSendPacket()
{
	if (!m_FixLenCircleBufForSendPacketForMainLogic.CopyAllItemsToOtherFixLenCircleBuf(&m_FixLenCircleBufForSendPacket))
	{ 
		char szError[512];
		sprintf(szError, "LSelectServer::PostAllSendPacket, Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
}
bool LSelectServer::FreeOneSendPacket(LPacketBroadCast* pBroadCastPacket)
{
	if (!m_SendPacketPoolManager.FreeOneItemToPool(pBroadCastPacket, pBroadCastPacket->GetPacketBufLen()))
	{ 
		char szError[512];
		sprintf(szError, "LSelectServer::FreeOneSendPacket, Failed\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#ifdef __EPOLL_TEST_STATISTIC__
	atomic_inc(&g_nSelectServerSendPacketFreeCount);
#endif
	return true;
}
bool LSelectServer::GetOneSendPacket(t_Send_Packet& tSendPacket)
{
	E_Circle_Error  errorID = m_FixLenCircleBufForSendPacket.GetOneItem((char*)&tSendPacket, sizeof(tSendPacket));
	if (errorID != E_Circle_Buf_No_Error)
	{
		return false;
	}
	return true;
}

//	分配一个连接
LSelectServerSession* LSelectServer::AllocSession()
{
	return m_SelectServerSessionManager.AllocSession();
}
//	查找一个连接
LSelectServerSession* LSelectServer::FindSession(unsigned int unSessionID)
{
	return m_SelectServerSessionManager.GetOneUsingSession(unSessionID);
}
//	释放一个连接
void LSelectServer::FreeSession(LSelectServerSession* pSelectServerSession)
{
	m_SelectServerSessionManager.FreeSession(pSelectServerSession);
}

//	删除一个连接
bool LSelectServer::RemoveSession(unsigned int unSessionID)
{
	return m_SelectServerSessionManager.RemoveOneUsingSession(unSessionID);
}

//	添加一个连接
bool LSelectServer::AddSession(LSelectServerSession* pSelectServerSession)
{
	return m_SelectServerSessionManager.AddOneUsingSession(pSelectServerSession);
}

//	===============================Select Server=============================== END



LSelectServerParamsReader::LSelectServerParamsReader()
{
	memset(&m_Params, 		0, sizeof(m_Params)); 
	memset(m_szIP, 			0, sizeof(m_szIP));
	memset(m_RecvBufDesc, 	0, sizeof(m_RecvBufDesc));
	m_unRecvBufDescCount 	= 0;
	memset(m_SendBufDesc, 	0, sizeof(m_SendBufDesc)); 
	m_unSendBufDescCount 	= 0;
}
LSelectServerParamsReader::~LSelectServerParamsReader()
{
} 
bool LSelectServerParamsReader::ReadIniFile(char* pszFileName)
{
	if (pszFileName == NULL)
	{
		return false;
	}
	LIniFileReadAndWrite iniReader;
	if (!iniReader.OpenIniFile(pszFileName))
	{
		return false;
	}
	char* pSection = "GlobalParams";

	char* pKey = "IP";
	if (iniReader.read_profile_string(pSection, pKey, m_szIP, sizeof(m_szIP) - 1, "") <= 0)
	{
		return false;
	}
	m_Params.pszIp = m_szIP;

	pKey = "PORT";
	int nPort = iniReader.read_profile_int(pSection, pKey, 0);
	if (nPort <= 0)
	{
		return false; 
	} 
	m_Params.usPort = (unsigned short)nPort;
	
	pKey = "ConnectWorkItemCount";
	int nConnectWorkItemCount = iniReader.read_profile_int(pSection, pKey, 0);
	if (nConnectWorkItemCount == 0)
	{
		nConnectWorkItemCount = 100;
	}
	m_Params.unConnectWorkItemCountForCircleBufLen = nConnectWorkItemCount;

	pKey = "SendQueueSizeForSession";
	int nSendQueueSizeForSession = iniReader.read_profile_int(pSection, pKey, 0);
	if (nSendQueueSizeForSession <= 0)
	{
		return false;
	}
	m_Params.unMaxSendQueueSize = nSendQueueSizeForSession;
	
	pKey = "MAXSESSIONCOUNT";
	int nMaxSessionCount = iniReader.read_profile_int(pSection, pKey, 0);
	if (nMaxSessionCount <= 0)
	{
		return false;
	}
	m_Params.unMaxSessionCount = nMaxSessionCount;

	pKey = "RecvedPacketQueueSize";
	int nRecvedPacketQueueSize = iniReader.read_profile_int(pSection, pKey, 0);
	if (nRecvedPacketQueueSize <= 0)
	{
		return false;
	}
	m_Params.unRecvedPacketQueueSize = nRecvedPacketQueueSize;

	pKey = "SendPacketQueueSize"; 
	int nSendPacketQueueSize = iniReader.read_profile_int(pSection, pKey, 0);
	if (nSendPacketQueueSize <= 0)
	{
		return false;
	}
	m_Params.unSendPacketSizeForSendThread = nSendPacketQueueSize;

	pKey = "SendPacketQueueSizeForMainLogicThread";
	int nSpqsfmlt = iniReader.read_profile_int(pSection, pKey, 0);
	if (nSpqsfmlt <= 0)
	{
		return false;
	}
	m_Params.unSendPacketSizeForMainLogic = nSpqsfmlt;

	pKey = "RecvBufTypeCount";
	int nrbtc = iniReader.read_profile_int(pSection, pKey, 0);
	if (nrbtc <= 0)
	{
		return false;
	}
	m_Params.usppdForRecvSize = nrbtc;

	pKey = "SendBufTypeCount";
	int nsbtc = iniReader.read_profile_int(pSection, pKey, 0);
	if (nsbtc <= 0)
	{
		return false;
	}
	m_Params.usppdForSendSize = nsbtc;

	pKey = "CloseQueueSize";
	int nCloseQueueSize = iniReader.read_profile_int(pSection, pKey, 0);
	if (nCloseQueueSize <= 0)
	{
		return false;
	}
	m_Params.unCloseWorkItemQueueSize = nCloseQueueSize;
	
	pKey = "RecvBufLenForSession";
	int nRecvBufLenForSession = iniReader.read_profile_int(pSection, pKey, 0);
	if (nRecvBufLenForSession <= 0)
	{
		return false;
	}
	m_Params.unRecvBufSize = nRecvBufLenForSession;

	m_Params.usListenListSize = 1000;

	pSection = "RecvBuf_Size";
	char szKey1[128];
	char szKey2[128];
	for (unsigned short usIndex = 0; usIndex < m_Params.usppdForRecvSize; ++usIndex)
	{
		memset(szKey1, 0, sizeof(szKey1));
		sprintf(szKey1, "rbs_%hd", usIndex + 1);
		int nLenType = iniReader.read_profile_int(pSection, szKey1, 0);
		if (nLenType <= 0)
		{
			return false;
		}
		m_RecvBufDesc[usIndex].usPacketLen = nLenType;
	}
	pSection = "RecvBuf_Desc";
	for (unsigned short usIndex = 0; usIndex < m_Params.usppdForRecvSize; ++usIndex)
	{
		sprintf(szKey1, "rbd%hd_init", usIndex + 1);
		sprintf(szKey2, "rbd%hd_max", usIndex + 1);
		int nInit = iniReader.read_profile_int(pSection, szKey1, 0);
		int nMax = iniReader.read_profile_int(pSection, szKey2, 0);
		if (nInit <= 0 || nMax <= 0)
		{
			return false;
		}
		if (nInit >= nMax)
		{
			return false;
		}
		m_RecvBufDesc[usIndex].unInitSize = nInit;
		m_RecvBufDesc[usIndex].unMaxAllocSize = nMax;
	}
	m_Params.pppdForRecv = m_RecvBufDesc;

	//	读取发送缓存分配描述
	pSection = "SendBuf_Size";	
	for (unsigned short usIndex = 0; usIndex < m_Params.usppdForSendSize; ++usIndex)
	{
		sprintf(szKey1, "sbs_%hd", usIndex + 1);
		int nLenType = iniReader.read_profile_int(pSection, szKey1, 0);
		if (nLenType <= 0)
		{
			return false;
		}
		m_SendBufDesc[usIndex].usPacketLen = nLenType;
	}

	pSection = "SendBuf_Desc";
	for (unsigned short usIndex = 0; usIndex < m_Params.usppdForSendSize; ++usIndex)
	{
		sprintf(szKey1, "sbd%hd_init", usIndex + 1);
		sprintf(szKey2, "sbd%hd_max", usIndex + 1);
		int nInit 	= iniReader.read_profile_int(pSection, szKey1, 0);
		int nMax 	= iniReader.read_profile_int(pSection, szKey2, 0);
		if (nInit <= 0 || nMax <= 0)
		{
			return false;
		}
		if (nInit >= nMax)
		{
			return false;
		}

		m_SendBufDesc[usIndex].unInitSize = nInit;
		m_SendBufDesc[usIndex].unMaxAllocSize = nMax;
	}
	m_Params.pppdForSend = m_SendBufDesc;
	return true;
}
t_Select_Server_Params& LSelectServerParamsReader::GetParams()
{
	return m_Params;
}


//	初始化连接管理器
bool LSelectServer::InitializeConnectWorkManagerThreadManager(unsigned int unMaxWorkItemCountInWorkQueue)
{ 
	if (unMaxWorkItemCountInWorkQueue == 0)
	{
		return false;
	}
	if (!m_FixLenCircleBufForConnectWork.Initialize(sizeof(t_Connector_Work_Result), unMaxWorkItemCountInWorkQueue))
	{
		return false;
	}
	if (!m_ConnectorWorkManager.Initialize(unMaxWorkItemCountInWorkQueue, 1, this, NULL))
	{
		return false;
	}

	if (!m_ConnectorWorkManager.Start())
	{
		return false;
	}
	return true;
}

//	添加一项连接工作
void LSelectServer::AddOneConnectWork(char* pszIP, unsigned short usPort, bool bMultiConnect)
{
	t_Connector_Work_Item workItem;
	strncpy(workItem.szIP, pszIP, sizeof(workItem.szIP) - 1);
	workItem.usPort = usPort;

	//	检查是否存在于已经连接或者正在连接的队列之中
//	for (size_t i = 0; i < m_vecWorkItem.size(); ++i)
//	{
//		t_Connector_Work_Item temp = m_vecWorkItem.at(i);
//		if ((strcmp(temp.szIP, workItem.szIP) == 0) && (temp.usPort == workItem.usPort))
//		{
//			return ;
//		}
//	}
	pthread_mutex_lock(&m_mutexForConnectToServer);
	if (!bMultiConnect)
	{
		for (size_t i = 0; i < m_vecConnectted.size(); ++i)
		{
			t_Connector_Work_Item temp = m_vecConnectted.at(i);
			if ((strcmp(temp.szIP, workItem.szIP) == 0) && (temp.usPort == workItem.usPort))
			{
				pthread_mutex_unlock(&m_mutexForConnectToServer);
				return ;
			}
		}
	}
//	for (size_t i = 0; i < m_vecConnectting.size(); ++i)
//	{ 
//		t_Connector_Work_Item temp = m_vecConnectting.at(i); 
//		if ((strcmp(temp.szIP, workItem.szIP) == 0) && (temp.usPort == workItem.usPort))
//		{
//			return ;
//		}
//	}

	//	将工作加入连接 工作 列表 中 

	if (m_ConnectorWorkManager.AddConnectWork(workItem.szIP, workItem.usPort, NULL, 0))
	{ 
		m_vecConnectted.push_back(workItem);
	}
	else
	{
		//	error print
	} 
	pthread_mutex_unlock(&m_mutexForConnectToServer);
}

//	完成一个连接工作
void LSelectServer::CompleteConnectWork(bool bSuccess, char* pszIP, unsigned short usPort, int nConnecttedSocket)
{
	//	连接成功，那么添加一个 连接 
	if (bSuccess)
	{
		t_Connector_Work_Result cwr;
		memset(&cwr, 0, sizeof(cwr));
		cwr.bSuccessed 	= bSuccess;
		strncpy(cwr.szIP, pszIP, sizeof(cwr.szIP) - 1);
		cwr.usPort 		= usPort;
		cwr.nSocket 	= nConnecttedSocket;
		E_Circle_Error errorID = m_FixLenCircleBufForConnectWork.AddItems((char*)&cwr, 1);
		if (errorID != E_Circle_Buf_No_Error)
		{
			close(nConnecttedSocket);
			//	error print
		}
	}
	//	否则
	else
	{
		pthread_mutex_lock(&m_mutexForConnectToServer);
		if (!m_ConnectorWorkManager.AddConnectWork(pszIP, usPort , NULL, 0))
		{
			//	error print
		}
		pthread_mutex_unlock(&m_mutexForConnectToServer);
	}
}


//	获取一个 已经 连接的连接工作 
bool LSelectServer::GetOneConnectedSession(t_Connector_Work_Result& cwr)
{
	E_Circle_Error errorID = m_FixLenCircleBufForConnectWork.GetOneItem((char*)&cwr, sizeof(cwr));
	if (errorID == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}



