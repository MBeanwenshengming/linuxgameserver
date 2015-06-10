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

#include "LGateServerMainLogic.h"
#include "LConnectToMasterServer.h"
#include "../include/Server_To_Server_Packet_Define.h"
#include "LGateServerPacketProcess_Proc.h"

LConnectToMasterServer::LConnectToMasterServer()
{
	m_pGateServerMainLogic 		= NULL;
	m_unMaxPacketProcessOnce 	= 100;
	m_unPacketProcessed		 	= 0; 
}
LConnectToMasterServer::~LConnectToMasterServer()
{
}

bool LConnectToMasterServer::Initialize(char* pConnectToMasterServerConfigFile)
{
	if (m_pGateServerMainLogic == NULL)
	{
		return false;
	}

	if (pConnectToMasterServerConfigFile == NULL)
	{
		return false;
	}
	LConnectorConfigProcessor ccp;
	if (!ccp.Initialize(pConnectToMasterServerConfigFile, "GateServer_To_MasterServer_1"))
	{
		return false;
	}
	if (!m_ConnectorToMasterServer.Initialize(ccp.GetConnectorInitializeParams()))
	{
		return false;
	}
	return true;
}
//	获取接收的数据
LPacketSingle* LConnectToMasterServer::GetOneRecvedPacket()
{
	return m_ConnectorToMasterServer.GetOneRecvedPacket();
}
//	释放一个接收的数据包
void LConnectToMasterServer::FreePacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return ;
	}
	return m_ConnectorToMasterServer.FreePacket(pPacket);
}

//	发送数据
//	取一个发送数据包
LPacketSingle* LConnectToMasterServer::GetOneSendPacketPool(unsigned short usPacketSize)
{
	return m_ConnectorToMasterServer.GetOneSendPacketPool(usPacketSize);
}

//	添加一个发送数据包
bool LConnectToMasterServer::AddOneSendPacket(LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}

	return m_ConnectorToMasterServer.AddOneSendPacket(pPacket);
}


bool LConnectToMasterServer::IsConnectted()
{
	return m_ConnectorToMasterServer.IsConnectted();
}

bool LConnectToMasterServer::StopConnectToMasterServerThread()
{
	return m_ConnectorToMasterServer.StopThreadAndStopConnector();
}

void LConnectToMasterServer::ReleaseConnectToMasterServerThreadResource()
{
	m_ConnectorToMasterServer.ReleaseConnectorResource();
}

LGateServerMainLogic* LConnectToMasterServer::GetGateServerMainLogic()
{
	return m_pGateServerMainLogic;
}

void LConnectToMasterServer::SetGateServerMainLogic(LGateServerMainLogic* pgsml)
{
	m_pGateServerMainLogic = pgsml;
}


void LConnectToMasterServer::ProcessPacket(unsigned int unMaxPacketProcessed)
{
	m_unPacketProcessed = 0;
	if (unMaxPacketProcessed == 0)
	{
		unMaxPacketProcessed = 100;
	}
	for (unsigned int unIndex = 0; unIndex < unMaxPacketProcessed; ++unIndex)
	{ 
		LPacketSingle* pPacket = GetOneRecvedPacket();
		if (pPacket == NULL)
		{
			break;
		} 
		m_unPacketProcessed++;
		if (pPacket->GetPacketType() == 1)
		{
			unsigned short usPacketLen = 128;
			//	连接上来了，发送本服务器的ID到MasterServer
			LPacketSingle* pSendPacket = m_ConnectorToMasterServer.GetOneSendPacketPool(usPacketLen);
			if (pSendPacket == NULL)
			{
				//	error
			}
			else
			{
				//	与MasterServer初始化发送序列
				//	发送服务器ID给MasterServer
				//	uint64_t u64ServerUniqueID = m_pGateServerMainLogic->GetServerUniqueID();
				//	压入数据包
				pSendPacket->SetPacketID(Packet_SS_Start_Req);

				//	pPacket->AddULongLong(u64ServerUniqueID);
				AddOneSendPacket(pSendPacket);
			}
		}
		else
		{ 
			//	分发数据处理包
			E_Packet_From_Type eFromType = E_Packet_From_MasterServer;
			m_pGateServerMainLogic->GetPacketProcessManager()->DispatchMessageProcess(0, pPacket, eFromType);
		}
		//	处理协议包
		//
		//	释放接受到的数据包到缓冲池
		FreePacket(pPacket);
	}
}

void LConnectToMasterServer::ReportServerIDToMasterServer()
{
	uint64_t u64UniqueServerID = m_pGateServerMainLogic->GetServerUniqueID();
	LPacketSingle* pPacket = m_ConnectorToMasterServer.GetOneSendPacketPool(120);
	if (pPacket == NULL)
	{
		return;
	}
	pPacket->SetPacketID(Packet_SS_Register_Server_Req1);
	pPacket->AddULongLong(u64UniqueServerID);

	m_ConnectorToMasterServer.AddOneSendPacket(pPacket); 
}


