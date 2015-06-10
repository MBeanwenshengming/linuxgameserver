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

#include "LLBConnectToGameDBServer.h"
#include "../NetWork/LPacketSingle.h" 
#include "LLobbyServerMainLogic.h"

LLBConnectToGameDBServer::LLBConnectToGameDBServer()
{
	m_unPacketProcessed = 0;
	m_unMaxPacketProcessOnce = 100;
	m_pLobbyServerMainLogic = NULL;
}

LLBConnectToGameDBServer::~LLBConnectToGameDBServer()
{
}

bool LLBConnectToGameDBServer::Initialize(char* pConnectToGameDBServerFileName)
{
	if (m_pLobbyServerMainLogic == NULL)
	{
		return false;
	}
	if (pConnectToGameDBServerFileName == NULL)
	{
		return false;
	}

	LConnectorConfigProcessor ccp;
	if (!ccp.Initialize(pConnectToGameDBServerFileName, "LobbyServer_To_GameDBServer_1"))
	{
		return false;
	}
	if (!m_ConnectToGameDBServer.Initialize(ccp.GetConnectorInitializeParams()))
	{
		return false;
	}
	return true;
}

void LLBConnectToGameDBServer::SetLobbyServerMainLogic(LLobbyServerMainLogic* plsml)
{
	m_pLobbyServerMainLogic = plsml;
}


//	获取接收的数据
LPacketSingle* LLBConnectToGameDBServer::GetOneRecvedPacket()
{
	return m_ConnectToGameDBServer.GetOneRecvedPacket();
}
//	释放一个接收的数据包
void LLBConnectToGameDBServer::FreePacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return ;
	}
	return m_ConnectToGameDBServer.FreePacket(pPacket);
}

//	发送数据
//	取一个发送数据包
LPacketSingle* LLBConnectToGameDBServer::GetOneSendPacketPool(unsigned short usPacketSize)
{
	return m_ConnectToGameDBServer.GetOneSendPacketPool(usPacketSize);
}

//	添加一个发送数据包
bool LLBConnectToGameDBServer::AddOneSendPacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}
	return m_ConnectToGameDBServer.AddOneSendPacket(pPacket);
}

bool LLBConnectToGameDBServer::IsConnectted()
{
	return m_ConnectToGameDBServer.IsConnectted();
}

bool LLBConnectToGameDBServer::StopConnectToGameDBServer()
{
	return m_ConnectToGameDBServer.StopThreadAndStopConnector();
}

void LLBConnectToGameDBServer::ReleaseConnectToGameDBServerResource()
{
	m_ConnectToGameDBServer.ReleaseConnectorResource();
}

void LLBConnectToGameDBServer::ProcessRecvedPacket()
{
	m_unPacketProcessed = 0;
	for (unsigned int unIndex = 0; unIndex < m_unMaxPacketProcessOnce; ++unIndex)
	{
		LPacketSingle* pPacket = GetOneRecvedPacket();
		if (pPacket == NULL)
		{
			return;
		}
		m_unPacketProcessed++;
		if (pPacket->GetPacketType() == 1)
		{
			LPacketSingle* pPacket = GetOneSendPacketPool(120);
			pPacket->SetPacketID(Packet_SS_Start_Req);
			AddOneSendPacket(pPacket);
		}
		else
		{
			//调用处理函数 
			E_LobbyServer_Packet_From_Type eFromType = E_LobbyServer_Packet_From_GameDBServer;
			m_pLobbyServerMainLogic->GetPacketProcessProcManager()->DispatchMessageProcess(0, pPacket, eFromType);
		}

		FreePacket(pPacket);
	}
}



