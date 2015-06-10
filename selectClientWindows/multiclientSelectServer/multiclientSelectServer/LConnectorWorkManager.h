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

typedef struct _Connector_Work_Item
{
	char szIP[32];
	unsigned short usPort;
	char* pExtendData;
	unsigned short usExtDataLen;
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

	int m_nUseWhichServerType;
	LSelectServer* m_pss;
	LNetWorkServices* m_pes;
};

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
	int m_nUseSelectServer;
	LSelectServer* m_pSelectServer;
	LNetWorkServices* m_pEpollServer;
};

#endif

