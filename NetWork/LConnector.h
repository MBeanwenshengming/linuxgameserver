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

#ifndef __LINUX_SINGLE_CONNECTOR_HEADER_DEFINED__
#define __LINUX_SINGLE_CONNECTOR_HEADER_DEFINED__

#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"
#include "LSocket.h"
#include "LPacketPoolManager.h" 
#include "LPacketSingle.h"

#define MAX_PACKET_LEN_FOR_CONNECTOR 8 * 1024

#define MAX_POOLLEN_TYPE_COUNT 50

typedef struct _Connector_Initialize_Params
{
	char szIP[128];					//	连接IP
	unsigned short usPort;			//	连接端口号
	int nReConnectPeriodTime;		//	断开后的重连间隔
	t_Packet_Pool_Desc ppdForRecvPacketPool[MAX_POOLLEN_TYPE_COUNT];	//	接收缓存的分配参数描述
	unsigned short usppdCountForRecvPacketPool;	//	数组大小 
	unsigned int unRecvedPacketQueueSize;		//	接收的数据包队列大小

	t_Packet_Pool_Desc ppdForSendPacketPool[MAX_POOLLEN_TYPE_COUNT];	//	发送缓存的分配参数描述
	unsigned short usppdCountForSendPacketPool;	//	数组大小
	unsigned int unSendPacketQueueSize;			//	发送数据包的队列大小

	_Connector_Initialize_Params()
	{
		memset(szIP, 0, sizeof(szIP));	
		usPort 						= 0;
		nReConnectPeriodTime 		= 10;
		memset(ppdForRecvPacketPool, 0, sizeof(ppdForRecvPacketPool));
		usppdCountForRecvPacketPool	= 0;
		unRecvedPacketQueueSize 	= 0;
		memset(ppdForSendPacketPool, 0, sizeof(ppdForSendPacketPool));
		usppdCountForSendPacketPool = 0;
		unSendPacketQueueSize 		= 0;
	} 
}t_Connector_Initialize_Params;

class LConnectorConfigProcessor
{
public:
	LConnectorConfigProcessor();
	~LConnectorConfigProcessor();
public:
	bool Initialize(char* pConfigFileName, char* pSectionHeader, bool bReadIpAndPort = true);
	t_Connector_Initialize_Params& GetConnectorInitializeParams();
private:
	t_Connector_Initialize_Params m_Cip;
//	char m_szIp[128];
//	t_Packet_Pool_Desc m_ppdForRecvPool[MAX_POOLLEN_TYPE_COUNT];
//	unsigned short m_usppdCountForRecvPool;
//	t_Packet_Pool_Desc m_ppdForSendPool[MAX_POOLLEN_TYPE_COUNT];
//	unsigned short m_usppdCountForSendPool; 
};

class LConnector : public LThreadBase
{
public:
	LConnector();
	~LConnector();
private:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
public:		// 

private:
	int CheckForSocketEvent();
	int OnRecv();
	int OnSend();
public:
	LSocket* GetSocket(); 

	//	nReconnectPeriodTime  seconds
	bool Initialize(t_Connector_Initialize_Params& cip, bool bConnectImmediate = false);
protected:
	bool ReConnect();
private:
	LSocket m_Socket;
	int m_nReconnectPeriodTime;
	bool m_bIsConnected;
	char m_szIP[128];
	unsigned short m_usPort;
	time_t m_nLastestConnected;
public:
	LPacketSingle* GetOneRecvedPacket();
	void FreePacket(LPacketSingle* pPacket);
protected:
	LPacketSingle* AllocOnePacket(unsigned short usPacketLen);
	bool AddOneRecvedPacket(LPacketSingle* pPacket);
private:
	int ParseRecvedData(char* pData, int nDataLen);
private:
	char m_szBufForRecv[128 * 1024];
	int m_nRemainedDataLen;
	LFixLenCircleBuf m_RecvedPacketQueue;		//	接收到的数据包放置在这里, 逻辑线程从这里取接收到的数据，并且，数据包处理完之后，释放该数据包到RecvPacketPoolManager中
	LPacketPoolManager<LPacketSingle> m_RecvPacketPoolManager;	//	接收数据包的缓存,接收网络线程从这里取包，将接收到的数据放到包中，然后将数据包放入m_RecvedPacketQueue中

public:
	LPacketSingle* GetOneSendPacketPool(unsigned short usPacketSize); 
	bool AddOneSendPacket(LPacketSingle* pPacket);
	//	停止连接线程，并且等待线程结束
	bool StopThreadAndStopConnector();
	//	释放占用的资源
	void ReleaseConnectorResource();
protected:
	int SendData();
	void FreeSendPacket(LPacketSingle* pPacket);
private:
	bool 	m_bSendable;
	LPacketPoolManager<LPacketSingle> m_SendPacketPoolManager;	//	发送数据包的缓存
	LFixLenCircleBuf m_SendPacketQueue;			//	发送队列


private:
	fd_set m_rdset;
	fd_set m_wrset;

public: 
	bool IsConnectted()
	{
		int nIsConnectted = atomic_read(&m_IsConnect);
		if (nIsConnectted == 1)
		{
			return true;
		}
		return false;
	}
private:
	void SetIsConnnectted(int i)
	{
		atomic_set(&m_IsConnect, i);
	}
	atomic_t m_IsConnect;
};
#endif



