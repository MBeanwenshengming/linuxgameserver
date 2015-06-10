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

#ifndef __LINUX_SERVER_BASE_NET_WORK__
#define __LINUX_SERVER_BASE_NET_WORK__

#include "IncludeHeader.h"
#include "LRecvThread.h"

class LNetWorkServices;
class LPacketBroadCast;

//class LSendPacketContainer
//{
//public:
//	LSendPacketContainer();
//	~LSendPacketContainer();
//public:
//	void SetNetWorkServices(LNetWorkServices* pNetWorkServices);
//	void SetSendPacket(LPacketBroadCast* pSendPacket);
//	LPacketBroadCast* GetSendPacket();
//private:
//	LNetWorkServices* m_pNetWorkServices;
//	LPacketBroadCast* m_pSendPacket;
//};
//
typedef struct _Session_Accepted
{
	char szIp[16];
	unsigned short usPort; 
	int nRecvThreadID;
	int nSendThreadID;
	unsigned short usExtDataLen;
	char szExtData[8 * 1024];
	_Session_Accepted()
	{
		memset(szIp, 0, sizeof(szIp));
		usPort = 0;
		nRecvThreadID = -1;
		nSendThreadID = -1;
		usExtDataLen  = 0;
		memset(szExtData, 0, sizeof(szExtData));
	}
}t_Session_Accepted;

class LServerBaseNetWork
{
public:
	LServerBaseNetWork();
	virtual ~LServerBaseNetWork();
public:
	bool InitializeNetWork(char* pConfigFile, unsigned int unMaxNumProcessPacketOnce, char* pSectionHeader, bool bInitializeAccept = true);
	bool NetWorkStart();
	void NetWorkDown();
public:
	virtual bool OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa);
	virtual void OnRemoveSession(uint64_t u64SessionID);
	virtual void OnRecvedPacket(t_Recv_Packet& tRecvedPacket);
public:
	void ProcessRecvedPacket();
	unsigned int GetPacketProcessed()
	{
		return m_unPacketProcessed;
	}
public:
	//	返回一个
	//	bool GetOneSendPacket(unsigned short usDataLen, LSendPacketContainer& spc);
	LPacketBroadCast* GetOneSendPacket(unsigned short usDataLen);
	//	bool SendOnePacket(uint64_t u64SessionID, int nSendThreadID, LSendPacketContainer* pPacketContainerForSend);
#ifdef __ADD_SEND_BUF_CHAIN__
	bool SendOnePacket(uint64_t u64SessionID, int nSendThreadID, LPacketBroadCast* pPacket);
#endif
#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
	bool SendOnePacket(uint64_t u64SessionID, int nSendThreadID, LPacketSingle* pPacket);
#endif
	void KickOutOneSession(uint64_t u64SessionID);
	//	将在本地缓存的数据包发送出去,在确认需要发送的连接全部调用过SendOnePacket后，调用该函数，因为发送的数据包时使用的是引用记数，如果提前调用，那么数据包有可能已经回收
	void FlushSendPacket();
	LNetWorkServices* GetNetWorkServices()
	{
		return m_pNetWorkServices;
	}
private: 
	LNetWorkServices* m_pNetWorkServices;
	unsigned int m_unMaxNumProcessPacketOnce;		//	一次最大处理的数据包数量

private:
	unsigned int m_unPacketProcessed;		//	本论循环处理的数据包数量
	void BuildSessionAcceptedPacket(t_Session_Accepted& tsa, LPacketSingle* pPacket);
public:
	void GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort);
};
#endif

