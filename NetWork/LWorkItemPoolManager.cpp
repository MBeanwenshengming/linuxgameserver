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

#include "LWorkItemPoolManager.h"
#include "LWorkItem.h"

LWorkItemPool::LWorkItemPool()
{
	m_usWorkItemBufSize = 0;
	m_unMaxWorkItemPoolSize = 0;
} 
LWorkItemPool::~LWorkItemPool()
{
}
bool LWorkItemPool::InitializePool(t_WorkItem_Pool_Init_Param twipip)
{
	if (twipip.unMaxPoolSize == 0 || twipip.usPoolBufSize == 0 || twipip.unPoolInitCount == 0)
	{
		return false;
	}
	if (!m_FixLenCircleBuf.Initialize(sizeof(LWorkItem*), twipip.unMaxPoolSize))
	{
		return false;
	}
	m_usWorkItemBufSize = twipip.usPoolBufSize;
	m_unMaxWorkItemPoolSize = twipip.unMaxPoolSize;
	for (unsigned int unIndex = 0; unIndex < twipip.unPoolInitCount; ++unIndex)
	{
		LWorkItem* pWorkItem = new LWorkItem;
		if (pWorkItem == NULL)
		{
			return false;
		}
		if (!pWorkItem->Initialize(twipip.usPoolBufSize))
		{
			delete pWorkItem;
			return false;
		}
		//m_queueWorkItemPool.push(pWorkItem);	
		E_Circle_Error errorCode = m_FixLenCircleBuf.AddItems((char*)&pWorkItem, 1);
		if (errorCode != E_Circle_Buf_No_Error)
		{
			delete pWorkItem;
			return false;
		}
	}
	return true;
}

LWorkItem* LWorkItemPool::AllocOneWorkItem()
{
	LWorkItem* pWorkItem = NULL;
	int nCurCount = m_FixLenCircleBuf.GetCurrentExistCount();
	if (nCurCount == 0)
	{
		pWorkItem = new LWorkItem;
		if (pWorkItem == NULL)
		{
			return pWorkItem;
		}
		if (!pWorkItem->Initialize(m_usWorkItemBufSize))
		{
			delete pWorkItem;
			return NULL;
		}
		return pWorkItem;
	}
	else
	{
		E_Circle_Error errorCode = m_FixLenCircleBuf.GetOneItem((char*)&pWorkItem, sizeof(LWorkItem*));
		if (errorCode == E_Circle_Buf_No_Error)
		{
			return pWorkItem;
		}
		return NULL;
	}
//	if (m_queueWorkItemPool.empty())
//	{
//		pWorkItem = new LWorkItem;
//		if (pWorkItem == NULL)
//		{
//			return pWorkItem;
//		}
//		if (!pWorkItem->Initialize(m_usWorkItemBufSize))
//		{
//			delete pWorkItem;
//			return NULL;
//		}
//		return pWorkItem;
//	}
//	else
//	{
//		pWorkItem = m_queueWorkItemPool.front();
//		m_queueWorkItemPool.pop();
//		return pWorkItem;
//	}
}
void LWorkItemPool::FreeOneWorkItem(LWorkItem* pWorkItem)
{
	if (pWorkItem == NULL)
	{
		return ;
	}
	pWorkItem->Reset();

	E_Circle_Error errorCode = m_FixLenCircleBuf.AddItems((char*)&pWorkItem, 1);
	if (errorCode != E_Circle_Buf_No_Error)
	{
		delete pWorkItem;
	}
//	if (m_queueWorkItemPool.size() >= m_unMaxWorkItemPoolSize)
//	{
//		delete pWorkItem;
//		return ;
//	}
//	unsigned short usWorkItemBufLen = pWorkItem->GetAllocBufLen();
//	if (usWorkItemBufLen != m_usWorkItemBufSize)
//	{
//		delete pWorkItem;
//		return ;
//	}
//	m_queueWorkItemPool.push(pWorkItem);
}
void LWorkItemPool::Release()
{
//	while (!m_queueWorkItemPool.empty())
//	{
//		LWorkItem* pWorkItem = m_queueWorkItemPool.front();
//		delete pWorkItem;
//		m_queueWorkItemPool.pop();
//	}
	
	LWorkItem* pWorkItem = NULL;
	E_Circle_Error errorCode = m_FixLenCircleBuf.GetOneItem((char*)&pWorkItem, sizeof(LWorkItem*));
	while (errorCode == E_Circle_Buf_No_Error)
	{
		delete pWorkItem;
		pWorkItem = NULL;
		errorCode = m_FixLenCircleBuf.GetOneItem((char*)&pWorkItem, sizeof(LWorkItem*)); 
	}
}


LWorkItemPoolManager::LWorkItemPoolManager()
{
}

LWorkItemPoolManager::~LWorkItemPoolManager()
{
}

bool LWorkItemPoolManager::Initialize(t_WorkItem_Pool_Init_Param* wpip, unsigned int unTypeCount)
{
	if (unTypeCount == 0)
	{
		return false;
	}
	for (unsigned int unIndex = 0; unIndex < unTypeCount; ++unIndex)
	{
		unsigned short usWorkItemBufSize = wpip[unIndex].usPoolBufSize;
		map<unsigned short, LWorkItemPool*>::iterator _ito = m_mapSizeToWorkItemPool.find(usWorkItemBufSize); 
		if (_ito != m_mapSizeToWorkItemPool.end())
		{
			continue;
		}

		LWorkItemPool* pWorkItemPool = new LWorkItemPool;
		if (pWorkItemPool == NULL)
		{
			return false;
		}
		if (!pWorkItemPool->InitializePool(wpip[unIndex]))
		{
			pWorkItemPool->Release();
			delete pWorkItemPool;
			return false;
		}
		m_mapSizeToWorkItemPool[usWorkItemBufSize] = pWorkItemPool;
	}
	return true;
}


LWorkItem* LWorkItemPoolManager::AllocOneWorkItem(unsigned short usBufSize)
{ 
	if (usBufSize == 0)
	{
		return NULL;
	}
	map<unsigned short, LWorkItemPool*>::iterator _ito = m_mapSizeToWorkItemPool.begin(); 
	while (_ito != m_mapSizeToWorkItemPool.end())
	{
		if (_ito->first >= usBufSize)
		{
			return _ito->second->AllocOneWorkItem();
		}
		++_ito;
	}
	return NULL;
}

void LWorkItemPoolManager::FreeOneWorkItem(LWorkItem* pWorkItem)
{ 
	if (pWorkItem == NULL)
	{
		return ;
	}
	unsigned short usWorkItemBufLen = pWorkItem->GetAllocBufLen();

	map<unsigned short, LWorkItemPool*>::iterator _ito = m_mapSizeToWorkItemPool.begin(); 
	while (_ito != m_mapSizeToWorkItemPool.end())
	{
		if (_ito->first == usWorkItemBufLen)
		{
			_ito->second->FreeOneWorkItem(pWorkItem);
			return ;
		}
		++_ito;
	}
	//	没有对应的队列，那么全部删除
	delete pWorkItem;
}

void LWorkItemPoolManager::ReleaseAllPool()
{ 
	map<unsigned short, LWorkItemPool*>::iterator _ito = m_mapSizeToWorkItemPool.begin(); 
	while (_ito != m_mapSizeToWorkItemPool.end())
	{
		_ito->second->Release();
		delete _ito->second;
		m_mapSizeToWorkItemPool.erase(_ito);
		_ito++;
	}
}

