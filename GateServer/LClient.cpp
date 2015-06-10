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
	m_u64SessionID 		= 0;
	m_nRecvThreadID 	= 0;		//	接收线程ID
	m_nSendThreadID 	= 0;		//	发送线程ID
	memset(m_szIP, 0, sizeof(m_szIP));				//	服务器的IP
	m_usPort 			= 0;						//	服务器的监听端口
	m_u64UniqueIDInDB	= 0;						//	玩家的唯一帐号ID
	m_eClientState		= E_Client_State_UnKnown;	//	玩家当前的状态
	m_u64CurrentServerUniqueID = 0;		//	当前正在连接的服务器唯一ID
	memset(m_szUserID, 0, sizeof(m_szUserID));
}
LClient::~LClient()
{
}

void LClient::Reset()
{
	m_u64SessionID 		= 0;
	m_nRecvThreadID 	= 0;		//	接收线程ID
	m_nSendThreadID 	= 0;		//	发送线程ID
	memset(m_szIP, 0, sizeof(m_szIP));				//	服务器的IP
	m_usPort 			= 0;						//	服务器的监听端口
	m_u64UniqueIDInDB	= 0;						//	玩家的唯一帐号ID
	m_eClientState		= E_Client_State_UnKnown;	//	玩家当前的状态
	m_u64CurrentServerUniqueID = 0;		//	当前正在连接的服务器唯一ID
	memset(m_szUserID, 0, sizeof(m_szUserID));
}

void LClient::SetUniqueIDInDB(uint64_t u64UniqueIDInDB)
{
	m_u64UniqueIDInDB = u64UniqueIDInDB;
}
uint64_t LClient::GetUniqueIDInDB()
{
	return m_u64UniqueIDInDB;
}
void LClient::SetClientState(E_Client_State eClientState)
{
	m_eClientState = eClientState;
}
E_Client_State LClient::GetClientState()
{
	return m_eClientState;
}
void LClient::SetCurrentServerUniqueID(uint64_t u64ServerID)
{
	m_u64CurrentServerUniqueID = u64ServerID;
}
uint64_t LClient::GetCurrentServerUniqueID()
{
	return m_u64CurrentServerUniqueID;
}

void LClient::SetUserID(char* pszUserID)
{
	if (pszUserID == NULL)
	{
		return;
	}
	strncpy(m_szUserID, pszUserID, MAX_USER_ID_LEN);
}

void LClient::GetUserID(char* pbuf, unsigned int unbufLen)
{
	if (pbuf == NULL || unbufLen < MAX_USER_ID_LEN)
	{
		return ;
	}
	strncpy(pbuf, m_szUserID, MAX_USER_ID_LEN);
}

void LClient::SetSessionID(uint64_t u64SessionID)
{
	m_u64SessionID = u64SessionID;
}

uint64_t LClient::GetSessionID()
{
	return m_u64SessionID;
}

int LClient::GetSendThreadID()
{
	return m_nSendThreadID;
}

void LClient::SetSendThreadID(int nSendThreadID)
{
	m_nSendThreadID = nSendThreadID;
}
int LClient::GetRecvThreadID()
{
	return m_nRecvThreadID;
}
void LClient::SetRecvThreadID(int nRecvThreadID)
{
	m_nRecvThreadID = nRecvThreadID;
}
void LClient::SetServerIp(char* pServerIp, unsigned int unSize)
{ 
	strncpy(m_szIP, pServerIp, sizeof(m_szIP) - 1);
}
void LClient::GetServerIp(char* pbuf, unsigned int unbufSize)
{
	strncpy(pbuf, m_szIP, unbufSize);
}
void LClient::SetServerPort(unsigned short usServerPort)
{
	m_usPort = usServerPort;
}
unsigned short LClient::GetServerPort()
{
	return m_usPort;
}


