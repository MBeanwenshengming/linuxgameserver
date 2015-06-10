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

#include "LGateServerMainLogic.h"
#include "../NetWork/LIniFileReadAndWrite.h"

LGateServerMainLogic::LGateServerMainLogic()
{
}

LGateServerMainLogic::~LGateServerMainLogic()
{
}

//	初始化对公网网络层和连接到MasterServer连接
bool LGateServerMainLogic::Initialize(char* pNetWorkConfigFileName, char* pConnectToMasterServerConfigFileName, char* pConnnectToServerConfigFileName, char* pGateServerConfigFileName)
{
	m_GateServerLogicProcess.SetGateServerMainLogic(this);

	//	初始化协议包处理函数
	m_PacketProcessManager.SetGateServerMainLogic(this);
	if (!m_PacketProcessManager.Initialize())
	{
		return false;
	}
	m_ClientManager.SetGateServerMainLogic(this);
	if (!m_ClientManager.InitializeClientPool(500))		//	从配置文件获得,暂时写固定
	{
		return false;
	}
	if (!ReadGateServerConfigFile(pGateServerConfigFileName))
	{
		return false;
	}

	//	初始化向MasterServer的连接
	m_ConnectToMasterServer.SetGateServerMainLogic(this);
	if (!m_ConnectToMasterServer.Initialize(pConnectToMasterServerConfigFileName))
	{
		return false;
	}

	//	初始化服务器之间的网络连接
	m_ConnectToServerNetWork.SetGateServerMainLogic(this);
	if (!m_ConnectToServerNetWork.Initialize(pConnnectToServerConfigFileName, 500))
	{
		return false;
	}

	//	初始化对客户端的网络
	if (!InitializeNetWork(pNetWorkConfigFileName, 500, "GateServer_1", true))
	{
		return false;
	}

	//	初始化连接任务线程
	m_ServerManager.SetGateServerMainLogic(this);
	if (!m_ServerManager.Initialize(500, 2, NULL, m_ConnectToServerNetWork.GetNetWorkServices()))
	{
		return false;
	}
	return true;
}

bool LGateServerMainLogic::ReadGateServerConfigFile(char* pConfigFile)
{
	if (pConfigFile == NULL)
	{
		return false;
	}
	LIniFileReadAndWrite ifrw;
	if (!ifrw.OpenIniFile(pConfigFile))
	{
		return false;
	}
	int nReadTemp = 0;

	char* pSection = "GateServer_1";
	char* pKey = "ServerType";
	nReadTemp = ifrw.read_profile_int(pSection, pKey, 0);
	if (nReadTemp <= 0 || nReadTemp >= 0xff)
	{
		return false;
	}
	unsigned char ucServerType = (unsigned char)nReadTemp;

	nReadTemp = 0;
	pKey = "AreaID";
	nReadTemp = ifrw.read_profile_int(pSection, pKey, 0);
	if (nReadTemp <= 0 || nReadTemp >= 0xffff)
	{
		return false;
	}
	unsigned short usAreaID = (unsigned short)nReadTemp;

	nReadTemp = 0;
	pKey = "GroupID";
	nReadTemp = ifrw.read_profile_int(pSection, pKey, 0);
	if (nReadTemp <= 0 || nReadTemp >= 0xffff)
	{
		return false;
	}
	unsigned short usGroupID = (unsigned short)nReadTemp;

	nReadTemp = 0;
	pKey = "ServerID";
	nReadTemp = ifrw.read_profile_int(pSection, pKey, 0);
	if (nReadTemp <= 0 || nReadTemp >= 0xffff)
	{
		return false;
	}
	unsigned short usServerID = (unsigned short)nReadTemp;
	if (!SetServerID((E_Server_Type)ucServerType, usAreaID, usGroupID, usServerID))
	{
		return false;
	}
	return true;
}

bool LGateServerMainLogic::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	if (!m_ClientManager.AddNewUpClient(u64SessionID, tsa))
	{
		this->KickOutOneSession(u64SessionID);
	}
	return true;
}

void LGateServerMainLogic::OnRemoveSession(uint64_t u64SessionID)
{
	m_GateServerLogicProcess.ClientSessionDisconnect(u64SessionID);
}

void LGateServerMainLogic::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	E_Packet_From_Type eFromType = E_Packet_From_Client;
	m_PacketProcessManager.DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, eFromType);
}

// 线程虚函数
int LGateServerMainLogic::ThreadDoing(void* pParam)
{
	time_t tLastSendServeCountToMasterServer = 0;

	while (1)
	{
		if (CheckForStop())
		{
			break;
		}

		time_t tNow = time(NULL);
		map<uint64_t, t_Session_Kick_Out>::iterator _ito = m_mapSessionIDToKickOut.begin(); 
		while (_ito != m_mapSessionIDToKickOut.end())
		{
			t_Session_Kick_Out tsko = _ito->second;
			if (tNow > tsko.tTimeToKickOut)
			{
				KickOutOneSession(tsko.u64SessionID);
				m_mapSessionIDToKickOut.erase(_ito);
			}
			_ito++;
		}
		//	处理MasterServer数据包
		m_ConnectToMasterServer.ProcessPacket(100);	
		unsigned int unMasterServerPacketProcessed = m_ConnectToMasterServer.GetPacketProcessed();
		//	处理服务器之间的数据包
		m_ConnectToServerNetWork.ProcessRecvedPacket();
		unsigned int unConnectToServerNetWorkPacketProcessed = m_ConnectToServerNetWork.GetPacketProcessed();

		//	处理客户端发送过来的数据包
		ProcessRecvedPacket();
		unsigned int unClientPacketProcessed = GetPacketProcessed();

		if (unConnectToServerNetWorkPacketProcessed == 0 && unClientPacketProcessed == 0 && unMasterServerPacketProcessed == 0)
		{ 
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		if (tLastSendServeCountToMasterServer == 0 || tNow - tLastSendServeCountToMasterServer >= 1)
		{
			unsigned int unServeCount = m_ClientManager.GetServeCount();
			LPacketSingle* pPacket = m_ConnectToMasterServer.GetOneSendPacketPool(128);
			pPacket->SetPacketID(Packet_SS_Server_Current_Serve_Count);
			pPacket->AddUInt(unServeCount);
			m_ConnectToMasterServer.AddOneSendPacket(pPacket);
		}
	}
	return 0;
}

bool LGateServerMainLogic::OnStart()
{
	//	所有的服务器组件初始化完必之后，才打开对客户端的网络
	//	对客户端的连接的初始化
	if (!NetWorkStart())
	{
		return false;
	}
	return true;
}

void LGateServerMainLogic::OnStop()
{
}

bool LGateServerMainLogic::StopGateServerMainLogicThread()
{
	Stop();
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes == 0)
		{
		}
	}

	//	停止服务器连接服务器线程
	m_ServerManager.StopAllConnectThread();

	//	停止连接到MasterServer的连接
	m_ConnectToMasterServer.StopConnectToMasterServerThread();

	//	停止服务器之间的连接
	m_ConnectToServerNetWork.StopConnectToServerNetWork();
	
	//	停止对客户端的网络服务
	NetWorkDown();

	return true;
}

void LGateServerMainLogic::ReleaseGateServerMainLogicThreadResource()
{
	m_ConnectToMasterServer.ReleaseConnectToMasterServerThreadResource();
	m_ConnectToServerNetWork.ReleaseConnectToServerNetWorkResource();
	m_ServerManager.ReleaseServerManagerResource();
	m_ClientManager.ReleaseClientManagerResoutce();
}

void LGateServerMainLogic::AddSessionIDToKickOutQueue(uint64_t u64SessionID, unsigned int unTimeToKeep)
{
	time_t tNow 		= time(NULL);
	time_t tKickOutTime = tNow + unTimeToKeep;

	t_Session_Kick_Out tsko;
	tsko.u64SessionID 	= u64SessionID;
	tsko.tTimeToKickOut = tKickOutTime;

	m_mapSessionIDToKickOut[u64SessionID] = tsko;
}

