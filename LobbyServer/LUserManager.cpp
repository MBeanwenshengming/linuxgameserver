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

#include "LUserManager.h"
#include "LUser.h"

LUserManager::LUserManager()
{
}
LUserManager::~LUserManager()
{
}
bool LUserManager::AddUser(LUser* pUser)
{ 
	if (pUser == NULL)
	{
		return false;
	}
	uint64_t u64UserUniqueIDInDB = pUser->GetUniqueUserIDInDB();
	map<uint64_t, LUser*>::iterator _ito = m_mapUserUniqueIDInDBToUser.find(u64UserUniqueIDInDB);
	if (_ito != m_mapUserUniqueIDInDBToUser.end())
	{
		return false;
	}
	m_mapUserUniqueIDInDBToUser[u64UserUniqueIDInDB] = pUser;
	return true;
}
LUser* LUserManager::FineUser(uint64_t u64UniqueUserIDInDB)
{
	map<uint64_t, LUser*>::iterator _ito = m_mapUserUniqueIDInDBToUser.find(u64UniqueUserIDInDB);
	if (_ito != m_mapUserUniqueIDInDBToUser.end())
	{
		return _ito->second;
	}
	return NULL;
}
void LUserManager::RemoveUser(uint64_t u64UniqueUserIDInDB)
{
	map<uint64_t, LUser*>::iterator _ito = m_mapUserUniqueIDInDBToUser.find(u64UniqueUserIDInDB);
	if (_ito == m_mapUserUniqueIDInDBToUser.end())
	{
		return ;
	}
	LUser* pUser = _ito->second;
	m_mapUserUniqueIDInDBToUser.erase(_ito);
	FreeUserToPool(pUser);
}




bool LUserManager::InitializeUserPool(unsigned int unUserPoolSize)
{
	if (unUserPoolSize == 0)
	{
		unUserPoolSize = 100;
	}
	for (unsigned int unIndex = 0; unIndex < unUserPoolSize; ++unIndex)
	{
		LUser* pUser = new LUser;
		if (pUser == NULL)
		{
			return false;
		}
		m_queueUserPool.push(pUser);
	}
	return true;
}
LUser* LUserManager::AllocUserFromPool()
{
	if (m_queueUserPool.empty())
	{
		return NULL;
	}
	LUser* pUser = m_queueUserPool.front();
	m_queueUserPool.pop();
	return pUser;
}
void LUserManager::FreeUserToPool(LUser* pUser)
{
	if (pUser != NULL)
	{
		pUser->Reset();
		m_queueUserPool.push(pUser);
	}
}



bool LUserManager::AddNewWillConnectUser(uint64_t u64UniqueUserIDInDB)
{ 
	map<uint64_t, uint64_t>::iterator _ito = m_mapWillConnectUser.find(u64UniqueUserIDInDB);
	if (_ito != m_mapWillConnectUser.end())
	{
		return true;
	}
	t_Will_Connect_Info twci;
	twci.uUserUniqueIDInDB = u64UniqueUserIDInDB;
	twci.tKickOutTime = time(NULL) + 5;
	m_queueKickOutConUser.push(twci);
	m_mapWillConnectUser[u64UniqueUserIDInDB] = u64UniqueUserIDInDB;
	return true;
}
bool LUserManager::ExistsWillConnectUser(uint64_t u64UniqueUserIDInDB)
{ 
	map<uint64_t, uint64_t>::iterator _ito = m_mapWillConnectUser.find(u64UniqueUserIDInDB);
	if (_ito != m_mapWillConnectUser.end())
	{
		m_mapWillConnectUser.erase(_ito);
		return true;
	}
	return false;
}
void LUserManager::ProcessWillConnectUser()
{
	unsigned int unProcessCount = 0;
	time_t tNow = time_t(NULL);
	while (1)
	{
		if (m_queueKickOutConUser.empty())
		{
			return ;
		}
		unProcessCount++;
		if (unProcessCount > 100)
		{
			return ;
		}
		t_Will_Connect_Info twci = m_queueKickOutConUser.front();
		if (twci.tKickOutTime > tNow)
		{ 
			map<uint64_t, uint64_t>::iterator _ito = m_mapWillConnectUser.find(twci.uUserUniqueIDInDB);
			if (_ito != m_mapWillConnectUser.end())
			{
				m_mapWillConnectUser.erase(_ito);
				m_queueKickOutConUser.pop(); 
			} 
		}
		else
		{
			return ;
		}
	}
}


unsigned int LUserManager::GetServeCount()
{
	return m_mapUserUniqueIDInDBToUser.size();
}

