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

#include "LMainLogicThread.h"
#include "LNetWorkServices.h"	

LMainLogicThread::LMainLogicThread()
{
	m_pNetWorkServices = NULL;
	m_tLastWriteTime = 0;
}
LMainLogicThread::~LMainLogicThread()
{
}
int LMainLogicThread::ThreadDoing(void* pParam)
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
//			sched_yield();
//		}
//		else
//		{
//			if (tRecvPacket.pPacket == NULL)		//	 you yi ge lian jie duan kai
//			{
//				//	qu diao you xi luo ji guan li de yi ge lian jie
//				nSessionCount--;
//			}
//			else if (tRecvPacket.pPacket == (LPacketSingle*)0xFFFFFFFF)		//	 you yi ge lian jie shang lai
//			{
//				//	zeng jia yi ge you xi luo ji guan li de lian jie
//				nSessionCount++;
//			}
//			else			//	 chu li shou dao de shu ju bao
//			{
//				unsigned short usRecvPacketDataLen = tRecvPacket.pPacket->GetDataLen();
//				LPacketBroadCast* pPacketForSend = m_pNetWorkServices->RequestOnePacket(usRecvPacketDataLen);
//				if (pPacketForSend != NULL)		//	  tian chong shu ju, fa song shu ju
//				{
//					pPacketForSend->AddData(tRecvPacket.pPacket->GetDataBuf(), tRecvPacket.pPacket->GetDataLen());
//				//	if (!m_pNetWorkServices->SendPacket(tRecvPacket.u64SessionID, pPacketForSend))
//				//	{
//				//		char szError[512];
//				//		sprintf(szError, "LMainLogicThread::ThreadDoing, SendPacket Failed\n");
//				//		g_ErrorWriter.WriteError(szError, strlen(szError));
//				//	}
//					//nSendPacketTestCount++;
//					//printf("Send Packet:%d\n",nSendPacketTestCount);
//					nSendCount++;
//					if (nSendCount >= 30)
//					{
//						m_pNetWorkServices->FlushAllPacketToSend();
//						nSendCount = 0;
//					}
//				}
//				else
//				{
//					char szError[512];
//					sprintf(szError, "LMainLogicThread::ThreadDoing, RequestOnePacket Failed\n");
//					g_ErrorWriter.WriteError(szError, strlen(szError));
//				}
//				//	hui shou jie shou de shu ju bao
//				if (!m_pNetWorkServices->FreeRecvedPacket(tRecvPacket.pPacket))
//				{
//					char szError[512];
//					sprintf(szError, "LMainLogicThread::ThreadDoing, FreeRecvedPacket Failed\n");
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
//			printf("SessionCount=%d\n", nSessionCount);
//			timeToPrintSessionCount = tNow;
//			m_pNetWorkServices->PrintRefCountInfoForAll();
//		}
//		m_pNetWorkServices->KickOutIdleSession();
//	}
	return 0;
}
bool LMainLogicThread::OnStart()
{
	return true;
}
void LMainLogicThread::OnStop()
{
}

bool LMainLogicThread::Initialize()
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


void LMainLogicThread::SetNetWorkServices(LNetWorkServices* pNetWorkServices)	
{
	m_pNetWorkServices = pNetWorkServices;
}

