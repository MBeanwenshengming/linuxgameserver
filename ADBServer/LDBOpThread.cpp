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

#include "LDBOpThread.h"
#include "../NetWork/LIniFileReadAndWrite.h"
#include "LDBServerMainLogicThread.h"

LDBOpThread::LDBOpThread()
{
	m_unDBJobProcessed = 0;
	m_unMaxJobProcessNumOnce = 100;
	m_pDBMainLogic = NULL;
}

LDBOpThread::~LDBOpThread()
{
}

bool LDBOpThread::Initialize(char* pConfigFileForDBServer, char* pHeadForConfig)
{
	if (m_pDBMainLogic == NULL)
	{
		return false;
	}
	if (pConfigFileForDBServer == NULL || pHeadForConfig == NULL)
	{
		return false;
	}
	m_DBOpProcessProcManager.SetDBOpThread(this);
	if (!m_DBOpProcessProcManager.Initialize())
	{
		return false;
	}
	LWorkQueueManagerConfigReader wqmcr;
	if (!wqmcr.ReadWorkQueueConfig(pConfigFileForDBServer, pHeadForConfig))
	{
		return false;
	}
	t_WorkItem_Pool_Init_Param* pArraywpip = NULL;
	unsigned short usArrayCount = 0;
	unsigned int unGlobalQueueSize = 0;
	wqmcr.GetInitParam(&pArraywpip, usArrayCount, unGlobalQueueSize);

	if (!m_LocalWorkQueue.InitializePool(pArraywpip, usArrayCount, unGlobalQueueSize))
	{
		return false;
	}

	if (!m_MySqlConnector.Initialize())
	{
		return false;
	}

	if (!ConnectToDBServer(pConfigFileForDBServer, pHeadForConfig))
	{
		return false;
	}
	if (!m_MySqlConnector.Connect())
	{
		return false;
	}
	return true;
}

int LDBOpThread::ThreadDoing(void* pParam)
{
	while (1)
	{
		if (CheckForStop())
		{
			break;
		}
		ProcessDBJob();
		if (m_unDBJobProcessed == 0)
		{ 
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
	}
	return 0;
} 

void LDBOpThread::ProcessDBJob()
{
	for (unsigned int unIndex = 0; unIndex < m_unMaxJobProcessNumOnce; ++unIndex)
	{
		LWorkItem* pWorkItem = m_LocalWorkQueue.GetOneWorkItemFromGlobalQueue();
		if (pWorkItem == NULL)
		{
			return;
		}
		m_unDBJobProcessed++;
		//	处理对应的数据库操作
		m_DBOpProcessProcManager.DispatchDBOpToProcessProc(pWorkItem);
		m_LocalWorkQueue.FreeOneWorkItem(pWorkItem);
	}
}

bool LDBOpThread::OnStart()
{
	return true;
}

void LDBOpThread::OnStop()
{
}

bool LDBOpThread::AddWorkItemToWorkQueue(LWorkItem* pWorkItem)
{
	if (pWorkItem == NULL)
	{
		return false;
	}
	return m_LocalWorkQueue.AddWorkItemToGlobalQueue(pWorkItem);
}

LWorkItem* LDBOpThread::GetOneFreeWorkItemFromPool(unsigned short usPoolBufLen)
{
	return m_LocalWorkQueue.AllocOneWorkItem(usPoolBufLen);
}

bool LDBOpThread::StopDBOpThread()
{
	Stop();

	pthread_t pThreadID = GetThreadHandle();
	if (pThreadID != 0)
	{
		pthread_join(pThreadID, NULL);
	}
	m_MySqlConnector.DisConnect();
	return true;
}

void LDBOpThread::ReleaseDBOpThreadResource()
{
	m_LocalWorkQueue.Release();
}

//	连接到数据库服务器
bool LDBOpThread::ConnectToDBServer(char *pConfigFile, char* pSection)
{
	if (pConfigFile == NULL || pSection == NULL)
	{
		return false;
	}

	LIniFileReadAndWrite ifrw;
	if (!ifrw.OpenIniFile(pConfigFile))
	{
		return false;
	}
	//	读取配置文件
	char szSection[256];
	sprintf(szSection, "%s_DB_ServerInfos", pSection);

	char* pKey = "ServerAddress";
	char szServerAddress[128];
	memset(szServerAddress, 0, sizeof(szServerAddress));
	int nReadResult = ifrw.read_profile_string(szSection, pKey, szServerAddress, 127, "");
	if (nReadResult <= 0)
	{
		return false;
	}

	pKey = "ServerDataBase";
	char szServerDataBase[128];
	memset(szServerDataBase, 0, sizeof(szServerDataBase));
	nReadResult = ifrw.read_profile_string(szSection, pKey, szServerDataBase, 127, "");
	if (nReadResult <= 0)
	{
		return false;
	}
	pKey = "ServerUser";
	char szServerUser[128];
	memset(szServerUser, 0, sizeof(szServerUser));
	nReadResult = ifrw.read_profile_string(szSection, pKey, szServerUser, 127, "");
	if (nReadResult <= 0)
	{
		return false;
	}
	pKey = "ServerPassWord";
	char szServerPassWord[128];
	memset(szServerPassWord, 0, sizeof(szServerPassWord));
	nReadResult = ifrw.read_profile_string(szSection, pKey, szServerPassWord, 127, "");
	if (nReadResult <= 0)
	{
		return false;
	}
	return m_MySqlConnector.SetDBServerAddress(szServerAddress, szServerDataBase, szServerUser, szServerPassWord);
}

void LDBOpThread::SetDBServerMainLogic(LDBServerMainLogicThread* pdbsmlt)
{
	m_pDBMainLogic = pdbsmlt;
}
LDBServerMainLogicThread* LDBOpThread::GetDBServerMainLogic()
{
	return m_pDBMainLogic;
}
