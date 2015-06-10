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
#include "LServer.h"

LServerManager::LServerManager()
{
}

LServerManager::~LServerManager()
{
}

bool LServerManager::NewUpServer(uint64_t u64SessionID, int nSendThreadID, int nRecvThreadID, char* pszIp, unsigned short usPort)
{ 
	if (u64SessionID == 0)
	{
		return false;
	}
	map<uint64_t, LServer*>::iterator _ito = m_mapServerSessionToServer.find(u64SessionID);
	if (_ito != m_mapServerSessionToServer.end())
	{
		return false;
	}
	LServer* pServer = new LServer;
	if (pServer == NULL)
	{
		return false;
	}
	pServer->SetSessionID(u64SessionID);
	pServer->SetSendThreadID(nSendThreadID);
	pServer->SetRecvThreadID(nRecvThreadID);
	pServer->SetServerIp(pszIp, strlen(pszIp));
	pServer->SetServerPort(usPort);
	m_mapServerSessionToServer[u64SessionID] = pServer;
	return true;
}

bool LServerManager::SetServerID(uint64_t u64SessionID, uint64_t u64ServerID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapServerSessionToServer.find(u64SessionID);
	if (_ito == m_mapServerSessionToServer.end())
	{
		return false;
	}
	LServer* pServer = _ito->second;
	if (!pServer->SetServerID(u64ServerID))
	{
		return false;
	}
	
	map<uint64_t, LServer*>::iterator _itoServerID = m_mapServerIDToServer.find(u64ServerID);
	if (_itoServerID != m_mapServerIDToServer.end())
	{
		return false;
	}
	m_mapServerIDToServer[u64ServerID] = pServer;
	return true;
}

LServer* LServerManager::FindServerBySessionID(uint64_t u64SessionID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapServerSessionToServer.find(u64SessionID);
	if (_ito == m_mapServerSessionToServer.end())
	{
		return NULL;
	}
	return _ito->second;
}

LServer* LServerManager::FindServerByServerID(uint64_t u64ServerID)
{
	map<uint64_t, LServer*>::iterator _itoServerID = m_mapServerIDToServer.find(u64ServerID);
	if (_itoServerID == m_mapServerIDToServer.end())
	{
		return NULL;
	}
	return _itoServerID->second;
} 

void LServerManager::RemoveServerBySessionID(uint64_t u64sessionID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapServerSessionToServer.find(u64sessionID);
	if (_ito == m_mapServerSessionToServer.end())
	{
		return ;
	}
	LServer* pServer = _ito->second;

	uint64_t u64ServerID = pServer->GetServerUniqueID();
	m_mapServerSessionToServer.erase(_ito);

	map<uint64_t, LServer*>::iterator _itoServerID = m_mapServerIDToServer.find(u64ServerID);
	if (_itoServerID == m_mapServerIDToServer.end())
	{
		delete pServer;
		return ;
	}
	m_mapServerIDToServer.erase(_itoServerID);

	delete pServer;
}

void LServerManager::RemoveServerByServerID(uint64_t u64ServerID)
{ 
	map<uint64_t, LServer*>::iterator _itoServerID = m_mapServerIDToServer.find(u64ServerID);
	if (_itoServerID == m_mapServerIDToServer.end())
	{
		return ;
	} 

	LServer* pServer = _itoServerID->second;
	uint64_t u64SessionID = pServer->GetSessionID();

	m_mapServerIDToServer.erase(_itoServerID);


	map<uint64_t, LServer*>::iterator _ito = m_mapServerSessionToServer.find(u64SessionID);
	if (_ito == m_mapServerSessionToServer.end())
	{
		delete pServer;
		return ;
	}
	m_mapServerSessionToServer.erase(_ito);
	delete pServer;
}

bool LServerManager::AddWillConnectToServer(uint64_t u64ServerUniqueID)
{ 
	map<uint64_t, t_Will_Connect_To_Server>::iterator _ito = m_mapWillConnectToServer.find(u64ServerUniqueID);
	t_Will_Connect_To_Server twcts;
	twcts.u64ServerUniqueID = u64ServerUniqueID;
	twcts.tTimeStart = time(NULL);

	m_mapWillConnectToServer[u64ServerUniqueID] = twcts;
	return true;
}
	

