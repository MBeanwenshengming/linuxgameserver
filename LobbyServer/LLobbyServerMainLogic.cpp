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

#include "LLobbyServerMainLogic.h"
#include "../NetWork/LIniFileReadAndWrite.h"

LLobbyServerMainLogic::LLobbyServerMainLogic()
{
}
LLobbyServerMainLogic::~LLobbyServerMainLogic()
{
}


bool LLobbyServerMainLogic::InitializeLobbyServerMainLogic(char* pConfigFileForNetWork, char* pConfigFileForMasterServer, char* pConfigFileForGameDBServer, char* pConfigFileForLobbyServer)
{
	m_PacketProcessProcManager.SetLobbyServerMainLogic(this);
	if (!m_PacketProcessProcManager.Initialize())
	{
		return false;
	}
	if (!m_UserManager.InitializeUserPool(500))
	{
		return false;
	}
	if (!ReadConfigForLobbyServer(pConfigFileForLobbyServer))
	{
		return false;
	}

//	m_ConnectToGameDBServer.SetLobbyServerMainLogic(this);
//	if (!m_ConnectToGameDBServer.Initialize(pConfigFileForGameDBServer))
//	{
//		return false;
//	}
	m_ConnectToServerNetWork.SetLobbyServerMainLogic(this);
	if (!m_ConnectToServerNetWork.Initialize(pConfigFileForLobbyServer, 500, "LobbyServer_1_Server"))
	{
		return false;
	}

	m_ConnectToMasterServer.SetLobbyServerMainLogic(this);
	if (!m_ConnectToMasterServer.Initialize(pConfigFileForMasterServer))
	{
		return false;
	}
	if (!this->InitializeNetWork(pConfigFileForNetWork, 500, "LobbyServer_1_Client"))
	{
		return false;
	}

	m_LBConnectToServerManager.SetLobbyServerMainLogic(this);
	if (!m_LBConnectToServerManager.Initialize(500, 2, NULL, m_ConnectToServerNetWork.GetNetWorkServices()))
	{
		return false;
	}
	return true;
}

bool LLobbyServerMainLogic::ReadConfigForLobbyServer(char* pConfigFileForLobbyServer)
{
	if (pConfigFileForLobbyServer == NULL)
	{
		return false;
	}
	//	打开文件
	LIniFileReadAndWrite inirw;
	if (!inirw.OpenIniFile(pConfigFileForLobbyServer))
	{
		return false;
	}
	int nReadTempValue = 0;
	char* pSection = "LobbyServer";
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

int LLobbyServerMainLogic::ThreadDoing(void* pParam)
{
	time_t tLastSendServeCountToMasterServer = 0;

	while (1)
	{
		if (CheckForStop())
		{
			break;
		}
		//	处理超时连接玩家
		m_UserManager.ProcessWillConnectUser();

		m_ConnectToMasterServer.ProcessRecvedPacket();
		unsigned int unPacketProcessedOfMasterServer = m_ConnectToMasterServer.GetPacketProcessed();

	//	m_ConnectToGameDBServer.ProcessRecvedPacket();
	//	unsigned int unPacketProcessedOfGameDBServer = m_ConnectToGameDBServer.GetPacketProcessed();
	
		m_ConnectToServerNetWork.ProcessRecvedPacket();
unsigned int unPacketProcessedOfGameDBServer = m_ConnectToServerNetWork.GetPacketProcessed();

		ProcessRecvedPacket();
		unsigned int unPacketProcessedOfClientNetWork = GetPacketProcessed();
		
		if (unPacketProcessedOfMasterServer == 0 && unPacketProcessedOfGameDBServer == 0 && unPacketProcessedOfClientNetWork == 0)
		{ 
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}

		time_t tNow = time(NULL);
		if (tLastSendServeCountToMasterServer == 0 || tNow - tLastSendServeCountToMasterServer >= 1)
		{
			unsigned int unServeCount = m_UserManager.GetServeCount();

			LPacketSingle* pPacket = m_ConnectToMasterServer.GetOneSendPacketPool(128);
			pPacket->SetPacketID(Packet_SS_Server_Current_Serve_Count);
			pPacket->AddUInt(unServeCount);
			m_ConnectToMasterServer.AddOneSendPacket(pPacket);

			tLastSendServeCountToMasterServer = tNow;
		}
	}
	return 0;
}
bool LLobbyServerMainLogic::OnStart()
{
	if (!NetWorkStart())
	{
		return false;
	}
	return true;
}
void LLobbyServerMainLogic::OnStop()
{
}

bool LLobbyServerMainLogic::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	if (!m_GateServerManager.AddNewUpServer(u64SessionID))
	{
		return false;
	}
	return true;
}
void LLobbyServerMainLogic::OnRemoveSession(uint64_t u64SessionID)
{
	m_GateServerManager.RemoveGateServerBySessionID(u64SessionID);
}
void LLobbyServerMainLogic::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	E_LobbyServer_Packet_From_Type eFromType = E_LobbyServer_Packet_From_GateServer;
	m_PacketProcessProcManager.DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, eFromType);
}

bool LLobbyServerMainLogic::StopLobbyServerMainLogic()
{
	Stop();
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		pthread_join(pID, NULL);
	}
	m_ConnectToMasterServer.StopConnectToMasterServer();

//	m_ConnectToGameDBServer.StopConnectToGameDBServer();
	m_LBConnectToServerManager.StopAllConnectThread();
	NetWorkDown();

	m_ConnectToServerNetWork.StopConnectToServersNetWork();
	return true;
}

void LLobbyServerMainLogic::ReleaseLobbyServerMainLogicResource()
{
	m_ConnectToMasterServer.ReleaseConnectToMasterServerResource();
	//m_ConnectToGameDBServer.ReleaseConnectToGameDBServerResource();
	m_LBConnectToServerManager.ReleaseServerManagerResource();
	m_ConnectToServerNetWork.ReleaseConnectToServersNetWork();
}

LLBServerManager* LLobbyServerMainLogic::GetConnectToServerManager()
{
	return &m_LBConnectToServerManager;
}

LLBConnectToServersNetWork* LLobbyServerMainLogic::GetConnectToServerNetWork()
{
	return &m_ConnectToServerNetWork;
}

