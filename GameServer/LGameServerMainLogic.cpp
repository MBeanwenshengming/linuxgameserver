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

#include "LGameServerMainLogic.h"
#include "../NetWork/LIniFileReadAndWrite.h"

LGameServerMainLogic::LGameServerMainLogic()
{
}
LGameServerMainLogic::~LGameServerMainLogic()
{
}
bool LGameServerMainLogic::InitializeGameServer(char* pGameServerConfigFile, unsigned int unGameServerID)
{
	if (pGameServerConfigFile == NULL)
	{
		return false;
	}
	char* pGSSection = "GameServer_1";
	if (!ReadServerIDConfig(pGameServerConfigFile, pGSSection))
	{
		return false;
	}

	m_GSPacketProcessManager.SetGameServerMainLogic(this);
	if (!m_GSPacketProcessManager.Initialize())
	{
		return false;
	}
	m_GSServerManager.SetGameServerMainLogic(this);
	
	m_GSConToMasterServer.SetGameServerMainLogic(this);
	char* pszHeaderConfig = "GameServer_To_Master_1";
	if (!m_GSConToMasterServer.Initialize(pGameServerConfigFile, pszHeaderConfig))
	{
		return false;
	}

	m_GSConToServerNetWork.SetGameServerMainLogic(this);
	unsigned int unMaxPacketProcessOnceInConToServer = 100;
	pszHeaderConfig = "GameServer_To_Servers_1";
	if (!m_GSConToServerNetWork.InitialzeGSConToServerNetWork(pGameServerConfigFile, unMaxPacketProcessOnceInConToServer, pszHeaderConfig))
	{
		return false;
	}

	unsigned int unGSServerManagerWorkQueueSize = 100;
	if (!m_GSServerManager.Initialize(unGSServerManagerWorkQueueSize, 2, NULL, m_GSConToServerNetWork.GetNetWorkServices()))
	{
		return false;
	}
	pszHeaderConfig = "GameServer_NetWork_ServerClient_1";
	if (!InitializeNetWork(pGameServerConfigFile, 500, pszHeaderConfig))
	{
		return false;
	}
	return true;
}

bool LGameServerMainLogic::ReadServerIDConfig(char* pGameServerConfigFile, char* pSection)
{
	if (pGameServerConfigFile == NULL || pSection == NULL)
	{
		return false;
	}
	//	打开文件
	LIniFileReadAndWrite inirw;
	if (!inirw.OpenIniFile(pGameServerConfigFile))
	{
		return false;
	}
	int nReadTempValue = 0;
	char* pKey = "ServerType";
	nReadTempValue = inirw.read_profile_int(pSection, pKey, 0);
	if (nReadTempValue <= 0 || nReadTempValue >= E_Server_Type_Max) 
	{
		return false;
	}
	E_Server_Type eServerType = (E_Server_Type)nReadTempValue;

	nReadTempValue = 0;
	pKey = "AreaID";
	nReadTempValue = inirw.read_profile_int(pSection, pKey, 0);
	if (nReadTempValue <= 0 || nReadTempValue >= 0xFFFF)
	{
		return false;
	}
	unsigned short usAreaID = (unsigned short)nReadTempValue;

	pKey = "GroupID";
	nReadTempValue = 0;
	nReadTempValue = inirw.read_profile_int(pSection, pKey, 0);
	if (nReadTempValue <= 0 || nReadTempValue >= 0xFFFF)
	{
		return false;
	}
	unsigned short usGroupID = (unsigned short)nReadTempValue;

	pKey = "ServerID";
	nReadTempValue = 0;
	nReadTempValue = inirw.read_profile_int(pSection, pKey, 0);
	if (nReadTempValue <= 0 || nReadTempValue >= 0xFFFF)
	{
		return false;
	}
	unsigned short usServerID = (unsigned short)nReadTempValue;

	return SetServerID(eServerType, usAreaID, usGroupID, usServerID); 
}

int LGameServerMainLogic::ThreadDoing(void* pParam)
{ 
	while (1)
	{
		if (CheckForStop())
		{
			break;
		}


		m_GSConToMasterServer.ProcessRecvedPacket();
		unsigned int unPacketProcessedOfMasterServer = m_GSConToMasterServer.GetPacketProcessed(); 
	
		m_GSConToServerNetWork.ProcessRecvedPacket();
unsigned int unPacketProcessedOfGameDBServer = m_GSConToServerNetWork.GetPacketProcessed();

		ProcessRecvedPacket();
		unsigned int unPacketProcessedOfClientNetWork = GetPacketProcessed();
		
		if (unPacketProcessedOfMasterServer == 0 && unPacketProcessedOfGameDBServer == 0 && unPacketProcessedOfClientNetWork == 0)
		{ 
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
	}
	return 0;
}
bool LGameServerMainLogic::OnStart()
{
	if (!NetWorkStart())
	{
		return false;
	}
	return true;
}
void LGameServerMainLogic::OnStop()
{
}

bool LGameServerMainLogic::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	if (!m_GSClientServerManager.AddGSNewUpServer(u64SessionID, tsa.nRecvThreadID, tsa.nSendThreadID, tsa.szIp, tsa.usPort))
	{
		return false;
	}
	return true;
}
void LGameServerMainLogic::OnRemoveSession(uint64_t u64SessionID)
{
	m_GSClientServerManager.RemoveGSClientServerBySessionID(u64SessionID);
}
void LGameServerMainLogic::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	E_GameServer_Packet_From_Type eFromType = E_GS_Packet_From_Client_Server;
	m_GSPacketProcessManager.DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, eFromType);
}

bool LGameServerMainLogic::StopGameServerMainLogic()
{
	Stop();
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		pthread_join(pID, NULL);
	}
	NetWorkDown();

	m_GSServerManager.StopGSServerManagerConnectThread();

	m_GSConToServerNetWork.StopGSConnectToServerNetWork();

	m_GSConToMasterServer.StopGSConnectToMasterServer(); 
	return true;
}
void LGameServerMainLogic::ReleaseGameServerMainLogicResource()
{
	m_GSConToMasterServer.ReleaseGSConnectToMasterServerResource();
	m_GSServerManager.ReleaseGSServerManagerConnectThreadResource();
	m_GSConToServerNetWork.ReleaseGSConnectToServerNetWork();
}
