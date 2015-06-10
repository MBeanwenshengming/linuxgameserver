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

#include "LConnectToAccountDBServer.h"

LConnectToAccountDBServer::LConnectToAccountDBServer()
{
	m_unMaxPacketProcessOnce = 100;
}

LConnectToAccountDBServer::~LConnectToAccountDBServer()
{
}

bool LConnectToAccountDBServer::Initialize(char* pConnectToAccountDBServerFileName)
{
	if (pConnectToAccountDBServerFileName == NULL)
	{
		return false;
	}
	LConnectorConfigProcessor ccp;
	if (!ccp.Initialize(pConnectToAccountDBServerFileName, "LoginServer_To_AccountDBServer"))
	{
		return false;
	}
	if (!m_ConnectToAccountDBServer.Initialize(ccp.GetConnectorInitializeParams()))
	{
		return false;
	}
	return true;
}

//	获取接收的数据
LPacketSingle* LConnectToAccountDBServer::GetOneRecvedPacket()
{
	return m_ConnectToAccountDBServer.GetOneRecvedPacket();
}

//	释放一个接收的数据包
void LConnectToAccountDBServer::FreePacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return;
	}
	m_ConnectToAccountDBServer.FreePacket(pPacket);
}

//	发送数据
//	取一个发送数据包
LPacketSingle* LConnectToAccountDBServer::GetOneSendPacketPool(unsigned short usPacketSize)
{
	return m_ConnectToAccountDBServer.GetOneSendPacketPool(usPacketSize);
}

//	添加一个发送数据包
bool LConnectToAccountDBServer::AddOneSendPacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}
	return m_ConnectToAccountDBServer.AddOneSendPacket(pPacket);
}

bool LConnectToAccountDBServer::IsConnectted()
{
	return m_ConnectToAccountDBServer.IsConnectted();
}

bool LConnectToAccountDBServer::StopConnectToAccountDBServer()
{
	return m_ConnectToAccountDBServer.StopThreadAndStopConnector();
}
void LConnectToAccountDBServer::ReleaseConnectToAccountDBServerResource()
{
	return m_ConnectToAccountDBServer.ReleaseConnectorResource();
}


