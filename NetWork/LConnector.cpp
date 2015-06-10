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
		//	如何没有连接或者连接断开，那么重连
		if (!m_bIsConnected)
		{
			if (!ReConnect())
			{
				 //sched_yield();
				struct timespec timeReq;
				timeReq.tv_sec 	= 0;
				timeReq.tv_nsec = 10;
				nanosleep(&timeReq, NULL);
			}
			//	防止连接不上，但是线程一直执行无法停止
			if (CheckForStop())
			{
				return 0;
			}
			continue;
		}
		// 检查网络事件
		int nWorkCount = CheckForSocketEvent();
		if (nWorkCount < 0)
		{
			break;
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
		if (errno == EINTR)
		{
			return 1;
		}
		return -1;
	}
	else if (iReturn == 0)
	{
		return 0;
	}
	if (FD_ISSET(m_Socket.GetSocket(), &m_rdset))		//	可以接收数据
	{
		OnRecv();
	}
	if (FD_ISSET(m_Socket.GetSocket(), &m_wrset))		//	可以继续发送数据了
	{
		OnSend();
	}
	return 1; 
}
int LConnector::OnRecv()
{
	ssize_t sRecved = recv(m_Socket.GetSocket(), m_szBufForRecv + m_nRemainedDataLen, 128 * 1024 - m_nRemainedDataLen, 0);
	if (sRecved == 0)	//	连接断开
	{
		m_bIsConnected = false;
		SetIsConnnectted(0);
		close(m_Socket.GetSocket());
		m_Socket.SetSocket(-1);
		return -1;
	}
	else if (sRecved > 0)	//	接收到数据
	{
		m_nRemainedDataLen += sRecved;
		int  nParsedDataLen = ParseRecvedData(m_szBufForRecv, m_nRemainedDataLen);
		if (nParsedDataLen < 0)	//	协议出错，需要断开连接
		{
			m_bIsConnected = false;
			SetIsConnnectted(0);
			close(m_Socket.GetSocket());
			m_Socket.SetSocket(-1);
			return -1;
		}
		m_nRemainedDataLen -= nParsedDataLen; 
		memcpy(m_szBufForRecv, ((char*)m_szBufForRecv) + nParsedDataLen, m_nRemainedDataLen); 
	}
	else		//	出现错误
	{
		if (errno == EAGAIN)
		{
			return 0;
		}
		else		//	未知错误，重新连接
		{
			m_bIsConnected = false;
			SetIsConnnectted(0);
			close(m_Socket.GetSocket());
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
	
	//	初始化接收缓冲池
	if (!m_RecvPacketPoolManager.Initialize(cip.ppdForRecvPacketPool, cip.usppdCountForRecvPacketPool))
	{
		return false;
	}
	//	初始化接收数据包链表
	if (!m_RecvedPacketQueue.Initialize(sizeof(LPacketSingle*), cip.unRecvedPacketQueueSize))
	{
		return false;
	}
	
	//	初始化发送缓冲池
	if (!m_SendPacketPoolManager.Initialize(cip.ppdForSendPacketPool, cip.usppdCountForSendPacketPool))
	{
		return false;
	}
	//	初始化发送队列
	if (!m_SendPacketQueue.Initialize(sizeof(LPacketSingle*), cip.unSendPacketQueueSize))
	{
		return false;
	}

	//	连接服务器
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
	int nsucc = inet_aton(m_szIP, &sockaddr.sin_addr);
	if (nsucc == 0)
	{
		return false;
	}
	if (bConnectImmediate)
	{
		nsucc = connect(nSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)); 
		if (nsucc != 0)
		{
			return false;
		}
		m_bIsConnected = true;
		SetIsConnnectted(1); 

		nsucc = fcntl(nSocket, F_SETFL, O_NONBLOCK);
		if (nsucc == -1)
		{
			return false;
		}
		m_Socket.SetSocket(nSocket);

		unsigned short usDataLen = 128;
		LPacketSingle* pPacket = AllocOnePacket(usDataLen);
		if (pPacket == NULL)
		{
			return true;
		}
		//	通知已经连接上了
		pPacket->SetPacketType(1);
		if (!AddOneRecvedPacket(pPacket))
		{
			delete pPacket;		//	这里应该一般清况朋不到
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
		//	如果原来的socket没有关闭，那么先关闭
		if (m_Socket.GetSocket() != -1)
		{
			close(m_Socket.GetSocket());
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
		int nsucc = inet_aton(m_szIP, &sockaddr.sin_addr);
		if (nsucc == 0)
		{
			close(nSocket);
			return false;
		}
		nsucc = connect(nSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)); 
		if (nsucc != 0)
		{
			return false;
		}
		nsucc = fcntl(nSocket, F_SETFL, O_NONBLOCK);
		if (nsucc == -1)
		{
			return false;
		}
		m_Socket.SetSocket(nSocket);
		m_bIsConnected = true;
		SetIsConnnectted(1);

		unsigned short usDataLen = 128;
		LPacketSingle* pPacket = AllocOnePacket(usDataLen);
		if (pPacket == NULL)
		{
			return true;
		}
		//	通知已经连接上了
		pPacket->SetPacketType(1);
		if (!AddOneRecvedPacket(pPacket))
		{
			delete pPacket;		//	这里应该一般清况朋不到
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

//	停止连接线程，并且等待线程结束
bool LConnector::StopThreadAndStopConnector()
{
	Stop();

	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes != 0)
		{
			return false;
		}
	}
	return true;
}

//	释放占用的资源
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
	//	释放发送缓存
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
		close(m_Socket.GetSocket());
	}
}


int LConnector::ParseRecvedData(char* pData, int nDataLen)
{
	//	如果现存的数据长度没有unsigned short长，那么说明数据包不完整,不需要处理
	if (nDataLen <= sizeof(unsigned short))
	{
		return 0;
	}

	int nParsedDataLen = 0;

	unsigned short usDataLen = 0;
	usDataLen = *((unsigned short*)pData);
	if (usDataLen > MAX_PACKET_LEN_FOR_CONNECTOR)	//	连接上的数据有错误
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

		if (pPacket != NULL)		//	因为只有这里会取数据包，所以可以现取出来，发送成功再删除
		{
			//	发送数据包
			ssize_t sSended = send(m_Socket.GetSocket(), pPacket->GetPacketBuf(), pPacket->GetPacketDataAndHeaderLen(), 0);
			if (sSended == -1)
			{
				if (errno == EAGAIN)
				{
					m_bSendable = false;
					return 1;
				}
				else		//	出现其它错误，显示为连接断开，重新连接
				{
					m_bIsConnected = false;
					SetIsConnnectted(0);

					close(m_Socket.GetSocket());
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

	//	读取连接IP和端口
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

	pKey = "SendPacketQueueSize";	//发送队列长度
	int nSendPacketQueueSize = ifraw.read_profile_int(pSection, pKey, 0);
	if (nSendPacketQueueSize <= 0)
	{
		return false;
	}
	m_Cip.unSendPacketQueueSize = nSendPacketQueueSize;

	pKey = "RecvPacketLenTypeCount";	//接收数据包长度类型数量
	int nRecvPacketLenTypeCount = ifraw.read_profile_int(pSection, pKey, 0);
	if (nRecvPacketLenTypeCount <= 0)
	{
		return false;
	}
	m_Cip.usppdCountForRecvPacketPool = nRecvPacketLenTypeCount;

	pKey = "SendPacketLenTypeCount";	//发送数据包长度类型数量
	int nSendPacketLenTypeCount = ifraw.read_profile_int(pSection, pKey, 0);
	if (nSendPacketLenTypeCount <= 0)
	{
		return false;
	}
	m_Cip.usppdCountForSendPacketPool = nSendPacketLenTypeCount;

	pKey = "ReConnectPeriodTime";		//断开后的重连时间
	int nReConnectPeriodTime = ifraw.read_profile_int(pSection, pKey, 0);
	if (nReConnectPeriodTime <= 0)
	{
		return false;
	}
	m_Cip.nReConnectPeriodTime = nReConnectPeriodTime;
	
	char szKeyNameBuf[128]; 	
	char szKeyValueNameIni[128];
	char szKeyValueNameMax[128];
	//	根据接收类型，读取数据类型
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
		//	读取分配信息
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


