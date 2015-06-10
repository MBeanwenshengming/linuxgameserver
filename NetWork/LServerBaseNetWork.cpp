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

#include "LNetWorkServices.h"
#include "LServerBaseNetWork.h"
#include "LNetWorkConfigFileProcessor.h"
#include "LErrorWriter.h"

bool g_bEpollETEnabled = false;
int g_nRecvBufLen = 8 * 1024;
int g_nSendBufLen = 128 * 1024;

LErrorWriter g_ErrorWriter;

//LSendPacketContainer::LSendPacketContainer()
//{
//	//	LNetWorkServices* m_pNetWorkServices;
//	//	LPacketBroadCast* m_pSendPacket;
//	m_pNetWorkServices	= NULL;
//	m_pSendPacket		= NULL;
//}
//LSendPacketContainer::~LSendPacketContainer()
//{
//	if (m_pNetWorkServices != NULL && m_pSendPacket != NULL)
//	{
//		m_pNetWorkServices->FlushAllPacketToSend();
//	}
//	m_pNetWorkServices = NULL;
//	m_pSendPacket = NULL;
//}
//void LSendPacketContainer::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
//{
//	m_pNetWorkServices = pNetWorkServices;
//}
//void LSendPacketContainer::SetSendPacket(LPacketBroadCast* pSendPacket)
//{
//	m_pSendPacket = pSendPacket;
//}
//LPacketBroadCast* LSendPacketContainer::GetSendPacket()
//{
//	return m_pSendPacket;
//}
//


LServerBaseNetWork::LServerBaseNetWork()
{
	//LNetWorkServices* m_pNetWorkServices;
	//unsigned int m_unMaxNumProcessPacketOnce;		//	一次最大处理的数据包数量
	//
	m_pNetWorkServices 			= NULL;
	m_unMaxNumProcessPacketOnce = 500;		//	一次最大处理的数据包数量
	m_unPacketProcessed			= 0;
}
LServerBaseNetWork::~LServerBaseNetWork()
{
}

bool LServerBaseNetWork::InitializeNetWork(char* pConfigFile, unsigned int unMaxNumProcessPacketOnce, char* pSectionHeader, bool bInitializeAccept)
{
	char* pErrorFileName = "NetWorkError.txt";
	if (!g_ErrorWriter.Initialize(pErrorFileName))
	{
		return false;
	}
	if (pConfigFile == NULL)
	{
		return false;
	}
	LNetWorkConfigFileProcessor nwcfp;
	if (!nwcfp.ReadConfig(pConfigFile, pSectionHeader))
	{
		return false;
	}
	m_pNetWorkServices = new LNetWorkServices;
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	if (!m_pNetWorkServices->Initialize(nwcfp.GetNetWorkServicesParams(), bInitializeAccept))
	{
		return false;
	}
	if (unMaxNumProcessPacketOnce != 0)
	{
		m_unMaxNumProcessPacketOnce = unMaxNumProcessPacketOnce;
	}
	return true;
}

bool LServerBaseNetWork::NetWorkStart()
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	return m_pNetWorkServices->Start();
}

void LServerBaseNetWork::NetWorkDown()
{
	if (m_pNetWorkServices != NULL)
	{
		m_pNetWorkServices->Stop();
		m_pNetWorkServices->ReleaseNetWorkServicesResource();
		delete m_pNetWorkServices;
	}
}

bool LServerBaseNetWork::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	return true;
}
void LServerBaseNetWork::OnRemoveSession(uint64_t u64SessionID)
{
	return ;
}
void LServerBaseNetWork::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	return ;
}
void LServerBaseNetWork::ProcessRecvedPacket()
{
	//	正确处理连接断开
	m_pNetWorkServices->ProcessWillCloseSessionInMainLogic();

	m_unPacketProcessed = 0;
	for (unsigned int uni = 0; uni < m_unMaxNumProcessPacketOnce; ++uni)
	{
		t_Recv_Packet tRecvPacket;
		bool bGetPacketSuccess = m_pNetWorkServices->GetOneRecvedPacket(&tRecvPacket);
		if (bGetPacketSuccess == false)
		{
			return ;
		}
		else
		{
			m_unPacketProcessed++;
			if (tRecvPacket.pPacket == NULL)
			{
				OnRemoveSession(tRecvPacket.u64SessionID);
				continue;
			}
			if (tRecvPacket.pPacket->GetPacketType() == 1)
			{
				t_Session_Accepted tsa;
				memset(&tsa, 0, sizeof(tsa));
				BuildSessionAcceptedPacket(tsa, tRecvPacket.pPacket);
				OnAddSession(tRecvPacket.u64SessionID, tsa);
				m_pNetWorkServices->FreeAcceptThreadPacket(tRecvPacket.pPacket);
			}
			else
			{
				OnRecvedPacket(tRecvPacket);
				m_pNetWorkServices->FreeRecvedPacket(tRecvPacket.pPacket);
			}
		}
	}
}
//	返回一个
//bool LServerBaseNetWork::GetOneSendPacket(unsigned short usDataLen, LSendPacketContainer& spc)
//{
//	spc.SetSendPacket(NULL);
//	spc.SetNetWorkServices(NULL);
//	LPacketBroadCast* pPacket = m_pNetWorkServices->RequestOnePacket(usDataLen);
//	if (pPacket == NULL)
//	{
//		return false;
//	}
//	spc.SetNetWorkServices(m_pNetWorkServices);
//	spc.SetSendPacket(pPacket);
//	return true;
//}

LPacketBroadCast* LServerBaseNetWork::GetOneSendPacket(unsigned short usDataLen)
{
#ifdef __ADD_SEND_BUF_CHAIN__
	return m_pNetWorkServices->RequestOnePacket(usDataLen);
#else
	return NULL;
#endif
}

//bool LServerBaseNetWork::SendOnePacket(uint64_t u64SessionID, int nSendThreadID, LSendPacketContainer* pPacketContainerForSend)
//{
//	if (u64SessionID == 0)
//	{
//		return false;
//	}
//	if (pPacketContainerForSend == NULL)
//	{
//		return false;
//	}
//	return m_pNetWorkServices->SendPacket(u64SessionID, nSendThreadID, pPacketContainerForSend->GetSendPacket());
//}
#ifdef __ADD_SEND_BUF_CHAIN__
bool LServerBaseNetWork::SendOnePacket(uint64_t u64SessionID, int nSendThreadID, LPacketBroadCast* pPacket)
{ 
	if (pPacket == NULL)
	{
		return false;
	}
	return m_pNetWorkServices->SendPacket(u64SessionID, nSendThreadID, pPacket);
}
#endif
#ifdef __USE_SESSION_BUF_TO_SEND_DATA__
bool LServerBaseNetWork::SendOnePacket(uint64_t u64SessionID, int nSendThreadID, LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}
	return m_pNetWorkServices->SendPacket(u64SessionID, nSendThreadID, pPacket);
}
#endif
void LServerBaseNetWork::KickOutOneSession(uint64_t u64SessionID)
{
	if (u64SessionID == 0)
	{
		return ;
	}
	m_pNetWorkServices->KickOutSession(u64SessionID);
}
//	将在本地缓存的数据包发送出去,在确认需要发送的连接全部调用过SendOnePacket后，调用该函数，因为发送的数据包时使用的是引用记数，如果提前调用，那么数据包有可能已经回收
void LServerBaseNetWork::FlushSendPacket()
{
	m_pNetWorkServices->FlushAllPacketToSend();
}


void LServerBaseNetWork::BuildSessionAcceptedPacket(t_Session_Accepted& tsa, LPacketSingle* pPacket)
{
	memset(&tsa, 0, sizeof(tsa));

	char szRemoteIP[20]; memset(szRemoteIP, 0, sizeof(szRemoteIP));
	unsigned short usRemotePort = 0;
	int nRecvThreadID = -1;
	int nSendThreadID = -1;

	pPacket->GetData(szRemoteIP, 20);
	pPacket->GetUShort(usRemotePort);
	pPacket->GetInt(nRecvThreadID);
	pPacket->GetInt(nSendThreadID); 

	strncpy(tsa.szIp, szRemoteIP, sizeof(tsa.szIp) - 1);
	tsa.usPort 			= usRemotePort;
	tsa.nRecvThreadID 	= nRecvThreadID;
	tsa.nSendThreadID 	= nSendThreadID; 

	unsigned short usExtDataLen = 0;
	pPacket->GetUShort(usExtDataLen);
	if (usExtDataLen != 0 && usExtDataLen < 7 * 1024)
	{
		pPacket->GetData(tsa.szExtData, usExtDataLen);
	}
}

void LServerBaseNetWork::GetListenIpAndPort(char* pBuf, unsigned int unBufSize, unsigned short& usPort)
{
	m_pNetWorkServices->GetListenIpAndPort(pBuf, unBufSize, usPort);
}


