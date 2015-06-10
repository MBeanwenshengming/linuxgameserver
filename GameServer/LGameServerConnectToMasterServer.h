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

#include "../NetWork/LConnector.h" 
class LPacketSingle;
class LGameServerMainLogic;

class LGameServerConnectToMasterServer
{
public:
	LGameServerConnectToMasterServer();
	~LGameServerConnectToMasterServer();
public:
	bool Initialize(char* pConfigFile, char* pConfigHeader);

public:
	//	获取接收的数据
	LPacketSingle* GetOneRecvedPacket();
	//	释放一个接收的数据包
	void FreePacket(LPacketSingle* pPacket);

	//	发送数据
	//	取一个发送数据包
	LPacketSingle* GetOneSendPacketPool(unsigned short usPacketSize); 
	//	添加一个发送数据包
	bool AddOneSendPacket(LPacketSingle* pPacket);

	bool IsConnectted();

public: 
	bool StopGSConnectToMasterServer();
	void ReleaseGSConnectToMasterServerResource();
public:
	void ProcessRecvedPacket();
	unsigned int GetPacketProcessed()
	{
		return m_unPacketProcess;
	}
	LGameServerMainLogic* GetGameServerMainLogic();
	void SetGameServerMainLogic(LGameServerMainLogic* pgsml);
private: 
	//	连接到帐号数据库缓冲服务器
	LConnector m_ConnectToMasterServer;
	unsigned int m_unMaxPacketProcessOnce;
	unsigned int m_unPacketProcess;
	LGameServerMainLogic* m_pGameServerMainLogic;
};

