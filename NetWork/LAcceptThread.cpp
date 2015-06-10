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

#include "LAcceptThread.h"
#include "LSessionManager.h"
#include "LRecvThreadManager.h"
#include "LSendThreadManager.h"
#include "LEpollThreadManager.h"
#include "LNetWorkServices.h"
#include "LErrorWriter.h"
#include "LPacketSingle.h"
#include <sys/prctl.h>
#include "LCloseSocketThread.h"

extern int errno;
extern LErrorWriter g_ErrorWriter;

#define ACCEPTED_PACKET_BUF_LEN  (8 * 1024 + 32) 

LAcceptThread::LAcceptThread(void)
{
	m_pNetWorkServices 	= NULL; 
	memset(m_szListenIp, 0, sizeof(m_szListenIp));
	m_usListenPort 		= 0;
}

LAcceptThread::~LAcceptThread(void)
{
}

bool LAcceptThread::Initialize(char* pListenIP, unsigned short usListenPort, unsigned int unListenListSize, unsigned int unWillServClientNum, bool bInitialListen) 
{
	if (bInitialListen)
	{
		if (pListenIP == NULL)
		{
			char szError[512];
			sprintf(szError, "LAcceptThread::Initialize, pListenIP == NULL\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		if (usListenPort == 0)
		{
			char szError[512];
			sprintf(szError, "LAcceptThread::Initialize, usListenPort == 0\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		strncpy(m_szListenIp, pListenIP, sizeof(m_szListenIp) - 1);
		m_usListenPort = usListenPort;

		if (unListenListSize == 0)
		{
			unListenListSize = 1000;
		}
		if (!m_listenSocket.Initialized(pListenIP, usListenPort))
		{
			char szError[512];
			sprintf(szError, "LAcceptThread::Initialize, m_listenSocket.Initialized\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		if (!m_listenSocket.Listen(unListenListSize))
		{
			char szError[512];
			sprintf(szError, "LAcceptThread::Initialize, !m_listenSocket.Listen\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
	}	
	if (!m_AccpetedSessionPacket.Initialize(sizeof(t_Recv_Packet), 10))
	{
		return false;
	}
	if (!m_AcceptedPacketPool.Initialize(sizeof(LPacketSingle*), unWillServClientNum + 2))
	{
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < unWillServClientNum; ++unIndex)
	{
		LPacketSingle* pPacket = new LPacketSingle(ACCEPTED_PACKET_BUF_LEN);
		if (pPacket == NULL)
		{
			return false;
		}
		pPacket->SetPacketType(1);
		E_Circle_Error error = m_AcceptedPacketPool.AddItems((char*)(&pPacket), 1);
		if (error != E_Circle_Buf_No_Error)
		{
			return false;
		}
	}
	return true;
}
int LAcceptThread::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "AcceptThread");
	prctl(PR_SET_NAME, szThreadName);

	while(true)
	{
		if (CheckForStop())		//	收到线程退出通知，那么退出线程
		{
			break;
		}

		int newClient = accept(m_listenSocket.GetSocket(), NULL, NULL);
		if (newClient == -1)
		{
			if (errno == EAGAIN || errno == EINTR)		//	
			{
				struct timespec timeReq;
				timeReq.tv_sec 	= 0;
				timeReq.tv_nsec = 10;
				nanosleep(&timeReq, NULL);
				continue;
			}
			else
			{
				char szError[512];
				sprintf(szError, "LAcceptThread::ThreadDoing, Accpeted Failed, ErrorCode:%d\n", errno);
				g_ErrorWriter.WriteError(szError, strlen(szError));
				break;		//	出错，那么退出接收线程
			}
		}

		//		she zhi wei fei zu sai tao jie zi 
		int nSetNonBlockSuccess = fcntl(newClient, F_SETFL, O_NONBLOCK);
		if (nSetNonBlockSuccess == -1)
		{
			close(newClient);
			char szError[512];
			sprintf(szError, "LAcceptThread::ThreadDoing, SetNonBlock Failed\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			continue;
		}
	
		//	we have client connect
		LMasterSessionManager* pMasterSessManager = &m_pNetWorkServices->GetSessionManager();
		LSession* pSession = pMasterSessManager->AllocSession();
		if (pSession == NULL)
		{
			close(newClient); 
			char szError[512];
			sprintf(szError, "LAcceptThread::ThreadDoing, AllocSession Failed\n");
			g_ErrorWriter.WriteError(szError, strlen(szError));
			continue;
		}
		pSession->SetSocket(newClient);
		pSession->SetSessionConnecttedTime();

//		int nRecvBufLen = 0;
//		unsigned int nRecvBufLenLen = sizeof(int);
//		int nSendBufLen = 0;
//		unsigned int nSendBufLenLen = sizeof(int);
//		if (pSession->GetSockOpt(SOL_SOCKET, SO_RCVBUF, &nRecvBufLen, &nRecvBufLenLen) != 0)
//		{
//
//		}
//		if (pSession->GetSockOpt(SOL_SOCKET, SO_SNDBUF, &nSendBufLen, &nSendBufLenLen) != 0)
//		{
//
//		}
		LRecvThreadManager* pRecvThreadManager = &m_pNetWorkServices->GetRecvThreadManager();
		if (!pRecvThreadManager->BindRecvThread(pSession))
		{ 
			char szError[512];
			sprintf(szError, "LAcceptThread::ThreadDoing, BindRecvThread Failed, SessionID:%lld\n", pSession->GetSessionID());
			g_ErrorWriter.WriteError(szError, strlen(szError));

			pMasterSessManager->FreeSession(pSession);
			pMasterSessManager->FreeSessionToPool(pSession);
			continue;
		}

		LSendThreadManager* pSendThreadManager = &m_pNetWorkServices->GetSendThreadManager();
		if (!pSendThreadManager->BindSendThread(pSession))
		{
			char szError[512];
			sprintf(szError, "LAcceptThread::ThreadDoing, BindSendThread Failed, SessionID:%lld\n", pSession->GetSessionID());
			g_ErrorWriter.WriteError(szError, strlen(szError));

			pRecvThreadManager->UnBindRecvThread(pSession);
			pMasterSessManager->FreeSession(pSession);
			pMasterSessManager->FreeSessionToPool(pSession);
			continue;
		}



		t_Recv_Packet tRecvPacket;		// tong zhi luo ji xian cheng ,ke hu  duan lian jie dao da
		LPacketSingle* pPacket = GetOneAcceptedPacket();
		//	将必要的信息填入数据包中
		BuildAcceptedPacket(pSession, pPacket);
		unsigned short usExtDataLen = 0;
		pPacket->AddUShort(usExtDataLen);

		tRecvPacket.u64SessionID = pSession->GetSessionID();
		tRecvPacket.pPacket = pPacket;
		if (m_AccpetedSessionPacket.AddItems((char*)&tRecvPacket, 1) != E_Circle_Buf_No_Error)
		{ 
			char szError[512];
			sprintf(szError, "LAcceptThread::ThreadDoing, AddItems Failed, can not notice client connect, SessionID:%lld\n", pSession->GetSessionID());
			g_ErrorWriter.WriteError(szError, strlen(szError));
			
			pSendThreadManager->UnBindSendThread(pSession);
			pRecvThreadManager->UnBindRecvThread(pSession);
			pMasterSessManager->FreeSession(pSession); 
			pMasterSessManager->FreeSessionToPool(pSession);
			continue;
		}



		if (!m_pNetWorkServices->CommitPackets(&m_AccpetedSessionPacket))
		{ 
			char szError[512];
			sprintf(szError, "LAcceptThread::ThreadDoing, CommitPackets Failed, can not notice client connect, pos2, SessionID:%lld\n", pSession->GetSessionID());
			g_ErrorWriter.WriteError(szError, strlen(szError));

			pSendThreadManager->UnBindSendThread(pSession);
			pRecvThreadManager->UnBindRecvThread(pSession);
			pMasterSessManager->FreeSession(pSession); 
			pMasterSessManager->FreeSessionToPool(pSession);
			continue;
		}

		LEpollThreadManager* pEpollThreadManager = &m_pNetWorkServices->GetEpollThreadManager();
		if (!pEpollThreadManager->BindEpollThread(pSession))
		{
			tRecvPacket.pPacket = (LPacketSingle*)0;
			if (m_AccpetedSessionPacket.AddItems((char*)&tRecvPacket, 1) != E_Circle_Buf_No_Error)
			{
				char szError[512];
				sprintf(szError, "LAcceptThread::ThreadDoing, AddItems Failed, can not notice client disconnect, pos1, SessionID:%lld\n", pSession->GetSessionID());
				g_ErrorWriter.WriteError(szError, strlen(szError));
			}
			if (!m_pNetWorkServices->CommitPackets(&m_AccpetedSessionPacket))
			{
				char szError[512];
				sprintf(szError, "LAcceptThread::ThreadDoing, CommitPackets Failed, can not notice client disconnect, pos2, SessionID:%lld\n", pSession->GetSessionID());
				g_ErrorWriter.WriteError(szError, strlen(szError));
			}

			char szError[512];
			sprintf(szError, "LAcceptThread::ThreadDoing, BindEpollThread Failed, SessionID:%lld\n", pSession->GetSessionID());
			g_ErrorWriter.WriteError(szError, strlen(szError));

//			pSendThreadManager->UnBindSendThread(pSession);
//			pRecvThreadManager->UnBindRecvThread(pSession);
//			pMasterSessManager->FreeSession(pSession);
//			pMasterSessManager->FreeSessionToPool(pSession);

			t_Client_Need_To_Close cntc;
			cntc.u64SessionID = pSession->GetSessionID();
			LCloseSocketThread* pCloseThread = &(m_pNetWorkServices->GetCloseSocketThread());
			pCloseThread->AppendToClose(cntc);
			pSession->SetCloseWorkSendedToCloseThread(1);
			continue;
		}

		//	投递接收数据事件
		if (!pEpollThreadManager->PostEpollReadEvent(pSession))
		{
			//	session closed
			tRecvPacket.pPacket = (LPacketSingle*)0;
			if (m_AccpetedSessionPacket.AddItems((char*)&tRecvPacket, 1) != E_Circle_Buf_No_Error)
			{ 
				char szError[512];
				sprintf(szError, "LAcceptThread::ThreadDoing, AddItems Failed, can not notice client disconnect, pos1, SessionID:%lld\n", pSession->GetSessionID());
				g_ErrorWriter.WriteError(szError, strlen(szError));
			}
			if (!m_pNetWorkServices->CommitPackets(&m_AccpetedSessionPacket))
			{
				char szError[512];
				sprintf(szError, "LAcceptThread::ThreadDoing, CommitPackets Failed, can not notice client disconnect, pos2, SessionID:%lld\n", pSession->GetSessionID());
				g_ErrorWriter.WriteError(szError, strlen(szError)); 
			}

			char szError[512];
			sprintf(szError, "LAcceptThread::ThreadDoing, PostEpollReadEvent Failed, SessionID:%lld\n", pSession->GetSessionID());
			g_ErrorWriter.WriteError(szError, strlen(szError));

//			pEpollThreadManager->UnBindEpollThread(pSession);
//			pSendThreadManager->UnBindSendThread(pSession);
//			pRecvThreadManager->UnBindRecvThread(pSession);
//			pMasterSessManager->FreeSession(pSession);
//			pMasterSessManager->FreeSessionToPool(pSession);

			t_Client_Need_To_Close cntc;
			cntc.u64SessionID = pSession->GetSessionID();
			LCloseSocketThread* pCloseThread = &(m_pNetWorkServices->GetCloseSocketThread());
			pCloseThread->AppendToClose(cntc);
			pSession->SetCloseWorkSendedToCloseThread(1);
			continue;
		}
		
	}
	return 0;
}
bool LAcceptThread::OnStart()
{
	return true;
}
void LAcceptThread::OnStop()
{
}

void LAcceptThread::SetNetWorkServices(LNetWorkServices* pNws)
{
	m_pNetWorkServices = pNws;
}


void LAcceptThread::FreeAcceptedPacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return ;
	}
	E_Circle_Error error = m_AcceptedPacketPool.AddItems((char*)(&pPacket), 1);

	if (error != E_Circle_Buf_No_Error)
	{
		delete pPacket;
		pPacket = NULL;
		char szError[512];
		sprintf(szError, "LAcceptThread::FreeAcceptedPacket, Fatal Error, Not Enough buf, impossible!!");
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
}

LPacketSingle* LAcceptThread::GetOneAcceptedPacket()
{
	LPacketSingle* pPacket = NULL;
	E_Circle_Error error = m_AcceptedPacketPool.GetOneItem((char*)&pPacket, sizeof(LPacketSingle*));
	if (error != E_Circle_Buf_No_Error)
	{
		//	如果没有数据包了，那么新分配一个
		pPacket = new LPacketSingle(ACCEPTED_PACKET_BUF_LEN);
		pPacket->SetPacketType(1);
	}

	return pPacket;
}


void LAcceptThread::BuildAcceptedPacket(LSession* pSession, LPacketSingle* pPacket)
{
	if (pSession == NULL) 
	{
		return ;
	}
	char szIPName[20]; memset(szIPName, 0, sizeof(szIPName));
	unsigned short usRemotePort = 0; 
	int nRes = pSession->GetPeerName(szIPName, sizeof(szIPName) - 1, usRemotePort);
	if (nRes != 0)
	{
		return ;
	}
	int nRecvThreadID = pSession->GetRecvThreadID();
	int nSendThreadID = pSession->GetSendThreadID();

	pPacket->AddData(szIPName, 20);
	pPacket->AddUShort(usRemotePort);
	pPacket->AddInt(nRecvThreadID);
	pPacket->AddInt(nSendThreadID); 
}


//	 释放线程使用的资源
void LAcceptThread::ReleaseAcceptThreadResource()
{
	if (m_AcceptedPacketPool.GetCurrentExistCount() != 0)
	{
		LPacketSingle* pPacket = NULL;
		while (1)
		{
			E_Circle_Error error = m_AcceptedPacketPool.GetOneItem((char*)&pPacket, sizeof(LPacketSingle*));
			
			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				delete pPacket;
				pPacket = NULL;
			}
		}
	}

	//	
	if (m_AccpetedSessionPacket.GetCurrentExistCount() != 0)
	{
		t_Recv_Packet tRecvPacket;
		while (1)
		{
			memset(&tRecvPacket, 0, sizeof(tRecvPacket));
			E_Circle_Error error = m_AccpetedSessionPacket.GetOneItem((char*)&tRecvPacket, sizeof(tRecvPacket));

			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				delete tRecvPacket.pPacket;
				tRecvPacket.pPacket = NULL;
			}
		}
	}
}

void LAcceptThread::StopAcceptThread()
{
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		//	停止线程
		Stop();
		//	等待返回
		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes != 0)
		{ 
			char szError[512];
			sprintf(szError, "LAcceptThread::StopAcceptThread, ErrorID:%d\n", errno);
			g_ErrorWriter.WriteError(szError, strlen(szError));
		}
	}
} 


void LAcceptThread::GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort)
{
	if (pBuf == NULL)
	{
		return ;
	}
	strncpy(pBuf, m_szListenIp, unBufSize);
	usPort = m_usListenPort;
}
