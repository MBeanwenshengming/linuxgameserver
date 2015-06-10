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

#include "LGSServerManager.h"
#include "LGSServer.h"

LGSServerManager::LGSServerManager()
{
	m_pGSMainLogic = NULL;
}
LGSServerManager::~LGSServerManager()
{
}
//	初始化连接线程
bool LGSServerManager::Initialize(int nMaxConnectWorkQueueSize, int nNetWorkModelType, LSelectServer* pss, LNetWorkServices* pes)
{
	if (m_pGSMainLogic == NULL)
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

bool LGSServerManager::AddNewServer(uint64_t n64ServerSessionID, char* pExtData, unsigned short usExtDataLen, int nRecvThreadID, int nSendThreadID, char* pszIp, unsigned short usPort)
{
	if (pExtData == NULL || pszIp == NULL)
	{
		return false;
	}
	if (FindGSServerBySessionID(n64ServerSessionID))
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
	if (FineGSServerByServerID(u64UpServerID))
	{
		return false;
	}
	LGSServer* pServer = new LGSServer;
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

LGSServer* LGSServerManager::FindGSServerBySessionID(uint64_t u64SessionID)
{
	map<uint64_t, LGSServer*>::iterator _ito = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return NULL;
	}
	return _ito->second;
}
LGSServer* LGSServerManager::FineGSServerByServerID(int64_t u64ServerID)
{
	map<uint64_t, LGSServer*>::iterator _ito = m_mapServerIDToServer.find(u64ServerID);
	if (_ito == m_mapServerIDToServer.end())
	{
		return NULL;
	}
	return _ito->second;
}

void LGSServerManager::RemoveGSServerBySessionID(uint64_t u64SessionID)
{
	map<uint64_t, LGSServer*>::iterator _ito = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return;
	}
	LGSServer* pServer = _ito->second;
	m_mapSessionIDToServer.erase(_ito);

	uint64_t u64ServerID = pServer->GetServerUniqueID();

	map<uint64_t, LGSServer*>::iterator _ito1 = m_mapServerIDToServer.find(u64ServerID);
	if (_ito1 != m_mapServerIDToServer.end())
	{
		m_mapServerIDToServer.erase(_ito1);
	}

	delete pServer;
}
void LGSServerManager::RemoveGSServerByServerID(uint64_t u64ServerID)
{
	map<uint64_t, LGSServer*>::iterator _ito = m_mapServerIDToServer.find(u64ServerID);
	if (_ito == m_mapServerIDToServer.end())
	{
		return ;
	}
	
	LGSServer* pServer = _ito->second;
	uint64_t u64SessionID = pServer->GetSessionID();
	m_mapServerIDToServer.erase(_ito);

	map<uint64_t, LGSServer*>::iterator _ito1 = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito1 != m_mapSessionIDToServer.end())
	{
		m_mapSessionIDToServer.erase(_ito1);
	}

	delete pServer;
}



void LGSServerManager::SetGameServerMainLogic(LGameServerMainLogic* pgsml)
{
	m_pGSMainLogic = pgsml;
}
LGameServerMainLogic* LGSServerManager::GetGameServerMainLogic()
{
	return m_pGSMainLogic;
}

//	连接任务
bool LGSServerManager::AddConnectWork(char* pszIP, unsigned short usPort, char* pExtData, unsigned short usExtDataLen)
{
	if (!m_ConnectorWorkManager.AddConnectWork(pszIP, usPort, pExtData, usExtDataLen))
	{
		return false;
	}
	return true;
}
bool LGSServerManager::StopGSServerManagerConnectThread()
{
	m_ConnectorWorkManager.Stop();
	pthread_t pID = m_ConnectorWorkManager.GetThreadHandle();
	if (pID != 0)
	{
		pthread_join(pID, NULL);
	}
	return true;
}
void LGSServerManager::ReleaseGSServerManagerConnectThreadResource()
{
}
