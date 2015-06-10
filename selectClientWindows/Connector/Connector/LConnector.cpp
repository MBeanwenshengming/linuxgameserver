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

#include "LConnector.h"
#include "LIniFileReadAndWrite.h"

extern int g_narrLen[]; 
extern int g_globalCount;

LConnector::LConnector()
{
	//	data for connect
	m_Socket.SetSocket(-1);
	m_nReconnectPeriodTime 	= 5;

	m_bIsConnected 			= false;
	SetIsConnnectted(0);

	memset(m_szIP, 0, sizeof(m_szIP));
	m_usPort 				= 0;
	m_nLastestConnected		= 0;
	m_bSendable				= true;
	m_nRemainedDataLen		= 0;
}

LConnector::~LConnector()
{ 
}

int LConnector::ThreadDoing(void* pParam)
{
	while (1)
	{
		if (!m_bIsConnected)
		{
			if (!ReConnect())
			{
				 //sched_yield();
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
				return 0;
			}
			continue;
		}
		int nWorkCount = CheckForSocketEvent();
		if (nWorkCount < 0)
		{
			break;
		}
		int nSendWorkCount = SendData();
		if (nWorkCount == 0 && nSendWorkCount == 0)
		{
#ifndef WIN32
			//	sched_yield();
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
			return 0;
		}
	}

	return 0; 
}
bool LConnector::OnStart()
{
	return true;
}
void LConnector::OnStop()
{
}

int LConnector::CheckForSocketEvent()
{
	if (!m_bIsConnected)
	{
		return 1;
	}
	
	FD_ZERO(&m_rdset);
	FD_ZERO(&m_wrset);
	FD_SET(m_Socket.GetSocket(), &m_rdset);
	FD_SET(m_Socket.GetSocket(), &m_wrset);

	struct timeval val;
	val.tv_sec 		= 0;
	val.tv_usec 	= 0;

	int iReturn 	= 0;
	if (!m_bSendable)
	{ 
		iReturn = select(m_Socket.GetSocket() + 1, &m_rdset, &m_wrset, NULL, &val);	
	}
	else
	{
		iReturn = select(m_Socket.GetSocket() + 1, &m_rdset, NULL, NULL, &val);	
	}
	if (iReturn == -1)
	{
#ifndef WIN32
		if (errno == EINTR)
#else
		if (errno == WSAEWOULDBLOCK)
#endif
		{
			return 1;
		}
		return -1;
	}
	else if (iReturn == 0)
	{
		return 0;
	}
	if (FD_ISSET(m_Socket.GetSocket(), &m_rdset))
	{
		OnRecv();
	}
	if (FD_ISSET(m_Socket.GetSocket(), &m_wrset))
	{
		OnSend();
	}
	return 1; 
}
int LConnector::OnRecv()
{
#ifndef WIN32
	ssize_t sRecved = recv(m_Socket.GetSocket(), m_szBufForRecv + m_nRemainedDataLen, 128 * 1024 - m_nRemainedDataLen, 0);
#else
	int sRecved = recv(m_Socket.GetSocket(), m_szBufForRecv + m_nRemainedDataLen, 128 * 1024 - m_nRemainedDataLen, 0);
#endif
	if (sRecved == 0)
	{
		m_bIsConnected = false;
		SetIsConnnectted(0);
#ifndef WIN32
		close(m_Socket.GetSocket());
#else
		closesocket(m_Socket.GetSocket());
#endif
		m_Socket.SetSocket(-1);
		return -1;
	}
	else if (sRecved > 0)
	{
		m_nRemainedDataLen += sRecved;
		int  nParsedDataLen = ParseRecvedData(m_szBufForRecv, m_nRemainedDataLen);
		if (nParsedDataLen < 0)
		{
			m_bIsConnected = false;
			SetIsConnnectted(0);
#ifndef WIN32
			close(m_Socket.GetSocket());
#else
			closesocket(m_Socket.GetSocket());
#endif
			m_Socket.SetSocket(-1);
			return -1;
		}
		m_nRemainedDataLen -= nParsedDataLen; 
		memcpy(m_szBufForRecv, ((char*)m_szBufForRecv) + nParsedDataLen, m_nRemainedDataLen); 
	}
	else
	{
#ifndef WIN32
		if (errno == EAGAIN)
#else
		if (errno == WSAEWOULDBLOCK)
#endif
		{
			return 0;
		}
		else
		{
			m_bIsConnected = false;
			SetIsConnnectted(0);
#ifndef WIN32
			close(m_Socket.GetSocket());
#else
			closesocket(m_Socket.GetSocket());
#endif
			m_Socket.SetSocket(-1);
			return -1;
		}
	}
	return 0;
}
int LConnector::OnSend()
{
	if (!m_bSendable)
	{
		m_bSendable = true;
	}
	return 0;
}



LSocket* LConnector::GetSocket()
{
	return &m_Socket;
}

//	nReconnectPeriodTime  seconds
bool LConnector::Initialize(t_Connector_Initialize_Params& cip, bool bConnectImmediate)
{
//	if (cip.pszIP == NULL || cip.usPort == 0)
//	{
//		return false;
//	}
	if (strlen(cip.szIP) == 0 || cip.usPort == 0)
	{
		return false;
	}

	if (cip.nReConnectPeriodTime == 0)
	{
		cip.nReConnectPeriodTime = 5;
	}
	m_nReconnectPeriodTime = cip.nReConnectPeriodTime;

	strncpy(m_szIP, cip.szIP, 128);
	m_usPort = cip.usPort;
	
	if (!m_RecvPacketPoolManager.Initialize(cip.ppdForRecvPacketPool, cip.usppdCountForRecvPacketPool))
	{
		return false;
	}
	if (!m_RecvedPacketQueue.Initialize(sizeof(LPacketSingle*), cip.unRecvedPacketQueueSize))
	{
		return false;
	}
	
	if (!m_SendPacketPoolManager.Initialize(cip.ppdForSendPacketPool, cip.usppdCountForSendPacketPool))
	{
		return false;
	}
	if (!m_SendPacketQueue.Initialize(sizeof(LPacketSingle*), cip.unSendPacketQueueSize))
	{
		return false;
	}

	int nSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (nSocket == -1)
	{
		return false;
	}
	sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port	= htons(m_usPort);
	//	sockaddr.sin_addr.s_addr
	int nsucc = 0;
#ifndef WIN32
	int nsucc = inet_aton(m_szIP, &sockaddr.sin_addr);
	if (nsucc == 0)
	{
		return false;
	}
#else
	sockaddr.sin_addr.S_un.S_addr = inet_addr(m_szIP);
#endif

	if (bConnectImmediate)
	{
		nsucc = connect(nSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)); 
		if (nsucc != 0)
		{
			return false;
		}
		m_bIsConnected = true;
		SetIsConnnectted(1); 
#ifndef WIN32
		nsucc = fcntl(nSocket, F_SETFL, O_NONBLOCK);
		if (nsucc == -1)
		{
			return false;
		}
#else
		unsigned long ul=1;  
		int nSetNonBlockSuccess = ioctlsocket(GetSocket()->GetSocket(), FIONBIO, (unsigned long *)&ul);
		if (nSetNonBlockSuccess == SOCKET_ERROR)
		{
			return false;
		}
#endif
		m_Socket.SetSocket(nSocket);

		unsigned short usDataLen = 128;
		LPacketSingle* pPacket = AllocOnePacket(usDataLen);
		if (pPacket == NULL)
		{
			return true;
		}
		pPacket->SetPacketType(1);
		if (!AddOneRecvedPacket(pPacket))
		{
			delete pPacket;
		}
	}
	//	m_Socket.SetSockOpt();
	if (!Start())
	{
		return false;
	}
	return true;	

}

bool LConnector::ReConnect()
{
	time_t timeNow = time(NULL);

	if ((m_nLastestConnected != 0 && timeNow - m_nLastestConnected> m_nReconnectPeriodTime) || m_nLastestConnected == 0)
	{
		m_nLastestConnected = timeNow;
		if (m_Socket.GetSocket() != -1)
		{
#ifndef WIN32
			close(m_Socket.GetSocket());
#else
			closesocket(m_Socket.GetSocket());
#endif
		}

		int nSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (nSocket == -1)
		{
			return false;
		}
		sockaddr_in sockaddr;
		memset(&sockaddr, 0, sizeof(sockaddr));
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_port	= htons(m_usPort);
		//	sockaddr.sin_addr.s_addr
		int nsucc = 0;
#ifndef WIN32
		int nsucc = inet_aton(m_szIP, &sockaddr.sin_addr);
		if (nsucc == 0)
		{
			close(nSocket);
			return false;
		}
#else
		sockaddr.sin_addr.S_un.S_addr = inet_addr(m_szIP);
#endif
		nsucc = connect(nSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)); 
		if (nsucc != 0)
		{
			return false;
		}
		m_Socket.SetSocket(nSocket);

#ifndef WIN32
		nsucc = fcntl(nSocket, F_SETFL, O_NONBLOCK);
		if (nsucc == -1)
		{
			return false;
		}
#else
		unsigned long ul=1;  
		int nSetNonBlockSuccess = ioctlsocket(GetSocket()->GetSocket(), FIONBIO, (unsigned long *)&ul);
		if (nSetNonBlockSuccess == SOCKET_ERROR)
		{
			return false;
		}
#endif		
		m_bIsConnected = true;
		SetIsConnnectted(1);

		unsigned short usDataLen = 128;
		LPacketSingle* pPacket = AllocOnePacket(usDataLen);
		if (pPacket == NULL)
		{
			return true;
		}
		pPacket->SetPacketType(1);
		if (!AddOneRecvedPacket(pPacket))
		{
			delete pPacket;
		}
	}
	else
	{
		return false;
	}
	return true; 
}


LPacketSingle* LConnector::AllocOnePacket(unsigned short usPacketLen)
{
	LPacketSingle* pPacket = NULL;
	pPacket = m_RecvPacketPoolManager.RequestOnePacket(usPacketLen);
	return pPacket;
}

void LConnector::FreePacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return ;
	}
	pPacket->Reset();
	m_RecvPacketPoolManager.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen());
}



LPacketSingle* LConnector::GetOneRecvedPacket()
{
	LPacketSingle* pPacket = NULL;
	m_RecvedPacketQueue.GetOneItem((char*)&pPacket, sizeof(LPacketSingle*));  
	return pPacket;
} 

bool LConnector::AddOneRecvedPacket(LPacketSingle* pPacket)
{
	if (m_RecvedPacketQueue.AddItems((char*)&pPacket, 1) == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}

bool LConnector::StopThreadAndStopConnector()
{
	Stop();

#ifndef WIN32
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes != 0)
		{
			return false;
		}
	}
#else
	int pThreadId = GetThreadHandle();
	WaitForSingleObject((HANDLE)pThreadId, INFINITE);
#endif
	return true;
}

void LConnector::ReleaseConnectorResource()
{
	m_RecvPacketPoolManager.ReleasePacketPoolManagerResource();

	if (m_RecvedPacketQueue.GetCurrentExistCount() != 0)
	{
		LPacketSingle* pPacketSingle = NULL;	
		while (1)
		{ 
			pPacketSingle = NULL;
			E_Circle_Error error = m_RecvedPacketQueue.GetOneItem((char*)&pPacketSingle, sizeof(LPacketSingle*));
				
			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				delete pPacketSingle;
			}
		}
	}
	m_SendPacketPoolManager.ReleasePacketPoolManagerResource();

	if (m_SendPacketQueue.GetCurrentExistCount() != 0)
	{ 
		LPacketSingle* pPacketSingle = NULL;	
		while (1)
		{ 
			pPacketSingle = NULL;
			E_Circle_Error error = m_SendPacketQueue.GetOneItem((char*)&pPacketSingle, sizeof(LPacketSingle*));
				
			if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
			{
				break;
			}
			else
			{
				delete pPacketSingle;
			}
		}
	}
	if (m_bIsConnected)
	{
#ifndef WIN32
		close(m_Socket.GetSocket());
#else
		closesocket(m_Socket.GetSocket());
#endif
	}
}


int LConnector::ParseRecvedData(char* pData, int nDataLen)
{
	if (nDataLen <= sizeof(unsigned short))
	{
		return 0;
	}

	int nParsedDataLen = 0;

	unsigned short usDataLen = 0;
	usDataLen = *((unsigned short*)pData);
	if (usDataLen > MAX_PACKET_LEN_FOR_CONNECTOR)
	{
		return -1;
	}
	while (nDataLen >= usDataLen)
	{
		LPacketSingle* pPacket = AllocOnePacket(usDataLen);
		if (pPacket == NULL)
		{
			return -1;
		}

		//pPacket->AddData(pData + sizeof(unsigned short), usDataLen - sizeof(unsigned short));
		pPacket->DirectSetData(pData + sizeof(unsigned short), usDataLen - sizeof(unsigned short));
		pData 		+= usDataLen;
		nDataLen 	-= usDataLen;
		nParsedDataLen += usDataLen;

//		bool bFinded = false;
//		for (int i = 0; i < g_globalCount; ++i)
//		{
//			if (usDataLen - sizeof(unsigned short) == g_narrLen[i])
//			{
//				bFinded = true;
//				break;
//			}
//		}
//		if (!bFinded)
//		{
//			printf("Len:%hd, %s\n", usDataLen, ((char*)pData) + sizeof(unsigned short));
//		}

		AddOneRecvedPacket(pPacket); 
		if (nDataLen >= sizeof(unsigned short))
		{
			usDataLen = *((unsigned short*)pData); 
		}
		else
		{
			break;
		}
	}
	return nParsedDataLen;
}


LPacketSingle* LConnector::GetOneSendPacketPool(unsigned short usPacketSize)
{
	LPacketSingle* pPacket = NULL;
	pPacket = m_SendPacketPoolManager.RequestOnePacket(usPacketSize);
	return pPacket;
}
bool LConnector::AddOneSendPacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}
	if (m_SendPacketQueue.AddItems((char*)&pPacket, 1) == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}

void LConnector::FreeSendPacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return;
	}
	m_SendPacketPoolManager.FreeOneItemToPool(pPacket, pPacket->GetPacketBufLen());
}

int LConnector::SendData()
{ 
	if (!m_bSendable)
	{
		return 1;
	}
	int nSended = 0;
	for (;;)
	{
		if (nSended >= 50)
		{
			break;
		}
		LPacketSingle* pPacket = NULL;
		E_Circle_Error errorID = m_SendPacketQueue.LookUpOneItem((char*)&pPacket, sizeof(LPacketSingle*));
		if (errorID != E_Circle_Buf_No_Error)
		{
			break;
		}
		bool bSendSuccess = false;

		if (pPacket != NULL)
		{
#ifndef WIN32
			ssize_t sSended = send(m_Socket.GetSocket(), pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen(), 0);
#else
			int sSended = send(m_Socket.GetSocket(), pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen(), 0);
#endif
			if (sSended == -1)
			{
#ifndef WIN32
				if (errno == EAGAIN)
#else
				if (errno == WSAEWOULDBLOCK)
#endif
				{
					m_bSendable = false;
					return 1;
				}
				else
				{
					m_bIsConnected = false;
					SetIsConnnectted(0);
#ifndef WIN32
					close(m_Socket.GetSocket());
#else
					closesocket(m_Socket.GetSocket());
#endif
					m_Socket.SetSocket(-1);
					return 1;
				}
			}
			else
			{
				bSendSuccess = true;
			}
		}
		if (bSendSuccess)
		{
			m_SendPacketQueue.DeleteOneItemAtHead();
			FreeSendPacket(pPacket); 
		}
		nSended++;
	}
	return nSended;
}
//	=============================================
//

LConnectorConfigProcessor::LConnectorConfigProcessor()
{
	memset(&m_Cip, 0, sizeof(m_Cip));
	m_Cip.nReConnectPeriodTime = 10;
}

LConnectorConfigProcessor::~LConnectorConfigProcessor()
{
}

bool LConnectorConfigProcessor::Initialize(char* pConfigFileName, char* pSectionHeader, bool bReadIpAndPort)
{
	if (pConfigFileName == NULL || pSectionHeader == NULL || strlen(pSectionHeader) >= 64)
	{
		return false;
	}
	LIniFileReadAndWrite ifraw;
	if (!ifraw.OpenIniFile(pConfigFileName))
	{
		return false;
	}

	char szSection[128 + 1];	
	memset(szSection, 0, sizeof(szSection));
	sprintf(szSection, "%s_Connector_Global", pSectionHeader);
	char* pSection = szSection;
	//	char* pSection = "Connector_Global";
	if (bReadIpAndPort)
	{
		char* pKey = "IP";
		if (!ifraw.read_profile_string(pSection, pKey, m_Cip.szIP, sizeof(m_Cip.szIP) - 1, ""))
		{
			return false;
		}

		pKey = "PORT";
		int nPort = ifraw.read_profile_int(pSection, pKey, 0);
		if (nPort <= 0)
		{
			return false;
		}
		m_Cip.usPort = nPort;
	}
	char* pKey = "RecvedPacketQueueSize";
	int nRecvedPacketQueueSize = ifraw.read_profile_int(pSection, pKey, 0);
	if (nRecvedPacketQueueSize <= 0)
	{
		return false;
	}
	m_Cip.unRecvedPacketQueueSize = nRecvedPacketQueueSize;

	pKey = "SendPacketQueueSize";
	int nSendPacketQueueSize = ifraw.read_profile_int(pSection, pKey, 0);
	if (nSendPacketQueueSize <= 0)
	{
		return false;
	}
	m_Cip.unSendPacketQueueSize = nSendPacketQueueSize;

	pKey = "RecvPacketLenTypeCount";
	int nRecvPacketLenTypeCount = ifraw.read_profile_int(pSection, pKey, 0);
	if (nRecvPacketLenTypeCount <= 0)
	{
		return false;
	}
	m_Cip.usppdCountForRecvPacketPool = nRecvPacketLenTypeCount;

	pKey = "SendPacketLenTypeCount";
	int nSendPacketLenTypeCount = ifraw.read_profile_int(pSection, pKey, 0);
	if (nSendPacketLenTypeCount <= 0)
	{
		return false;
	}
	m_Cip.usppdCountForSendPacketPool = nSendPacketLenTypeCount;

	pKey = "ReConnectPeriodTime";
	int nReConnectPeriodTime = ifraw.read_profile_int(pSection, pKey, 0);
	if (nReConnectPeriodTime <= 0)
	{
		return false;
	}
	m_Cip.nReConnectPeriodTime = nReConnectPeriodTime;
	
	char szKeyNameBuf[128]; 	
	char szKeyValueNameIni[128];
	char szKeyValueNameMax[128];
	for (int i = 1; i <= m_Cip.usppdCountForRecvPacketPool; ++i)
	{
		memset(szSection, 0, sizeof(szSection));
		sprintf(szSection, "%s_RecvPacketLenType", pSectionHeader);
		//	pSection = "RecvPacketLenType";
		sprintf(szKeyNameBuf, "rplt_%d", i);
		int nBufLen = ifraw.read_profile_int(pSection, szKeyNameBuf, 0);
		if (nBufLen <= 0)
		{
			return false;
		}
		//	m_ppdForRecvPool[i - 1].usPacketLen = nBufLen;
		m_Cip.ppdForRecvPacketPool[i - 1].usPacketLen = nBufLen;
		memset(szSection, 0, sizeof(szSection));
		sprintf(szSection, "%s_RecvPacketPool_Params", pSectionHeader);
		//pSection = "RecvPacketPool_Params";
		sprintf(szKeyValueNameIni, "rppp_%d_init", i);
		sprintf(szKeyValueNameMax, "rppp_%d_max", i);
		int nIni = ifraw.read_profile_int(pSection, szKeyValueNameIni, 0);
		if (nIni <= 0)
		{
			return false;
		}
		int nMax = ifraw.read_profile_int(pSection, szKeyValueNameMax, 0);
		if (nMax <= 0)
		{
			return false;
		}
		//	m_ppdForRecvPool[i - 1].unInitSize = nIni;
		m_Cip.ppdForRecvPacketPool[i - 1].unInitSize = nIni;
		//	m_ppdForRecvPool[i - 1].unMaxAllocSize = nMax;
		m_Cip.ppdForRecvPacketPool[i - 1].unMaxAllocSize = nMax;
	}


	for (int i = 1; i <= m_Cip.usppdCountForSendPacketPool; ++i)
	{
		memset(szSection, 0, sizeof(szSection));
		sprintf(szSection, "%s_SendPacketLenType", pSectionHeader);
		//pSection = "SendPacketLenType";
		sprintf(szKeyNameBuf, "splt_%d", i);
		int nBufLen = ifraw.read_profile_int(pSection, szKeyNameBuf, 0);
		if (nBufLen <= 0)
		{
			return false;
		}
		//	m_ppdForSendPool[i - 1].usPacketLen = nBufLen;
		m_Cip.ppdForSendPacketPool[i - 1].usPacketLen = nBufLen;

		memset(szSection, 0, sizeof(szSection));
		sprintf(szSection, "%s_SendPacketPool_Params", pSectionHeader);
		//pSection = "SendPacketPool_Params";
		sprintf(szKeyValueNameIni, "sppp_%d_init", i);
		sprintf(szKeyValueNameMax, "sppp_%d_max", i);
		int nIni = ifraw.read_profile_int(pSection, szKeyValueNameIni, 0);
		if (nIni <= 0)
		{
			return false;
		}
		int nMax = ifraw.read_profile_int(pSection, szKeyValueNameMax, 0);
		if (nMax <= 0)
		{
			return false;
		}
		//	m_ppdForSendPool[i - 1].unInitSize = nIni;
		m_Cip.ppdForSendPacketPool[i - 1].unInitSize = nIni;
		//	m_ppdForSendPool[i - 1].unMaxAllocSize = nMax;
		m_Cip.ppdForSendPacketPool[i - 1].unMaxAllocSize = nMax;
	}
	//	m_Cip.ppdForRecvPacketPool = m_ppdForRecvPool;
	//	m_Cip.ppdForSendPacketPool = m_ppdForSendPool;
	return true;
}
t_Connector_Initialize_Params& LConnectorConfigProcessor::GetConnectorInitializeParams()
{
	return m_Cip;
}


