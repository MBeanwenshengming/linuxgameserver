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

#include "LClient.h"


LClient::LClient()
{
	m_u64SessionID = 0;	
	memset(&m_SA, 0, sizeof(m_SA)); 
	m_u64ClientUniqueIDInADB = 0;
	memset(m_szUserID, 0, sizeof(m_szUserID));
}

LClient::~LClient()
{
}


void LClient::Reset()
{ 
	m_u64SessionID = 0;	
	memset(&m_SA, 0, sizeof(m_SA));
}
void LClient::SetClientInfo(uint64_t u64SesssionID, t_Session_Accepted& tas)
{ 
	m_u64SessionID = u64SesssionID;	
	m_SA = tas;
}
uint64_t LClient::GetSessionID()
{
	return m_u64SessionID;
}
void LClient::GetSessionInfo(t_Session_Accepted& tSAInfo)
{
	tSAInfo = m_SA;
}

int LClient::GetSendThreadID()
{
	return m_SA.nSendThreadID;
}

int LClient::GetRecvThreadID()
{
	return m_SA.nRecvThreadID;
}
void LClient::GetClientIpAndPort(char* pbuf, unsigned int unbufLen, unsigned short& usPort)
{
	if (pbuf == NULL || unbufLen == 0)
	{
		return ;
	}
	strncpy(pbuf, m_SA.szIp, unbufLen);
	usPort = m_SA.usPort;
}

void LClient::SetClientUniqueIDInADB(uint64_t u64ClientUniqueIDInADB)
{
	m_u64ClientUniqueIDInADB = u64ClientUniqueIDInADB;
}
uint64_t LClient::GetClientUniqueIDInADB()
{
	return m_u64ClientUniqueIDInADB;
}
void LClient::SetUserID(char* pUserID)
{
	if (pUserID == NULL)
	{
		return ;
	}
	strncpy(m_szUserID, pUserID, MAX_USER_ID_LEN);
}
void LClient::GetUserID(char* pbuf, unsigned int unbufLen)
{
	if (unbufLen < MAX_USER_ID_LEN)
	{
		return ;
	}
	strncpy(pbuf, m_szUserID, MAX_USER_ID_LEN);
}
