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
//					if (pBroadCastPacket != NULL)
//					{
//						pBroadCastPacket->AddData(tRecvedPacket.pPacketForRecv->GetDataBuf() + sizeof(int), tRecvedPacket.pPacketForRecv->GetDataLen() - sizeof(int));
						//	广播数据包到各个连接
						map<unsigned int, unsigned int>::iterator _ito = m_mapSessionConnected.begin();
						while (_ito != m_mapSessionConnected.end())
						{
//							m_pSelectServer->AddOneSendPacket(_ito->first, pBroadCastPacket);
							if (!m_pSelectServer->SendPacket(_ito->first, tRecvedPacket.pPacketForRecv))
							{
								m_pSelectServer->AddOneCloseSessionWork(_ito->first);
							}
							_ito++;
						}
						//	发送出去
//						m_pSelectServer->PostAllSendPacket();
					//}
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
		time_t tNow = time(NULL);
		if (tLastPrintTime == 0 || tNow - tLastPrintTime > 5)
		{
//			m_pSelectServer->PrintBufStatus();
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
	map<unsigned int, unsigned int>::iterator _ito = m_mapSessionConnected.find(unSessionID);
	if (_ito != m_mapSessionConnected.end())
	{
		return;
	}
	m_mapSessionConnected[unSessionID] = unSessionID;
}
void SelectServerMainLogicThread::OnSessionDisconnect(unsigned int unSessionID)
{
	map<unsigned int, unsigned int>::iterator _ito = m_mapSessionConnected.find(unSessionID);
	if (_ito != m_mapSessionConnected.end())
	{
		m_mapSessionConnected.erase(_ito);
	}
}

void SelectServerMainLogicThread::SetSelectServer(LSelectServer* pSelectServer)
{
	m_pSelectServer = pSelectServer;
}
