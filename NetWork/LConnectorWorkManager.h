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

#ifndef __LINUX_CONNECTOR_WORK_MANAGER_HEADER_DEFINED__
#define __LINUX_CONNECTOR_WORK_MANAGER_HEADER_DEFINED__

#include "LThreadBase.h"
#include "LFixLenCircleBuf.h"

class LSelectServer;
class LNetWorkServices;

//	连接工作的内容
//	每个连接都开起一个线程，每个线程起动一个连接connect
typedef struct _Connector_Work_Item
{
	char szIP[32];					//	需要连接的IP
	unsigned short usPort;			//	连接的端口号
	char* pExtendData;				//	附加数据
	unsigned short usExtDataLen;	//	附加数据长度
	_Connector_Work_Item()
	{
		memset(szIP, 0, sizeof(szIP));
		usPort = 0;
		pExtendData = NULL;
		usExtDataLen = 0;
	}
}t_Connector_Work_Item;

typedef struct _Connector_Work_Result
{
	bool bSuccessed;
	char szIP[32];
	unsigned short usPort;
	int nSocket;
	char* pExtendData;
	unsigned short usExtDataLen;	
	_Connector_Work_Result()
	{
		bSuccessed = false;
		memset(szIP, 0, sizeof(szIP));
		usPort = 0;
		nSocket = -1;
		pExtendData = NULL;
		usExtDataLen = 0;
	}
}t_Connector_Work_Result;

class LConnectorWorkManager : public LThreadBase
{
public:
	LConnectorWorkManager();
	~LConnectorWorkManager();
public: 
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
public:
	bool Initialize(unsigned int unMaxWorkItemCount, int nUseWhichServerType, LSelectServer* pss, LNetWorkServices* pes);

	bool AddConnectWork(char* pszIP, unsigned short usPort, char* pExtData, unsigned short usDataLen);
private:
	LFixLenCircleBuf m_FixCircleBufForConnectWork;

	//	使用何种服务器
	int m_nUseWhichServerType;
	LSelectServer* m_pss;
	LNetWorkServices* m_pes;
};

//	连接线程，连接完成，自动关闭
class LConnectorWorkThread : public LThreadBase
{
public:
	LConnectorWorkThread();
	~LConnectorWorkThread();
public: 
	bool SetParams(char* pszIP, unsigned short usPort, int nUseWhichServerType, LSelectServer* pss, LNetWorkServices* pes, char* pExtData, unsigned short usExtDataLen);

	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();
private:
	char m_szIP[32];
	unsigned short m_usPort;
	char* m_pExtData;
	unsigned short m_usExtDataLen;
	int m_nUseSelectServer;		//	使用那种连接服务 , 1使用SelectServer, 2使用EpollServer
	//	SelectServer
	LSelectServer* m_pSelectServer;
	LNetWorkServices* m_pEpollServer;
};

#endif

