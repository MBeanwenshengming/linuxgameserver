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

#include "LNetWorkConfigFileProcessor.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

LNetWorkConfigFileProcessor::LNetWorkConfigFileProcessor(void)
{	
	memset(m_szSectionHeader, 0, sizeof(m_szSectionHeader));

	memset(&m_NetWorkServicesParams, 0, sizeof(m_NetWorkServicesParams));

	m_nRecvPoolTypeCount = 0;
	memset(m_unRecvPacketLenType, 0, sizeof(m_unRecvPacketLenType));
	m_nSendPoolTypeCount = 0;
	memset(m_unSendPacketLenType, 0, sizeof(m_unSendPacketLenType));

	memset(m_szIp, 0, sizeof(m_szIp));

	//	ȫ�ֽ��ջ���	
	memset(m_GlobalRecvPacketPool, 0, sizeof(m_GlobalRecvPacketPool));
	//	ȫ�ַ��ͻ���	
	memset(m_GlobalSendPacketPool, 0, sizeof(m_GlobalSendPacketPool));

	//	�����̱߳��ػ���	
	memset(m_RecvThreadLocalRecvPacketPool, 0, sizeof(m_RecvThreadLocalRecvPacketPool));

	//	�����̱߳��ػ���	
	memset(m_SendThreadLocalSendPacketPool, 0, sizeof(m_SendThreadLocalSendPacketPool));

	//	�ر��̱߳��ػ���	
	memset(m_CloseThreadLocalSendPacketPool, 0, sizeof(m_CloseThreadLocalSendPacketPool));
}

LNetWorkConfigFileProcessor::~LNetWorkConfigFileProcessor(void)
{
}

bool LNetWorkConfigFileProcessor::ReadConfig(char* pFileName, char* pSectionHeader)
{
	if (pFileName == NULL || pSectionHeader == NULL)
	{
		return false;
	}
	if (strlen(pSectionHeader) >= 64)
	{
		return false;
	}

	strncpy(m_szSectionHeader, pSectionHeader, sizeof(m_szSectionHeader) - 1);

	if (!m_Inifile.OpenIniFile(pFileName))
	{
		return false;
	}

	if (!ReadServicesGlobalParam())
	{
		return false;
	}
	if (!ReadServicesGlobalPool())
	{
		return false;
	}
	if (!ReadServicesBase())
	{
		return false;
	}
	if (!ReadServicesSession())
	{
		return false;
	}
	if (!ReadServicesRecvThread())
	{
		return false;
	}
	if (!ReadServicesSendThread())
	{
		return false;
	}
	if (!ReadServicesEpollThread())
	{
		return false;
	}
	if (!ReadServicesCloseThread())
	{
		return false;
	}
	return true;
}

bool LNetWorkConfigFileProcessor::ReadServicesBase()
{
	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Base", m_szSectionHeader);

	char* pSection = szSection;
	
	
	char* pKey = "IP";
	if (!m_Inifile.read_profile_string(pSection, pKey, m_szIp, sizeof(m_szIp) - 1, ""))
	{
		return false;
	}	
	if (strlen(m_szIp) == 0)
	{
		return false;
	}
	pKey = "PORT";
	int nPort = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nPort <= 0)
	{
		return false;
	}
	pKey = "LISTENLISTSIZE";
	int nListenListSize = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nListenListSize <= 0)
	{
		return false;
	}
	m_NetWorkServicesParams.nwspBase.pListenIP			= m_szIp;
	m_NetWorkServicesParams.nwspBase.usListenPort		= nPort;
	m_NetWorkServicesParams.nwspBase.unListenListSize	= nListenListSize;
	return true;
}

bool LNetWorkConfigFileProcessor::ReadServicesSession()
{

	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Session", m_szSectionHeader);

	//	char* pSection = "NetWork_Session";
	char* pSection = szSection;

	char* pKey = "SessionManagerCount";
	int nSessionManagerCount = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nSessionManagerCount <= 0)
	{
		return false;
	}

	pKey = "MaxSessionCountPerSessionManager";
	int nMaxSessionCountPerSessionManager = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nMaxSessionCountPerSessionManager <= 0)
	{
		return false;
	}

	pKey = "SessionSendChainSize";
	int nSessionSendChainSize = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nSessionSendChainSize <= 0)
	{
		return false;
	}

	pKey = "KickOutIdleSessionTime";

	int nKickOutIdleSessionTime = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nKickOutIdleSessionTime < 0)
	{
		return false;
	}
	if (nKickOutIdleSessionTime > 0xffff)
	{
		nKickOutIdleSessionTime = 0xffff - 1;
	}

	m_NetWorkServicesParams.nwspSession.usSessionManagerCount = nSessionManagerCount;
	m_NetWorkServicesParams.nwspSession.unMaxSessionCountPerSessionManager = nMaxSessionCountPerSessionManager;
	m_NetWorkServicesParams.nwspSession.unSendBufChainSize = nSessionSendChainSize;
	m_NetWorkServicesParams.nwspSession.usKickOutSessionTime = (unsigned short)nKickOutIdleSessionTime;
	return true;

}
bool LNetWorkConfigFileProcessor::ReadServicesRecvThread()
{
	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_RecvThread", m_szSectionHeader);

	//	char* pSection = "NetWork_RecvThread";
	char* pSection = szSection;

	char* pKey = "RecvedPacketLocalPoolSize";
	int nRecvedPacketLocalPoolSize = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nRecvedPacketLocalPoolSize <= 0)
	{
		return false;
	}

	pKey = "RecvWorkItemCount";
	int nRecvWorkItemCount = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nRecvWorkItemCount <= 0)
	{
		return false;
	}
	
	m_NetWorkServicesParams.nwspRecvThread.unRecvLocalPacketPoolSize	= nRecvedPacketLocalPoolSize;
	m_NetWorkServicesParams.nwspRecvThread.unRecvWorkItemCount			= nRecvWorkItemCount;

	//	�������صĽ�����ݰ��
	//	pSection = "NetWork_Pool_Recv_Thread_Local_Pool";

	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Pool_Recv_Thread_Local_Pool", m_szSectionHeader);

	pSection = szSection;

	char szKeyNameIni[128];
	char szKeyNameMax[128];
	for (int i = 1; i <= m_nRecvPoolTypeCount; ++i)
	{
		memset(szKeyNameIni, 0, sizeof(szKeyNameIni));
		memset(szKeyNameMax, 0, sizeof(szKeyNameMax));

		sprintf(szKeyNameIni, "rtlpt%d_Ini", i);
		sprintf(szKeyNameMax, "rtlpt%d_Max", i);
		
		int nIni = m_Inifile.read_profile_int(pSection, szKeyNameIni, 0);
		if (nIni < 0)
		{
			return false;
		}
		int nMax = m_Inifile.read_profile_int(pSection, szKeyNameMax, 0);
		if (nMax <= 0)
		{
			return false;
		}
		if (nIni >= nMax)
		{
			return false;
		}

		m_RecvThreadLocalRecvPacketPool[i - 1].usPacketLen		= m_unRecvPacketLenType[i - 1];
		m_RecvThreadLocalRecvPacketPool[i - 1].unMaxAllocSize	= nMax;
		m_RecvThreadLocalRecvPacketPool[i - 1].unInitSize		= nIni;
	}
	m_NetWorkServicesParams.nwspRecvThread.pArrppdForRecvLocalPool			= m_RecvThreadLocalRecvPacketPool;
	m_NetWorkServicesParams.nwspRecvThread.unRecvppdForRecvLocalPoolCount	= m_nRecvPoolTypeCount;
	return true;
}
bool LNetWorkConfigFileProcessor::ReadServicesSendThread()
{
	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_SendThread", m_szSectionHeader);

	char* pSection = szSection;
	//	char* pSection = "NetWork_SendThread";

	char* pKey = "SendWorkItemCount";
	int nSendWorkItemCount = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nSendWorkItemCount <= 0)
	{
		return false;
	}

	pKey = "EpollOutEventMaxCount";
	int nEpollOutEventMaxCount = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nEpollOutEventMaxCount <= 0)
	{
		return false;
	}
	m_NetWorkServicesParams.nwspSendThread.unSendWorkItemCount		= nSendWorkItemCount;
	m_NetWorkServicesParams.nwspSendThread.unEpollOutEventMaxCount	= nEpollOutEventMaxCount;

	

	//	�������ط��Ͱ��ͷŻ�����
	//	pSection = "NetWork_Pool_Send_Thread_Local_Pool";
	
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Pool_Send_Thread_Local_Pool", m_szSectionHeader);

	pSection = szSection;

	char szKeyNameIni[128];
	char szKeyNameMax[128];

	for (int i = 1; i <= m_nSendPoolTypeCount; ++i)
	{
		memset(szKeyNameIni, 0, sizeof(szKeyNameIni));
		memset(szKeyNameMax, 0, sizeof(szKeyNameMax));
		sprintf(szKeyNameIni, "stlpt%d_Ini", i);
		sprintf(szKeyNameMax, "stlpt%d_Max", i);

		int nIni = m_Inifile.read_profile_int(pSection, szKeyNameIni, 0);
		if (nIni < 0)
		{
			return false;
		}
		int nMax = m_Inifile.read_profile_int(pSection, szKeyNameMax, 0);
		if (nMax <= 0)
		{
			return false;
		}
		if (nIni >= nMax)
		{
			return false;
		}
		m_SendThreadLocalSendPacketPool[i - 1].unInitSize		= nIni;
		m_SendThreadLocalSendPacketPool[i - 1].unMaxAllocSize	= nMax;
		m_SendThreadLocalSendPacketPool[i - 1].usPacketLen		= m_unSendPacketLenType[i - 1];
	}
	m_NetWorkServicesParams.nwspSendThread.pArrspdForSend = m_SendThreadLocalSendPacketPool;
	m_NetWorkServicesParams.nwspSendThread.usspdSendCount = m_nSendPoolTypeCount;
	return true;
}
bool LNetWorkConfigFileProcessor::ReadServicesEpollThread()
{
	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_EpollThread", m_szSectionHeader);

	char* pSection = szSection;
	//	char* pSection = "NetWork_EpollThread";

	char* pKey = "RecvThreadWorkItemLocalCount";
	int nRecvThreadWorkItemLocalCount = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nRecvThreadWorkItemLocalCount <= 0)
	{
		return false;
	}
	pKey = "SendThreadWorkItemLocalCount";
	int nSendThreadWorkItemLocalCount = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nSendThreadWorkItemLocalCount <= 0)
	{
		return false;
	}
	pKey = "WaitClientSizePerEpoll";
	int nWaitClientSizePerEpoll = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nWaitClientSizePerEpoll <= 0)
	{
		return false;
	}
	m_NetWorkServicesParams.nwspEpollThread.unRecvThreadWorkItemCount	= nRecvThreadWorkItemLocalCount;
	m_NetWorkServicesParams.nwspEpollThread.unSendThreadWorkItemCount	= nSendThreadWorkItemLocalCount;
	m_NetWorkServicesParams.nwspEpollThread.unWaitClientSizePerEpoll	= nWaitClientSizePerEpoll;
	return true;
}
bool LNetWorkConfigFileProcessor::ReadServicesGlobalPool()
{
	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Pool_Global_Recv_Pool", m_szSectionHeader);

	char* pSection = szSection;
	//	ȫ�ֽ��ջ�����
	//	char* pSection = "NetWork_Pool_Global_Recv_Pool";

	char szKeyNameIni[128];
	char szKeyNameMax[128];
	for (int i = 1; i <= m_nRecvPoolTypeCount; ++i)
	{
		memset(szKeyNameIni, 0, sizeof(szKeyNameIni));
		memset(szKeyNameMax, 0, sizeof(szKeyNameMax));

		sprintf(szKeyNameIni, "grpt%d_Ini", i);
		sprintf(szKeyNameMax, "grpt%d_Max", i);
		
		int nIni = m_Inifile.read_profile_int(pSection, szKeyNameIni, 0);
		if (nIni < 0)
		{
			return false;
		}
		int nMax = m_Inifile.read_profile_int(pSection, szKeyNameMax, 0);
		if (nMax <= 0)
		{
			return false;
		}
		if (nIni >= nMax)
		{
			return false;
		}

		m_GlobalRecvPacketPool[i - 1].usPacketLen		= m_unRecvPacketLenType[i - 1];
		m_GlobalRecvPacketPool[i - 1].unMaxAllocSize	= nMax;
		m_GlobalRecvPacketPool[i - 1].unInitSize		= nIni;
	}
	m_NetWorkServicesParams.nwspGlobalPool.pArrppdForGlobalRecvPacketPool		= m_GlobalRecvPacketPool;
	m_NetWorkServicesParams.nwspGlobalPool.unppdForGlobalRecvPacketPoolCount	= m_nRecvPoolTypeCount;

	//	ȫ�ַ��ͻ�����
	//	pSection = "NetWork_Pool_Global_Send_Pool";

	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Pool_Global_Send_Pool", m_szSectionHeader);

	pSection = szSection;

	for (int i = 1; i <= m_nSendPoolTypeCount; ++i)
	{
		memset(szKeyNameIni, 0, sizeof(szKeyNameIni));
		memset(szKeyNameMax, 0, sizeof(szKeyNameMax));
		sprintf(szKeyNameIni, "gspt%d_Ini", i);
		sprintf(szKeyNameMax, "gspt%d_Max", i);

		int nIni = m_Inifile.read_profile_int(pSection, szKeyNameIni, 0);
		if (nIni < 0)
		{
			return false;
		}
		int nMax = m_Inifile.read_profile_int(pSection, szKeyNameMax, 0);
		if (nMax <= 0)
		{
			return false;
		}
		if (nIni >= nMax)
		{
			return false;
		}
		m_GlobalSendPacketPool[i - 1].unInitSize		= nIni;
		m_GlobalSendPacketPool[i - 1].unMaxAllocSize	= nMax;
		m_GlobalSendPacketPool[i - 1].usPacketLen		= m_unSendPacketLenType[i - 1];
	}
	m_NetWorkServicesParams.nwspGlobalPool.pArrppdForGlobalSendPacketPool	 = m_GlobalSendPacketPool;
	m_NetWorkServicesParams.nwspGlobalPool.unppdForGlobalSendPacketPoolCount = m_nSendPoolTypeCount;
	return true;
}

bool LNetWorkConfigFileProcessor::ReadServicesGlobalParam()	//	��ȡȫ�ֱ���
{
	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Global", m_szSectionHeader);

	char* pSection = szSection;
	//	char* pSection = "NetWork_Global";

	char* pKey = "SendThreadCount";
	int nSendThreadCount = m_Inifile.read_profile_int(pSection, pKey, 0);	//	��ȡ�����߳�����
	if (nSendThreadCount <= 0)
	{
		return false;
	}
	pKey = "RecvThreadCount";
	int nRecvThreadCount = m_Inifile.read_profile_int(pSection, pKey, 0);	//	��ȡ�����߳�����
	if (nRecvThreadCount <= 0)
	{
		return false;
	}
	pKey = "EpollThreadCount";
	int nEpollThreadCount = m_Inifile.read_profile_int(pSection, pKey, 0);	//	��ȡepoll�߳�����
	if (nEpollThreadCount <= 0)
	{
		return false;
	}
	pKey = "Recv_Pool_Type_Count";
	int nRecvPoolTypeCount = m_Inifile.read_profile_int(pSection, pKey, 0);	//	��ȡ���ջ��泤�����͵�����
	if (nRecvPoolTypeCount <= 0)
	{
		return false;
	}
	m_nRecvPoolTypeCount = nRecvPoolTypeCount;
	if (m_nRecvPoolTypeCount > MAX_RECV_POOL_TYPE_SIZE)
	{
		return false;
	}
	pKey = "Send_Pool_Type_Count";
	int nSendPoolTypeCount = m_Inifile.read_profile_int(pSection, pKey, 0); //	���ͻ��泤�����͵�����
	if (nSendPoolTypeCount <= 0)
	{
		return false;
	}
	m_nSendPoolTypeCount = nSendPoolTypeCount;
	if (m_nSendPoolTypeCount > MAX_SEND_POOL_TYPE_SIZE)
	{
		return false;
	}
	pKey = "RecvedPacketPoolSize";
	int nRecvedPacketPoolSize = m_Inifile.read_profile_int(pSection, pKey, 0);	//	���յ�����ݰ�Ļ�����ȣ����̻߳���Ľ��յ�����ݰ������
	if (nRecvedPacketPoolSize <= 0)
	{
		return false;
	}
	pKey = "SendPacketLocalPoolSize";
	int nSendPacketLocalPoolSize = m_Inifile.read_profile_int(pSection, pKey, 0);	//	���̱߳��ط�����ݰ����ʱ��������
	if (nSendPacketLocalPoolSize <= 0)
	{
		return false;
	}
	
	//	�������
	m_NetWorkServicesParams.nwspEpollThread.unEpollThreadCount	= nEpollThreadCount;
	m_NetWorkServicesParams.nwspEpollThread.unRecvThreadCount	= nRecvThreadCount;
	m_NetWorkServicesParams.nwspEpollThread.unSendThreadCount	= nSendThreadCount;

	m_NetWorkServicesParams.nwspRecvThread.usThreadCount		= nRecvThreadCount;

	m_NetWorkServicesParams.nwspSendThread.unSendThreadCount	= nSendThreadCount;

	m_NetWorkServicesParams.nwspGlobalPool.unRecvedPacketPoolSize = nRecvedPacketPoolSize;
	m_NetWorkServicesParams.nwspGlobalPool.unSendLocalPoolSizeForNetWorkServices = nSendPacketLocalPoolSize;

	//	��ȡ������ݰ�Ĵ�С������Ϣ
	//	pSection = "NetWork_Recv_Pool_Type"; 
	memset(szSection, 0, sizeof(szSection)); 
	sprintf(szSection, "%s_NetWork_Recv_Pool_Type", m_szSectionHeader); 
	pSection = szSection;

	char szKeyType[256]; 
	for (unsigned int i = 1; i <= m_nRecvPoolTypeCount; ++i)
	{
		memset(szKeyType, 0, sizeof(szKeyType));
		sprintf(szKeyType, "rpt_%d", i);
		int nLenValue = m_Inifile.read_profile_int(pSection, szKeyType, 0);
		if (nLenValue <= 0)
		{
			return false;
		}
		m_unRecvPacketLenType[i - 1] = nLenValue;
	}
	//	��ȡ������ݰ�Ĵ�С������Ϣ
	//	pSection = "NetWork_Send_Pool_Type";

	memset(szSection, 0, sizeof(szSection)); 
	sprintf(szSection, "%s_NetWork_Send_Pool_Type", m_szSectionHeader); 
	pSection = szSection;

	for (unsigned int i = 1; i <= m_nSendPoolTypeCount; ++i)
	{
		memset(szKeyType, 0, sizeof(szKeyType));
		sprintf(szKeyType, "spt_%d", i);
		int nLenValue = m_Inifile.read_profile_int(pSection, szKeyType, 0);
		if (nLenValue <= 0)
		{
			return false;
		}
		m_unSendPacketLenType[i - 1] = nLenValue;
	}
	return true;
}
bool LNetWorkConfigFileProcessor::ReadServicesCloseThread()
{
	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_CloseThread", m_szSectionHeader);

	char* pSection = szSection;
	//	char* pSection	= "NetWork_CloseThread";

	char* pKey		= "CloseWorkItemCount";
	
	int nCloseWorkItemCount = m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nCloseWorkItemCount <= 0)
	{
		return false;
	}
	m_NetWorkServicesParams.nwspCloseThread.unCloseWorkItemCount = nCloseWorkItemCount;

	//	�������ط��Ͱ��ͷŻ�����
	//	pSection = "NetWork_Pool_Close_Thread_Local_Pool";
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Pool_Close_Thread_Local_Pool", m_szSectionHeader);

	char szKeyNameIni[128];
	char szKeyNameMax[128];

	for (int i = 1; i <= m_nSendPoolTypeCount; ++i)
	{
		memset(szKeyNameIni, 0, sizeof(szKeyNameIni));
		memset(szKeyNameMax, 0, sizeof(szKeyNameMax));
		sprintf(szKeyNameIni, "ctlpt%d_Ini", i);
		sprintf(szKeyNameMax, "ctlpt%d_Max", i);

		int nIni = m_Inifile.read_profile_int(pSection, szKeyNameIni, 0);
		if (nIni < 0)
		{
			return false;
		}
		int nMax = m_Inifile.read_profile_int(pSection, szKeyNameMax, 0);
		if (nMax <= 0)
		{
			return false;
		}
		if (nIni >= nMax)
		{
			return false;
		}
		m_CloseThreadLocalSendPacketPool[i - 1].unInitSize		= nIni;
		m_CloseThreadLocalSendPacketPool[i - 1].unMaxAllocSize	= nMax;
		m_CloseThreadLocalSendPacketPool[i - 1].usPacketLen		= m_unSendPacketLenType[i - 1];
	}
	m_NetWorkServicesParams.nwspCloseThread.pArrppdForCloseLocalPool	= m_CloseThreadLocalSendPacketPool;
	m_NetWorkServicesParams.nwspCloseThread.unppdForCloseLocalPoolCount	= m_nSendPoolTypeCount;
	return true;
}

bool LNetWorkConfigFileProcessor::ReadServicesGlobalIdlePacketParams()
{
	char szSection[128 + 1];
	memset(szSection, 0, sizeof(szSection));

	sprintf(szSection, "%s_NetWork_Base", m_szSectionHeader);

	char* pSection = szSection;

	//	char* pSection	= "NetWork_Base";

	int nTempValue = 0;
	char* pKey		= "IdlePacketThreadWorkItemQueue";
	nTempValue 		= m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nTempValue < 0)
	{
		return false;
	}
	m_NetWorkServicesParams.nwspis.unIdleThreadWorkQueueSize = (unsigned int)nTempValue;
	pKey = "IdlePacketThreadMaxPoolSize"; 
	nTempValue 		= m_Inifile.read_profile_int(pSection, pKey, 0);
	if (nTempValue <= 0)
	{
		return false;
	}
	m_NetWorkServicesParams.nwspis.unLocalPoolMaxSize = (unsigned int)nTempValue;
	return true;
}


