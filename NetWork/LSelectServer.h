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
	unsigned int unMaxSessionCount;		//	最大的连接数
	unsigned int unMaxSendQueueSize;	//	每个连接发送数据的队列的最大数量
	unsigned int unRecvBufSize;			//	接收缓存的大小
	bool bIsConnector;					//	是一个连接器，需要连接多个服务器时使用
	unsigned int unRecvedPacketQueueSize;	//	接收到的数据包的队列长度
	unsigned int unCloseWorkItemQueueSize;	//	需要关闭的连接的队列长度
	t_Packet_Pool_Desc* pppdForRecv;		//	接收缓存初始化描述参数	
	unsigned short  usppdForRecvSize;		//	描述参数数组大小
	t_Packet_Pool_Desc* pppdForSend;		//	发送缓存初始化描述参数
	unsigned short usppdForSendSize;		//	描述参数数组大小

	unsigned int unSendPacketSizeForMainLogic;	//	主逻辑线程本地使用的发送数据包的缓存
	unsigned int unSendPacketSizeForSendThread;	//	发送线程本地的缓存大小
	unsigned short usListenListSize;			//	监听队列长度	
	unsigned int unConnectWorkItemCountForCircleBufLen;	//	连接工作队列的最大长度
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
	//	有数据的时候，接收数据
	int SessionRecv();
	//	发送数据
	int SessionSend(LPacketBroadCast* pPacket);
	//	发送在队列中的数据
	int SessionSendInSendQueue();
	//	重置
	void Reset();
	//	释放没有发送的数据
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
	//	连接ID
	unsigned int m_unSessionID;
	//	接收缓存
	char* m_pszRecvBuf;		
	unsigned int m_unRecvBufSize;
	//	char m_szRecvBuf[128 * 1024];
	//	遗留没有解析的数据
	unsigned int m_unRemainDataLen;
	//	是否可以发送数据
	bool m_bSendable;
	//	每个连接有一个发送队列
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
	//	初始化SelectServer
	bool Initialize(t_Select_Server_Params& tssp);
	LSelectServerSession* AllocSession();
	void FreeSession(LSelectServerSession* pSession);

	//	添加一个正在使用的连接到管理器
	bool AddOneUsingSession(LSelectServerSession* pSession);
	//	查找一个正在使用的连接
	LSelectServerSession* GetOneUsingSession(unsigned int unSessionID);
	//	移除一个正在使用的连接
	bool RemoveOneUsingSession(unsigned int unSessionID);
private:
	//	当前存在的连接
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
	fd_set GetRecvSet();
	fd_set GetSendSet();
	map<int, LSelectServerSession*>* GetSessionManagerForEvent();
	LSelectServerSession* GetSessionFromSocketToSession(int nSocket);
	int GetMaxSocketPlus1()
	{
		return m_nMaxSocketPlus1;
	}
private:
	//	接收事件监听套接字
	fd_set m_SETRecvSocket;
	//	发送事件监听套接字
	fd_set m_SETSendSocket;
	//	监听SOCKET对应的连接
	map<int, LSelectServerSession*> m_mapSocketMapToSession;
	int m_nMaxSocketPlus1;	//	最大socket加1
};

typedef struct _Recved_Packet		//	接收到的数据包
{
	unsigned int unSessionID;
	LPacketSingle* pPacketForRecv;
	_Recved_Packet()
	{
		unSessionID 	= 0;
		pPacketForRecv 	= NULL;
	}
}t_Recved_Packet;

typedef struct _Send_Packet 		//	需要发送的数据包
{
	unsigned int unSessionID;
	LPacketBroadCast* pPacketForSend;
	_Send_Packet()
	{
		unSessionID = 0;
		pPacketForSend = NULL;
	}
}t_Send_Packet;

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

//	接收相关	
public:	
	bool AddOneCloseSessionWork(unsigned int unSessionID);
	bool GetOneRecvedPacket(t_Recved_Packet& tRecvedPacket);
	bool FreeOneRecvedPacket(LPacketSingle* pRecvPacket);
	LPacketSingle* AllocOneRecvPacket(unsigned short usPacketLen); 
	bool AddOneRecvedPacket(unsigned int unSessionID, LPacketSingle* pRecvPacket);
	bool GetOneCloseSessionWork(unsigned int& unSessionID);
private:
	LFixLenCircleBuf m_FixLenCircleBufForRecvedPacket;	//	接收到的数据包的队列
	pthread_mutex_t m_mutexForCloseSessionQueue;		//	需要关闭的连接的队列的保护器
	LFixLenCircleBuf m_FixLenCircleBufForCloseSession;	//	需要关闭连接的队列
	LPacketPoolManager<LPacketSingle> m_RecvPacketPoolManager;	//	接收数据包的缓冲池
// 	发送相关
public:
	LPacketBroadCast* AllocOneSendPacket(unsigned short usPacketLen);
	bool AddOneSendPacket(unsigned int unSessionID, LPacketBroadCast* pBroadCastPacket);
	void PostAllSendPacket();
	bool FreeOneSendPacket(LPacketBroadCast* pBroadCastPacket);
	bool GetOneSendPacket(t_Send_Packet& tSendPacket);
	int SendData();
#ifdef __EPOLL_TEST_STATISTIC__
	void PrintBufStatus();
#endif

private:
	LFixLenCircleBuf m_FixLenCircleBufForSendPacketForMainLogic;	//	主线程本地使用，因为涉及到引用记数，所以需要先放到本地，然后才推送到发送队列
	LFixLenCircleBuf m_FixLenCircleBufForSendPacket;	//	发送队列
	LPacketPoolManager<LPacketBroadCast> m_SendPacketPoolManager;	//	发送数据包的缓冲池

public:
	//	分配一个连接
	LSelectServerSession* AllocSession();
	//	查找一个连接
	LSelectServerSession* FindSession(unsigned int unSessionID);
	//	释放一个连接
	void FreeSession(LSelectServerSession* pSelectServerSession);
	//	删除一个连接
	bool RemoveSession(unsigned int unSessionID);
	//	添加一个连接
	bool AddSession(LSelectServerSession* pSelectServerSession);
private:
	//	连接管理器
	LSelectServerSessionManager m_SelectServerSessionManager;

public:
	//	初始化连接管理器
	bool InitializeConnectWorkManagerThreadManager(unsigned int unMaxWorkItemCountInWorkQueue);
	//	添加一项连接工作
	void AddOneConnectWork(char* pszIP, unsigned short usPort, bool bMultiConnect = false);
	//	完成一个连接工作
	void CompleteConnectWork(bool bSuccess, char* pszIP, unsigned short usPort, int nConnecttedSocket);
	//	获取一个 已经 连接的连接工作 
	bool GetOneConnectedSession(t_Connector_Work_Result& cwr); 
		
	//	是否存在该连接在需要连接的队列
	//	bool IsExistInConnected(char* pszIP, unsigned short usPort);
private:
	//	vector<t_Connector_Work_Item> m_vecWorkItem;	//	需要连接的套接字
	pthread_mutex_t m_mutexForConnectToServer;		
	vector<t_Connector_Work_Item> m_vecConnectted;	//	已经连接的套接字，在连接断开时，判断是否需要重新连接
	//	vector<t_Connector_Work_Item> m_vecConnectting;	//	正在连接的套接字
	LFixLenCircleBuf m_FixLenCircleBufForConnectWork;	//	连接返回的工作队列
	LConnectorWorkManager m_ConnectorWorkManager;
};

#endif


