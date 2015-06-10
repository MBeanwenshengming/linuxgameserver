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
#include "../NetWork/LServerBaseNetWork.h"
#include "../NetWork/LThreadBase.h" 
#include "LMasterServer_PacketProcess_Proc.h"
#include "LUserManager.h"

class LMainLogicThreadReadConfig
{
public:
	LMainLogicThreadReadConfig();
	~LMainLogicThreadReadConfig();
public:
	bool ReadConfig(char* pConfigFileName);
	void Reset();
public:
	inline unsigned char GetServerType()
	{
		return m_ucServerType;
	}
	inline unsigned short GetAreaID()
	{
		return m_usAreaID;
	}
	inline unsigned short GetGroupID()
	{
		return m_usGroupID;
	}
	inline unsigned short GetServerID()
	{
		return m_usServerID;
	}
	inline unsigned int GetMaxClientServered()
	{
		return m_unMaxClientServered;
	}
private:
	unsigned char m_ucServerType;
	unsigned short m_usAreaID;
	unsigned short m_usGroupID;
	unsigned short m_usServerID;
	unsigned int   m_unMaxClientServered;	//最大服务数量
};


class LMainLogicThread : public LServerBaseNetWork, public LThreadBase
{
public:
	LMainLogicThread();
	~LMainLogicThread();
public:
	bool Initialize(char* pServerConfigFileName, char* pNetWorkConfigFileName);
public:			//	线程虚函数 
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();

	void StopMainLogicThread();
public:			//	网络虚函数 
	virtual bool OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa);
	virtual void OnRemoveSession(uint64_t u64SessionID);
	virtual void OnRecvedPacket(t_Recv_Packet& tRecvedPacket);
private:
	LMainLogicThreadReadConfig m_ConfigInfo;
	LMasterServerPacketProcessProc m_PacketProcessProcManager;
public:
	LUserManager* GetUserManager()
	{
		return &m_UserManager;
	}
private:
	LUserManager m_UserManager;
};



