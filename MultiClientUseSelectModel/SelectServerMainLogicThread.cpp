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

#include "SelectServerMainLogicThread.h"
#include "LSelectServer.h"


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


SelectServerMainLogicThread::SelectServerMainLogicThread()
{
	m_pSelectServer = NULL;
}

SelectServerMainLogicThread::~SelectServerMainLogicThread()
{
}

int SelectServerMainLogicThread::ThreadDoing(void* pParam)
{
	time_t tLastPrintTime = 0;
	while (true)
	{
		int nRecvedPacket = 0;
		for (;;)
		{
			t_Recved_Packet tRecvedPacket;

			if (m_pSelectServer->GetOneRecvedPacket(tRecvedPacket))
			{
				nRecvedPacket++;
				if (tRecvedPacket.pPacketForRecv == (LPacketSingle*)0xFFFFFFFF)		//	连接 上来
				{
					this->OnAddSession(tRecvedPacket.unSessionID);
				}
				else if (tRecvedPacket.pPacketForRecv == NULL)		//	连接断开
				{
					this->OnSessionDisconnect(tRecvedPacket.unSessionID);
				}
				else	//	接收到数据包
				{
//					LPacketBroadCast* pBroadCastPacket = m_pSelectServer->AllocOneSendPacket(tRecvedPacket.pPacketForRecv->GetDataLen());
//
//					if (pBroadCastPacket != NULL)
//					{
//						pBroadCastPacket->AddData(tRecvedPacket.pPacketForRecv->GetDataBuf() + sizeof(int), tRecvedPacket.pPacketForRecv->GetDataLen() - sizeof(int));
//						//	广播数据包到各个连接
//						map<unsigned int, unsigned int>::iterator _ito = m_mapSessionConnected.begin();
//						while (_ito != m_mapSessionConnected.end())
//						{
//							m_pSelectServer->AddOneSendPacket(_ito->first, pBroadCastPacket);
//							_ito++;
//						}
//						//	发送出去
//						m_pSelectServer->PostAllSendPacket();
//					}
					//	检查接收的数据包是否合法
					printf("SessionID:%u,Recved:%s\n", tRecvedPacket.unSessionID, tRecvedPacket.pPacketForRecv->GetDataBuf() + sizeof(int));
				}
				//	释放接收到的数据包到缓冲池
				if (tRecvedPacket.pPacketForRecv != NULL && tRecvedPacket.pPacketForRecv != (LPacketSingle*)0xFFFFFFFF)
				{
					m_pSelectServer->FreeOneRecvedPacket(tRecvedPacket.pPacketForRecv);
				}
			}
			else
			{
				break;
			}
		}
		//	发送数据包
		time_t tNow = time(NULL);

		map<unsigned int, t_Session_Info>::iterator _ito = m_mapSessionConnected.begin();
		while (_ito != m_mapSessionConnected.end())
		{
			if (tNow > _ito->second.tLastSendPacketTime + _ito->second.nSendInterval)
			{
				int nIndex = rand() % g_globalCount;

				//	发送一个数据包
				LPacketBroadCast* pBroadCastPacket = m_pSelectServer->AllocOneSendPacket(g_narrLen[nIndex]);

				if (pBroadCastPacket != NULL)
				{
					pBroadCastPacket->AddData(pGlobalStatus[nIndex], g_narrLen[nIndex]);
					printf("Session:%u, Send:%s\n", _ito->first, pGlobalStatus[nIndex]);
					m_pSelectServer->AddOneSendPacket(_ito->first, pBroadCastPacket);
				}
				_ito->second.tLastSendPacketTime = tNow;
			}
			_ito++;
		}
		m_pSelectServer->PostAllSendPacket();

		if (nRecvedPacket == 0)
		{
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		if (CheckForStop())		//	线程停止
		{
			break;
		}
		if (tLastPrintTime == 0 || tNow - tLastPrintTime > 5)
		{
			printf("Session Connectted Count:%d\n", m_mapSessionConnected.size());
			m_pSelectServer->PrintBufStatus();
			tLastPrintTime = tNow;
		}
	}
	return 0;
}
bool SelectServerMainLogicThread::OnStart()
{
	return true;
}
void SelectServerMainLogicThread::OnStop()
{

}
void SelectServerMainLogicThread::OnAddSession(unsigned int unSessionID)
{
	t_Session_Info tSessionInfo;
	tSessionInfo.unSessionID = unSessionID;
	tSessionInfo.tLastSendPacketTime = time(NULL);
	int nInterval = rand() % 10;
	tSessionInfo.nSendInterval = nInterval > 0 ? nInterval : 1;

	map<unsigned int, t_Session_Info>::iterator _ito = m_mapSessionConnected.find(unSessionID);
	if (_ito != m_mapSessionConnected.end())
	{
		return;
	}
	m_mapSessionConnected[unSessionID] = tSessionInfo;
}
void SelectServerMainLogicThread::OnSessionDisconnect(unsigned int unSessionID)
{
	map<unsigned int, t_Session_Info>::iterator _ito = m_mapSessionConnected.find(unSessionID);
	if (_ito != m_mapSessionConnected.end())
	{
		m_mapSessionConnected.erase(_ito);
	}
}

void SelectServerMainLogicThread::SetSelectServer(LSelectServer* pSelectServer)
{
	m_pSelectServer = pSelectServer;
}
