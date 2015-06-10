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

#include "LServerManager.h"
#include "LServer.h"
#include "../NetWork/LServerBaseNetWork.h"
#include "LMainLogicThread.h"
#include "../NetWork/LPacketBroadCast.h"
#include "../include/Server_To_Server_Packet_Define.h"

LServerManager* LServerManager::m_pServerManagerInstance = NULL;

LServerManager* LServerManager::GetServerManagerInstance()
{
	if (m_pServerManagerInstance == NULL)
	{
		m_pServerManagerInstance = new LServerManager;
	}
	return m_pServerManagerInstance;
}
LServerManager::~LServerManager()
{
	map<uint64_t, LServer*>::iterator _ito = m_mapServers.begin();
	while (_ito != m_mapServers.end())
	{
		delete _ito->second;
		_ito++;
	} 
}
LServerManager::LServerManager()
{ 
	m_pMainLogicThread = NULL;
}
LServerManager::LServerManager(const LServerManager& sm)
{
	sm;
}
LServerManager& LServerManager::operator=(const LServerManager& sm)
{
	sm;
	return *this;
}

void LServerManager::SetMainLogicThread(LMainLogicThread* pmlt)
{
	m_pMainLogicThread = pmlt;
}
LMainLogicThread* LServerManager::GetMainLogicThread()
{
	return m_pMainLogicThread;
}

//	添加一个新连接上来的服务器，因为没有服务器ID等信息，那么临时存放在待确任表中
bool LServerManager::AddNewUpServer(uint64_t un64SessionID, t_Session_Accepted& tsa)
{
	LServer* pServer = new LServer;
	if (pServer == NULL)
	{
		return false;
	}
	pServer->SetServerBaseInfo(tsa);
	pServer->SetServerState(E_Server_State_Unknow);
	pServer->SetNetWorkSessionID(un64SessionID);

	m_mapNewUpServers[un64SessionID] = pServer;
	return true;	
}

//	添加一个服务器
bool LServerManager::AddServer(uint64_t un64SessionID, unsigned char ucServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapNewUpServers.find(un64SessionID);	//	新连接上来的服务器信息
	if (_ito == m_mapNewUpServers.end())
	{
		return false;
	}

	LServer* pNewUpServer = _ito->second;

	E_Server_Type eServerType = (E_Server_Type)ucServerType;
	if (eServerType <= E_Server_Type_Invalid|| eServerType >= E_Server_Type_Max)
	{
		delete _ito->second;
		m_mapNewUpServers.erase(_ito);
		return false;
	}
	LServer* pServer = CreateServer(ucServerType);
	if (pServer == NULL)
	{
		delete _ito->second;
		m_mapNewUpServers.erase(_ito);
		return false;
	}
	t_Session_Accepted tsa;
	pNewUpServer->GetServerSessionBaseInfo(tsa);

	pServer->SetNetWorkSessionID(un64SessionID);
	pServer->SetServerBaseInfo(tsa);
	pServer->SetServerID((E_Server_Type)ucServerType, usAreaID, usGroupID, usServerID);
	pServer->SetServerState(E_Server_State_Registered);

	//	删除NewUpManager上的连接信息
	delete _ito->second; 
	m_mapNewUpServers.erase(_ito);

	uint64_t uUniqueServerID = pServer->GetServerUniqueID();
		
	map<uint64_t, LServer*>::iterator _itoNow = m_mapServers.find(uUniqueServerID);
	if (_itoNow != m_mapServers.end())
	{
		delete pServer;
		return false;
	}

	map<uint64_t, LServer*>::iterator _itoSessionIDToServer = m_mapSessionToServers.find(un64SessionID);
	if (_itoSessionIDToServer != m_mapSessionToServers.end())
	{
		delete pServer;
		return false;
	}
	m_mapServers[uUniqueServerID] = pServer;
	m_mapSessionToServers[un64SessionID]= pServer;
	return true;
}

//	查找服务器
LServer* LServerManager::FindServer(uint64_t u64ServerID)
{
	map<uint64_t, LServer*>::iterator _ito = m_mapServers.find(u64ServerID);
	if (_ito != m_mapServers.end())
	{
		return _ito->second;
	}
	return NULL;
}

LServer* LServerManager::FindServerBySessionID(uint64_t u64SessionID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapSessionToServers.find(u64SessionID);
	if (_ito != m_mapSessionToServers.end())
	{
		return _ito->second;
	}

	map<uint64_t, LServer*>::iterator _ito1 = m_mapNewUpServers.find(u64SessionID);	//	新连接上来的服务器信息
	if (_ito1 != m_mapNewUpServers.end())
	{
		return _ito1->second;
	}
	return NULL;
}

LServer* LServerManager::FindServer(unsigned char ucServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID)
{ 
	int64_t nServerID = 0;

	int64_t uTemp = 0; 
	uTemp = ucServerType;
	nServerID =  uTemp << (6 * 8);

	uTemp = usAreaID;
	nServerID |= uTemp << (4 * 8);

	uTemp = usGroupID;
	nServerID |= uTemp << (2 * 8);

	uTemp = usServerID;
	nServerID |= uTemp; 

	return FindServer(nServerID);
}

void LServerManager::RemoveServerByNetWorkSessionID(uint64_t u64NetWorkSessionID)
{ 
	map<uint64_t, LServer*>::iterator _itoNewUp = m_mapNewUpServers.find(u64NetWorkSessionID);
	if (_itoNewUp != m_mapNewUpServers.end())
	{
		delete _itoNewUp->second;
		m_mapNewUpServers.erase(_itoNewUp);
	}

	//	查找已经连接好的服务器，看是否存在 
//	map<int64_t, LServer*>::iterator _ito = m_mapServers.begin();
//	while (_ito != m_mapServers.end())
//	{
//		uint64_t u64NetWorkSessionID = _ito->second->GetNetWorkSessionID();
//		if (u64NetWorkSessionID == u64NetWorkSessionID)
//		{
//			delete _ito->second;
//			m_mapServers.erase(_ito++);
//			continue;
//		}
//		_ito++;
//	}
	map<uint64_t, LServer*>::iterator _ito1 = m_mapSessionToServers.find(u64NetWorkSessionID);
	if (_ito1 != m_mapSessionToServers.end())
	{
		LServer* pServer = _ito1->second;
		uint64_t u64ServerUniqueID = pServer->GetServerUniqueID();

		m_mapSessionToServers.erase(_ito1);

		uint64_t u64SeverUniqueID = pServer->GetServerUniqueID();
		map<uint64_t, LServer*>::iterator _ito2 = m_mapServers.find(u64SeverUniqueID);
		if (_ito2 != m_mapServers.end())
		{
			m_mapServers.erase(_ito2);
		}
		delete pServer;
	}
} 
//	移除服务器
void LServerManager::RemoveServer(uint64_t u64ServerID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapServers.find(u64ServerID);
	if (_ito != m_mapServers.end())
	{
		LServer* pServer = _ito->second;
		m_mapServers.erase(_ito);

		uint64_t u64NetWorkSessionID = pServer->GetNetWorkSessionID();
		map<uint64_t, LServer*>::iterator _ito1 = m_mapSessionToServers.find(u64NetWorkSessionID);
		if (_ito1 != m_mapSessionToServers.end())
		{
			m_mapSessionToServers.erase(_ito1);
		}
		delete pServer;
	}
}
void LServerManager::RemoveServer(unsigned char ucServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID)
{
	int64_t nServerID = 0;

	int64_t uTemp = ucServerType;
	nServerID =  uTemp << (6 * 8);

	uTemp = usAreaID;
	nServerID |= uTemp << (4 * 8);

	uTemp = usGroupID;
	nServerID |= uTemp << (2 * 8);

	uTemp = usServerID;
	nServerID |= uTemp; 
	RemoveServer(nServerID);
}

LServer* LServerManager::CreateServer(unsigned char ucServerType)
{
	E_Server_Type eServerType = (E_Server_Type)ucServerType;
	switch(eServerType)
	{
		case E_Server_Type_Login_Server:
			{
				return new LLoginServer;
			}
			break;
		case E_Server_Type_Gate_Server:
			{
				return new LGateServer;
			}
			break;
		case E_Server_Type_Lobby_Server: 
			{
				return new LLobbyServer;
			}
			break;
		case E_Server_Type_Game_Server:
			{
				return new LGameServer;
			}
			break;
		case E_Server_Type_DB_Server:
			{
				return new LDBServer;
			}
			break;
		case E_Server_Type_Account_DB_Server:
			{
				return new LAccountDBServer;
			}
			break;
		default:
			return NULL;
	}
	return NULL;
}

void LServerManager::SendPacketToServer(uint64_t u64ServerID, LPacketBroadCast* pPacket)
{ 
	if (pPacket == NULL)
	{
		return ;
	}
	LServer* pServer = FindServer(u64ServerID);
	if (pServer == NULL)
	{
		return;
	}
	uint64_t u64SessionID = pServer->GetNetWorkSessionID();
	int nSendThreadID = pServer->GetSendThreadID();
	m_pMainLogicThread->SendOnePacket(u64SessionID, nSendThreadID, pPacket);
}
void LServerManager::SendPacketToServer(unsigned char ucServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID, LPacketBroadCast* pPacket)
{ 
	int64_t nServerID = 0;

	int64_t uTemp = ucServerType;
	nServerID =  uTemp << (6 * 8);

	uTemp = usAreaID;
	nServerID |= uTemp << (4 * 8);

	uTemp = usGroupID;
	nServerID |= uTemp << (2 * 8);

	uTemp = usServerID;
	nServerID |= uTemp; 

	SendPacketToServer(nServerID, pPacket);
}
//	向服务器类型广播数据包
void LServerManager::BroadCastToServersByServerType(unsigned char ucServerType, LPacketBroadCast* pPacket)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapServers.begin();
	while (_ito != m_mapServers.end())
	{
		LServer* pServer = _ito->second;
		if (pServer->GetServerType() == ucServerType)
		{ 
			uint64_t u64SessionID = pServer->GetNetWorkSessionID();
			int nSendThreadID = pServer->GetSendThreadID(); 
			m_pMainLogicThread->SendOnePacket(u64SessionID, nSendThreadID, pPacket);
		}
		_ito++;
	}
}
//	向指定区的服务器广播数据包
void LServerManager::BroadCastToServersByServerArea(unsigned short usAreaID,  LPacketBroadCast* pPacket)
{
}


//	给某个服务器发送可以连接的服务器的ID信息
void LServerManager::SendPacketToServerCanConnectToServerInfos(uint64_t u64RegisterServerUniqueID)
{
	LServer* pServer = FindServer(u64RegisterServerUniqueID);
	if (pServer == NULL)
	{
		return ;
	}
	unsigned char ucServerType = pServer->GetServerType();
	E_Server_Type eServerType = (E_Server_Type)ucServerType;

	bool bSendPacket = false;
	map<uint64_t, LServer*>::iterator _ito = m_mapServers.begin();
	while (_ito != m_mapServers.end())
	{
		bSendPacket = false;
		LServer* pServerTemp = _ito->second;
		unsigned char ucServerTypeTemp = pServerTemp->GetServerType();
		E_Server_Type eServerTypeTemp = (E_Server_Type)ucServerTypeTemp;
		uint64_t uServerUniqueIDTemp = pServerTemp->GetServerUniqueID();
		switch (eServerType)
		{
			case E_Server_Type_Login_Server:
				{
					//	发送AccountDBServer信息
					if (eServerTypeTemp == E_Server_Type_Account_DB_Server)
					{
						bSendPacket = true;
					}
				}
				break;
			case E_Server_Type_Gate_Server:
				{
					if (eServerTypeTemp == E_Server_Type_Lobby_Server|| eServerTypeTemp == E_Server_Type_Game_Server)
					{
						bSendPacket = true;
					}
				}
				break;
			case E_Server_Type_Lobby_Server:
				{
					if (eServerTypeTemp == E_Server_Type_DB_Server)
					{ 
						bSendPacket = true; 
					}
				}
				break;
			case E_Server_Type_Game_Server:
				{
					if (eServerTypeTemp == E_Server_Type_DB_Server)
					{ 
						bSendPacket = true; 
					}
				}
				break;
			default:
				break;
		}
		if (bSendPacket)
		{
			unsigned short usPacketLen = 32;
			LPacketBroadCast* pSendPacket = m_pMainLogicThread->GetOneSendPacket(usPacketLen);
			pSendPacket->SetPacketID(Packet_SS_Server_Can_ConnectTo_Server_Infos);
			pSendPacket->AddULongLong(u64RegisterServerUniqueID);
			pSendPacket->AddULongLong(uServerUniqueIDTemp);
			m_pMainLogicThread->SendOnePacket(pServer->GetNetWorkSessionID(), pServer->GetSendThreadID(), pSendPacket);
			m_pMainLogicThread->FlushSendPacket();
		}
		_ito++;
	} 
}

bool LServerManager::SelectBestGateServer(uint64_t& u64SelecttedGateServerSessionID)
{ 
	map<uint64_t, LServer*>::iterator _ito = m_mapServers.begin();
	while (_ito != m_mapServers.end())
	{
		LServer* pServer = _ito->second;
		unsigned char ucServerType = pServer->GetServerType();
		E_Server_Type eServerType = (E_Server_Type)ucServerType;
		if (eServerType == E_Server_Type_Gate_Server) 
		{
			u64SelecttedGateServerSessionID = pServer->GetNetWorkSessionID();
			return true;
		}
		_ito++;
	}
	return false;
}

bool LServerManager::SelectBestLobbyServerAndGameDBServer(uint64_t& uSelectedLobbyServerUniqueID, uint64_t& uSelectedGameDBServerUniqueID)
{
	return true;
}
