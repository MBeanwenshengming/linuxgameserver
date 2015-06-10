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

#include "LMainLogicBroadCast.h"
#include "LNetWorkServices.h"	

LMainLogicBroadCast::LMainLogicBroadCast()
{
 	m_tLastWriteTime = 0;
	m_pNetWorkServices = NULL;
	m_nSendPacketFreeCount = 0;
}
LMainLogicBroadCast::~LMainLogicBroadCast()
{

}

bool LMainLogicBroadCast::Initialize()
{
	if (m_pNetWorkServices == NULL)
	{
		return false;
	}
	char szFileName[256];
	sprintf(szFileName, "RecvGlobalStatusInfo.txt");
	if (!m_RecvStatusWriter.Initialize(szFileName))
	{
		return false;
	}

	sprintf(szFileName, "SendGlobalStatusInfo.txt");
	if (!m_SendStatusWriter.Initialize(szFileName))
	{
		return false;
	}
	return true;
}
void LMainLogicBroadCast::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices = pNetWorkServices;
}

bool LMainLogicBroadCast::AddSession(uint64_t u64SessionID)
{
	map<uint64_t, uint64_t>::iterator _ito = m_mapSessionManaged.find(u64SessionID);
	if (_ito != m_mapSessionManaged.end())
	{
		return false;
	}
	m_mapSessionManaged[u64SessionID] = u64SessionID;
	return true;
}
void LMainLogicBroadCast::RemoveSession(uint64_t u64SessionID)
{ 
	map<uint64_t, uint64_t>::iterator _ito = m_mapSessionManaged.find(u64SessionID);
	if (_ito != m_mapSessionManaged.end())
	{
		m_mapSessionManaged.erase(_ito);
	}
}

int LMainLogicBroadCast::ThreadDoing(void* pParam)
{
//	int nSendCount = 0;
//	int nSessionCount = 0;
//	time_t timeToPrintSessionCount = 0;
//	//	int nSendPacketTestCount = 0;
//	while(1)
//	{
//		t_Recv_Packet tRecvPacket;
//		bool bGetPacketSuccess = m_pNetWorkServices->GetOneRecvedPacket(&tRecvPacket);
//		if (bGetPacketSuccess == false)
//		{
//			//	sched_yield();
//			struct timespec timeReq;
//			timeReq.tv_sec 	= 0;
//			timeReq.tv_nsec = 10;
//			nanosleep(&timeReq, NULL);
//		}
//		else
//		{
//			if (tRecvPacket.pPacket == NULL)		//	 you yi ge lian jie duan kai
//			{
//				//	qu diao you xi luo ji guan li de yi ge lian jie
//				//	nSessionCount--;
//				RemoveSession(tRecvPacket.u64SessionID);
//			}
//			else if (tRecvPacket.pPacket == (LPacketSingle*)0xFFFFFFFF)		//	 you yi ge lian jie shang lai
//			{
//				//	zeng jia yi ge you xi luo ji guan li de lian jie
//				//	nSessionCount++;
//				if (!AddSession(tRecvPacket.u64SessionID))
//				{
//					char szError[512];
//					sprintf(szError, "LMainLogicBroadCast::ThreadDoing, AddSession Failed\n");
//					g_ErrorWriter.WriteError(szError, strlen(szError));
//				}
//			}
//			else			//	 chu li shou dao de shu ju bao
//			{
//					//	unsigned short usRecvPacketDataLen = tRecvPacket.pPacket->GetPacketDataLen();
//				unsigned short usRecvPackLen = tRecvPacket.pPacket->GetPacketDataAndHeaderLen();
//				LPacketBroadCast* pPacketForSend = m_pNetWorkServices->RequestOnePacket(usRecvPackLen);
//				if (pPacketForSend != NULL)		//	  tian chong shu ju, fa song shu ju
//				{
//					pPacketForSend->AddData(tRecvPacket.pPacket->GetDataBuf(), tRecvPacket.pPacket->GetDataLen());
//
//					/*
//					if (!m_pNetWorkServices->SendPacket(tRecvPacket.u64SessionID, pPacketForSend))
//					{
//						char szError[512];
//						sprintf(szError, "LMainLogicBroadCast::ThreadDoing, SendPacket Failed\n");
//						g_ErrorWriter.WriteError(szError, strlen(szError));
//					}
//					*/
//					//nSendPacketTestCount++;
//					//printf("Send Packet:%d\n",nSendPacketTestCount);
//					//int nRandNum = rand() % 100;
//					//if (nRandNum >= 80)
//					//{
//						map<uint64_t, uint64_t>::iterator _ito = m_mapSessionManaged.begin();
//						while(_ito != m_mapSessionManaged.end())
//						{
//			//				if (!m_pNetWorkServices->SendPacket(_ito->first, pPacketForSend))
//			//				{
//			//					char szError[512];
//			//					sprintf(szError, "LMainLogicBroadCast::ThreadDoing, SendPacket Failed\n");
//			//					g_ErrorWriter.WriteError(szError, strlen(szError));
//			//				}
//							_ito++;
//							nSendCount++;
//						}
//					//}
//					//else
//					//{
//					//	if (!m_pNetWorkServices->SendPacket(tRecvPacket.u64SessionID, pPacketForSend))
//					//	{
//					//		char szError[512];
//					//		sprintf(szError, "LMainLogicBroadCast::ThreadDoing, SendPacket Failed\n");
//					//		g_ErrorWriter.WriteError(szError, strlen(szError));
//					//	}
//					//}
//					if (pPacketForSend->GetCurRefCount() == 0)	//	  shuo ming mei you yige lian jie yin yong gai bao ,na me zhi jie shi fang
//					{
//						m_nSendPacketFreeCount++;
//						delete 	pPacketForSend;
//					}
//					if (nSendCount >= 30)
//					{
//						m_pNetWorkServices->FlushAllPacketToSend();
//						nSendCount = 0;
//					}
//				}
//				else
//				{
//					char szError[512];
//					sprintf(szError, "LMainLogicBroadCast::ThreadDoing, RequestOnePacket Failed\n");
//					g_ErrorWriter.WriteError(szError, strlen(szError));
//				}
//				//	hui shou jie shou de shu ju bao
//				if (!m_pNetWorkServices->FreeRecvedPacket(tRecvPacket.pPacket))
//				{
//					char szError[512];
//					sprintf(szError, "LMainLogicBroadCast::ThreadDoing, FreeRecvedPacket Failed\n");
//					g_ErrorWriter.WriteError(szError, strlen(szError));
//					//	 mei you cheng gong tian jia dao dui lie zhong ,na me zhi jie shi fang nei cun
//					delete tRecvPacket.pPacket;
//				}
//			}
//		}
//		if (nSendCount > 0)
//		{
//			m_pNetWorkServices->FlushAllPacketToSend();
//			nSendCount = 0;
//		}
//		if (CheckForStop())
//		{
//			break;
//		}
//		//	for Test
//		time_t tNow = time(NULL);
//		if (tNow - m_tLastWriteTime > 60)
//		{
//			m_pNetWorkServices->PrintNetWorkServiceBufStatus(&m_RecvStatusWriter, &m_SendStatusWriter);
//			m_tLastWriteTime = tNow;
//		}
//		if (tNow - timeToPrintSessionCount > 10)
//		{
//			printf("SessionCount=%d, FreeSendCount:%d\n", m_mapSessionManaged.size(), m_nSendPacketFreeCount);
//			timeToPrintSessionCount = tNow;
//			m_pNetWorkServices->PrintRefCountInfoForAll();
//		}
//		m_pNetWorkServices->KickOutIdleSession();
//	}
	return 0;
}
bool LMainLogicBroadCast::OnStart()
{
	return true;
}
void LMainLogicBroadCast::OnStop()
{
}
