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

class LConnectToAccountDBServer
{
public:
	LConnectToAccountDBServer();
	~LConnectToAccountDBServer();
public:
	bool Initialize(char* pConnectToAccountDBServerFileName);
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
	bool StopConnectToAccountDBServer();
	void ReleaseConnectToAccountDBServerResource();
private:
	//	连接到帐号数据库缓冲服务器
	LConnector m_ConnectToAccountDBServer;
	unsigned int m_unMaxPacketProcessOnce;
};


