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

#include "LLBServerManager.h"
#include "LLobbyServerMainLogic.h"

LLBServerManager::LLBServerManager()
{
	m_pLBMainLogic = NULL;
}

LLBServerManager::~LLBServerManager()
{
}

bool LLBServerManager::Initialize(int nMaxConnectWorkQueueSize, int nNetWorkModelType, LSelectServer* pss, LNetWorkServices* pes)
{
	if (m_pLBMainLogic == NULL)
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
bool LLBServerManager::AddNewServer(uint64_t n64ServerSessionID, char* pExtData, unsigned short usExtDataLen, int nRecvThreadID, int nSendThreadID, char* pszIp, unsigned short usPort)
{
	uint64_t u64UpServerUniqueID = 0;
	memcpy(&u64UpServerUniqueID, pExtData, sizeof(u64UpServerUniqueID));
	LServerID serverID;
	if (!serverID.SetServerID(u64UpServerUniqueID))
	{
		return false;
	}
	if (FindServerByServerID(u64UpServerUniqueID) != NULL)
	{
		return false;
	}
	if (FindServerBySessionID(n64ServerSessionID) != NULL)
	{
		return false;
	}
	LLBServer* pNewServer = new LLBServer;
	if (pNewServer == NULL)
	{
		return false;
	}
	pNewServer->SetServerID(u64UpServerUniqueID);
	pNewServer->SetSessionID(n64ServerSessionID);
	pNewServer->SetRecvThreadID(nRecvThreadID);
	pNewServer->SetSendThreadID(nSendThreadID);
	pNewServer->SetServerIp(pszIp, strlen(pszIp));
	pNewServer->SetServerPort(usPort);

	m_mapSessionIDToServer[n64ServerSessionID] = pNewServer;
	m_mapServerIDToServer[u64UpServerUniqueID] = pNewServer;
	return true;
}

//	根据ServerID查找服务器
LLBServer* LLBServerManager::FindServerByServerID(uint64_t n64ServerID)
{
	map<uint64_t, LLBServer*>::iterator _ito = m_mapServerIDToServer.find(n64ServerID);
	if (_ito == m_mapServerIDToServer.end())
	{
		return NULL;
	}
	return _ito->second; 
}

void LLBServerManager::RemoveServerByServerID(uint64_t n64ServerID)
{
	map<uint64_t, LLBServer*>::iterator _ito = m_mapServerIDToServer.find(n64ServerID);
	if (_ito == m_mapServerIDToServer.end())
	{
		return ;
	}
	
	LLBServer* pServer = _ito->second;
	uint64_t u64SessionID = pServer->GetSessionID();
	m_mapServerIDToServer.erase(_ito);

	map<uint64_t, LLBServer*>::iterator _ito1 = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito1 != m_mapSessionIDToServer.end())
	{
		m_mapSessionIDToServer.erase(_ito1);
	}

	delete pServer; 
}
bool LLBServerManager::ExistServerByServerID(uint64_t n64ServerID)
{
	map<uint64_t, LLBServer*>::iterator _ito = m_mapServerIDToServer.find(n64ServerID);
	if (_ito == m_mapServerIDToServer.end())
	{
		return false;
	}
	return true; 
}

//	根据SessionID查找服务器信息
LLBServer* LLBServerManager::FindServerBySessionID(uint64_t n64SessionID)
{
	map<uint64_t, LLBServer*>::iterator _ito = m_mapSessionIDToServer.find(n64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return NULL;
	}
	return _ito->second; 
}
void LLBServerManager::RemoveServerBySessionID(uint64_t n64SessionID)
{
	map<uint64_t, LLBServer*>::iterator _ito = m_mapSessionIDToServer.find(n64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return;
	}
	LLBServer* pServer = _ito->second;
	m_mapSessionIDToServer.erase(_ito);

	uint64_t u64ServerID = pServer->GetServerUniqueID();

	map<uint64_t, LLBServer*>::iterator _ito1 = m_mapServerIDToServer.find(u64ServerID);
	if (_ito1 != m_mapServerIDToServer.end())
	{
		m_mapServerIDToServer.erase(_ito1);
	}

	delete pServer; 
}

//	停止所有的连接线程
bool LLBServerManager::StopAllConnectThread()
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
void LLBServerManager::ReleaseServerManagerResource()
{
}

//	连接任务
bool LLBServerManager::AddConnectWork(char* pszIP, unsigned short usPort, char* pExtData, unsigned short usExtDataLen)
{
	if (!m_ConnectorWorkManager.AddConnectWork(pszIP, usPort, pExtData, usExtDataLen))
	{
		return false;
	}
	return true; 
}

void LLBServerManager::SetLobbyServerMainLogic(LLobbyServerMainLogic* plbsml)
{
	m_pLBMainLogic = plbsml;
}
LLobbyServerMainLogic* LLBServerManager::GetLobbyServerMainLogic()
{
	return m_pLBMainLogic;
}

