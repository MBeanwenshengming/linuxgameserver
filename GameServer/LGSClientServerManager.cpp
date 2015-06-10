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

#include "LGSClientServerManager.h"
#include "LGameServerMainLogic.h"
#include "LGSClientServer.h"


LGSClientServerManager::LGSClientServerManager()
{
	m_pGSMainLogic = NULL;
}
LGSClientServerManager::~LGSClientServerManager()
{
}
bool LGSClientServerManager::AddGSNewUpServer(uint64_t u64SessionID, int nRecvThreadID, int nSendThreadID, char* pszIp, unsigned short usPort)
{ 
	if (pszIp == NULL || u64SessionID == 0)
	{
		return false;
	}
	map<uint64_t, LGSClientServer*>::iterator _ito = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito != m_mapSessionIDToServer.end())
	{
		return false;
	}

	LGSClientServer* pGSClientServer = new LGSClientServer;
	if (pGSClientServer == NULL)
	{
		return false;
	}
	pGSClientServer->SetSessionID(u64SessionID);
	pGSClientServer->SetRecvThreadID(nRecvThreadID);
	pGSClientServer->SetSendThreadID(nSendThreadID);
	pGSClientServer->SetServerIp(pszIp, strlen(pszIp));
	pGSClientServer->SetServerPort(usPort);
	m_mapSessionIDToServer[u64SessionID] = pGSClientServer;
	return true;
}
bool LGSClientServerManager::UpdateServerIDForServer(uint64_t u64SessionID, uint64_t u64ServerID)
{ 
	LServerID serverID;
	if (!serverID.SetServerID(u64ServerID))
	{
		return false;
	}
	map<uint64_t, LGSClientServer*>::iterator _ito = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return false;
	}
	LGSClientServer* pGSClientServer = _ito->second;
	pGSClientServer->SetServerID(u64ServerID);
	m_mapServerIDToServer[u64ServerID] = pGSClientServer;
	return true;
}

LGSClientServer* LGSClientServerManager::FindGSClientServerBySessionID(uint64_t u64SessionID)
{ 
	map<uint64_t, LGSClientServer*>::iterator _ito = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return NULL;
	}
	return _ito->second;
}
LGSClientServer* LGSClientServerManager::FineGSClientServerByServerID(int64_t u64ServerID)
{
	map<uint64_t, LGSClientServer*>::iterator _ito = m_mapServerIDToServer.find(u64ServerID); 
	if (_ito == m_mapServerIDToServer.end())
	{
		return NULL;
	}
	return _ito->second;
}

void LGSClientServerManager::RemoveGSClientServerBySessionID(uint64_t u64SessionID)
{ 
	map<uint64_t, LGSClientServer*>::iterator _ito = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito == m_mapSessionIDToServer.end())
	{
		return ;
	}
	LGSClientServer* pGSClientServer = _ito->second;
	m_mapSessionIDToServer.erase(_ito);

	uint64_t u64ServerID = pGSClientServer->GetServerUniqueID();
	map<uint64_t, LGSClientServer*>::iterator _ito1 = m_mapServerIDToServer.find(u64ServerID); 
	if (_ito1 != m_mapServerIDToServer.end())
	{
		m_mapServerIDToServer.erase(_ito1);
	}
	delete pGSClientServer;
}

void LGSClientServerManager::RemoveGSClientServerByServerID(uint64_t u64ServerID)
{
	map<uint64_t, LGSClientServer*>::iterator _ito1 = m_mapServerIDToServer.find(u64ServerID); 
	if (_ito1 == m_mapServerIDToServer.end())
	{
		return ;
	}

	LGSClientServer* pGSClientServer = _ito1->second;
	m_mapServerIDToServer.erase(_ito1);
	
	uint64_t u64SessionID = pGSClientServer->GetSessionID();
	map<uint64_t, LGSClientServer*>::iterator _ito = m_mapSessionIDToServer.find(u64SessionID);
	if (_ito != m_mapSessionIDToServer.end())
	{
		m_mapSessionIDToServer.erase(_ito);
	}
	delete pGSClientServer;
}


void LGSClientServerManager::SetGameServerMainLogic(LGameServerMainLogic* pgsml)
{
	m_pGSMainLogic = pgsml;
}
LGameServerMainLogic* LGSClientServerManager::GetGameServerMainLogic()
{
	return m_pGSMainLogic;
}
