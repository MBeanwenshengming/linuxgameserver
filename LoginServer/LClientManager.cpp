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

#include "LClientManager.h"
#include "LClient.h"
#include "LLoginServerMainLogic.h"

LClientManager::~LClientManager()
{
	m_pLSMainLogic = NULL;
}
LClientManager::LClientManager()
{
}
//	初始化连接
bool LClientManager::Initialize(unsigned int unMaxClientServ)
{
	if (m_pLSMainLogic == NULL)
	{
		return false;
	}
	if (unMaxClientServ == 0)
	{
		unMaxClientServ = 1000;
	}
	for (unsigned int ui = 0; ui < unMaxClientServ; ++ui)
	{
		LClient *pClient = new LClient;
		if (pClient == NULL)
		{
			return false;
		}
		m_queueClientPool.push(pClient);
	}
	return true;
}
//	网络层SessionID，tsa连接信息
bool LClientManager::AddNewUpSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	map<uint64_t, LClient*>::iterator _ito = m_mapClientManagerBySessionID.find(u64SessionID);
	if (_ito != m_mapClientManagerBySessionID.end())
	{
		return false;
	}

	LClient* pClient = GetOneClientFromPool();
	if (pClient == NULL)
	{
		return false;
	}
	pClient->SetClientInfo(u64SessionID, tsa);


	m_mapClientManagerBySessionID[u64SessionID] = pClient;
	return true;
}

void LClientManager::RemoveOneSession(uint64_t u64SessionID)
{
	map<uint64_t, LClient*>::iterator _ito = m_mapClientManagerBySessionID.find(u64SessionID);
	if (_ito != m_mapClientManagerBySessionID.end())
	{
		FreeOneClientToPool(_ito->second);
		m_mapClientManagerBySessionID.erase(_ito);
	} 
}

LClient* LClientManager::GetOneClientFromPool()
{
	if (m_queueClientPool.empty())
	{
		return NULL;
	}
	LClient* pClient = m_queueClientPool.front();
	m_queueClientPool.pop();
	return pClient;
}
void LClientManager::FreeOneClientToPool(LClient* pClient)
{
	if (pClient == NULL)
	{
		return ;
	}
	m_queueClientPool.push(pClient);
}

LClient* LClientManager::FindClientBySessionID(uint64_t u64SessionID)
{ 
	map<uint64_t, LClient*>::iterator _ito = m_mapClientManagerBySessionID.find(u64SessionID);
	if (_ito == m_mapClientManagerBySessionID.end())
	{
		return NULL;
	}
	return _ito->second;
}

void LClientManager::SetLSMainLogic(LLoginServerMainLogic* plsml)
{
	m_pLSMainLogic = plsml;
}
LLoginServerMainLogic* LClientManager::GetLSMainLogic()
{
	return m_pLSMainLogic;
}


unsigned int LClientManager::GetClientCount()
{
	return m_mapClientManagerBySessionID.size();
}

