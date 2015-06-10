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

#include "LDBServerConnectToMasterServer.h"
#include "LDBServerMainLogicThread.h"
#include "../include/Server_To_Server_Packet_Define.h"

LDBServerConnectToMasterServer::LDBServerConnectToMasterServer()
{
	m_pDBServerMainLogic = NULL;
	m_unMaxPacketProcessOnce = 100;
}
LDBServerConnectToMasterServer::~LDBServerConnectToMasterServer()
{
}

bool LDBServerConnectToMasterServer::Initialize(char* pConnectToDBServerFileName, char* pConfigHeader)
{
	if (m_pDBServerMainLogic == NULL)
	{ 
		return false;
	}
	if (pConnectToDBServerFileName == NULL)
	{
		return false;
	}
	LConnectorConfigProcessor ccp;
	if (!ccp.Initialize(pConnectToDBServerFileName, pConfigHeader))
	{
		return false;
	}
	if (!m_ConnectToMasterServer.Initialize(ccp.GetConnectorInitializeParams()))
	{
		return false;
	}
	return true;
}

void LDBServerConnectToMasterServer::SetDBServerMainLogic(LDBServerMainLogicThread* pdbsml)
{
	m_pDBServerMainLogic = pdbsml;
}

//	获取接收的数据
LPacketSingle* LDBServerConnectToMasterServer::GetOneRecvedPacket()
{
	return m_ConnectToMasterServer.GetOneRecvedPacket();
}
//	释放一个接收的数据包
void LDBServerConnectToMasterServer::FreePacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return ;
	}
	return m_ConnectToMasterServer.FreePacket(pPacket);
}

//	发送数据
//	取一个发送数据包
LPacketSingle* LDBServerConnectToMasterServer::GetOneSendPacketPool(unsigned short usPacketSize)
{
	return m_ConnectToMasterServer.GetOneSendPacketPool(usPacketSize);
}
//	添加一个发送数据包
bool LDBServerConnectToMasterServer::AddOneSendPacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}

	return m_ConnectToMasterServer.AddOneSendPacket(pPacket);
}

bool LDBServerConnectToMasterServer::IsConnectted()
{
	return m_ConnectToMasterServer.IsConnectted();
}

bool LDBServerConnectToMasterServer::StopConnectToMasterServer()
{
	return m_ConnectToMasterServer.StopThreadAndStopConnector();
}
void LDBServerConnectToMasterServer::ReleaseConnectToMasterServerResource()
{
	m_ConnectToMasterServer.ReleaseConnectorResource();
}

void LDBServerConnectToMasterServer::ProcessRecvedPacket()
{
	m_unPacketProcess = 0;
	for (unsigned int unIndex = 0; unIndex < m_unMaxPacketProcessOnce; ++unIndex)
	{
		LPacketSingle* pPacket = GetOneRecvedPacket();
		if (pPacket == NULL)
		{
			return ;
		}

		m_unPacketProcess++;

		if (pPacket->GetPacketType() == 1)
		{
			LPacketSingle* pPacket = GetOneSendPacketPool(120);
			pPacket->SetPacketID(Packet_SS_Start_Req);
			AddOneSendPacket(pPacket);
		}
		else
		{ 
			//	调用处理函数
			E_DBServer_Packet_From_Type eFromType = E_DBServer_Packet_From_Master;
			m_pDBServerMainLogic->GetPacketProcessManager()->DispatchMessageProcess(0, pPacket, eFromType);
		}
		FreePacket(pPacket);
	}
}

void LDBServerConnectToMasterServer::ReportServerIDToMasterServer()
{
	uint64_t u64LobbyServerID = m_pDBServerMainLogic->GetServerUniqueID();
	LPacketSingle* pSendPacket = m_ConnectToMasterServer.GetOneSendPacketPool(120);
	pSendPacket->SetPacketID(Packet_SS_Register_Server_Req1);
	pSendPacket->AddULongLong(u64LobbyServerID);
	AddOneSendPacket(pSendPacket);
}


