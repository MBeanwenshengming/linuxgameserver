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

#include "LIniFileReadAndWrite.h"

//typedef struct _Packet_Pool_Desc
//{
//	unsigned short usPacketLen;
//	unsigned int unInitSize;
//	unsigned int unMaxAllocSize; 
	//_Packet_Pool_Desc()
	//{ 
	//	usPacketLen		= 0;
	//	unInitSize 		= 0;
	//	unMaxAllocSize 	= 0; 
	//}
//}t_Packet_Pool_Desc;

//typedef struct _NetWorkServices_Params_Base
//{
//	char* pListenIP;
//	unsigned short	usListenPort;
//	unsigned int	unListenListSize;
//}t_NetWorkServices_Params_Base;

//typedef struct _NetWorkServices_Params_Session
//{
//	unsigned short usSessionManagerCount;
//	unsigned int unMaxSessionCountPerSessionManager;
//	unsigned int unSendBufChainSize;
//}t_NetWorkServices_Params_Session;

//typedef struct _NetWorkServices_Params_Recv_Thread
//{
//	unsigned short usThreadCount;
//	unsigned int unRecvWorkItemCount;
//	t_Packet_Pool_Desc*	pArrppdForRecvLocalPool;
//	unsigned int unRecvppdForRecvLocalPoolCount;
//	unsigned int unRecvLocalPacketPoolSize;
//}t_NetWorkServices_Params_Recv_Thread;
//
//typedef struct _NetWorkServices_Params_Send_Thread
//{
//	unsigned int unSendThreadCount;
//	unsigned int unSendWorkItemCount;
//	t_Packet_Pool_Desc*	pArrspdForSend;
//	unsigned int usspdSendCount;
//	unsigned int unEpollOutEventMaxCount;
//}t_NetWorkServices_Params_Send_Thread;
//
//typedef struct _NetWorkServices_Params_Epoll_Thread
//{
//	unsigned int unEpollThreadCount;
//	unsigned int unRecvThreadCount;
//	unsigned int unRecvThreadWorkItemCount;
//	unsigned int unSendThreadCount;
//	unsigned int unSendThreadWorkItemCount;
//	unsigned int uepoll_ctl FailednWaitClientSizePerEpoll;
//}t_NetWorkServices_Params_Epoll_Thread;
//
//typedef struct _NetWorkServices_Params_Close_Thread
//{
//	unsigned int unCloseWorkItemCount;
//	t_Packet_Pool_Desc*	pArrppdForCloseLocalPool;
//	unsigned int unppdForCloseLocalPoolCount;
//}t_NetWorkServices_Params_Close_Thread;
//
//typedef struct _NetWorkServices_Params_Global_Pool
//{
//	unsigned int unRecvedPacketPoolSize;
//	t_Packet_Pool_Desc* pArrppdForGlobalRecvPacketPool;
//	unsigned int unppdForGlobalRecvPacketPoolCount;
//	t_Packet_Pool_Desc* pArrppdForGlobalSendPacketPool;
//	unsigned int unppdForGlobalSendPacketPoolCount;
//	unsigned int unSendLocalPoolSizeForNetWorkServices;
//}t_NetWorkServices_Params_Global_Pool;
//
//typedef struct _NetWorkServices_Params
//{ 
//	t_NetWorkServices_Params_Base nwspBase;
//	t_NetWorkServices_Params_Session nwspSession;
//	t_NetWorkServices_Params_Recv_Thread nwspRecvThread;
//	t_NetWorkServices_Params_Send_Thread nwspSendThread;
//	t_NetWorkServices_Params_Epoll_Thread nwspEpollThread;
//	t_NetWorkServices_Params_Close_Thread nwspCloseThread;
//	t_NetWorkServices_Params_Global_Pool nwspGlobalPool;
//}t_NetWorkServices_Params;
//
//
//
//typedef struct _NetWorkServices_Params_Idle_SendPacket
//{
//	unsigned int unIdleThreadWorkQueueSize;
//	unsigned int unLocalPoolMaxSize;
//}t_NetWorkServices_Params_Idle_SendPacket;
#include "LPacketPoolManager.h"
#include "LNetWorkServices.h"

#define MAX_RECV_POOL_TYPE_SIZE 30
#define MAX_SEND_POOL_TYPE_SIZE 30

class LNetWorkConfigFileProcessor
{
public:
	LNetWorkConfigFileProcessor(void);
	~LNetWorkConfigFileProcessor(void);
public:
	bool ReadConfig(char* pFileName, char* pSectionHeader);
	t_NetWorkServices_Params& GetNetWorkServicesParams()
	{
		return m_NetWorkServicesParams;
	}
private:
	bool ReadServicesBase();
	bool ReadServicesSession();
	bool ReadServicesRecvThread();
	bool ReadServicesSendThread();
	bool ReadServicesEpollThread();
	bool ReadServicesCloseThread();
	bool ReadServicesGlobalPool();
	bool ReadServicesGlobalParam();
	bool ReadServicesGlobalIdlePacketParams();
private:
	t_NetWorkServices_Params m_NetWorkServicesParams;
	LIniFileReadAndWrite m_Inifile;

private:
	char m_szSectionHeader[64 + 1];
	int m_nRecvPoolTypeCount;
	unsigned int m_unRecvPacketLenType[MAX_RECV_POOL_TYPE_SIZE];
	int m_nSendPoolTypeCount;
	unsigned int m_unSendPacketLenType[MAX_SEND_POOL_TYPE_SIZE];

	char m_szIp[64 + 1];
	

	t_Packet_Pool_Desc m_GlobalRecvPacketPool[MAX_RECV_POOL_TYPE_SIZE];

	t_Packet_Pool_Desc m_GlobalSendPacketPool[MAX_SEND_POOL_TYPE_SIZE];


	t_Packet_Pool_Desc m_RecvThreadLocalRecvPacketPool[MAX_SEND_POOL_TYPE_SIZE];


	t_Packet_Pool_Desc m_SendThreadLocalSendPacketPool[MAX_SEND_POOL_TYPE_SIZE];


	t_Packet_Pool_Desc m_CloseThreadLocalSendPacketPool[MAX_SEND_POOL_TYPE_SIZE];
};
