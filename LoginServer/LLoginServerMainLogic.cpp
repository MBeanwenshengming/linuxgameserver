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

#include "LLoginServerMainLogic.h"
#include "LClientManager.h"
#include "../NetWork/LIniFileReadAndWrite.h"
#include "../NetWork/LPacketSingle.h"
#include "../include/Server_To_Server_Packet_Define.h"

LLoginServerMainLogic::LLoginServerMainLogic()
{ 
	m_unPacketProcessedFromMasterServer = 0;
	m_unPacketProcessedFromAccountDBServer = 0;
	m_unMaxClientServ = 1000;
}

LLoginServerMainLogic::~LLoginServerMainLogic()
{
}

bool LLoginServerMainLogic::Initialize(char* pLoginServerConfigFileName, char* pNetWorkConfigFile, char* pMasterServerConfigFileName, char* pAccountDBServerConfigFileName)
{
	//	初始化协议处理函数
	m_PacketProcessManager.SetLoginServerMainLogic(this);
	if (!m_PacketProcessManager.Initialize())
	{
		return false;
	}

	if (!ReadLoginServerConfigFile(pLoginServerConfigFileName))
	{
		return false;
	}
	m_ClientManager.SetLSMainLogic(this);
	if (!m_ClientManager.Initialize(m_unMaxClientServ))
	{
		return false;
	}
	//	连接其它服务器的网络
	m_LSConToServerNetWork.SetLoginServerMainLogic(this);	
	if (!m_LSConToServerNetWork.Initialize(pLoginServerConfigFileName, 500, "LoginServer_To_Server_1"))
	{
		return false;
	}

	m_LSServerManager.SetLoginServerMainLogic(this);
	if (!m_LSServerManager.Initialize(500, 2, NULL, m_LSConToServerNetWork.GetNetWorkServices()))
	{
		return false;
	}
	//	读取本程序配置文件
	if (!m_ConnectToMasterServer.Initialize(pMasterServerConfigFileName))
	{
		return false;
	}
	//	初始化网络库
	if (!InitializeNetWork(pNetWorkConfigFile, 500, "LoginServer_1"))
	{
		return false;
	}
	if (!NetWorkStart())
	{
		return false;
	}

	return true;
}

int LLoginServerMainLogic::ThreadDoing(void* pParam)
{
	time_t tLastSendServeCountToMasterServer = 0;
	while (1)
	{
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
		//ProcessPacketFromAccountDBServer();
		ProcessPacketFromMasterServer();
		ProcessRecvedPacket();
		m_LSConToServerNetWork.ProcessRecvedPacket();

		unsigned int unPacketProcessedFromClient = GetPacketProcessed();
		unsigned int unPacketProcessedFromOtherServers = m_LSConToServerNetWork.GetPacketProcessed();
		if (unPacketProcessedFromClient == 0 && m_unPacketProcessedFromMasterServer == 0 && unPacketProcessedFromOtherServers == 0)
		{
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}


		if (tLastSendServeCountToMasterServer == 0 || tNow - tLastSendServeCountToMasterServer >= 1)
		{
			//	发送当前服务的客户端数量
			unsigned int unServeCount = m_ClientManager.GetClientCount();
			LPacketSingle* pPacket = m_ConnectToMasterServer.GetOneSendPacketPool(128);
			pPacket->SetPacketID(Packet_SS_Server_Current_Serve_Count);
			pPacket->AddUInt(unServeCount);
			m_ConnectToMasterServer.AddOneSendPacket(pPacket);

			tLastSendServeCountToMasterServer = tNow;
		}
		if (CheckForStop())
		{
			break;
		}
	}
	return 0;
}

//	停止主线程
bool LLoginServerMainLogic::StopLoginServerMainLogicThread()
{
	Stop();

	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{
		int nJoinRes = pthread_join(pID, NULL);
		if (nJoinRes != 0)
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
	//	停掉所有的网络相关的线程
	NetWorkDown();
	m_LSConToServerNetWork.StopConnectToServersNetWork();
	m_LSServerManager.StopAllConnectThread();
	m_ConnectToMasterServer.StopConnectToMaster();
	//m_ConnectToAccountDBServer.StopConnectToAccountDBServer();
	return true;
}
//	释放主线程资源
bool LLoginServerMainLogic::ReleaseLoginServerMainLogicThreadResource()
{
	m_ConnectToMasterServer.ReleaseConnectToMasterResource();
	//m_ConnectToAccountDBServer.ReleaseConnectToAccountDBServerResource();
	m_LSConToServerNetWork.ReleaseConnectToServersResource();
	m_LSServerManager.ReleaseServerManagerResource();
	return true;
}

bool LLoginServerMainLogic::OnStart()
{
	return true;
}

void LLoginServerMainLogic::OnStop()
{
}

//	网络虚函数 
bool LLoginServerMainLogic::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	if (!m_ClientManager.AddNewUpSession(u64SessionID, tsa))
	{
		KickOutOneSession(u64SessionID);			
		return false;
	}
	return true;
}

void LLoginServerMainLogic::OnRemoveSession(uint64_t u64SessionID)
{
	m_ClientManager.RemoveOneSession(u64SessionID);
}


//	处理接收到的协议信息
void LLoginServerMainLogic::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	//	将数据包分发的对应线程处理 
	m_PacketProcessManager.DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket, E_LoginServer_Packet_From_Client);	
}


//	处理连接到masterserver的连接上的数据包
void LLoginServerMainLogic::ProcessPacketFromMasterServer()
{
	m_unPacketProcessedFromMasterServer = 0;
	for (unsigned int ui = 0; ui < 100; ++ui)
	{
		LPacketSingle* pPacket = m_ConnectToMasterServer.GetOneRecvedPacket();
		if (pPacket == NULL)
		{
			break;
		}
		m_unPacketProcessedFromMasterServer++;
		if (pPacket->GetPacketType() == 1)	//	表示连接成功，那么处理连接的信息
		{
			LPacketSingle* pPacket = NULL;
			pPacket = m_ConnectToMasterServer.GetOneSendPacketPool(100);
			if (pPacket != NULL)
			{
				pPacket->SetPacketID(Packet_SS_Start_Req);
				m_ConnectToMasterServer.AddOneSendPacket(pPacket);
			}
		}
		else
		{
			//	处理接收到的数据包
			//
			//
			//	处理完置后，释放数据包
			m_PacketProcessManager.DispatchMessageProcess(0, pPacket, E_LoginServer_Packet_From_MasterServer);	
		}
		m_ConnectToMasterServer.FreePacket(pPacket);
	}
}

//	处理从AccountDBServer上来的数据包
void LLoginServerMainLogic::ProcessPacketFromAccountDBServer()
{
//	m_unPacketProcessedFromAccountDBServer = 0;
//	for (unsigned int ui = 0; ui < 100; ++ui)
//	{
//		LPacketSingle* pPacket = m_ConnectToAccountDBServer.GetOneRecvedPacket();
//		if (pPacket == NULL)
//		{
//			break;
//		}
//		m_unPacketProcessedFromAccountDBServer++;
//		m_PacketProcessManager.DispatchMessageProcess(0, pPacket, E_LoginServer_Packet_From_AccountDBServer);	
//		//	处理来自数据库服务器的消息包
//		m_ConnectToAccountDBServer.FreePacket(pPacket);
//	}
}

//	获取指向MasterServer连接
LConnectToMaster* LLoginServerMainLogic::GetConnectToMaster()
{
	return &m_ConnectToMasterServer;
}
//	获取指向AccountDBServer连接
LConnectToAccountDBServer* LLoginServerMainLogic::GetConnectToAccountDBServer()
{
	//return &m_ConnectToAccountDBServer;
	return NULL;
}

LLoginServerPacketProcess_Proc* LLoginServerMainLogic::GetPacketProcessProcManager()
{
	return &m_PacketProcessManager;
}

bool LLoginServerMainLogic::ReadLoginServerConfigFile(char* pFileName)
{
	if (pFileName == NULL)
	{
		return false;
	}
	//	打开文件
	LIniFileReadAndWrite inirw;
	if (!inirw.OpenIniFile(pFileName))
	{
		return false;
	}
	int nReadTempValue = 0;
	char* pSection = "LoginServer_1";
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

	pKey = "MaxServClient";
	nReadTempValue = 0;
	nReadTempValue = inirw.read_profile_int(pSection, pKey, -1);
	if (nReadTempValue < 0)
	{
		return false;
	} 
	if (nReadTempValue > 0)
	{
		m_unMaxClientServ = (unsigned int)nReadTempValue;
	}

	return SetServerID(eServerType, usAreaID, usGroupID, usServerID);
}

LLSServerManager* LLoginServerMainLogic::GetConnectToServerManager()
{
	return &m_LSServerManager;
}
LLoginServerConnectToServerNetWork* LLoginServerMainLogic::GetLoginServerConToServerNetWork()
{
	return &m_LSConToServerNetWork;
}


void LLoginServerMainLogic::AddSessionIDToKickOutQueue(uint64_t u64SessionID, unsigned int unTimeToKeep)
{
	time_t tNow 		= time(NULL);
	time_t tKickOutTime = tNow + unTimeToKeep;

	t_Session_Kick_Out tsko;
	tsko.u64SessionID 	= u64SessionID;
	tsko.tTimeToKickOut = tKickOutTime;

	m_mapSessionIDToKickOut[u64SessionID] = tsko;
}
