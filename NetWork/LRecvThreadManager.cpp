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

#include "LRecvThreadManager.h"
#include "LSession.h"
#include "LNetWorkServices.h" 
#include "LErrorWriter.h"


extern LErrorWriter g_ErrorWriter;

LRecvThreadManager::LRecvThreadManager()
{
	m_parrRecvDesc = NULL;
	m_usThreadCount = 0;	
	m_pNetWorkServices = NULL;
}
LRecvThreadManager::~LRecvThreadManager()
{
}

bool LRecvThreadManager::StartAllRecvThread()
{
	if (m_parrRecvDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LRecvThreadManager::StartAllRecvThread, m_parrRecvDesc == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		if (!(&m_parrRecvDesc[usIndex])->pRecvThread->Start())
		{
			char szError[512];
			sprintf(szError, "LRecvThreadManager::StartAllRecvThread, pRecvThread->Start() Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
	}
	return true;
}
bool LRecvThreadManager::StopAllRecvThread()
{
	if (m_parrRecvDesc == NULL)
	{
		return true;
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		if ((&m_parrRecvDesc[usIndex])->pRecvThread != NULL)
		{
			(&m_parrRecvDesc[usIndex])->pRecvThread->Stop(); 
		}
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		if ((&m_parrRecvDesc[usIndex])->pRecvThread != NULL)
		{
			pthread_t pID =(&m_parrRecvDesc[usIndex])->pRecvThread->GetThreadHandle(); 
			if (pID != 0)
			{
				int nJoinRes = pthread_join(pID, NULL); 
				if (nJoinRes != 0)
				{ 
					char szError[512];
					sprintf(szError, "LRecvThreadManager::StopAllRecvThread,Join Failed, ErrorID:%d\n", errno); 
					g_ErrorWriter.WriteError(szError, strlen(szError));
				}
			} 
		}
	} 
	return true;
}

bool LRecvThreadManager::Init(unsigned short usThreadCount, unsigned int unRecvWorkItemCount, t_Packet_Pool_Desc ppdForLocalPool[], unsigned int unRecvppdForLocalPoolCount, unsigned int unRecvLocalPacketPoolSize)
{
	if (m_pNetWorkServices == NULL)
	{
		char szError[512];
		sprintf(szError, "LRecvThreadManager::Init, m_pNetWorkServices == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (usThreadCount == 0)
	{
		usThreadCount = 1;
	}
	m_usThreadCount = usThreadCount;

	m_parrRecvDesc = new t_Recv_Thread_Desc[usThreadCount];
	if (m_parrRecvDesc == NULL)
	{
		char szError[512];
		sprintf(szError, "LRecvThreadManager::Init, m_parrRecvDesc == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		(&m_parrRecvDesc[usIndex])->pRecvThread = new LRecvThread;
		if ((&m_parrRecvDesc[usIndex])->pRecvThread == NULL)
		{
			char szError[512];
			sprintf(szError, "LRecvThreadManager::Init, m_parrRecvDesc[usIndex])->pRecvThread == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		(&m_parrRecvDesc[usIndex])->pRecvThread->SetNetServices(m_pNetWorkServices);
		m_parrRecvDesc[usIndex].pRecvThread->m_nThreadID = usIndex;
		(&m_parrRecvDesc[usIndex])->usThreadID = usIndex;
		if (!(&m_parrRecvDesc[usIndex])->pRecvThread->Initialize(unRecvWorkItemCount, ppdForLocalPool, unRecvppdForLocalPoolCount, unRecvLocalPacketPoolSize))
		{
			char szError[512];
			sprintf(szError, "LRecvThreadManager::Init, m_parrRecvDesc[usIndex])->pRecvThread->Initialize Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
	}
	return true; 
}
bool LRecvThreadManager::BindRecvThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LRecvThreadManager::BindRecvThread, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	int nSelectedThreadID = SelectRecvThread();
	if (nSelectedThreadID < 0)
	{
		char szError[512];
		sprintf(szError, "LRecvThreadManager::BindRecvThread, nSelectedThreadID < 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	pSession->SetRecvThreadID(nSelectedThreadID);
	__sync_add_and_fetch(&m_parrRecvDesc[nSelectedThreadID].nBindSessionCount, 1);
	return true;
}
bool LRecvThreadManager::UnBindRecvThread(LSession* pSession)
{
	if (pSession == NULL)
	{
		char szError[512];
		sprintf(szError, "LRecvThreadManager::UnBindRecvThread, pSession == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	int unRecvThreadID = pSession->GetRecvThreadID();
	if (unRecvThreadID >= m_usThreadCount || unRecvThreadID < 0)
	{
		char szError[512];
		sprintf(szError, "LRecvThreadManager::UnBindRecvThread, unRecvThreadID >= m_usThreadCount || unRecvThreadID < 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	__sync_sub_and_fetch(&m_parrRecvDesc[unRecvThreadID].nBindSessionCount, 1);
	pSession->SetRecvThreadID(-1);
	return true;
}
//	protected member
int LRecvThreadManager::SelectRecvThread()
{
	int nThreadMinBindSessionCount = 0x0fffffff;
	int nSelectThread = -1;
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		int nRefCount = __sync_add_and_fetch(&m_parrRecvDesc[usIndex].nBindSessionCount, 0);
		if (nRefCount < nThreadMinBindSessionCount)
		{
			nThreadMinBindSessionCount = nRefCount;
			nSelectThread = usIndex;
		}
	}
	return nSelectThread;
}
LRecvThread* LRecvThreadManager::GetRecvThread(unsigned short usThreadID)
{
	if (usThreadID >= m_usThreadCount)
	{
		return NULL;
	}
	return m_parrRecvDesc[usThreadID].pRecvThread;
}
void LRecvThreadManager::SetNetWorkServices(LNetWorkServices* pNetWorkServices)
{
	m_pNetWorkServices = pNetWorkServices;
}


void LRecvThreadManager::PrintSendThreadRefStatus()
{
	printf("\tRecvThreadManager, Thread RefInfo\n");
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		int nRefCount = __sync_add_and_fetch(&m_parrRecvDesc[usIndex].nBindSessionCount, 0);
		printf("\t\tRecvThreadID:%hd, RefCount:%d\n", usIndex, nRefCount);
	}
}


void LRecvThreadManager::ReleaseRecvThreadManagerResource()
{ 
	if (m_parrRecvDesc == NULL)
	{
		return ;
	}
	for (unsigned short usIndex = 0; usIndex < m_usThreadCount; ++usIndex)
	{
		if (m_parrRecvDesc[usIndex].pRecvThread != NULL)
		{
			m_parrRecvDesc[usIndex].pRecvThread->ReleaseRecvThreadResource();
			delete m_parrRecvDesc[usIndex].pRecvThread;
			m_parrRecvDesc[usIndex].pRecvThread = NULL;
		}
	}
	delete[] m_parrRecvDesc;
	m_parrRecvDesc = NULL;
}


