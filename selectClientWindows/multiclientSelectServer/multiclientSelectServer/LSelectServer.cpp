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
	m_unRemainDataLen = 0;
	m_pszRecvBuf	= NULL;
	m_unRecvBufSize = 0;
	m_bSendable = true;
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
		char szError[512];
		sprintf(szError, "LSelectServerSession::Initialize, m_pSelectServer == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (tssp.unMaxSendQueueSize == 0)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::Initialize, tssp.unMaxSendQueueSize == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (tssp.unRecvBufSize == 0)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::Initialize, tssp.unRecvBufSize == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#ifdef __ADD_SEND_BUF_CHAIN__
	if (!m_FixLenCircleBufForSendQueue.Initialize(sizeof(LPacketBroadCast*), tssp.unMaxSendQueueSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::Initialize, !m_FixLenCircleBufForSendQueue.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	if (!m_FixLenCircleBufForSendQueue.Initialize(1, 128 * 1024))
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::Initialize, !m_FixLenCircleBufForSendQueue.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#endif

	//m_unRecvBufSize = tssp.unRecvBufSize;
	m_unRecvBufSize = 128 * 1024;
	m_pszRecvBuf = new char[m_unRecvBufSize];
	if (m_pszRecvBuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::Initialize, m_pszRecvBuf == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
int LSelectServerSession::SessionRecv()
{
	int nRecvedSize = Recv(m_pszRecvBuf + m_unRemainDataLen, m_unRecvBufSize - m_unRemainDataLen);
	if (nRecvedSize > 0)
	{
		m_unRemainDataLen += nRecvedSize;
		int nParsedLen = ParsePacket(m_pszRecvBuf, m_unRemainDataLen);
		if (nParsedLen < 0)
		{
			char szError[512];
			sprintf(szError, "LSelectServerSession::SessionRecv, nParsedLen < 0, SessionID:%u\n", m_unSessionID); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
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
				char szError[512];
				sprintf(szError, "LSelectServerSession::SessionRecv, nParsedLen > m_unRemainDataLen, SessionID:%u\n", m_unSessionID); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
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
		char szError[512];
		sprintf(szError, "LSelectServerSession::SessionRecv, nParsedLen > m_unRemainDataLen, SessionID:%u, ErrorID:%d\n", m_unSessionID, nRecvedSize); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return -1;
	}
	return 0;
}

#ifdef __ADD_SEND_BUF_CHAIN__
int LSelectServerSession::SessionSend(LPacketBroadCast* pPacket)
{
	if (!m_bSendable)
	{ 
		E_Circle_Error errorID = m_FixLenCircleBufForSendQueue.AddItems((char*)&pPacket, 1);
		if (errorID == E_Circle_Buf_No_Error)
		{
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
			return -1;
		}
	}
	else
	{
		int nCurrentCount = m_FixLenCircleBufForSendQueue.GetCurrentExistCount();
		if (nCurrentCount == 0)
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
		else
		{
			E_Circle_Error errorID = m_FixLenCircleBufForSendQueue.AddItems((char*)&pPacket, 1);
			if (errorID == E_Circle_Buf_No_Error)
			{
				LPacketBroadCast* pPacketForSendTemp = NULL;
				E_Circle_Error errorIDTemp = m_FixLenCircleBufForSendQueue.LookUpOneItem((char*)&pPacketForSendTemp, sizeof(LPacketBroadCast*));
				while (errorIDTemp == E_Circle_Buf_No_Error)
				{
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
	}
}

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
#endif
#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
bool LSelectServerSession::SessionSend(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::SessionSend, pPacket == NULL, SessionID:%u\n", m_unSessionID); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	E_Circle_Error errorID = m_FixLenCircleBufForSendQueue.AddItems(pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen());
	if (errorID != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::SessionSend, errorID != E_Circle_Buf_No_Error, SessionID:%u, ErrorID:%d\n", m_unSessionID, errorID); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
int LSelectServerSession::SendDataInBuf(bool& bSystemBufFull)
{
	int nContentLen = m_FixLenCircleBufForSendQueue.GetCurrentExistCount();
	if (nContentLen == 0)
	{
		return 0;
	}
	int nReadIndex = 0;
	int nWriteIndex = 0;
	m_FixLenCircleBufForSendQueue.GetCurrentReadAndWriteIndex(nReadIndex, nWriteIndex);
	if (nReadIndex == nWriteIndex)
	{
		return 0;
	}
	char* pBufStart = m_FixLenCircleBufForSendQueue.GetBuf();

	if (nReadIndex < nWriteIndex)
	{
		int nResult = Send(pBufStart + nReadIndex, nWriteIndex - nReadIndex);
		if (nResult > 0)
		{
			m_FixLenCircleBufForSendQueue.SetReadIndex(nWriteIndex);
			return nResult;
		}
		else
		{
			if (E_Socket_Send_System_Buf_Full == nResult)
			{
				bSystemBufFull = true;
				m_bSendable = false;
				return 0;
			}
			else
			{
				char szError[512];
				sprintf(szError, "LSelectServerSession::SendDataInBuf, SendError, SessionID:%u, Result:%d\n", m_unSessionID, nResult); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return -1;
			}
		}
	}
	else
	{
		int nSendCount = 0;

		int nResult = Send(pBufStart + nReadIndex, m_FixLenCircleBufForSendQueue.GetMaxItemCount() - nReadIndex);
		if (nResult > 0)
		{
			m_FixLenCircleBufForSendQueue.SetReadIndex(0);
			nSendCount += nResult;
		}
		else
		{
			if (nResult == E_Socket_Send_System_Buf_Full)
			{
				bSystemBufFull = true;
				m_bSendable = false;
				return nSendCount;
			}
			else
			{
				char szError[512];
				sprintf(szError, "LSelectServerSession::SendDataInBuf, SendError, step 1, SessionID:%u, Result:%d\n", m_unSessionID, nResult); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return -1;
			}
		}
		if (nWriteIndex == 0)
		{
			return nSendCount;
		}
		nResult = Send(pBufStart, nWriteIndex);
		if (nResult > 0)
		{
			m_FixLenCircleBufForSendQueue.SetReadIndex(nWriteIndex);
			nSendCount += nResult;
			return nSendCount;
		}
		else
		{
			if (nResult == E_Socket_Send_System_Buf_Full)
			{
				bSystemBufFull = true;
				m_bSendable = false;
				return nSendCount;
			}
			else
			{
				char szError[512];
				sprintf(szError, "LSelectServerSession::SendDataInBuf, SendError, step 2, SessionID:%u, Result:%d\n", m_unSessionID, nResult); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return -1;
			}
		}
	}
}
#endif

void LSelectServerSession::ReleaseAllPacketInSendQueue()
{ 
#ifdef __ADD_SEND_BUF_CHAIN__
	int nCurrentCount = m_FixLenCircleBufForSendQueue.GetCurrentExistCount();
	if (nCurrentCount == 0)
	{
		return ;
	}
	else
	{ 
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
#endif
}

int LSelectServerSession::ParsePacket(char* pData, unsigned int unDataLen)
{
	return unDataLen;
	unsigned int unParsedLen = 0;

	if (pData == NULL)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::ParsePacket, SessionID:%u\n", m_unSessionID); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return -1;
	}
	if (unDataLen < 2)
	{
		return 0;
	}
	unsigned short usPacketLen = *((unsigned short*)pData);
	if (usPacketLen > 8 * 1024)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::ParsePacket, usPacketLen > 8 * 1024, SessionID:%u\n", m_unSessionID); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return -1;
	}
	if (usPacketLen < sizeof(unsigned short)) 
	{
		char szError[512];
		sprintf(szError, "LSelectServerSession::ParsePacket, usPacketLen < sizeof(unsigned short), SessionID:%u\n", m_unSessionID); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return -2;
	}
	while (unDataLen >= usPacketLen)
	{
		LPacketSingle* pPacketSingle = m_pSelectServer->AllocOneRecvPacket(usPacketLen);
		if (pPacketSingle == NULL)
		{
			char szError[512];
			sprintf(szError, "LSelectServerSession::ParsePacket, pPacketSingle == NULL, SessionID:%u\n", m_unSessionID); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return -3;
		}
		pPacketSingle->DirectSetData(pData + sizeof(unsigned short), usPacketLen - sizeof(unsigned short));
		if (!m_pSelectServer->AddOneRecvedPacket(GetSessionID(), pPacketSingle))
		{
			char szError[512];
			sprintf(szError, "LSelectServerSession::ParsePacket, AddOneRecvedPacket Failed, SessionID:%u\n", m_unSessionID); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return -4;
		}

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
				char szError[512];
				sprintf(szError, "LSelectServerSession::ParsePacket, usPacketLen > 8 * 1024 second, SessionID:%u\n", m_unSessionID); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return -1;
			}
			if (usPacketLen < sizeof(unsigned short)) 
			{
				char szError[512];
				sprintf(szError, "LSelectServerSession::ParsePacket, usPacketLen < sizeof(unsigned short) second, SessionID:%u\n", m_unSessionID); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
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
	m_nMaxSocketPlus1 = 0;
	m_mapSocketMapToSession.clear();
}
LSelectServerSessionManager::~LSelectServerSessionManager()
{
}
bool LSelectServerSessionManager::Initialize(t_Select_Server_Params& tssp)
{
	if (m_pSelectServer == NULL)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::Initialize, m_pSelectServer == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (tssp.unMaxSessionCount == 0)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::Initialize, tssp.unMaxSessionCount == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < tssp.unMaxSessionCount; ++unIndex)
	{ 
		LSelectServerSession* pSession = new LSelectServerSession;
		if (pSession == NULL)
		{
			char szError[512];
			sprintf(szError, "LSelectServerSessionManager::Initialize, pSession == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		pSession->SetSelectServer(m_pSelectServer);
		if (!pSession->Initialize(tssp))
		{	
			char szError[512];
			sprintf(szError, "LSelectServerSessionManager::Initialize, !pSession->Initialize\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
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
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::AllocSession, return NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
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
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::FreeSession, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return ;
	} 
	pSession->Reset();
	m_queueSessionPool.push(pSession);
}

bool LSelectServerSessionManager::AddOneUsingSession(LSelectServerSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::AddOneUsingSession, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	unsigned int unSessionID = pSession->GetSessionID();
	if (unSessionID == 0)
	{
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::AddOneUsingSession, unSessionID == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	map<unsigned int, LSelectServerSession*>::iterator _ito = m_mapSessionManager.find(unSessionID);
	if (_ito != m_mapSessionManager.end())
	{
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::AddOneUsingSession, _ito != m_mapSessionManager.end()\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_mapSessionManager[unSessionID] = pSession;
	FD_SET(pSession->GetSocket(), &m_SETRecvSocket);
	m_mapSocketMapToSession[pSession->GetSocket()] = pSession;
	return true;
}

LSelectServerSession* LSelectServerSessionManager::GetOneUsingSession(unsigned int unSessionID)
{
	map<unsigned int, LSelectServerSession*>::iterator _ito = m_mapSessionManager.find(unSessionID);
	if (_ito != m_mapSessionManager.end())
	{
		return _ito->second;
	}
	char szError[512];
	sprintf(szError, "LSelectServerSessionManager::GetOneUsingSession, return NULL, SessionID:%u\n", unSessionID); 
	g_ErrorWriter.WriteError(szError, strlen(szError));
	return NULL; 
}

bool LSelectServerSessionManager::RemoveOneUsingSession(unsigned int unSessionID)
{
	map<unsigned int, LSelectServerSession*>::iterator _ito = m_mapSessionManager.find(unSessionID);
	if (_ito != m_mapSessionManager.end())
	{
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::RemoveOneUsingSession, SessionID:%u\n", unSessionID); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		int nSocket = _ito->second->GetSocket();
		map<int, LSelectServerSession*>::iterator _itoMapToSession = m_mapSocketMapToSession.find(nSocket);
		if (_itoMapToSession != m_mapSocketMapToSession.end())
		{
			m_mapSocketMapToSession.erase(_itoMapToSession);
		}
#ifndef WIN32
		FD_CLR(nSocket, &m_SETRecvSocket);
		FD_CLR(nSocket, &m_SETSendSocket);
		close(nSocket);		
#else
		FD_CLR(nSocket, &m_SETRecvSocket);
		FD_CLR(nSocket, &m_SETSendSocket);
		closesocket(nSocket);		
#endif
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

fd_set& LSelectServerSessionManager::GetRecvSet()
{
	return m_SETRecvSocket;
}
fd_set& LSelectServerSessionManager::GetSendSet()
{
	return m_SETSendSocket;
}

void LSelectServerSessionManager::RemoveFromSendSet(int nSocket)
{
	FD_CLR(nSocket, &m_SETSendSocket);
}
void LSelectServerSessionManager::RemoveFromRecvSet(int nSocket)
{
	FD_CLR(nSocket, &m_SETRecvSocket);
}
void LSelectServerSessionManager::AddToSendSet(int nSocket)
{
	FD_SET(nSocket, &m_SETSendSocket);
}
void LSelectServerSessionManager::AddToRecvSet(int nSocket)
{
	FD_SET(nSocket, &m_SETRecvSocket);
}
LSelectServerSession* LSelectServerSessionManager::GetSessionFromSocketToSession(int nSocket)
{
	map<int, LSelectServerSession*>::iterator _ito = m_mapSocketMapToSession.find(nSocket);
	if (_ito == m_mapSocketMapToSession.end())
	{
		char szError[512];
		sprintf(szError, "LSelectServerSessionManager::GetSessionFromSocketToSession, return NULL, SocketID:%d\n", nSocket); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
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
#ifndef WIN32
	pthread_mutex_init(&m_mutexForCloseSessionQueue, NULL);
	pthread_mutex_init(&m_mutexForConnectToServer, NULL);
#else
	InitializeCriticalSectionAndSpinCount(&m_mutexForCloseSessionQueue, 4000);
	InitializeCriticalSectionAndSpinCount(&m_mutexForConnectToServer, 4000);
#endif
#ifdef __EPOLL_TEST_STATISTIC__
	atomic_set(&g_nSelectServerRecvPacketAllocCount, 0);
	atomic_set(&g_nSelectServerRecvPacketFreeCount, 0);
	atomic_set(&g_nSelectServerSendPacketAllocCount, 0);
	atomic_set(&g_nSelectServerSendPacketFreeCount, 0);
#endif
}
LSelectServer::~LSelectServer()
{
	m_ConnectorWorkManager.Stop();
#ifndef WIN32
	pthread_mutex_destroy(&m_mutexForCloseSessionQueue);
	pthread_mutex_destroy(&m_mutexForConnectToServer);
#else
	DeleteCriticalSection(&m_mutexForCloseSessionQueue);
	DeleteCriticalSection(&m_mutexForConnectToServer);
#endif
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
	m_SelectServerSessionManager.SetSelectServer(this);
	if (!m_SelectServerSessionManager.Initialize(tssp))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_SelectServerSessionManager.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	if (!m_FixLenCircleBufForRecvedPacket.Initialize(sizeof(t_Recved_Packet), tssp.unRecvedPacketQueueSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_FixLenCircleBufForRecvedPacket.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_FixLenCircleBufForCloseSession.Initialize(sizeof(unsigned int), tssp.unCloseWorkItemQueueSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_FixLenCircleBufForCloseSession.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_RecvPacketPoolManager.Initialize(tssp.pppdForRecv, tssp.usppdForRecvSize))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_RecvPacketPoolManager.Initialize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#ifdef __ADD_SEND_BUF_CHAIN__
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
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	if (!m_FixLenCircleBufForSendSessionID.Initialize(sizeof(t_Send_Packet), tssp.unSendPacketSizeForMainLogic))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::Initialize, !m_FixLenCircleBufForSendSessionID.Initialize\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#endif
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
	m_SelectServerSessionManager.AddToRecvSet(GetSocket());
	return true;
}
int LSelectServer::ThreadDoing(void* pParam)
{ 
	while (1)
	{
		unsigned short usProcessedCloseWorkCount = 0;
		unsigned int unSessionIDForClose = 0;
		while (GetOneCloseSessionWork(unSessionIDForClose))
		{
			char szError[512];
			sprintf(szError, "LSelectServer::ThreadDoing, SessionToClose, ID:%u\n", unSessionIDForClose);
			g_ErrorWriter.WriteError(szError, strlen(szError));

			LSelectServerSession* pSession = FindSession(unSessionIDForClose);	
			if (pSession != NULL)
			{
				char szError[512];
				sprintf(szError, "LSelectServer::ThreadDoing, SessionToClose, ISClosing, ID:%u\n", unSessionIDForClose);
				g_ErrorWriter.WriteError(szError, strlen(szError));

				bool bReconnect = pSession->GetReconnect();
				pSession->ReleaseAllPacketInSendQueue();
				RemoveSession(unSessionIDForClose);
				FreeSession(pSession);
				AddOneRecvedPacket(unSessionIDForClose, NULL);
				if (bReconnect)
				{
//					char szIP[32];
//					unsigned short usPort = 0;
//#ifndef WIN32
//					pthread_mutex_lock(&m_mutexForConnectToServer);
//#else
//					EnterCriticalSection(&m_mutexForConnectToServer);
//#endif
//					if (!m_ConnectorWorkManager.AddConnectWork(szIP, usPort, NULL, 0))
//					{
//						//	error print
//					}
//#ifndef WIN32
//					pthread_mutex_unlock(&m_mutexForConnectToServer);
//#else
//					LeaveCriticalSection(&m_mutexForConnectToServer);
//#endif
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
				char szError[512];
				sprintf(szError, "LSelectServer::ThreadDoing, AllocSession, pSession == NULL\n");
				g_ErrorWriter.WriteError(szError, strlen(szError));
#ifndef WIN32
				close(cwr.nSocket);
				pthread_mutex_lock(&m_mutexForConnectToServer);
#else
				closesocket(cwr.nSocket);
				EnterCriticalSection(&m_mutexForConnectToServer);
#endif

				
				if (!m_ConnectorWorkManager.AddConnectWork(cwr.szIP, cwr.usPort, NULL, 0))
				{
					//	error print
				}
#ifndef WIN32
				pthread_mutex_unlock(&m_mutexForConnectToServer);
#else
				LeaveCriticalSection(&m_mutexForConnectToServer);
#endif
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
				int nSendBufLen = 0;
				int nRecvBufLen = 0;
				socklen_t sLen = sizeof(int);
				socklen_t sLenrcv = sizeof(int);
				pSession->GetSockOpt(SOL_SOCKET, SO_SNDBUF, &nSendBufLen, &sLen);
				pSession->GetSockOpt(SOL_SOCKET, SO_RCVBUF, &nRecvBufLen, &sLenrcv);
				nRecvBufLen = 130 * 1024;
				if (pSession->SetSockOpt(SOL_SOCKET, SO_RCVBUF, &nRecvBufLen, sLenrcv) != 0)
				{
					//OutputDebugString("Set Recv Buf Len Error");
				}
				nRecvBufLen = 0;
				pSession->GetSockOpt(SOL_SOCKET, SO_RCVBUF, &nRecvBufLen, &sLenrcv);
				if (!AddSession(pSession))
				{
					char szError[512];
					sprintf(szError, "LSelectServer::ThreadDoing, AddSession(pSession), SessionID:%u\n", pSession->GetSessionID());
					g_ErrorWriter.WriteError(szError, strlen(szError));
#ifndef WIN32
					//m_SelectServerSessionManager.RemoveFromRecvSet(cwr.nSocket);
					close(cwr.nSocket);					
					pthread_mutex_lock(&m_mutexForConnectToServer);
#else
					//m_SelectServerSessionManager.RemoveFromRecvSet(cwr.nSocket);
					closesocket(cwr.nSocket);
					EnterCriticalSection(&m_mutexForConnectToServer);
#endif
					
					if (!m_ConnectorWorkManager.AddConnectWork(cwr.szIP, cwr.usPort, NULL, 0))
					{
						//	error print
					}
#ifndef WIN32
					pthread_mutex_unlock(&m_mutexForConnectToServer);
#else
					LeaveCriticalSection(&m_mutexForConnectToServer);
#endif
					FreeSession(pSession);
				}
				else
				{ 
					pSession->SetReconnect(true);
					AddOneRecvedPacket(pSession->GetSessionID(), (LPacketSingle*)0xFFFFFFFF);
				}
			}
			nProcessConnecttedWorkCount++;
			if (nProcessConnecttedWorkCount >= 200)
			{
				break;
			}
		}

		int nWorkCount = CheckForSocketEvent();
		if (nWorkCount < 0)
		{
			char szError[512];
			sprintf(szError, "LSelectServer::ThreadDoing, Thread quit 1\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return 0;
		}
		int nSendWorkCount = SendData();
		if (nWorkCount == 0 && nSendWorkCount == 0)
		{
			//	sched_yield();
#ifndef WIN32
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
#else
			Sleep(0);
#endif
		}
		if (CheckForStop())
		{
			char szError[512];
			sprintf(szError, "LSelectServer::ThreadDoing, Thread quit 2\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return 0;
		} 
	}
	char szError[512];
	sprintf(szError, "LSelectServer::ThreadDoing, Thread quit 3\n");
	g_ErrorWriter.WriteError(szError, strlen(szError));
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
	//if (!m_SelectServerSessionManager.BuildSet())
	//{
	//	return -1;
	//}

	fd_set rdset = m_SelectServerSessionManager.GetRecvSet();
	fd_set wrset = m_SelectServerSessionManager.GetSendSet();
	map<int, LSelectServerSession*>* pMapSocket = m_SelectServerSessionManager.GetSessionManagerForEvent();

	struct timeval val;
	val.tv_sec 		= 0;
	val.tv_usec 	= 0;
	int iReturn = select(m_SelectServerSessionManager.GetMaxSocketPlus1(), &rdset, &wrset, NULL, &val);	
	if (iReturn == -1)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::CheckForSocketEvent, select Error, ErrorID:%d\n", iReturn);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return -1;
	}
	else if (iReturn == 0)
	{
		return 0;
	}
	int nListenSocket = GetSocket(); 
	if (FD_ISSET(nListenSocket, &rdset))
	{
		int nAcceptedSocket = accept(nListenSocket, NULL, NULL);
		if (nAcceptedSocket != -1)
		{
			LSelectServerSession* pSession = AllocSession();
			if (pSession == NULL)
			{
				char szError[512];
				sprintf(szError, "LSelectServer::CheckForSocketEvent, AllocSession(), pSession == NULL\n");
				g_ErrorWriter.WriteError(szError, strlen(szError));
#ifndef WIN32
				close(nAcceptedSocket);
#else
				closesocket(nAcceptedSocket);
#endif
			}
			else
			{
				pSession->SetSocket(nAcceptedSocket);
				if (!AddSession(pSession))
				{
					char szError[512];
					sprintf(szError, "LSelectServer::CheckForSocketEvent, !AddSession\n");
					g_ErrorWriter.WriteError(szError, strlen(szError));
#ifndef WIN32
					//m_SelectServerSessionManager.RemoveFromRecvSet(nAcceptedSocket);
					close(nAcceptedSocket);
#else
					//m_SelectServerSessionManager.RemoveFromRecvSet(nAcceptedSocket);
					closesocket(nAcceptedSocket);
#endif
					FreeSession(pSession);
				}
				else
				{ 
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
			char szError[512];
			sprintf(szError, "LSelectServer::CheckForSocketEvent, pSelectServerSession == NULL\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			_ito++;
			continue;
		}
		if (FD_ISSET(nTempSocket, &rdset))
		{
			int nErrorID = pSelectServerSession->SessionRecv();
			if (nErrorID < 0)
			{
				char szError[512];
				sprintf(szError, "LSelectServer::CheckForSocketEvent, pSelectServerSession->SessionRecv(), ErrorID:%d\n", nErrorID);
				g_ErrorWriter.WriteError(szError, strlen(szError));

				pSelectServerSession->SetSendable(false);
				AddOneCloseSessionWork(pSelectServerSession->GetSessionID());
				_ito++;
				continue;
			}
		}
		if (FD_ISSET(nTempSocket, &wrset))
		{
			m_SelectServerSessionManager.RemoveFromSendSet(nTempSocket);
			pSelectServerSession->SetSendable(true);
		}
		_ito++;
	}

	return 1; 
}

int	LSelectServer::SendData()
{
	unsigned int unSendedPacket = 0;

	t_Send_Packet tSendPacket;
	memset(&tSendPacket, 0, sizeof(tSendPacket));
#ifdef __ADD_SEND_BUF_CHAIN__
	while (GetOneSendPacket(tSendPacket))
	{
		LSelectServerSession* pSession = FindSession(tSendPacket.unSessionID);
		if (pSession != NULL)
		{
			int nSendResult = pSession->SessionSend(tSendPacket.pPacketForSend);
			if (nSendResult < 0)
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
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	while (GetOneSessionToSend(tSendPacket))
	{
		
		LSelectServerSession* pSession = FindSession(tSendPacket.unSessionID);
		if (pSession != NULL)
		{
			bool bSystemBufFull = false;
			int nSendResult = pSession->SendDataInBuf(bSystemBufFull);
			if (nSendResult < 0)	
			{
				char szError[512];
				sprintf(szError, "LSelectServer::SendData, pSession->SendDataInBuf(bSystemBufFull), ErrorID:%d\n", nSendResult);
				g_ErrorWriter.WriteError(szError, strlen(szError));

				AddOneCloseSessionWork(tSendPacket.unSessionID);
			}
			if (bSystemBufFull)
			{
				m_SelectServerSessionManager.AddToSendSet(pSession->GetSocket());
			}
		}
		unSendedPacket++;
		if (unSendedPacket >= 2000)
		{
			break;
		}
		memset(&tSendPacket, 0, sizeof(tSendPacket));
	}
#endif

	return unSendedPacket;
}

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
bool LSelectServer::SendPacket(unsigned int unSessionID, LPacketSingle* pPacket)
{
	LSelectServerSession* pSession = FindSession(unSessionID);
	if (pSession != NULL)
	{
		if (!pSession->SessionSend(pPacket))
		{
			char szError[512];
			sprintf(szError, "LSelectServer::SendPacket, !pSession->SessionSend, SessionID:%u\n", unSessionID);
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
	}
	else
	{
		char szError[512];
		sprintf(szError, "LSelectServer::SendPacket, pSession != NULL, SessionID:%u\n", unSessionID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	t_Send_Packet tsp;
	tsp.unSessionID 		= unSessionID;

	E_Circle_Error  errorID = m_FixLenCircleBufForSendSessionID.AddItems((char*)&tsp, 1);
	if (errorID != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::SendPacket, errorID != E_Circle_Buf_No_Error\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}

bool LSelectServer::GetOneSessionToSend(t_Send_Packet& tSendPacket)
{
	E_Circle_Error  errorID = m_FixLenCircleBufForSendSessionID.GetOneItem((char*)&tSendPacket, sizeof(tSendPacket));
	if (errorID != E_Circle_Buf_No_Error)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::GetOneSessionToSend, errorID != E_Circle_Buf_No_Error, ErrorId:%d\n", errorID);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
#endif

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
bool LSelectServer::AddOneCloseSessionWork(unsigned int unSessionID)
{
#ifndef WIN32
	pthread_mutex_lock(&m_mutexForCloseSessionQueue);
#else
	EnterCriticalSection(&m_mutexForCloseSessionQueue);
#endif
	E_Circle_Error errorID = m_FixLenCircleBufForCloseSession.AddItems((char*)&unSessionID, 1);
#ifndef WIN32
	pthread_mutex_unlock(&m_mutexForCloseSessionQueue);
#else
	LeaveCriticalSection(&m_mutexForCloseSessionQueue);
#endif
	if (errorID == E_Circle_Buf_No_Error)
	{
		return true;
	}
	char szError[512];
	sprintf(szError, "LSelectServer::AddOneCloseSessionWork, AddItems Error!, SessionID:%u\n", unSessionID);
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


#ifdef __ADD_SEND_BUF_CHAIN__
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
#endif

LSelectServerSession* LSelectServer::AllocSession()
{
	return m_SelectServerSessionManager.AllocSession();
}
LSelectServerSession* LSelectServer::FindSession(unsigned int unSessionID)
{
	return m_SelectServerSessionManager.GetOneUsingSession(unSessionID);
}
void LSelectServer::FreeSession(LSelectServerSession* pSelectServerSession)
{
	m_SelectServerSessionManager.FreeSession(pSelectServerSession);
}

bool LSelectServer::RemoveSession(unsigned int unSessionID)
{
	return m_SelectServerSessionManager.RemoveOneUsingSession(unSessionID);
}

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


bool LSelectServer::InitializeConnectWorkManagerThreadManager(unsigned int unMaxWorkItemCountInWorkQueue)
{ 
	if (unMaxWorkItemCountInWorkQueue == 0)
	{
		char szError[512];
		sprintf(szError, "LSelectServer::InitializeConnectWorkManagerThreadManager, unMaxWorkItemCountInWorkQueue == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_FixLenCircleBufForConnectWork.Initialize(sizeof(t_Connector_Work_Result), unMaxWorkItemCountInWorkQueue))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::InitializeConnectWorkManagerThreadManager, !m_FixLenCircleBufForConnectWork.Initialize\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!m_ConnectorWorkManager.Initialize(unMaxWorkItemCountInWorkQueue, 1, this, NULL))
	{
		char szError[512];
		sprintf(szError, "LSelectServer::InitializeConnectWorkManagerThreadManager, !m_ConnectorWorkManager.Initialize\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	if (!m_ConnectorWorkManager.Start())
	{
		char szError[512];
		sprintf(szError, "LSelectServer::InitializeConnectWorkManagerThreadManager, !m_ConnectorWorkManager.Start()\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}

void LSelectServer::AddOneConnectWork(char* pszIP, unsigned short usPort, bool bMultiConnect)
{
	t_Connector_Work_Item workItem;
	strncpy(workItem.szIP, pszIP, sizeof(workItem.szIP) - 1);
	workItem.usPort = usPort;

//	for (size_t i = 0; i < m_vecWorkItem.size(); ++i)
//	{
//		t_Connector_Work_Item temp = m_vecWorkItem.at(i);
//		if ((strcmp(temp.szIP, workItem.szIP) == 0) && (temp.usPort == workItem.usPort))
//		{
//			return ;
//		}
//	}
#ifndef WIN32
	pthread_mutex_lock(&m_mutexForConnectToServer);
#else
	EnterCriticalSection(&m_mutexForConnectToServer);
#endif
	if (!bMultiConnect)
	{
		for (size_t i = 0; i < m_vecConnectted.size(); ++i)
		{
			t_Connector_Work_Item temp = m_vecConnectted.at(i);
			if ((strcmp(temp.szIP, workItem.szIP) == 0) && (temp.usPort == workItem.usPort))
			{
#ifndef WIN32
				pthread_mutex_unlock(&m_mutexForConnectToServer);
#else
				LeaveCriticalSection(&m_mutexForConnectToServer);
#endif
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


	if (m_ConnectorWorkManager.AddConnectWork(workItem.szIP, workItem.usPort, NULL, 0))
	{ 
		m_vecConnectted.push_back(workItem);
	}
	else
	{
		//	error print
	} 
#ifndef WIN32
	pthread_mutex_unlock(&m_mutexForConnectToServer);
#else
	LeaveCriticalSection(&m_mutexForConnectToServer);
#endif
}

void LSelectServer::CompleteConnectWork(bool bSuccess, char* pszIP, unsigned short usPort, int nConnecttedSocket)
{
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
			char szError[512];
			sprintf(szError, "LSelectServer::CompleteConnectWork, errorID != E_Circle_Buf_No_Error, ErrorID:%d\n", errorID);
			g_ErrorWriter.WriteError(szError, strlen(szError));
#ifndef WIN32
			close(nConnecttedSocket);
#else
			closesocket(nConnecttedSocket);
#endif
			//	error print
		}
	}
	else
	{
#ifndef WIN32
		pthread_mutex_lock(&m_mutexForConnectToServer);
		if (!m_ConnectorWorkManager.AddConnectWork(pszIP, usPort , NULL, 0))
		{
			char szError[512];
			sprintf(szError, "LSelectServer::CompleteConnectWork, !m_ConnectorWorkManager.AddConnectWork\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			//	error print
		}
		pthread_mutex_unlock(&m_mutexForConnectToServer);
#else
		EnterCriticalSection(&m_mutexForConnectToServer);
		if (!m_ConnectorWorkManager.AddConnectWork(pszIP, usPort , NULL, 0))
		{
			char szError[512];
			sprintf(szError, "LSelectServer::CompleteConnectWork, !m_ConnectorWorkManager.AddConnectWork\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			//	error print
		}
		LeaveCriticalSection(&m_mutexForConnectToServer);
#endif
	}
}


bool LSelectServer::GetOneConnectedSession(t_Connector_Work_Result& cwr)
{
	E_Circle_Error errorID = m_FixLenCircleBufForConnectWork.GetOneItem((char*)&cwr, sizeof(cwr));
	if (errorID == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}


