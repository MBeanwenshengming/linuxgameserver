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

#include "LUserInfoManager.h"
#include "LUserInfo.h"

LUserInfoManager::LUserInfoManager()
{
}
LUserInfoManager::~LUserInfoManager()
{
}
bool LUserInfoManager::Initialize(unsigned int unUserInfoPoolSize)
{
	if (unUserInfoPoolSize == 0)
	{
		unUserInfoPoolSize = 100;
	}
	for (unsigned int unIndex = 0; unIndex < unUserInfoPoolSize; ++unIndex)
	{
		LUserInfo* pUserInfo = new LUserInfo;
		if (pUserInfo == NULL)
		{
			return false;
		}
		pUserInfo->Reset();
		m_queueUserInfoPool.push(pUserInfo);
	}
	return true;
}
void LUserInfoManager::ReleaseResource()
{ 
	map<string, LUserInfo*>::iterator _ito = m_mapUserInfoManager.begin();
	while (_ito != m_mapUserInfoManager.end())
	{
		delete _ito->second;
		_ito++;
	}
	while (!m_queueUserInfoPool.empty())
	{
		LUserInfo* pTemp = m_queueUserInfoPool.front();
		m_queueUserInfoPool.pop();
		delete pTemp;
	}
}
bool LUserInfoManager::AddUserInfo(LUserInfo* pUserInfo)
{
	if (pUserInfo == NULL)
	{
		return false;
	}
	char szUserID[MAX_USER_ID_LEN + 1];
	memset(szUserID, 0, sizeof(szUserID));
	bool bSuccess = pUserInfo->GetUserID(szUserID, MAX_USER_ID_LEN);
	if (!bSuccess)
	{
		return false;
	}
	string sUserID = szUserID;

	map<string, LUserInfo*>::iterator _ito = m_mapUserInfoManager.find(sUserID);
	if (_ito != m_mapUserInfoManager.end())
	{
		return false;
	}
	m_mapUserInfoManager[sUserID] = pUserInfo;
	return true;
}
LUserInfo* LUserInfoManager::FindUserInfo(char* pszUserID)
{
	if (pszUserID == NULL)
	{
		return NULL;
	}
	string sUserID = pszUserID;

	map<string, LUserInfo*>::iterator _ito = m_mapUserInfoManager.find(sUserID);
	if (_ito != m_mapUserInfoManager.end())
	{
		return _ito->second;
	}
	return NULL;
}
void LUserInfoManager::RemoveUserInfo(char* pszUserID)
{ 
	if (pszUserID == NULL)
	{
		return ;
	}
	string sUserID = pszUserID;

	map<string, LUserInfo*>::iterator _ito = m_mapUserInfoManager.find(sUserID);
	if (_ito != m_mapUserInfoManager.end())
	{
		m_mapUserInfoManager.erase(_ito);
	}
}

LUserInfo* LUserInfoManager::AllocOneUserInfo()
{
	LUserInfo* pUserInfo = NULL;
	if (!m_queueUserInfoPool.empty())
	{
		pUserInfo = m_queueUserInfoPool.front();
		m_queueUserInfoPool.pop();
		return pUserInfo;
	}
	else
	{
		pUserInfo = new LUserInfo;
		if (pUserInfo == NULL)
		{
			return NULL;
		}
		pUserInfo->Reset();
		return pUserInfo;
	}
}
void LUserInfoManager::FreeOneUserInfo(LUserInfo* pUserInfo)
{
	if (pUserInfo == NULL)
	{
		return ;
	}
	pUserInfo->Reset();
	m_queueUserInfoPool.push(pUserInfo);
}

unsigned int LUserInfoManager::GetServeCount()
{
	return m_mapUserInfoManager.size();
}
