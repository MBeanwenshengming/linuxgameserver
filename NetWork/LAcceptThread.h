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

#ifndef __ACCEPT_THREAD_DEFINED__
#define __ACCEPT_THREAD_DEFINED__
#include "LThreadBase.h"
#include "LListenSocket.h"
#include "LFixLenCircleBuf.h"

class LNetWorkServices;
class LSession;
class LPacketSingle;

class LAcceptThread :
	public LThreadBase
{
public:
	LAcceptThread(void);
	virtual ~LAcceptThread(void);
public:
	bool Initialize(char* pListenIP, unsigned short usListenPort, unsigned int unListenListSize, unsigned int unWillServClientNum, bool bInitialListen = true);
	void SetNetWorkServices(LNetWorkServices* pNws);

	void GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort);
public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop(); 
	
	void StopAcceptThread();
	//	 释放线程使用的资源
	void ReleaseAcceptThreadResource();

private:
	LListenSocket m_listenSocket;
	LNetWorkServices* m_pNetWorkServices;
public:
	//	NetWorkServices释放数据包回缓冲区
	void FreeAcceptedPacket(LPacketSingle* pPacket);
public:
	//	连接上来时，分配一个包
	LPacketSingle* GetOneAcceptedPacket();
	LFixLenCircleBuf m_AcceptedPacketPool;		//	接收数据的数据包的缓冲
	void BuildAcceptedPacket(LSession* pSession, LPacketSingle* pPacket);
private:
	LFixLenCircleBuf m_AccpetedSessionPacket;

	char m_szListenIp[30];
	unsigned short m_usListenPort;
};
#endif

