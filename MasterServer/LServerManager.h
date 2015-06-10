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

#pragma once

#include "LServer.h"
#include <map>
using namespace std;

class LMainLogicThread;

class LServerManager
{
public:
	static LServerManager* GetServerManagerInstance();
	~LServerManager();
protected:
	LServerManager();
	LServerManager(const LServerManager& sm);
	LServerManager& operator=(const LServerManager& sm);
private:
	static LServerManager* m_pServerManagerInstance;
public:
	//	添加一个新连接上来的服务器，因为没有服务器ID等信息，那么临时存放在待确任表中
	bool AddNewUpServer(uint64_t un64SessionID, t_Session_Accepted& tsa);

	//	添加一个服务器
	bool AddServer(uint64_t un64SessionID, unsigned char ucServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID);
	
	//	查找服务器
	LServer* FindServer(uint64_t u64ServerID);
	LServer* FindServerBySessionID(uint64_t u64SessionID);
	LServer* FindServer(unsigned char ucServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID);

	//	移除服务器
	void RemoveServerByNetWorkSessionID(uint64_t u64NetWorkSessionID);
	void RemoveServer(uint64_t u64ServerID);
	void RemoveServer(unsigned char ucServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID);


	bool SelectBestLobbyServerAndGameDBServer(uint64_t& uSelectedLobbyServerUniqueID, uint64_t& uSelectedGameDBServerUniqueID);
public:
	//	向一个服务器发包
	void SendPacketToServer(uint64_t u64ServerID, LPacketBroadCast* pPacket);
	void SendPacketToServer(unsigned char ucServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID, LPacketBroadCast* pPacket);
	//	向服务器类型广播数据包
	void BroadCastToServersByServerType(unsigned char ucServerType, LPacketBroadCast* pPacket);
	//	向指定区的服务器广播数据包
	void BroadCastToServersByServerArea(unsigned short usAreaID,  LPacketBroadCast* pPacket);

	//	给某个服务器发送可以连接的服务器的ID信息
	void SendPacketToServerCanConnectToServerInfos(uint64_t u64RegisterServerUniqueID);
private:
	LServer* CreateServer(unsigned char ucServerType);
private:
	map<uint64_t, LServer*> m_mapServers;
	map<uint64_t, LServer*> m_mapSessionToServers;
	map<uint64_t, LServer*> m_mapNewUpServers;	//	新连接上来的服务器信息


public:
	void SetMainLogicThread(LMainLogicThread* pmlt);
	LMainLogicThread* GetMainLogicThread();
private:
	LMainLogicThread* m_pMainLogicThread;

public:
	bool SelectBestGateServer(uint64_t& u64SelecttedGateServerSessionID);
};

