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

#ifndef __LINUX_SELECT_SERVER_HEADER_DEFINDED__
#define __LINUX_SELECT_SERVER_HEADER_DEFINDED__

#include "LThreadBase.h"
#include "LSocket.h"
#include <map>
#include <queue>
#include "LPacketSingle.h"
#include "LPacketBroadCast.h"
#include "LListenSocket.h"
#include "LPacketPoolManager.h"
#include "LConnectorWorkManager.h"

using namespace std;
class LSelectServer;
typedef struct _Connect_Destination
{
	char 			szIP[32];
	unsigned short  usPort;
	_Connect_Destination()
	{
		memset(szIP, 0, sizeof(szIP));
		usPort = 0;
	}	
}t_Connect_Destination;

typedef struct _Select_Server_Params
{
	char* pszIp;
	unsigned short usPort;
	unsigned int unMaxSessionCount;
	unsigned int unMaxSendQueueSize;
	unsigned int unRecvBufSize;
	bool bIsConnector;
	unsigned int unRecvedPacketQueueSize;
	unsigned int unCloseWorkItemQueueSize;
	t_Packet_Pool_Desc* pppdForRecv;
	unsigned short  usppdForRecvSize;
	t_Packet_Pool_Desc* pppdForSend;
	unsigned short usppdForSendSize;
	unsigned int unSendPacketSizeForMainLogic;
	unsigned int unSendPacketSizeForSendThread;
	unsigned short usListenListSize;
	unsigned int unConnectWorkItemCountForCircleBufLen;
	_Select_Server_Params()
	{
		pszIp 				= NULL;
		usPort 				= 0;
		unMaxSessionCount 	= 0;
		unMaxSendQueueSize 	= 0;
		unRecvBufSize 		= 0;
		bIsConnector 		= false;
		unRecvedPacketQueueSize 	= 0;
		unCloseWorkItemQueueSize 	= 0;
		pppdForRecv 		= NULL;
		usppdForRecvSize 	= 0;
		pppdForSend 		= NULL;
		usppdForSendSize 	= 0; 
		unSendPacketSizeForMainLogic 	= 0;
		unSendPacketSizeForSendThread 	= 0;
		usListenListSize = 1000;
		unConnectWorkItemCountForCircleBufLen = 0;
	}
}t_Select_Server_Params;

#define MAX_SELECT_SERVER_BUF_TYPE_COUNT 20

class LSelectServerParamsReader
{
public:
	LSelectServerParamsReader();
	~LSelectServerParamsReader();
public:
	bool ReadIniFile(char* pszFileName);
	t_Select_Server_Params& GetParams();
private:
	t_Select_Server_Params m_Params; 
	char m_szIP[64];
	t_Packet_Pool_Desc m_RecvBufDesc[MAX_SELECT_SERVER_BUF_TYPE_COUNT];
	unsigned int m_unRecvBufDescCount;
	t_Packet_Pool_Desc m_SendBufDesc[MAX_SELECT_SERVER_BUF_TYPE_COUNT];
	unsigned int m_unSendBufDescCount;
};
class LSelectServerSession : public LSocket
{
public:
	LSelectServerSession();
	~LSelectServerSession();
public:
	bool Initialize(t_Select_Server_Params& tssp);
	int SessionRecv();

#ifdef __ADD_SEND_BUF_CHAIN__
	int SessionSend(LPacketBroadCast* pPacket);
	int SessionSendInSendQueue();
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	bool SessionSend(LPacketSingle* pPacket);
	int SendDataInBuf(bool& bSystemBufFull);
#endif

	void Reset();
	void ReleaseAllPacketInSendQueue();
	void SetSessionID(unsigned int unSessionID)
	{
		m_unSessionID = unSessionID;
	}
	unsigned int GetSessionID()
	{
		return m_unSessionID;
	}
	bool GetSendable()
	{
		return m_bSendable;
	}
	void SetSendable(bool bSendable)
	{
		m_bSendable = bSendable;
	}
protected:
	int ParsePacket(char* pData, unsigned int unDataLen);
private:
	unsigned int m_unSessionID;
	char* m_pszRecvBuf;		
	unsigned int m_unRecvBufSize;
	unsigned int m_unRemainDataLen;
	bool m_bSendable;
	LFixLenCircleBuf m_FixLenCircleBufForSendQueue;
public:
	void SetSelectServer(LSelectServer* pSelectServer)
	{
		m_pSelectServer = pSelectServer;
	}
private:
	LSelectServer* m_pSelectServer;
public:
	void SetReconnect(bool bReconnect)
	{
		m_bReconnect = bReconnect;
	}
	bool GetReconnect()
	{
		return m_bReconnect;
	}
private:
	bool m_bReconnect;
};

class LSelectServerSessionManager
{
public:
	LSelectServerSessionManager(); 
	~LSelectServerSessionManager();
public:
	bool Initialize(t_Select_Server_Params& tssp);
	LSelectServerSession* AllocSession();
	void FreeSession(LSelectServerSession* pSession);

	bool AddOneUsingSession(LSelectServerSession* pSession);
	LSelectServerSession* GetOneUsingSession(unsigned int unSessionID);
	bool RemoveOneUsingSession(unsigned int unSessionID);
private:
	map<unsigned int, LSelectServerSession*> m_mapSessionManager;
	queue<LSelectServerSession*> m_queueSessionPool;
	unsigned int m_unCurrentSessionID;
public:
	void SetSelectServer(LSelectServer* pSelectServer)
	{
		m_pSelectServer = pSelectServer;
	}
private:
	LSelectServer* m_pSelectServer;

public:
	bool BuildSet();
	fd_set& GetRecvSet();
	fd_set& GetSendSet();
	void RemoveFromSendSet(int nSocket);
	void AddToSendSet(int nSocket);
	void AddToRecvSet(int nSocket);
	void RemoveFromRecvSet(int nSocket);
	map<int, LSelectServerSession*>* GetSessionManagerForEvent();
	LSelectServerSession* GetSessionFromSocketToSession(int nSocket);
	int GetMaxSocketPlus1()
	{
		return m_nMaxSocketPlus1;
	}
private:
	fd_set m_SETRecvSocket;
	fd_set m_SETSendSocket;
	map<int, LSelectServerSession*> m_mapSocketMapToSession;
	int m_nMaxSocketPlus1;
};

typedef struct _Recved_Packet
{
	unsigned int unSessionID;
	LPacketSingle* pPacketForRecv;
	_Recved_Packet()
	{
		unSessionID 	= 0;
		pPacketForRecv 	= NULL;
	}
}t_Recved_Packet;

#ifdef __ADD_SEND_BUF_CHAIN__
typedef struct _Send_Packet
{
	unsigned int unSessionID;
	LPacketBroadCast* pPacketForSend;
	_Send_Packet()
	{
		unSessionID = 0;
		pPacketForSend = NULL;
	}
}t_Send_Packet;
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
typedef struct _Send_Packet 		//	闇�鍙戦�鐨勬暟鎹寘
{
	unsigned int unSessionID;
	_Send_Packet()
	{
		unSessionID = 0;
	}
}t_Send_Packet;
#endif

class LSelectServer : public LThreadBase, public LListenSocket
{
public:
	LSelectServer();
	~LSelectServer();
public:
	bool Initialize(t_Select_Server_Params& tssp);

public:	
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop(); 
protected:
	int CheckForSocketEvent();

public:	
	bool AddOneCloseSessionWork(unsigned int unSessionID);
	bool GetOneRecvedPacket(t_Recved_Packet& tRecvedPacket);
	bool FreeOneRecvedPacket(LPacketSingle* pRecvPacket);
	LPacketSingle* AllocOneRecvPacket(unsigned short usPacketLen); 
	bool AddOneRecvedPacket(unsigned int unSessionID, LPacketSingle* pRecvPacket);
	bool GetOneCloseSessionWork(unsigned int& unSessionID);
private:
	LFixLenCircleBuf m_FixLenCircleBufForRecvedPacket;
#ifndef WIN32
	pthread_mutex_t m_mutexForCloseSessionQueue;
#else
	CRITICAL_SECTION m_mutexForCloseSessionQueue;
#endif
	LFixLenCircleBuf m_FixLenCircleBufForCloseSession;
	LPacketPoolManager<LPacketSingle> m_RecvPacketPoolManager;
public:
#ifdef __ADD_SEND_BUF_CHAIN__
	LPacketBroadCast* AllocOneSendPacket(unsigned short usPacketLen);
	bool AddOneSendPacket(unsigned int unSessionID, LPacketBroadCast* pBroadCastPacket);
	void PostAllSendPacket();
	bool FreeOneSendPacket(LPacketBroadCast* pBroadCastPacket);
	bool GetOneSendPacket(t_Send_Packet& tSendPacket);
#endif
	int SendData();

#ifdef __EPOLL_TEST_STATISTIC__
	void PrintBufStatus();
#endif
#ifdef __ADD_SEND_BUF_CHAIN__
private:
	LFixLenCircleBuf m_FixLenCircleBufForSendPacketForMainLogic;
	LFixLenCircleBuf m_FixLenCircleBufForSendPacket;
	LPacketPoolManager<LPacketBroadCast> m_SendPacketPoolManager;
#endif

#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
public:	
	bool SendPacket(unsigned int unSessionID, LPacketSingle* pPacket);
	bool GetOneSessionToSend(t_Send_Packet& tSendPacket);
private:
	LFixLenCircleBuf m_FixLenCircleBufForSendSessionID;
#endif

public:
	LSelectServerSession* AllocSession();
	LSelectServerSession* FindSession(unsigned int unSessionID);
	void FreeSession(LSelectServerSession* pSelectServerSession);
	bool RemoveSession(unsigned int unSessionID);
	bool AddSession(LSelectServerSession* pSelectServerSession);
private:
	LSelectServerSessionManager m_SelectServerSessionManager;

public:
	bool InitializeConnectWorkManagerThreadManager(unsigned int unMaxWorkItemCountInWorkQueue);
	void AddOneConnectWork(char* pszIP, unsigned short usPort, bool bMultiConnect = false);
	void CompleteConnectWork(bool bSuccess, char* pszIP, unsigned short usPort, int nConnecttedSocket);
	bool GetOneConnectedSession(t_Connector_Work_Result& cwr); 		
private:
#ifndef WIN32
	pthread_mutex_t m_mutexForConnectToServer;		
#else
	CRITICAL_SECTION m_mutexForConnectToServer;
#endif
	vector<t_Connector_Work_Item> m_vecConnectted;
	LFixLenCircleBuf m_FixLenCircleBufForConnectWork;
	LConnectorWorkManager m_ConnectorWorkManager;
};

#endif