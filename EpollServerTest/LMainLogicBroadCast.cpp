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
#include <sys/prctl.h>

int g_globalCount = 7;
char *pGlobalStatus[] =
{
	"this is Test Client!!",
	"We Will go There, you can go too, would you like toghter? you said,haha, it is a bad idea!!!superman coming retreat??",
	"The connection to the server was reset while the page was loading[[",
	"If your computer or network is protected by a firewall or proxy, make sure that Firefox is permitted to access the Web]]",
	"If you are unable to load any pages, check your computer's network connection==",
	"The reference library is only one of the services that you can access through the Red Hat customer portal. You can also use the portal to++",
	"Click on a product in the menu to the left, then click a product version and a document title. Red Hat documentation has been translated into twenty-two languages and is available in multi-page HTML, single-page HTML, PDF, and EPUB formats..",
};
int g_narrLen[] =
{
	strlen(pGlobalStatus[0]),
	strlen(pGlobalStatus[1]),
	strlen(pGlobalStatus[2]),
	strlen(pGlobalStatus[3]),
	strlen(pGlobalStatus[4]),
	strlen(pGlobalStatus[5]),
	strlen(pGlobalStatus[6]),
};


LMainLogicBroadCast::LMainLogicBroadCast()
{
 	m_tLastWriteTime = 0;
	m_nSendPacketFreeCount = 0;
	m_nLastBroadCastID	= 0;
}
LMainLogicBroadCast::~LMainLogicBroadCast()
{

}

bool LMainLogicBroadCast::Initialize(char* pszConfigFile)
{
	if (pszConfigFile == NULL)
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
	if (!this->InitializeNetWork(pszConfigFile, 500, "Test", true))
	{
		return false;
	}
	return true;
}


bool LMainLogicBroadCast::OnAddSession(uint64_t u64SessionID, t_Session_Accepted& tsa)
{
	t_Session_Info tSessionInfo;
	tSessionInfo.u64SessionID 		= u64SessionID;
	tSessionInfo.unSendThreadID 	= tsa.nSendThreadID;

	map<uint64_t, t_Session_Info>::iterator _ito = m_mapSessionManaged.find(u64SessionID);
	if (_ito != m_mapSessionManaged.end())
	{
		return false;
	}
	m_mapSessionManaged[u64SessionID] = tSessionInfo;
	return true;
}
void LMainLogicBroadCast::OnRemoveSession(uint64_t u64SessionID)
{ 
	map<uint64_t, t_Session_Info>::iterator _ito = m_mapSessionManaged.find(u64SessionID);
	if (_ito != m_mapSessionManaged.end())
	{
		m_mapSessionManaged.erase(_ito);
	}
}
void LMainLogicBroadCast::OnRecvedPacket(t_Recv_Packet& tRecvedPacket)
{
	unsigned short usRecvPackLen = tRecvedPacket.pPacket->GetDataLen() + sizeof(unsigned short) + sizeof(int);
//	LPacketBroadCast* pPacketForSend = this->GetOneSendPacket(usRecvPackLen);
//
//	//printf("received data:%s\n", tRecvedPacket.pPacket->GetDataBuf() + sizeof(int));
//
//	if (pPacketForSend != NULL)
//	{
//		bool bFind = false;
//		for (int i = 0; i < g_globalCount; ++i)
//		{
//			if (tRecvedPacket.pPacket->GetDataLen() - sizeof(int) == g_narrLen[i])
//			{
//				bFind = true;
//			}
//		}
//		if (!bFind)
//		{
//			printf("error Received:%d\n", tRecvedPacket.pPacket->GetDataLen() - sizeof(int));
//		}

//		pPacketForSend->AddData(tRecvedPacket.pPacket->GetDataBuf() + sizeof(int), tRecvedPacket.pPacket->GetDataLen() - sizeof(int));
		if (!tRecvedPacket.pPacket->CheckCRC32Code())
		{
			printf("error Received Packet CRC32Code, Error, sessionID:%llu\n", tRecvedPacket.u64SessionID);
		}

		char cPacketType = 0;
		tRecvedPacket.pPacket->GetChar(cPacketType);

		int nLastSendIndex = 0;
		tRecvedPacket.pPacket->GetInt(nLastSendIndex);

		map<uint64_t, t_Session_Info>::iterator _itoSession = m_mapSessionManaged.find(tRecvedPacket.u64SessionID);
		if (_itoSession == m_mapSessionManaged.end())
		{
			printf("Error Session Not Finded!!!");
			return ;
		}
		if (_itoSession->second.nLastRecvPacketID != nLastSendIndex)
		{
			printf("Session Packet Error , SessionID:%llu\n", tRecvedPacket.u64SessionID);
			return ;
		}
		_itoSession->second.nLastRecvPacketID++;

		unsigned int unSendCount = 0;
		if (random() % 10 > 11)
		{
			unsigned short usPacketLen = 11 + random() % 1000;

			LPacketSingle packetToSend(usPacketLen);
			BuildRandomPacket(&packetToSend, true, m_nLastBroadCastID);
			map<uint64_t, t_Session_Info>::iterator _ito = m_mapSessionManaged.begin();
			while(_ito != m_mapSessionManaged.end())
			{
				if (!SendOnePacket(_ito->first, _ito->second.unSendThreadID, &packetToSend))
				{
					char szError[512];
					sprintf(szError, "LMainLogicBroadCast::ThreadDoing, SendPacket Failed\n");
					g_ErrorWriter.WriteError(szError, strlen(szError));
				}
				unSendCount++;
				if (unSendCount > 30)
				{
					FlushSendPacket();
					unSendCount = 0;
				}
				_ito++;
			}
		}
		else
		{

			unsigned short usPacketLen = 11 + random() % 1000;

			LPacketSingle packetToSend(usPacketLen);
			BuildRandomPacket(&packetToSend, false, nLastSendIndex);
			SendOnePacket(_itoSession->first, _itoSession->second.unSendThreadID, &packetToSend);
			unSendCount++;

		}
		//	将数据包发送出去
		if (unSendCount > 0)
		{
			FlushSendPacket();
		}
//	}
}

int LMainLogicBroadCast::ThreadDoing(void* pParam)
{
	char szThreadName[128];
	sprintf(szThreadName, "MainLogicBroadCastThread");
	prctl(PR_SET_NAME, szThreadName);

	time_t tTimeToPrint = 0;
	while(1)
	{
		ProcessRecvedPacket();
		if (this->GetPacketProcessed() == 0)
		{
			//	sched_yield();
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}

		if (CheckForStop())
		{
			break;
		}
		time_t tNow = time(NULL);
		if (tTimeToPrint == 0 || tNow - tTimeToPrint > 5)
		{
			//	NetWork __EPOLL_TEST_STATISTIC__
			printf("SessionCount:%d,", m_mapSessionManaged.size());
			this->GetNetWorkServices()->PrintfAllocAndFreeCount();
			tTimeToPrint = tNow;
			this->GetNetWorkServices()->KickOutIdleSession();

			this->GetNetWorkServices()->GetSessionManager().PrintAllSessionInfos();
			this->GetNetWorkServices()->PrintRefCountInfoForAll();
		}
	} 
	return 0;
}
bool LMainLogicBroadCast::OnStart()
{
	this->NetWorkStart();
	return true;
}
void LMainLogicBroadCast::OnStop()
{
	this->NetWorkDown();
}


bool LMainLogicBroadCast::BuildRandomPacket(LPacketSingle* pPacket, bool bBroadCast, int& nPacketSendIndex)
{
	if (pPacket == NULL)
	 {
	  return false;
	 }
	 unsigned short usReserveBytes = 4 + 2 + 1 + 4;
	 unsigned short usPacketBufLenAllBytes = pPacket->GetPacketBufLen();
	 if (usPacketBufLenAllBytes < usReserveBytes)
	 {
	  return false;
	 }
	 // 留下4字节填写CRC32码和2字节包头，1字节数据包类型(单播0，广播1)，4字节的数据包顺序
	 unsigned short usPacketRandomBufLen = pPacket->GetPacketBufLen() - usReserveBytes;
	 if (usPacketRandomBufLen >= 8000)
	 {
	  return false;
	 }

	 unsigned short usPacketLenVar = usPacketRandomBufLen;
	 char szPacketbuf[8000]; memset(szPacketbuf, 0, sizeof(szPacketbuf));
	 int nLen = 0;
	 while (usPacketLenVar--)
	 {
	  if (rand() % 2 == 0)
	  {
	   szPacketbuf[nLen] = 'a' + rand() % 26;
	  }
	  else
	  {
	   szPacketbuf[nLen] = 'A' + rand() % 26;
	  }
	  nLen++;
	 }

	 char cPacketType = bBroadCast ? 1 : 0;
	 int npacketID = nPacketSendIndex;

	 pPacket->AddChar(cPacketType);
	 pPacket->AddInt(npacketID);
	 pPacket->AddData(szPacketbuf, usPacketRandomBufLen);
	 pPacket->MakeCRC32CodeToPacket();
	 nPacketSendIndex++;

	 pPacket->CheckCRC32Code();
	 return true;
}
