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

#include "LUserInfo.h"

LUserInfo::LUserInfo()
{
	m_eUserDataState = E_User_State_NULL;
	memset(m_szUserID, 0, sizeof(m_szUserID));
	memset(m_szUserPassWord, 0, sizeof(m_szUserPassWord));
	m_u64UserUniqueIDInDB = 0;
}
LUserInfo::~LUserInfo()
{
}
void LUserInfo::Reset()
{
	m_eUserDataState = E_User_State_NULL;
	memset(m_szUserID, 0, sizeof(m_szUserID));
	memset(m_szUserPassWord, 0, sizeof(m_szUserPassWord));
	m_u64UserUniqueIDInDB = 0;
}
E_User_State LUserInfo::GetUserState()
{
	return m_eUserDataState;
}
void LUserInfo::SetUserState(E_User_State eUserState)
{
	m_eUserDataState = eUserState;
}

void LUserInfo::SetUserID(char* pUserID)
{
	strncpy(m_szUserID, pUserID, MAX_USER_ID_LEN);
}
void LUserInfo::SetUserPassWord(char* pPassWord)
{
	strncpy(m_szUserPassWord, 0, sizeof(m_szUserPassWord));
}
void LUserInfo::SetUserUniqueIDInDB(uint64_t u64UserUniqueIDInDB)
{
	m_u64UserUniqueIDInDB = u64UserUniqueIDInDB;
}

bool LUserInfo::GetUserID(char* pbuf, unsigned int unbufLen)
{
	if (unbufLen < MAX_USER_ID_LEN)
	{
		return false;
	}
	strncpy(pbuf, m_szUserID, MAX_USER_ID_LEN);
	return true;
}
bool LUserInfo::GetUserPassWord(char* pbuf, unsigned int unbufLen)
{
	if (unbufLen < MAX_PASSWORD_LEN)
	{
		return false;
	}
	strncpy(pbuf, m_szUserPassWord, MAX_PASSWORD_LEN); 
	return true;
}
uint64_t LUserInfo::GetUserUniqueIDInDB()
{
	return m_u64UserUniqueIDInDB;
}



bool LUserInfo::CheckUserAndPassWord(char* szUserID, char* szPassWord)
{
	if (strncmp(szUserID, m_szUserID, MAX_USER_ID_LEN) == 0 && strncmp(szPassWord , m_szUserPassWord, MAX_PASSWORD_LEN) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
