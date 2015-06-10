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

LUserManager::LUserManager()
{
}
LUserManager::~LUserManager()
{
}
bool LUserManager::AddNewUserInfo(LUser* pUser)
{
	if (pUser == NULL)
	{
		return false;
	}
	uint64_t u64UserUniqueIDInDB = pUser->GetUserUniqueIDInDB();
	if (IsExistUserInfo(u64UserUniqueIDInDB))
	{
		return false;
	}
	m_mapUniqueIDInServerToUser[u64UserUniqueIDInDB] = pUser;
	return true;
}

bool LUserManager::IsExistUserInfo(uint64_t u64UserUniqueIDInDB)
{
	map<uint64_t, LUser*>::iterator _ito = m_mapUniqueIDInServerToUser.find(u64UserUniqueIDInDB);
	if (_ito == m_mapUniqueIDInServerToUser.end())
	{
		return false;
	}
	else
	{
		return true;
	}

}

LUser* LUserManager::FindUser(uint64_t u64UserUniqueIDInDB)
{
	map<uint64_t, LUser*>::iterator _ito = m_mapUniqueIDInServerToUser.find(u64UserUniqueIDInDB);
	if (_ito == m_mapUniqueIDInServerToUser.end())
	{
		return NULL;
	}
	return _ito->second;
}

void LUserManager::RemoveUser(uint64_t u64UserUniqueIDInDB)
{
	map<uint64_t, LUser*>::iterator _ito = m_mapUniqueIDInServerToUser.find(u64UserUniqueIDInDB);
	if (_ito == m_mapUniqueIDInServerToUser.end())
	{
		return ;
	}
	LUser* pUser = _ito->second;
	m_mapUniqueIDInServerToUser.erase(_ito);
	FreeOneUser(pUser);
}

bool LUserManager::InitializeUserPool(unsigned int unPoolSize)
{
	if (unPoolSize == 0)
	{
		unPoolSize = 100;
	}
	for (unsigned int unIndex = 0; unIndex < unPoolSize; ++unIndex)
	{
		LUser* pUser = new LUser;
		if (pUser == NULL)
		{
			return false;
		}
		m_pUserPool.push(pUser);
	}
	return true;
}

LUser* LUserManager::AllocOneUser()
{
	LUser* pUser  = NULL;
	if (m_pUserPool.empty())
	{
		pUser = new LUser; 
	}
	else
	{
		pUser = m_pUserPool.front();
		m_pUserPool.pop();
	}
	return pUser;
}

void LUserManager::FreeOneUser(LUser* pUser)
{
	if (pUser != NULL)
	{
		pUser->Reset();
		m_pUserPool.push(pUser);
	}
}

bool LUserManager::UpdateUserState(uint64_t u64UserUniqueIDInDB, E_User_State eUserState)
{
	LUser* pUser = FindUser(u64UserUniqueIDInDB);
	if (pUser == NULL)
	{
		return false;
	}
	E_User_State eOldUserState = pUser->GetUserState();
	if (E_User_Login_Waiting_GateServer_Reponse == eUserState)
	{
		if (eOldUserState != E_User_Created)
		{
			return false;
		}
		pUser->SetUserState(E_User_Login_Waiting_GateServer_Reponse);
		return true;
	}
	return false;
}
