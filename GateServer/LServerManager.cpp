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

#include "LServerManager.h"
#include "LGateServerMainLogic.h"
#include "LServer.h"

LServerManager::LServerManager()
{
	m_pGateServerMainLogic = NULL;
}

LServerManager::~LServerManager()
{
}

bool LServerManager::Initialize(int nMaxConnectWorkQueueSize, int nNetWorkModelType, LSelectServer* pss, LNetWorkServices* pes)
{
	if (m_pGateServerMainLogic == NULL)
	{
		return false;
	}

	if (!m_ConnectorWorkManager.Initialize(nMaxConnectWorkQueueSize, nNetWorkModelType, pss, pes))
	{
		return false;
	}
	if (!m_ConnectorWorkManager.Start())
	{
		return false;
	}
	return true;
}

//	添加，查找，删除服务器
bool LServerManager::AddNewServer(uint64_t n64ServerSessionID, char* pExtData, unsigned short usExtDataLen, int nRecvThreadID, int nSendThreadID, char* pszIp, unsigned short usPort)
{
	if (pExtData == NULL || pszIp == NULL)
	{
		return false;
	}
	if (FindServerBySessionID(n64ServerSessionID))
	{
		return false;
	}
	LServerID serverID;
	uint64_t u64UpServerID = 0;
	memcpy(&u64UpServerID, pExtData, sizeof(u64UpServerID));
	if (!serverID.SetServerID(u64UpServerID))
	{
		return false;
	}
	if (FindServerByServerID(u64UpServerID))
	{
		return false;
	}
	LServer* pServer = new LServer;
	pServer->SetSessionID(n64ServerSessionID);
	pServer->SetRecvThreadID(nRecvThreadID);
	pServer->SetSendThreadID(nSendThreadID);
	pServer->SetServerID(u64UpServerID);
	pServer->SetServerIp(pszIp, strlen(pszIp));
	pServer->SetServerPort(usPort);

	m_mapServerIDToServer[u64UpServerID] = pServer;
	m_mapSessionIDToServer[n64ServerSessionID] = pServer;
	return true;
}

LServer* LServerManager::FindServerByServerID(uint64_t n64ServerID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapServerIDToServer.find(n64ServerID);
	if (_ito == m_mapServerIDToServer.end())
	{
		return NULL;
	}
	return _ito->second;
}

void LServerManager::RemoveServerByServerID(uint64_t n64ServerID)
{
	map<uint64_t, LServer*>::iterator _ito = m_mapServerIDToServer.find(n64ServerID);
	if (_ito == m_mapServerIDToServer.end())
	{
		return ;
	}
	
	LServer* pServer = _ito->second;
	uint64_t u64SessionID = pServer->GetSessionID();
	m_mapServerIDToServer.erase(_ito);

	map<uint64_t, LServer*>::iterator _ito1 = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito1 != m_mapSessionIDToServer.end())
	{
		m_mapSessionIDToServer.erase(_ito1);
	}

	delete pServer;
} 

bool LServerManager::ExistServerByServerID(uint64_t n64ServerID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapServerIDToServer.find(n64ServerID);
	if (_ito == m_mapServerIDToServer.end())
	{
		return false;
	}
	return true;
}

//	根据SessionID查找服务器信息
LServer* LServerManager::FindServerBySessionID(uint64_t n64SessionID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapSessionIDToServer.find(n64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return NULL;
	}
	return _ito->second;
}

void LServerManager::RemoveServerBySessionID(uint64_t n64SessionID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapSessionIDToServer.find(n64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return;
	}
	LServer* pServer = _ito->second;
	m_mapSessionIDToServer.erase(_ito);

	uint64_t u64ServerID = pServer->GetServerUniqueID();

	map<uint64_t, LServer*>::iterator _ito1 = m_mapServerIDToServer.find(u64ServerID);
	if (_ito1 != m_mapServerIDToServer.end())
	{
		m_mapServerIDToServer.erase(_ito1);
	}

	delete pServer;
}
//	停止所有的连接线程
bool LServerManager::StopAllConnectThread()
{
	m_ConnectorWorkManager.Stop();
	pthread_t pID = m_ConnectorWorkManager.GetThreadHandle();
	if (pID != 0)
	{
		pthread_join(pID, NULL);
	}
	return true;
}

//	释放服务器管理器资源
void LServerManager::ReleaseServerManagerResource()
{
}

//	连接任务
bool LServerManager::AddConnectWork(char* pszIP, unsigned short usPort, char* pExtData, unsigned short usExtDataLen)
{
	if (!m_ConnectorWorkManager.AddConnectWork(pszIP, usPort, pExtData, usExtDataLen))
	{
		return false;
	}
	return true;
}


LGateServerMainLogic* LServerManager::GetGateServerMainLogic()
{
	return m_pGateServerMainLogic;
}

void LServerManager::SetGateServerMainLogic(LGateServerMainLogic* pgsml)
{
	m_pGateServerMainLogic = pgsml;
}



