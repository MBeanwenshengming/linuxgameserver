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

#include "LDBServerMainLogicThread.h"
#include "../NetWork/LIniFileReadAndWrite.h"

LDBServerMainLogicThread::LDBServerMainLogicThread()
{
	m_unDBMessageFromDBThreadProcessed = 0;
}

LDBServerMainLogicThread::~LDBServerMainLogicThread()
{
} 

bool LDBServerMainLogicThread::InitializeDBServer(char* pConfigFileForDBServer)
{
	if (pConfigFileForDBServer == NULL)
	{
		return false;
	}
	m_DBServerDBMessageProcessManager.SetDBServerMainLogicThread(this);

	m_PacketProcessManager.SetDBServerMainLogicThread(this);
	if (!m_PacketProcessManager.Initialize())
	{
		return false;
	}
	unsigned int unMaxUserInfoPoolSize = 200;
	if (!m_UserInfoManager.Initialize(unMaxUserInfoPoolSize))
	{
		return false;
	}

	if (!ReadDBServerConfigFile(pConfigFileForDBServer, "ADBServer_1"))
	{
		return false;
	}

	LWorkQueueManagerConfigReader wqmcr;
	if (!wqmcr.ReadWorkQueueConfig(pConfigFileForDBServer, "ADBServer_1"))
	{
		return false;
	}
	unsigned short usArraySize = 0;
	unsigned int unGlobalQueueItemSize = 0;
	t_WorkItem_Pool_Init_Param* pwpip = NULL;
	wqmcr.GetInitParam(&pwpip, usArraySize, unGlobalQueueItemSize);
	if (!m_LocalWorkQueueManager.InitializePool(pwpip, usArraySize, unGlobalQueueItemSize))
	{
		return false;
	}
	unsigned short usServerID = GetServerID();
	//char szConfigHeader[];
	//	初始化网络库
	if (!InitializeNetWork(pConfigFileForDBServer, 500, "ADBServer_1"))
	{
		return false;
	}
	//m_LocalWorkQueueManager.InitializePool;
	if (!NetWorkStart())
	{
		return false;
	}
	if (!InitializeDBOpThread(pConfigFileForDBServer, "ADBServer_DBOpThread_1"))
	{
		return false;
	}
	if (!m_DBOpThreadManager.Start())
	{
		return false;
	}
	m_DBConToMasterServer.SetDBServerMainLogic(this);
	if (!m_DBConToMasterServer.Initialize(pConfigFileForDBServer, "ADBServer_1_Connect_To_Master"))
	{
		return false;
	}
	return true;
}

bool LDBServerMainLogicThread::InitializeDBOpThread(char* pConfigFile, char* pSectionHeader)
{
	if (pConfigFile == NULL || pSectionHeader == NULL)
	{
		return false;
	}
	m_DBOpThreadManager.SetDBServerMainLogic(this);
	if (!m_DBOpThreadManager.Initialize(pConfigFile, pSectionHeader))
	{
		return false;
	}
	return true;
}

bool LDBServerMainLogicThread::ReadDBServerConfigFile(char* pConfigFileForDBServer, char* pSectionName)
{
	if (pConfigFileForDBServer == NULL)
	{
		return false;
	}
	LIniFileReadAndWrite ifrw;
	if (!ifrw.OpenIniFile(pConfigFileForDBServer))
	{
		return false;
	}
//	char szSection[256];
//	memset(szSection, 0, sizeof(szSection));
//	sprintf(szSection, "DBServer_%c_%hs", GetServerType(), GetServerID());
//	sprintf(szSection, "ADBServer_1");
	char* pSection = pSectionName;
	
	char* pKey = "ServerType";
	int nTemp = 0; 
	nTemp = ifrw.read_profile_int(pSection, pKey, 0);
	if (nTemp <= 0 || nTemp >= E_Server_Type_Max)
	{
		return false;
	}
	unsigned char ucServerType = (unsigned char)nTemp;

	nTemp = 0;
	pKey = "AreaID";
	nTemp = ifrw.read_profile_int(pSection, pKey, 0);
	if (nTemp <= 0 || nTemp > 0xffff)
	{
		return false;
	}
	unsigned short usAreaID = (unsigned short)nTemp;

	nTemp = 0;
	pKey = "GroupID";
	nTemp = ifrw.read_profile_int(pSection, pKey, 0);
	if (nTemp <= 0 || nTemp > 0xffff)
	{
		return false;
	}
	unsigned short usGroupID = (unsigned short)nTemp;

	nTemp = 0;
	pKey = "ServerID";
	nTemp = ifrw.read_profile_int(pSection, pKey, 0);
	if (nTemp <= 0 || nTemp > 0xffff)
	{
		return false;
	}
	unsigned short usServerID = (unsigned short)nTemp;
	E_Server_Type eServerType = (E_Server_Type)ucServerType;
	return SetServerID(eServerType, usAreaID, usGroupID, usServerID);
}

int LDBServerMainLogicThread::ThreadDoing(void* pParam)
{
	time_t tLastSendServeCountToMasterServer = 0;
	while (1)
	{
		if (CheckForStop())
		{
			break;
		}

		ProcessRecvedPacket();
		unsigned int unPacketProcessed = GetPacketProcessed();
		m_DBConToMasterServer.ProcessRecvedPacket();

		unsigned int unPacketProcessedFromMasterServer = m_DBConToMasterServer.GetPacketProcessed();

		ProcessDBMessageFromDTThread();

		if (unPacketProcessed == 0 && unPacketProcessedFromMasterServer == 0 && m_unDBMessageFromDBThreadProcessed == 0)
		{
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		} 

		time_t tNow = time(NULL);
		if (tLastSendServeCountToMasterServer == 0 || tNow - tLastSendServeCountToMasterServer >= 1)
		{
			unsigned int unServeCount = m_UserInfoManager.GetServeCount();
			LPacketSingle* pPacket = m_DBConToMasterServer.GetOneSendPacketPool(128);
			pPacket->SetPacketID(Packet_SS_Server_Current_Serve_Count);
			pPacket->AddUInt(unServeCount);
			m_DBConToMasterServer.AddOneSendPacket(pPacket);

			tLastSendServeCountToMasterServer = tNow;
		}
	}
	return 0;
}
bool LDBServerMainLogicThread::OnStart()
{
	return true;
}
void LDBServerMainLogicThread::OnStop()
{
}
bool LDBServerMainLogicThread::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{ 
	return m_ServerManager.NewUpServer(u64SessionID, tsa.nSendThreadID, tsa.nRecvThreadID, tsa.szIp, tsa.usPort); 
}
void LDBServerMainLogicThread::OnRemoveSession(uint64_t u64SessionID)
{
	m_ServerManager.RemoveServerBySessionID(u64SessionID);
}
void LDBServerMainLogicThread::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	E_DBServer_Packet_From_Type eFromType = E_DBServer_Packet_From_Client;
	m_PacketProcessManager.DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, eFromType);
}

//	停止主线程
bool LDBServerMainLogicThread::StopDBServerMainLogicThread()
{
	Stop();
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		pthread_join(pID, NULL);
	}
	NetWorkDown();
	m_DBOpThreadManager.StopDBOpThread();
	return true;
}
//	释放主线程资源
bool LDBServerMainLogicThread::ReleaseDBServerMainLogicThreadResource()
{
	m_LocalWorkQueueManager.Release();
	m_DBOpThreadManager.ReleaseDBOpThreadResource();
	return true;
}

LDBServerPacketProcess* LDBServerMainLogicThread::GetPacketProcessManager()
{
	return &m_PacketProcessManager;
}


LDBServerConnectToMasterServer* LDBServerMainLogicThread::GetDBServerConnectToMasterServer()
{
	return &m_DBConToMasterServer;
}


LServerManager* LDBServerMainLogicThread::GetServerManager()
{
	return &m_ServerManager;
}

LWorkQueueManager* LDBServerMainLogicThread::GetWorkQueueManager()
{
	return &m_LocalWorkQueueManager;
}

LWorkItem* LDBServerMainLogicThread::GetOneFreeWorkItemFromPoolInMainLogic(unsigned short usNeedBufLen)
{
	return m_LocalWorkQueueManager.AllocOneWorkItem(usNeedBufLen);
}
bool LDBServerMainLogicThread::AddOneWorkItemToMainLogic(LWorkItem* pWorkItem)
{
	if (pWorkItem == NULL)
	{
		return false;
	}
	return m_LocalWorkQueueManager.AddWorkItemToGlobalQueue(pWorkItem);
}


void LDBServerMainLogicThread::ProcessDBMessageFromDTThread()
{
	m_unDBMessageFromDBThreadProcessed = 0;
	unsigned int unMaxMessageProcessCount = 100;
	for (unsigned int unIndex = 0; unIndex < unMaxMessageProcessCount; ++unIndex)
	{
		LWorkItem* pWorkItem = m_LocalWorkQueueManager.GetOneWorkItemFromGlobalQueue();
		if (pWorkItem == NULL)
		{
			return;
		}

		m_DBServerDBMessageProcessManager.DispatchMessageToProcess(pWorkItem);
		m_unDBMessageFromDBThreadProcessed++;
		m_LocalWorkQueueManager.FreeOneWorkItem(pWorkItem);
	}
}
