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

#include "LUser.h"

LUser::LUser()
{
	m_u64UserIDInDB = 0;
	memset(m_szUserID, 0, sizeof(m_szUserID));
	m_eUserState = E_User_Unknown;
}
LUser::~LUser()
{
}
void LUser::Reset()
{
	m_u64UserIDInDB = 0;
	memset(m_szUserID, 0, sizeof(m_szUserID));
	m_eUserState = E_User_Unknown;
}
void LUser::SetUserUniqueIDInDB(uint64_t u64UserUniqueIDInDB)
{
	m_u64UserIDInDB = u64UserUniqueIDInDB;
}
uint64_t LUser::GetUserUniqueIDInDB()
{
	return m_u64UserIDInDB;
}
void LUser::SetUserID(char* pUserID)
{
	strncpy(m_szUserID, pUserID, MAX_USER_ID_LEN);
}
void LUser::GetUserID(char* pbuf, unsigned int unbufLen)
{
	strncpy(pbuf, m_szUserID, unbufLen);
}
void LUser::SetUserState(E_User_State eUserState)
{
	m_eUserState = eUserState;
}
E_User_State LUser::GetUserState()
{
	return m_eUserState;
}

