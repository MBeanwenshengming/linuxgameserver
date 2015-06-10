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

#include "IncludeHeader.h"
//#include ""
//
//
#include "LPacketPoolManager.h"
#include "LNetWorkServices.h"
#include "LMainLogicThread.h"
#include "LErrorWriter.h"
#include "LMainLogicBroadCast.h"
#include "LNetWorkConfigFileProcessor.h"
#include "LConnector.h"
#include <termios.h>
//bool g_bEpollETEnabled = false;
//int g_nRecvBufLen = 8 * 1024;
//int g_nSendBufLen = 128 * 1024;
//int g_nSendBufLen = 1024;

//char* wg_pListenIP = "172.30.10.40";
//char* wg_pListenIP =  "222.73.13.26"; 
//char* wg_pListenIP =  "127.0.0.1";

//unsigned short wg_usListenPort = 6000;
//unsigned int wg_unListenListSize = 1000;
//
//
//
//unsigned int wg_sRecvCircleBufItemCount = 10000;
//unsigned int wg_sEpollWorkItemCount = 5000;
//unsigned int wg_unRecvThreadCount = 6; 
//unsigned int wg_unEpollThreadCount = 6;
//unsigned int wg_unEpollThreadEpollInLocalItemPoolSize = 500;
//unsigned int wg_unEpollThreadEpollOutLocalItemPoolSize = 500;
//unsigned int wg_unWaitClientPerEpoll = 5000;
//unsigned int wg_unMaxClientSize = 20000;
//
//unsigned int wg_unSendWorkItemCount = 10000; 
//unsigned int wg_unSendThreadCount = 6;
//unsigned int wg_unNetWorkServicesSendLocalCacheSize = 10000;	//	  zhe ge xu yao she zhi zu gou da,zhe yang,mei ci neng gou wan cheng suo you shu ju bao de be di huan cun 
//unsigned int wg_unEpollOutEventCountForSendThread = 5000;
//
//unsigned int wg_nSendLocalFreePacketNeedToCommitGlobalNum = 300;
//// ben di huan cun de jie shou dao de shu ju bao de shu liang,yi ci ti jiao
//unsigned int wg_sRecvedPacketLocalKeepSize = 200;
//unsigned int wg_sRecvedPacketGlobalKeepSize = 10000;
//
//
//unsigned short wg_usSessionManagerNum = 10;
//unsigned int wg_unMaxSessionPerSessionManager = 1000; 
//unsigned int wg_SessionSendBufChainCount = 200;
//unsigned int wg_unSendLocalPoolSizeForNetWorkServices = 3000;
//
////	unsigned short usPacketLen;
////	unsigned int unInitSize;
////	unsigned int unMaxAllocSize; 
//t_Packet_Pool_Desc wg_RecvLocalPacketPool[]
//= {{128, 300, 301},
//	{256, 300, 301},
//	{512, 300, 301},
//	{1024, 300, 301},
//	{2 * 1024, 300, 301},
//	{3 * 1024, 100, 101},
//	{4 * 1024, 100, 101},
//	{5 * 1024, 100, 101},
//	{6 * 1024, 100, 101},
//	{7 * 1024, 100, 101},
//	{8 * 1024, 100, 101},
//};
//unsigned int wg_sRecvLocalPacketPoolCount = sizeof(wg_RecvLocalPacketPool) / sizeof(wg_RecvLocalPacketPool[0]);
//
//t_Packet_Pool_Desc wg_RecvGlobalPacketPool[] 
//= {{128, 1500, 5000},
//	{256, 1500, 5000},
//	{512, 1500, 5000},
//	{1024, 1500, 5000},
//	{2 * 1024, 1500, 3000},
//	{3 * 1024, 1500, 3000},
//	{4 * 1024, 1500, 3000},
//	{5 * 1024, 1500, 3000},
//	{6 * 1024, 1500, 3000},
//	{7 * 1024, 1500, 3000},
//	{8 * 1024, 1500, 3000},
//};
//unsigned int wg_sRecvGlobalPacketPoolCount = sizeof(wg_RecvGlobalPacketPool) / sizeof(wg_RecvGlobalPacketPool[0]);
//
//t_Packet_Pool_Desc wg_SendPacketNeedToFreeLocalPool[]
//={{128, 0, 100},
//	{256, 0, 100},
//	{512, 0, 100},
//	{1024, 0, 100},
//	{2 * 1024, 0, 100},
//	{3 * 1024, 0, 100},
//	{4 * 1024, 0, 100},
//	{5 * 1024, 0, 100},
//	{6 * 1024, 0, 100},
//	{7 * 1024, 0, 100},
//	{8 * 1024, 0, 100},
//};
//unsigned int wg_sSendPacketNeedToFreeLocalPool = sizeof(wg_SendPacketNeedToFreeLocalPool) / sizeof(wg_SendPacketNeedToFreeLocalPool[0]);
//
//t_Packet_Pool_Desc wg_SendGlobalPacketPool[]
//={{128, 2000, 3000},
//	{256, 1500, 3000},
//	{512, 1500, 3000},
//	{1024, 1500, 3000},
//	{2 * 1024, 1500, 3000},
//	{3 * 1024, 1500, 3000},
//	{4 * 1024, 1500, 3000},
//	{5 * 1024, 1500, 3000},
//	{6 * 1024, 1500, 3000},
//	{7 * 1024, 1500, 3000},
//	{8 * 1024, 1500, 3000},
//};
//
//unsigned int wg_sSendGlobalPacketPoolCount = sizeof(wg_SendGlobalPacketPool) / sizeof(wg_SendGlobalPacketPool[0]);
//
//unsigned int wg_sCloseSessionMaxCount = 10000;

extern LErrorWriter g_ErrorWriter;
static struct termios oldt;

void RestoreTerminalState()
{
	tcsetattr(0, TCSANOW, &oldt);
}

void EnableTerminalEnterString()
{
	struct termios newt;


	tcgetattr(0, &oldt);

	newt = oldt;

	newt.c_iflag &= ~(ICANON | ECHO);

	tcsetattr(0, TCSANOW, &newt);

	atexit(RestoreTerminalState);
}


int main(int nargc, char* argv[])
{
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGPIPE, &action, NULL);

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGHUP, &action, NULL);

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGPIPE);
	int n = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	if (n != 0)
	{
		return -1;
	}

	EnableTerminalEnterString();
//	char* pErrorFileName = "Error.txt";
//	if (!g_ErrorWriter.Initialize(pErrorFileName))
//	{
//		return -5;
//	}
	//	测试连接器
//	LConnectorConfigProcessor ccfp;
//	if (!ccfp.Initialize("NetConnectorConfig.ini"))
//	{
//		return -11;
//	}
//
//	LConnector connector;
//	if (!connector.Initialize(ccfp.GetConnectorInitializeParams()))
//	{
//		return -12;
//	}



	char* pFileName = "123.txt";
	//	LMainLogicThread lMainLogicThread;
	LMainLogicBroadCast lMainLogicThread;


//	t_NetWorkServices_Params nwsp;
//
//	LNetWorkConfigFileProcessor nwcfp;
//	if (!nwcfp.ReadConfig("NetWorkConfig.ini", "Test"))
//	{
//		return -1;
//	}
//	if (!lNetWorkServices.Initialize(nwcfp.GetNetWorkServicesParams()))
//	{
//		return -1;
//	}
//	printf("lNetWorkServices.Initialize Success Step 1\n");
//	lMainLogicThread.SetNetWorkServices(&lNetWorkServices);
	if (!lMainLogicThread.Initialize("NetWorkConfig.ini"))
	{
		return -2;
	}
	printf("lMainLogicThread.Initialize Success Step 2\n");
	if (!lMainLogicThread.Start())
	{
		return -3;
		printf("lMainLogicThread.Start Success Step 3\n");
	}

	char* pszTest = "Test for write";
	g_ErrorWriter.WriteError(pszTest, strlen(pszTest), __FILE__, __FUNCTION__, __LINE__);

	printf("lNetWorkServices.Start Success Step 4 NetServices Started\n");

	int ch = 0;
	while (1)
	{
		ch = getchar();
		if (ch == 'Q')
		{
			lMainLogicThread.Stop();
			break;
		}
		printf("Enter char is:%c\n", ch);
	}
	pthread_join(lMainLogicThread.GetThreadHandle(), NULL);

	return 0;
}

