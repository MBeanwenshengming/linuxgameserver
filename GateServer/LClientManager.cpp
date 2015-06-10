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
#include "LConnectToMasterServer.h"
#include "../NetWork/LPacketSingle.h"
#include "../include/Server_To_Server_Packet_Define.h"
#include "LClient.h"
#include "LGateServerMainLogic.h"

LClientManager::LClientManager()
{
	m_pGateServerMainLogic = NULL;
	m_unLoginKeyWillConnect = 0;	
}

LClientManager::~LClientManager()
{
}

//	玩家新连接上来
bool LClientManager::AddNewUpClient(uint64_t u64SesssionID, t_Session_Accepted& tas)
{ 
	map<uint64_t, LClient*>::iterator _ito = m_mapSessionIDToClient.find(u64SesssionID);
	if (_ito != m_mapSessionIDToClient.end())
	{
		return false;
	}

	LClient* pClient = AllocClient();
	if (pClient == NULL)
	{
		return false;
	}
	pClient->SetClientState(E_Client_State_Just_Connect);
	pClient->SetSessionID(u64SesssionID);
	pClient->SetSendThreadID(tas.nSendThreadID);
	pClient->SetRecvThreadID(tas.nRecvThreadID);
	pClient->SetServerIp(tas.szIp, 16);
	pClient->SetServerPort(tas.usPort);
	m_mapSessionIDToClient[u64SesssionID] = pClient;
	return true;
}

//	检查玩家发送的的登录信息是否存在
bool LClientManager::CheckClientInfo(uint64_t u64SessionID, uint64_t u64UserUniqueIDInDB, char* pszLoginKey)
{ 
	map<uint64_t, LClient*>::iterator _ito = m_mapSessionIDToClient.find(u64SessionID);
	if (_ito == m_mapSessionIDToClient.end())
	{
		return false;
	}
	LClient* pClient = _ito->second;

	map<uint64_t, LClient*>::iterator _itouuID = m_mapUniqueIDInDBToClient.find(u64UserUniqueIDInDB);
	if (_itouuID != m_mapUniqueIDInDBToClient.end())
	{
		return false;
	}
	
	map<uint64_t, t_Client_Will_Connect>::iterator _itoC = m_mapClientWillConnectInfos.find(u64UserUniqueIDInDB);
	if (_itoC == m_mapClientWillConnectInfos.end())
	{
		return false;
	}
	t_Client_Will_Connect tcwc = _itoC->second;
	m_mapClientWillConnectInfos.erase(_itoC);

	if (strncmp(tcwc.szLoginKey, pszLoginKey, 128) != 0)
	{
		return false;
	}
	pClient->SetUniqueIDInDB(u64UserUniqueIDInDB);
	pClient->SetClientState(E_Client_State_Initialize);
	pClient->SetUserID(tcwc.szUserID);
		
	m_mapUniqueIDInDBToClient[u64UserUniqueIDInDB] = pClient;	
	return true;
}

//	查找玩家
LClient* LClientManager::FindClientBySessionID(uint64_t u64SessionID)
{
	map<uint64_t, LClient*>::iterator _ito = m_mapSessionIDToClient.find(u64SessionID);
	if (_ito == m_mapSessionIDToClient.end())
	{
		return NULL;
	}
	return _ito->second;
}

LClient* LClientManager::FineClientByUserUniqueIDInDB(uint64_t u64UserUniqueIDInDB)
{
	map<uint64_t, LClient*>::iterator _itouuID = m_mapUniqueIDInDBToClient.find(u64UserUniqueIDInDB);
	if (_itouuID == m_mapUniqueIDInDBToClient.end())
	{
		return NULL;
	}
	return _itouuID->second;
}

//	移除玩家
void LClientManager::RemoveClientBySessionID(uint64_t u64SessionID)
{ 
	map<uint64_t, LClient*>::iterator _ito = m_mapSessionIDToClient.find(u64SessionID);
	if (_ito == m_mapSessionIDToClient.end())
	{
		return ;
	}
	LClient* pClient = _ito->second;
	E_Client_State eClientState = pClient->GetClientState();
	if (eClientState < E_Client_State_Initialize)
	{
		m_mapSessionIDToClient.erase(_ito);
		FreeClient(pClient);
		return ;
	}
	uint64_t u64UserUniqueIDInDB = pClient->GetUniqueIDInDB();

	map<uint64_t, LClient*>::iterator _itoIDInDB = m_mapUniqueIDInDBToClient.find(u64UserUniqueIDInDB);
	if (_itoIDInDB != m_mapUniqueIDInDBToClient.end())
	{
		m_mapUniqueIDInDBToClient.erase(_itoIDInDB);
	} 
	FreeClient(pClient);
}
void LClientManager::RemoveClientByUserUniqueIDInDB(uint64_t u64UserUniqueIDInDB)
{
	map<uint64_t, LClient*>::iterator _itoIDInDB = m_mapUniqueIDInDBToClient.find(u64UserUniqueIDInDB);
	if (_itoIDInDB == m_mapUniqueIDInDBToClient.end())
	{
		return ;
	}
	LClient* pClient = _itoIDInDB->second;

	m_mapUniqueIDInDBToClient.erase(_itoIDInDB);
	uint64_t u64SessionID = pClient->GetSessionID();

	map<uint64_t, LClient*>::iterator _ito = m_mapSessionIDToClient.find(u64SessionID);
	if (_ito != m_mapSessionIDToClient.end())
	{
		m_mapSessionIDToClient.erase(_ito);
	}
	FreeClient(pClient);
}


bool LClientManager::InitializeClientPool(unsigned int unPoolSize)
{
	if (m_pGateServerMainLogic == NULL)
	{
		return false;
	}
	if (unPoolSize == 0)
	{
		unPoolSize = 100;
	}
	for (unsigned int unIndex = 0; unIndex < unPoolSize; ++unIndex)
	{
		LClient* pClient = new LClient;
		if (pClient == NULL)
		{
			return false;
		}
		m_ClientPool.push(pClient);
	}
	return true;
}

void LClientManager::ReleaseClientManagerResoutce()
{
	while(!m_ClientPool.empty())
	{
		LClient* pClient = m_ClientPool.front();
		m_ClientPool.pop();
		delete pClient;
	}
	//
	map<uint64_t, LClient*>::iterator _ito = m_mapSessionIDToClient.begin();
	while (_ito != m_mapSessionIDToClient.end())
	{
		LClient* pClient = _ito->second;
		uint64_t u64UserUniqueIDInDB = pClient->GetUniqueIDInDB();
		m_mapSessionIDToClient.erase(_ito);

		map<uint64_t, LClient*>::iterator _ito1 = m_mapUniqueIDInDBToClient.find(u64UserUniqueIDInDB);
		if (_ito1 != m_mapUniqueIDInDBToClient.end())
		{
			m_mapUniqueIDInDBToClient.erase(_ito1);
		}
		
		delete pClient;
		++_ito;
	}
	if (m_mapUniqueIDInDBToClient.size() != 0)
	{
		//	error log
	}
}


LClient* LClientManager::AllocClient()
{
	if (m_ClientPool.empty())
	{
		return NULL;
	}
	LClient* pClient = m_ClientPool.front();
	m_ClientPool.pop();
	return pClient;
}

void LClientManager::FreeClient(LClient* pClient)
{
	if (pClient == NULL)
	{
		return ;
	}
	m_ClientPool.push(pClient);
}


bool LClientManager::AddWillConnectClientInfo(uint64_t u64UserUniqueIDInDB, char* pszLoginKey, char* pszUserID)
{ 
	if (pszLoginKey == NULL || pszUserID == NULL)
	{
		return false;
	}
	map<uint64_t, t_Client_Will_Connect>::iterator _ito = m_mapClientWillConnectInfos.find(u64UserUniqueIDInDB);
	if (_ito != m_mapClientWillConnectInfos.end())
	{
		return true;
	}
	//	增加玩家信息
	t_Client_Will_Connect tcwc;
	strncpy(tcwc.szLoginKey, pszLoginKey, sizeof(tcwc.szLoginKey));
	strncpy(tcwc.szUserID, pszUserID, MAX_USER_ID_LEN);
	m_mapClientWillConnectInfos[u64UserUniqueIDInDB] = tcwc;

	//	增加超时踢掉信息
	t_Client_Will_Connect_TimeOutInfo tcwctoi;
	tcwctoi.tMoveTime = time(NULL) + 5;
	tcwctoi.u64UserUniqueIDInDB = u64UserUniqueIDInDB;
	m_queueTimeOutQueue.push(tcwctoi);

	return true;
}
bool LClientManager::FindWillConnectClientInfo(uint64_t u64UserUniqueIDInDB,char* pszLoginKeyBuf, unsigned int unBufLen, char* pszUserIDBuf, unsigned int unUIDBufLen)
{ 
	if (pszLoginKeyBuf == NULL || unBufLen < 128)
	{
		return false;
	}
	if (pszUserIDBuf == NULL || unUIDBufLen < MAX_USER_ID_LEN)
	{
		return false;
	}
	map<uint64_t, t_Client_Will_Connect>::iterator _ito = m_mapClientWillConnectInfos.find(u64UserUniqueIDInDB);
	if (_ito == m_mapClientWillConnectInfos.end())
	{
		return false;
	}
	strncpy(pszLoginKeyBuf, _ito->second.szLoginKey, 128);
	strncpy(pszUserIDBuf, _ito->second.szUserID, MAX_USER_ID_LEN);
	return true;
}

void LClientManager::ProcessTimeOutWillConnect()
{
	time_t tNow = time(NULL);
	unsigned int unProcessCount = 0;
	while (!m_queueTimeOutQueue.empty())
	{
		t_Client_Will_Connect_TimeOutInfo tTempInfo = m_queueTimeOutQueue.front();
		unProcessCount++;
		if (unProcessCount > 100)		//	最多一次处理100个
		{
			break;
		}

		if (tTempInfo.tMoveTime < tNow)
		{
			m_queueTimeOutQueue.pop();
			//	移除玩家登录信息

			map<uint64_t, t_Client_Will_Connect>::iterator _ito = m_mapClientWillConnectInfos.find(tTempInfo.u64UserUniqueIDInDB);
			if (_ito != m_mapClientWillConnectInfos.end())
			{
				m_mapClientWillConnectInfos.erase(_ito);
			}
			//	向MasterServer发送玩家被踢出信息
			LConnectToMasterServer* pConToMasterServer = m_pGateServerMainLogic->GetConnectToMasterServer();
			unsigned short usPacketLen = 32;
			LPacketSingle* pSendPacket = pConToMasterServer->GetOneSendPacketPool(usPacketLen);
			pSendPacket->SetPacketID(Packet_GA2M_USER_GATESERVER_OFFLINE);
			pSendPacket->AddULongLong(tTempInfo.u64UserUniqueIDInDB);
			pConToMasterServer->AddOneSendPacket(pSendPacket);
		}
		else
		{
			break;
		}
	}
}

bool LClientManager::BuildLoginKey(char* pbuf, unsigned int unbufLen)
{
	if (pbuf == NULL || unbufLen == 0)
	{
		return false;
	}
	m_unLoginKeyWillConnect++;
	sprintf(pbuf, "%u", m_unLoginKeyWillConnect);
	return true;
}

void LClientManager::SetGateServerMainLogic(LGateServerMainLogic* pgsml)
{
	m_pGateServerMainLogic = pgsml;
}
LGateServerMainLogic* LClientManager::GetGateServerMainLogic()
{
	return m_pGateServerMainLogic;
}


unsigned int LClientManager::GetServeCount()
{
	return m_mapSessionIDToClient.size();
}
