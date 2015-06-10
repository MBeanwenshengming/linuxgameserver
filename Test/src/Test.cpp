//============================================================================
// Name        : Test.cpp
// Author      : wenshengming
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "pthread.h"
#include <iostream>
#include <unistd.h>

using namespace std;

#include "LFixLenCircleBuf.h"
#include "LErrorWriter.h"

LErrorWriter g_ErrorWriter;


LFixLenCircleBuf g_FixLenCircleBuf;

volatile int g_nReadStop = 0;
volatile int g_nWriteStop = 0;

//#define Thread_Count 5000
//volatile int nr_thread = Thread_Count;
//volatile int g_TestValue = 10;
//
//void* func(void *)
//{
//	__sync_sub_and_fetch(&nr_thread, 1);
//	return NULL;
//}

typedef struct _Data_To_Check
{
	int64_t nID;
	char szMsg[32];
	_Data_To_Check()
	{
		nID = 0;
		memset(szMsg, 0, sizeof(szMsg));
	}
}t_Data_To_Check;

void MakeMsg(char szMsg[32])
{
	int nRandLen = rand() % 32;
	for (int i = 0; i < nRandLen; ++i)
	{
		int nData = rand() % 2;
		if (nData == 0)
		{
			szMsg[i] = 'a' + rand() % 26;
		}
		else
		{
			szMsg[i] = 'A' + rand() % 26;
		}
	}
}
void* funcReadData(void* )
{
	int64_t nStart = 0;

	bool bLastAdded = true;

	t_Data_To_Check tDataToCheck;


	time_t tLastPrintWriteCount = time(NULL);

	bool bPrinted = false;
	while (1)
	{

		if (bLastAdded)
		{
			nStart++;
			tDataToCheck.nID = nStart;
			MakeMsg(tDataToCheck.szMsg);
		}
		E_Circle_Error eError = g_FixLenCircleBuf.AddItems((char*)&tDataToCheck, 1);
		if (eError != E_Circle_Buf_No_Error)
		{
			bLastAdded = false;
			if (!bPrinted)
			{
				//printf("buf full!!\n");
				bPrinted = true;
			}
		}
		else
		{
			bLastAdded 	= true;
			bPrinted 	= false;
		}
		struct timespec timeReq;
		timeReq.tv_sec 	= 0;
		timeReq.tv_nsec 	= 10;
		nanosleep(&timeReq, NULL);

		time_t tNow = time(NULL);
		if (tNow - tLastPrintWriteCount >= 5)
		{
			int nReadIndex = 0;
			int nWriteIndex = 0;
			g_FixLenCircleBuf.GetCurrentReadAndWriteIndex(nReadIndex, nWriteIndex);
			printf("Write Count:%lld, readIndex:%d, WriteIndex:%d\n", nStart, nReadIndex, nWriteIndex);
			tLastPrintWriteCount = tNow;
		}
		if (g_nWriteStop == 1)
		{
			break;
		}
//		if (rand() % 100 > 97)
//		{
//			sleep(1);
//		}
	}
	return NULL;
}
void* funcWriteData(void* )
{
	int64_t n64LastID = 0;

	time_t tLastPrintWriteCount = time(NULL);
	t_Data_To_Check tDataToCheck;
	while (1)
	{
		memset(&tDataToCheck, 0, sizeof(tDataToCheck));

		E_Circle_Error eError = g_FixLenCircleBuf.GetOneItem((char*)&tDataToCheck, sizeof(t_Data_To_Check));
		if (eError == E_Circle_Buf_No_Error)
		{
			if (n64LastID == 0)
			{
				n64LastID = tDataToCheck.nID;
			}
			else
			{
				if (tDataToCheck.nID != n64LastID + 1)
				{
					printf("ErrorDetected\n");
				}
				else
				{
					n64LastID = tDataToCheck.nID;
				}
			}
		}
		else
		{

		}
		struct timespec timeReq;
		timeReq.tv_sec 	= 0;
		timeReq.tv_nsec 	= 10;
		nanosleep(&timeReq, NULL);

		time_t tNow = time(NULL);
		if (tNow - tLastPrintWriteCount >= 5)
		{
			int nReadIndex = 0;
			int nWriteIndex = 0;
			g_FixLenCircleBuf.GetCurrentReadAndWriteIndex(nReadIndex, nWriteIndex);
			printf("Read Count:%lld, ReadINded:%d, writeIndex:%d\n", n64LastID, nReadIndex, nWriteIndex);
			tLastPrintWriteCount = tNow;
		}
		if (g_nReadStop == 1)
		{
			break;
		}
//		if (rand() % 100 > 90)
//		{
//			sleep(rand() % 4);
//		}
	}

	return NULL;
}

int main()
{
	if (!g_ErrorWriter.Initialize("./Error.txt"))
	{
		return -1;
	}
	if (g_FixLenCircleBuf.Initialize(sizeof(t_Data_To_Check), 1000) != true)
	{
		return -1;
	}
	pthread_t pReadthreadID = 0;
	pthread_create(&pReadthreadID, NULL, funcWriteData, NULL);

	pthread_t pWritethreadID = 0;
	pthread_create(&pWritethreadID, NULL, funcReadData, NULL);

	int ch = 0;
	while (1)
	{
		ch = getchar();
		if (ch == 'Q')
		{
			__sync_lock_test_and_set(&g_nWriteStop, 1);
			__sync_lock_test_and_set(&g_nReadStop, 1);
			return 0;
		}
	}
	pthread_join(pWritethreadID, NULL);
	pthread_join(pReadthreadID, NULL);

//	__sync_lock_test_and_set(&g_TestValue, 0);
//
//	int nValue = g_TestValue;
//
//	pthread_t pthreadID = 0;
//	int n = 0;
//	for (int i = 0; i < Thread_Count; ++i) {
//	    if (0 != pthread_create(&pthreadID, NULL, func, NULL)) {
//	        n++;
//	    }
//	}
//	__sync_sub_and_fetch(&nr_thread, n);
//
//	while (nr_thread != 0) {
//	    usleep(100000);
//	}
	return 0;
}




