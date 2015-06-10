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
class LGateServerMainLogic;

class LConnectToMasterServer
{
public:
	LConnectToMasterServer();
	~LConnectToMasterServer();
public:
	//	初始化连接,读取配置文件
	bool Initialize(char* pConnectToMasterServerConfigFile); 
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
	bool StopConnectToMasterServerThread();
	void ReleaseConnectToMasterServerThreadResource();
private:
	//	连接到MasterServer的连接
	LConnector m_ConnectorToMasterServer; 
	//	一次处理数据包的最大数量
	unsigned int m_unMaxPacketProcessOnce;

public:
	//	处理接收的数据包
	void ProcessPacket(unsigned int unMaxPacketProcessed);
	unsigned int GetPacketProcessed()
	{
		return m_unPacketProcessed;
	}
	LGateServerMainLogic* GetGateServerMainLogic();
	void SetGateServerMainLogic(LGateServerMainLogic* pgsml);
private:
	unsigned int m_unPacketProcessed;
	LGateServerMainLogic* m_pGateServerMainLogic;

public:
	void ReportServerIDToMasterServer();
};

