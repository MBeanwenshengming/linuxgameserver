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

#include "LWorkQueueManager.h"
#include "LIniFileReadAndWrite.h"

LWorkQueueManagerConfigReader::LWorkQueueManagerConfigReader()
{ 
	m_pArrayWipip 	= NULL;
	m_usArrayCount 	= 0;
	m_unGlobalWorkQueueItemSize = 0;
}

LWorkQueueManagerConfigReader::~LWorkQueueManagerConfigReader()
{
	if (m_pArrayWipip != NULL)
	{
		delete[] m_pArrayWipip;
		m_pArrayWipip = NULL;
	}
}

bool LWorkQueueManagerConfigReader::ReadWorkQueueConfig(char* pConfigFile, char* pHeader)
{
	if (pConfigFile == NULL || pHeader == NULL)
	{
		return false;
	}

	char szSection[256];
	memset(szSection, 0, sizeof(szSection));
	sprintf(szSection, "%s_Queue_Pool_Param", pHeader);

	LIniFileReadAndWrite ifraw;
	if (!ifraw.OpenIniFile(pConfigFile))
	{
		return false;
	}
	
	char* pSection = szSection;

	int nTemp = 0;
	char* pKey = "WorkQueueSize"; 
	//	读取workqueue的最大成员数量
	nTemp = ifraw.read_profile_int(pSection, pKey, 0);
	if (nTemp <= 0)
	{
		return false;
	}
	m_unGlobalWorkQueueItemSize = (unsigned int)nTemp;

	//	读取buf种类数量	
	pKey = "PoolSizeTypeCount";
	nTemp = ifraw.read_profile_int(pSection, pKey, 0);
	if (nTemp <= 0)
	{
		return false; 
	}
	m_usArrayCount = (unsigned int)nTemp;
	if (m_usArrayCount > 100)
	{
		return false;
	}
	m_pArrayWipip = new t_WorkItem_Pool_Init_Param[m_usArrayCount];
	if (m_pArrayWipip == NULL)
	{
		return false;
	}

	for (unsigned int unIndex = 0; unIndex < m_usArrayCount; ++unIndex)
	{
		int nTempBufSize = 0;
		int nTempBufInitCount = 0;
		int nTempMaxPoolSize = 0;

		char szKeyBufSize[256];
		char szKeyBufInitCount[256];
		char szKeyBufMaxPoolSize[256];

		sprintf(szKeyBufSize, "BufSize_%u", unIndex + 1); 
		sprintf(szKeyBufInitCount, "BufInitCount_%u", unIndex + 1);
		sprintf(szKeyBufMaxPoolSize, "BufMaxSize_%u", unIndex + 1);

		nTempBufSize 		= ifraw.read_profile_int(pSection, szKeyBufSize, 0);
		if (nTempBufSize <= 0 || nTempBufSize >= 0xffff)
		{
			return false;
		}
		nTempBufInitCount 	= ifraw.read_profile_int(pSection, szKeyBufInitCount, 0);
		if (nTempBufInitCount < 0)
		{
			return false;
		}
		nTempMaxPoolSize 	= ifraw.read_profile_int(pSection, szKeyBufMaxPoolSize, 0);
		if (nTempMaxPoolSize <= 0)
		{
			return false;
		}
		m_pArrayWipip[unIndex].usPoolBufSize = (unsigned short)nTempBufSize;
		m_pArrayWipip[unIndex].unPoolInitCount = (unsigned int)nTempBufInitCount;
		m_pArrayWipip[unIndex].unMaxPoolSize = (unsigned int)nTempMaxPoolSize; 
	}
	return true;
}
	

void LWorkQueueManagerConfigReader::GetInitParam(t_WorkItem_Pool_Init_Param** pwpip, unsigned short& usArrayCount, unsigned int& unGlobalWorkQueueItemSize)
{
	*pwpip 						= m_pArrayWipip;
	usArrayCount 				= m_usArrayCount;
	unGlobalWorkQueueItemSize 	= m_unGlobalWorkQueueItemSize;
}


LWorkQueueManager::LWorkQueueManager()
{
	m_unGlobalWorkQueueItemSize = 0;
}

LWorkQueueManager::~LWorkQueueManager()
{
}

bool LWorkQueueManager::InitializePool(t_WorkItem_Pool_Init_Param* wpip, unsigned int unTypeCount, unsigned int unGlobalWorkQueueItemSize)
{
	if (!m_WorkItemPoolManager.Initialize(wpip, unTypeCount))
	{
		return false;
	}
	m_unGlobalWorkQueueItemSize = unGlobalWorkQueueItemSize;
	if (m_unGlobalWorkQueueItemSize != 0)
	{
		if (!m_FixLenCircleBufWorkQueue.Initialize(sizeof(LWorkItem*), m_unGlobalWorkQueueItemSize))
		{
			return false;
		}
	}
	return true;
}

LWorkItem* LWorkQueueManager::AllocOneWorkItem(unsigned short usBufLen)
{
	return m_WorkItemPoolManager.AllocOneWorkItem(usBufLen);
}

void LWorkQueueManager::FreeOneWorkItem(LWorkItem* pWorkItem)
{
	m_WorkItemPoolManager.FreeOneWorkItem(pWorkItem);
}

bool LWorkQueueManager::AddWorkItemToLocal(LWorkItem* pWorkItem)
{
	if (pWorkItem == NULL)
	{
		return false;
	}
	unsigned int unWorkUniqueID = pWorkItem->GetWorkUniqueID();

	map<unsigned int, LWorkItem*>::iterator _ito = m_mapLocalWorkQueue.find(unWorkUniqueID);
	if (_ito != m_mapLocalWorkQueue.end())
	{
		return false;
	}
	m_mapLocalWorkQueue[unWorkUniqueID] = pWorkItem;
	return true;
}

LWorkItem* LWorkQueueManager::FindLocalWorkItem(unsigned int unWorkItemID)
{ 
	map<unsigned int, LWorkItem*>::iterator _ito = m_mapLocalWorkQueue.find(unWorkItemID);
	if (_ito == m_mapLocalWorkQueue.end())
	{
		return NULL;
	}
	return _ito->second;
}

void LWorkQueueManager::RemoveLocalWorkItem(unsigned int unWorkItemID)
{
	map<unsigned int, LWorkItem*>::iterator _ito = m_mapLocalWorkQueue.find(unWorkItemID);
	if (_ito == m_mapLocalWorkQueue.end())
	{
		return ;
	}
	LWorkItem* pWorkItem = _ito->second;
	m_mapLocalWorkQueue.erase(_ito);
	FreeOneWorkItem(pWorkItem); 
}

void LWorkQueueManager::Release()
{
	m_WorkItemPoolManager.ReleaseAllPool();

	map<unsigned int, LWorkItem*>::iterator _ito = m_mapLocalWorkQueue.begin();
	while (_ito != m_mapLocalWorkQueue.end())
	{
		delete _ito->second;
		_ito++;
	}
	ReleaseWorkItemInGlobalQueue();
}


bool LWorkQueueManager::AddWorkItemToGlobalQueue(LWorkItem* pWorkItem)
{
	if (pWorkItem == NULL)
	{
		return false;
	}
	E_Circle_Error error = m_FixLenCircleBufWorkQueue.AddItems((char*)&pWorkItem, 1);
	if (error != E_Circle_Buf_No_Error)
	{
		return false;
	}
	return true;
}

LWorkItem* LWorkQueueManager::GetOneWorkItemFromGlobalQueue()
{
	LWorkItem* pWorkItem = NULL;

	E_Circle_Error error = m_FixLenCircleBufWorkQueue.GetOneItem((char*)&pWorkItem, sizeof(LWorkItem*));
	if (error != E_Circle_Buf_No_Error)
	{
		return NULL;
	}
	return pWorkItem;
}

void LWorkQueueManager::ReleaseWorkItemInGlobalQueue()
{ 
	LWorkItem* pWorkItem = NULL;
	E_Circle_Error error = m_FixLenCircleBufWorkQueue.GetOneItem((char*)&pWorkItem, sizeof(LWorkItem*));
	while (error == E_Circle_Buf_No_Error)
	{
		delete pWorkItem;
		pWorkItem = NULL;
		error = m_FixLenCircleBufWorkQueue.GetOneItem((char*)&pWorkItem, sizeof(LWorkItem*)); 
	}
}

