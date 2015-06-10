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

#include "LConnectorTest.h"
#include "stdio.h"
#include <string>

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

LConnectorTest::LConnectorTest()
{
	m_tLastSendData = 0;
}

LConnectorTest::~LConnectorTest()
{
}

bool LConnectorTest::Initialize(char* pConfigFileName)
{
	if (pConfigFileName == NULL)
	{
		return false;
	}
	if (!m_ConfigProcessor.Initialize(pConfigFileName, "Test", true))
	{
		return false;
	}
	if (!m_Connector.Initialize(m_ConfigProcessor.GetConnectorInitializeParams()))
	{
		return false;
	}
	return true;
}

int LConnectorTest::ThreadDoing(void* pParam)
{
	while (1)
	{
		LPacketSingle* pPacket = NULL;
		pPacket = m_Connector.GetOneRecvedPacket();

		if (pPacket != NULL)
		{

			if (pPacket->GetPacketType() == 1)
			{
				printf("Server Connectted\n");
			}
			else
			{
				if (!pPacket->CheckCRC32Code())
				{
					printf("Packet Receive CrC32Code Error\n");
				}
				//	处理接收的数据包
				//int nDataLen = pPacket->GetDataLen() - sizeof(int);
				//	printf("%hd, %s\n", pPacket->GetPacketDataAndHeaderLen(), pPacket->GetBuf());
				printf("Recv:%s\n", pPacket->GetDataBuf());
//				bool bFinded = false;
//				for (int i = 0; i < g_globalCount; ++i)
//				{
//					if (nDataLen == g_narrLen[i])
//					{
//						bFinded = true;
//						break;
//					}
//				}
//				if (!bFinded)
//				{
//					printf("not found\n");
//				}
			}
			//	回收处理完的数据包
			m_Connector.FreePacket(pPacket); 
		}
		else
		{
			//sched_yield(); 
			struct timespec timeReq;
			timeReq.tv_sec 	= 0;
			timeReq.tv_nsec = 10;
			nanosleep(&timeReq, NULL);
		}
		time_t tNow = time(NULL);
		//	查看是否需要发送数据包
		if (m_tLastSendData == 0 || tNow - m_tLastSendData >= 1)
		{
			int nSelectedIndex = rand() % g_globalCount;
			int nDataLen = strlen(pGlobalStatus[nSelectedIndex]);

			LPacketSingle* pPacketForSend = NULL;
			pPacketForSend = m_Connector.GetOneSendPacketPool(nDataLen + sizeof(unsigned short) + sizeof(unsigned long));
			if (pPacketForSend == NULL)
			{
				printf("pPacketForSend == NULL\n");
				return -1;
			}
			pPacketForSend->Reset();
			pPacketForSend->AddData(pGlobalStatus[nSelectedIndex], nDataLen);
			pPacketForSend->MakeCRC32CodeToPacket();

			if (!m_Connector.AddOneSendPacket(pPacketForSend))
			{
				printf("!m_Connector.AddOneSendPacket Failed\n");
				return -1;
			}
			printf("Send:%s\n", pPacketForSend->GetDataBuf());
			m_tLastSendData = tNow;
		}
	}
	return 0;
}
bool LConnectorTest::OnStart()
{
	return true;
}
void LConnectorTest::OnStop()
{
	m_Connector.Stop();
	return ;
}

