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

#include "LLBGateServerManager.h"
#include "LLBGateServer.h"

LLBGateServerManager::LLBGateServerManager()
{
}
LLBGateServerManager::~LLBGateServerManager()
{
}
bool LLBGateServerManager::Initialize()
{
	return true;
}
bool LLBGateServerManager::AddNewUpServer(uint64_t uSessionID)
{ 
	if (uSessionID == 0)
	{
		return false;
	}

	map<uint64_t, LLBGateServer*>::iterator _ito = m_mapSessionIDToGateServer.find(uSessionID);
	if (_ito != m_mapSessionIDToGateServer.end())
	{
		return false;
	}
	LLBGateServer* pGateServer = new LLBGateServer;
	if (pGateServer == NULL)
	{
		return false;
	}
	pGateServer->SetSessionID(uSessionID);
	pGateServer->SetServerState(E_GateServer_NewUp);
	m_mapSessionIDToGateServer[uSessionID] = pGateServer;
	return true;
}

bool LLBGateServerManager::UpdateServerID(uint64_t uSessionID, uint64_t uServerID)
{ 
	map<uint64_t, LLBGateServer*>::iterator _ito = m_mapSessionIDToGateServer.find(uSessionID);
	if (_ito == m_mapSessionIDToGateServer.end())
	{
		return false;
	}

	map<uint64_t, LLBGateServer*>::iterator _itoForServerID = m_mapServerIDToGateServer.find(uServerID);
	if (_itoForServerID != m_mapServerIDToGateServer.end())
	{
		return true;
	}

	LLBGateServer* pGateServer = _ito->second;
	pGateServer->SetServerID(uServerID);
	pGateServer->SetServerState(E_GateServer_ServerID_Set);

	m_mapServerIDToGateServer[uServerID] = pGateServer;

	return true;
}

LLBGateServer* LLBGateServerManager::FindGateServerBySessionID(uint64_t u64SessionID)
{ 
	map<uint64_t, LLBGateServer*>::iterator _ito = m_mapSessionIDToGateServer.find(u64SessionID);
	if (_ito == m_mapSessionIDToGateServer.end())
	{
		return NULL;
	}
	return _ito->second;
}

LLBGateServer* LLBGateServerManager::FindGateServerByServerID(uint64_t u64ServerID)
{
	map<uint64_t, LLBGateServer*>::iterator _ito = m_mapServerIDToGateServer.find(u64ServerID);
	if (_ito == m_mapServerIDToGateServer.end())
	{
		return NULL;
	}
	return _ito->second;
}

void LLBGateServerManager::RemoveGateServerBySessionID(uint64_t u64SessionID)
{
	map<uint64_t, LLBGateServer*>::iterator _ito = m_mapSessionIDToGateServer.find(u64SessionID);
	if (_ito == m_mapSessionIDToGateServer.end())
	{
		return ;
	}

	LLBGateServer* pGateServer = _ito->second;
	uint64_t u64ServerID = pGateServer->GetServerUniqueID();
	
	//	从SessionID移除
	m_mapSessionIDToGateServer.erase(_ito);

	//	从ServerID移除
	if (u64ServerID == 0)
	{
		delete pGateServer;
		return ;
	}

	map<uint64_t, LLBGateServer*>::iterator _ito1 = m_mapServerIDToGateServer.find(u64ServerID);
	if (_ito1 == m_mapServerIDToGateServer.end())
	{
		delete pGateServer;
		return ;
	}
	m_mapServerIDToGateServer.erase(_ito1);

	delete pGateServer;
}

void LLBGateServerManager::RemoveGateServerByServerID(uint64_t u64ServerID)
{
	map<uint64_t, LLBGateServer*>::iterator _ito1 = m_mapServerIDToGateServer.find(u64ServerID);
	if (_ito1 == m_mapServerIDToGateServer.end())
	{
		return ;
	}

	LLBGateServer* pGateServer = _ito1->second;
	uint64_t u64SessionID = pGateServer->GetSessionID();

	m_mapServerIDToGateServer.erase(_ito1);


	map<uint64_t, LLBGateServer*>::iterator _ito = m_mapSessionIDToGateServer.find(u64SessionID);
	if (_ito == m_mapSessionIDToGateServer.end())
	{
		delete pGateServer;
		return ;
	}
	m_mapSessionIDToGateServer.erase(_ito);

	delete pGateServer;
}

void LLBGateServerManager::AddWillConnectServer(uint64_t u64ServerID)
{ 
	map<uint64_t, t_Server_Will_Connect>::iterator _ito = m_mapWillConnectServer.find(u64ServerID);
	if (_ito != m_mapWillConnectServer.end())
	{
		_ito->second.unReqStartTime = time(NULL);
	}
	else
	{
		t_Server_Will_Connect tswc;
		tswc.u64ServerID 	= u64ServerID;
		tswc.unReqStartTime = time(NULL);
		m_mapWillConnectServer[u64ServerID] = tswc;
	}
}
bool LLBGateServerManager::ExistWillConnectServer(uint64_t u64ServerID)
{ 
	map<uint64_t, t_Server_Will_Connect>::iterator _ito = m_mapWillConnectServer.find(u64ServerID);
	if (_ito != m_mapWillConnectServer.end())
	{
		return true;
	}
	return false;
}
void LLBGateServerManager::RemoveWillConnectServer(uint64_t u64ServerID)
{
	map<uint64_t, t_Server_Will_Connect>::iterator _ito = m_mapWillConnectServer.find(u64ServerID);
	if (_ito != m_mapWillConnectServer.end())
	{
		m_mapWillConnectServer.erase(_ito);
	}
}
