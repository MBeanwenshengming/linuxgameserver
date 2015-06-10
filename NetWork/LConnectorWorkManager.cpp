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

#include "LConnectorWorkManager.h"
#include "LSelectServer.h"
#include "LNetWorkServices.h"

LConnectorWorkManager::LConnectorWorkManager()
{
	m_nUseWhichServerType 	= 0;
	m_pss 					= NULL;
	m_pes 					= NULL;
}
LConnectorWorkManager::~LConnectorWorkManager()
{
}
int LConnectorWorkManager::ThreadDoing(void* pParam)
{
	while (1)
	{
		t_Connector_Work_Item cwi;
		memset(&cwi, 0, sizeof(cwi));

		E_Circle_Error errorID = m_FixCircleBufForConnectWork.GetOneItem((char*)&cwi, sizeof(cwi));
		if (errorID == E_Circle_Buf_Input_Buf_Null || errorID == E_Circle_Buf_Input_Buf_Not_Enough_Len)
		{
			return -1;
		}
		if (errorID == E_Circle_Buf_No_Error)		//	有连接任务
		{
			LConnectorWorkThread* pcwt = new LConnectorWorkThread;
			pcwt->SetParams(cwi.szIP, cwi.usPort, m_nUseWhichServerType, m_pss, m_pes, cwi.pExtendData, cwi.usExtDataLen);
			//	起动连接线程
			pcwt->Start();
		}
		else
		{ 
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
bool LConnectorWorkManager::OnStart()
{
	return true;
}
void LConnectorWorkManager::OnStop()
{
}

bool LConnectorWorkManager::Initialize(unsigned int unMaxWorkItemCount, int nUseWhichServerType, LSelectServer* pss, LNetWorkServices* pes)
{
	if (unMaxWorkItemCount == 0)
	{
		return false;
	}

	if (!m_FixCircleBufForConnectWork.Initialize(sizeof(t_Connector_Work_Item), unMaxWorkItemCount))
	{
		return false;
	}
	m_nUseWhichServerType 	= nUseWhichServerType;
	m_pss 					= pss;
	m_pes 					= pes;

	return true;
}

bool LConnectorWorkManager::AddConnectWork(char* pszIP, unsigned short usPort, char* pExtData, unsigned short usDataLen)
{
	if (pszIP == NULL)
	{
		return false;
	}
	t_Connector_Work_Item cwi;
	memset(&cwi, 0, sizeof(cwi));

	strncpy(cwi.szIP, pszIP, sizeof(cwi.szIP) - 1);
	cwi.usPort 			= usPort;
	cwi.pExtendData 	= pExtData;
	cwi.usExtDataLen 	= usDataLen;
	E_Circle_Error errorID = m_FixCircleBufForConnectWork.AddItems((char*)&cwi, 1);
	if (errorID == E_Circle_Buf_No_Error)
	{
		return true;
	}
	return false;
}

//	==================================LConnectorWorkThread============================
LConnectorWorkThread::LConnectorWorkThread()
{ 
	memset(m_szIP, 0, sizeof(m_szIP)); 
	m_usPort 				= 0; 
	m_pExtData 				= NULL;
	m_usExtDataLen 			= 0;
	//	使用那种连接服务 , 1使用SelectServer, 2使用EpollServer 
	m_nUseSelectServer 	= 0;			
	m_pSelectServer 	= NULL;
	m_pEpollServer 		= NULL;
}
LConnectorWorkThread::~LConnectorWorkThread()
{
}

bool LConnectorWorkThread::SetParams(char* pszIP, unsigned short usPort, int nUseWhichServerType, LSelectServer* pss, LNetWorkServices* pes, char* pExtData, unsigned short usExtDataLen)
{
	strncpy(m_szIP, pszIP, sizeof(m_szIP) - 1);
	m_usPort 			= usPort;
	m_nUseSelectServer 	= nUseWhichServerType;
	m_pSelectServer 	= pss;
	m_pEpollServer 		= pes;
	m_pExtData 			= pExtData;
	m_usExtDataLen 		= usExtDataLen;
	return true;
}

int LConnectorWorkThread::ThreadDoing(void* pParam)
{
	int nSocket 		= -1;
	bool bConnected 	= false;
	while(1)
	{
		nSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (nSocket == -1)
		{
			break;
		}
		sockaddr_in serverAddr;
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family 		= AF_INET;
		serverAddr.sin_port 		= htons(m_usPort);
		int nConvertSuccess 		= inet_aton(m_szIP, &serverAddr.sin_addr);
		if (nConvertSuccess == 0)
		{
//			char szError[512];
//			sprintf(szError, "LListenSocket::Initialized, inet_aton\n"); 
//			g_ErrorWriter.WriteError(szError, strlen(szError));
//			return false;
			break;			
		}
		int nConnected = connect(nSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
		if (nConnected == -1)
		{
			break;
		}
		bConnected = true;
		break;
	}
	if (bConnected)		//	连接成功
	{
		if (m_nUseSelectServer == 1)
		{
			m_pSelectServer->CompleteConnectWork(true, m_szIP, m_usPort, nSocket);
		}
		else if (m_nUseSelectServer == 2)
		{
			if (!m_pEpollServer->AddConnecttedSocketToSessionManager(nSocket, m_pExtData, m_usExtDataLen))
			{
				close(nSocket);
			}
		}
		else
		{
		}
	}
	else				//	连接失败
	{
		if (m_nUseSelectServer == 1)
		{
			m_pSelectServer->CompleteConnectWork(false, m_szIP, m_usPort, nSocket);
		}
		else if (m_nUseSelectServer == 2)
		{
			//m_pEpollServer;
		}
		else
		{
		}
	}
	delete[] m_pExtData;		//	删除附加数据
	//delete this;
	return 0;
}

bool LConnectorWorkThread::OnStart()
{
	return true;
}
void LConnectorWorkThread::OnStop()
{
}


