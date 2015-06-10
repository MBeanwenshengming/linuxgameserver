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

#include "LMainLogicThread.h"
#include "LServer.h"
#include "LServerManager.h"
#include "../NetWork/LIniFileReadAndWrite.h"
#include "../NetWork/LNetWorkServices.h"

LMainLogicThread::LMainLogicThread()
{
}

LMainLogicThread::~LMainLogicThread()
{
}

bool LMainLogicThread::Initialize(char* pServerConfigFileName, char* pNetWorkConfigFileName)
{
	LServerManager::GetServerManagerInstance()->SetMainLogicThread(this);
	//	注册处理函数
	m_PacketProcessProcManager.SetMainLogicThread(this);
	if (!m_PacketProcessProcManager.Initialize())
	{
		return false;
	}

	if (pServerConfigFileName == NULL || pNetWorkConfigFileName == NULL)
	{
		return false;
	}
	if (!m_ConfigInfo.ReadConfig(pServerConfigFileName))
	{
		return false;
	}
	if (!InitializeNetWork(pNetWorkConfigFileName, 500, "MasterServer"))
	{
		return false;
	}
	if (!NetWorkStart())
	{
		return false;
	}
	return true;
}


int LMainLogicThread::ThreadDoing(void* pParam)
{
	while (1)
	{
		ProcessRecvedPacket();

		unsigned int unPacketProcessed = GetPacketProcessed();
		if (unPacketProcessed == 0)
		{
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}

		if (CheckForStop())
		{
			break;
		} 
		//	删除没有发送数据的死连接
		GetNetWorkServices()->KickOutIdleSession();
	}
	return 0;
}

bool LMainLogicThread::OnStart()
{
	return true;
}

void LMainLogicThread::OnStop()
{
}

//	网络虚函数 
bool LMainLogicThread::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	return LServerManager::GetServerManagerInstance()->AddNewUpServer(u64SessionID, tsa);
}

void LMainLogicThread::OnRemoveSession(uint64_t u64SessionID)
{
	LServerManager::GetServerManagerInstance()->RemoveServerByNetWorkSessionID(u64SessionID);
}

void LMainLogicThread::StopMainLogicThread()
{
	Stop();
	pthread_t pID = GetThreadHandle();
	if (pID != 0)
	{ 
		pthread_join(pID, NULL);
	}
	//	主线程停止之后，才停止网络线程
	//	停掉所有的网络相关的线程
	NetWorkDown(); 
}

//	处理接收到的协议信息
void LMainLogicThread::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	//	将数据包分发的对应线程处理
	m_PacketProcessProcManager.DispatchMessageProcess(tRecvedPacket.u64SessionID, tRecvedPacket.pPacket);
}


LMainLogicThreadReadConfig::LMainLogicThreadReadConfig()
{
	m_ucServerType 			= 0;
	m_usAreaID 				= 0;
	m_usGroupID 			= 0;
	m_usServerID 			= 0;
	m_unMaxClientServered 	= 0;	//最大服务数量
}
LMainLogicThreadReadConfig::~LMainLogicThreadReadConfig()
{
}
bool LMainLogicThreadReadConfig::ReadConfig(char* pConfigFileName)
{
	LIniFileReadAndWrite iniFileReadAndWrite;
	bool bOpenSuccess = iniFileReadAndWrite.OpenIniFile(pConfigFileName);
	if (!bOpenSuccess)
	{
		return false;
	}
	char* pSection = "MasterServer";

	char* pKey = "ServerType";
	int nReadValue = iniFileReadAndWrite.read_profile_int(pSection, pKey, 0);
	if (nReadValue <= 0)
	{
		return false;
	}
	if (nReadValue <= (int)E_Server_Type_Invalid || nReadValue >= (int)E_Server_Type_Max)
	{
		return false;
	}
	m_ucServerType = (unsigned char) nReadValue;

	pKey = "AreaID"; 
	nReadValue = iniFileReadAndWrite.read_profile_int(pSection, pKey, 0);
	if (nReadValue <= 0)
	{
		return false;
	}
	if (nReadValue >= 0xFFFF)
	{
		return false;
	}
	m_usAreaID = (unsigned short)nReadValue;

	pKey = "GroupID"; 
	nReadValue = iniFileReadAndWrite.read_profile_int(pSection, pKey, 0);
	if (nReadValue <= 0)
	{
		return false;
	} 
	if (nReadValue >= 0xFFFF)
	{
		return false;
	}
	m_usGroupID = (unsigned short)nReadValue;

	pKey = "ServerID"; 
	nReadValue = iniFileReadAndWrite.read_profile_int(pSection, pKey, 0);
	if (nReadValue <= 0)
	{
		return false;
	}

	if (nReadValue >= 0xFFFF)
	{
		return false;
	}
	m_usServerID = (unsigned short)nReadValue;

	pKey = "MaxClientServered"; 
	nReadValue = iniFileReadAndWrite.read_profile_int(pSection, pKey, 0);
	if (nReadValue <= 0)
	{
		return false;
	} 
	m_unMaxClientServered = (unsigned int)nReadValue;

	return true;
}

void LMainLogicThreadReadConfig::Reset()
{
	m_ucServerType 			= 0;
	m_usAreaID 				= 0;
	m_usGroupID 			= 0;
	m_usServerID 			= 0;
	m_unMaxClientServered 	= 0;	//最大服务数量
}



