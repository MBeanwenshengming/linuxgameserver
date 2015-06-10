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

#ifndef __LINUX_PACKET_POOL_MANAGER_HEADER_INCLUDED_DEFINED__
#define __LINUX_PACKET_POOL_MANAGER_HEADER_INCLUDED_DEFINED__

#include "LPacketBase.h"
#include "LFixLenCircleBuf.h"
#include "LErrorWriter.h"


extern LErrorWriter g_ErrorWriter;

typedef struct _Packet_Pool_Desc
{
	unsigned short usPacketLen;
	unsigned int unInitSize;
	unsigned int unMaxAllocSize; 
	//_Packet_Pool_Desc()
	//{ 
	//	usPacketLen		= 0;
	//	unInitSize 		= 0;
	//	unMaxAllocSize 	= 0; 
	//}
}t_Packet_Pool_Desc;

template<class T> class LPacketPoolManager
{
public:
	LPacketPoolManager()
	{ 
		m_usMinPacketSize 		= 0;
		m_usMaxPacketSize 		= 0;
		m_unPacketLenTypeCount 	= 0;
		m_parrPacketPoolDesc 	= NULL;
		m_parrFixLenCircleBuf 	= NULL;
		m_pGlobalPoolManager 	= NULL;
		m_bEnableAllocBuf 		= false;
	}
	~LPacketPoolManager()
	{
	}

	void ReleasePacketPoolManagerResource()
	{ 
		if (m_parrPacketPoolDesc != NULL)
		{
			delete[] m_parrPacketPoolDesc;
		}
		if (m_parrFixLenCircleBuf != NULL)
		{
			for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
			{
				T* p = NULL;
				while (1)
				{
					E_Circle_Error error = m_parrFixLenCircleBuf[unIndex].GetOneItem((char*)&p, sizeof(T*));
					if (error == E_Circle_Buf_Input_Buf_Null || error == E_Circle_Buf_Input_Buf_Not_Enough_Len || error == E_Circle_Buf_Is_Empty || error == E_Circle_Buf_Uninitialized)
					{
						break;
					}
					else
					{
						if (p->DecrementRefCountAndResultIsTrue())
						{
							delete p;
						}
						p = NULL;
					}
				}
			}
			delete[] m_parrFixLenCircleBuf;
		}
	}

	void SetReuqestPoolFromGlobalManager(LPacketPoolManager<T>* pGlobalPoolManager)
	{
		m_pGlobalPoolManager = pGlobalPoolManager;
	}
	void SetEnableAlloc(bool bEnableAllocBuf)
	{
		m_bEnableAllocBuf = bEnableAllocBuf;
	}
public:
	int GetCurrentContentCount()
	{
		int nCurrentCount = 0;
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			nCurrentCount += m_parrFixLenCircleBuf[unIndex].GetCurrentExistCount();
		}
		return nCurrentCount;
	}
	bool Initialize(t_Packet_Pool_Desc tpacketDesc[], size_t sArraySize)
	{
		if (sArraySize == 0)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::Initialize, sArraySize == 0\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		if (OrderAndSetPacketDesc(tpacketDesc, sArraySize) == false)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::Initialize, OrderAndSetPacketDesc Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		m_parrFixLenCircleBuf = new LFixLenCircleBuf[m_unPacketLenTypeCount];
		if (m_parrFixLenCircleBuf == NULL)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::Initialize, m_parrFixLenCircleBuf == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			if (!m_parrFixLenCircleBuf[unIndex].Initialize(sizeof(T*), m_parrPacketPoolDesc[unIndex].unMaxAllocSize))
			{
				char szError[512];
				sprintf(szError, "LPacketPoolManager::Initialize, m_parrFixLenCircleBuf[unIndex].Initialize Failed\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return false;
			}
			for (unsigned int unInnerIndex = 0; unInnerIndex < m_parrPacketPoolDesc[unIndex].unInitSize; ++unInnerIndex)
			{
				T* pTemp = new T(m_parrPacketPoolDesc[unIndex].usPacketLen);
				if (pTemp == NULL)
				{
					char szError[512];
					sprintf(szError, "LPacketPoolManager::Initialize, pTemp == NULL\n"); 
					g_ErrorWriter.WriteError(szError, strlen(szError));
					return false;
				}
				if (m_parrFixLenCircleBuf[unIndex].AddItems((char*)&pTemp, 1) != 0)
				{
					char szError[512];
					sprintf(szError, "LPacketPoolManager::Initialize, m_parrFixLenCircleBuf[unIndex].AddItems\n"); 
					g_ErrorWriter.WriteError(szError, strlen(szError));
					//	当前数据没有加入队列，那么需要释放该数据
					delete pTemp;
					return false;
				}
			}
		}
		return true;
	}

	T* RequestOnePacket(unsigned short usPacketLen)
	{
		T* pTemp = NULL;
		int nSelectedIndex = SelectIndexForPacketLen(usPacketLen);
		if (nSelectedIndex == -1)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::RequestOnePacket, nSelectedIndex == -1\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return NULL;
		}
//		while (1)
//		{
			if (m_parrFixLenCircleBuf[nSelectedIndex].GetOneItem((char*)&pTemp, sizeof(T*)) == E_Circle_Buf_No_Error)
			{
				return pTemp;
			}
//			else
//			{
//				if (m_pGlobalPoolManager != NULL)   // cong quan ju guan li qi qu yi xie buf 
//				{
					//	ru guo  cong quan ju qu de le huan cun, na me ji xu 
					//	fou ze qu yi ge geng da de huan cun shi yong ,zhi dao dou mei you wei zhi
//					if (m_pGlobalPoolManager->Request(&m_parrFixLenCircleBuf[nSelectedIndex], m_parrPacketPoolDesc[nSelectedIndex].unMaxAllocSize - 1, m_parrPacketPoolDesc[nSelectedIndex].usPacketLen) > 0)
//					{
//						continue;
//					}
//				}
//				if (nSelectedIndex + 1 < m_unPacketLenTypeCount)
//				{
//					nSelectedIndex++;
//					continue;
//				}
//				else
//				{
//					break;
//				}
//			}
//		}
		return pTemp;
	}
	//	qi ta de guan li qi cong gai guan li qi zhong qu yi bu fen huan cun 
	int Request(LFixLenCircleBuf* pFixLenCircleBuf, unsigned int unExpecttedCount, unsigned short usPoolItemLen)
	{
		if (pFixLenCircleBuf == NULL)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::Request, pFixLenCircleBuf == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return 0;
		}
		if (unExpecttedCount == 0)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::Request, unExpecttedCount == 0\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return 0;
		}
		int nSelectedIndex = SelectIndexForPacketLenEqual(usPoolItemLen);
		if (nSelectedIndex == -1)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::Request, nSelectedIndex == -1\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return 0;
		}
		int nGettedCount = m_parrFixLenCircleBuf[nSelectedIndex].RequestItems(pFixLenCircleBuf, unExpecttedCount);
		return nGettedCount;
	}
	bool FreeOneItemToPool(T* pValue, unsigned short usExtractBufLen)
	{
		int nSelectIndex = SelectIndexForPacketLenEqual(usExtractBufLen);
		if (nSelectIndex < 0)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::FreeOneItemToPool, nSelectIndex < 0\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		if (m_parrFixLenCircleBuf[nSelectIndex].AddItems((char*)&pValue, 1) != E_Circle_Buf_No_Error)
		{
			//char szError[512];
			//sprintf(szError, "LPacketPoolManager::FreeOneItemToPool, AddItems Failed, PacketLen:%hd\n", usExtractBufLen); 
			//g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		return true;
	}
	bool CommitToAnother(LPacketPoolManager<T> *pPool, unsigned short usPoolSize)
	{
		if (pPool == NULL)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::CommitToAnother, pPool == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		int nSelectIndex = SelectIndexForPacketLenEqual(usPoolSize);
		if (nSelectIndex < 0)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::CommitToAnother, nSelectIndex < 0\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		unsigned int unCanAddMaxItemCountToIt = 0;
		LFixLenCircleBuf* pDestBuf = pPool->GetFixLenCircleBuf(usPoolSize, unCanAddMaxItemCountToIt);
		if (pDestBuf == NULL)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::CommitToAnother, pDestBuf == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		if (!m_parrFixLenCircleBuf[nSelectIndex].CopyAllItemsToOtherFixLenCircleBuf(pDestBuf))
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::CommitToAnother, CopyAllItemsToOtherFixLenCircleBuf Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		return true;
	}
	LFixLenCircleBuf* GetFixLenCircleBuf(unsigned short usPoolSize, unsigned int& unMaxCanAddtoPoolCount)
	{ 
		int nSelectIndex = SelectIndexForPacketLenEqual(usPoolSize);
		if (nSelectIndex == -1)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::GetFixLenCircleBuf, nSelectIndex < 0\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return NULL;
		}
		unMaxCanAddtoPoolCount = m_parrPacketPoolDesc[nSelectIndex].unMaxAllocSize - 1;
		return &m_parrFixLenCircleBuf[nSelectIndex];
	}
protected:
	bool OrderAndSetPacketDesc(t_Packet_Pool_Desc tPacketDesc[], size_t sArraySize)
	{
		unsigned int unArraySize = sArraySize;
		if (unArraySize == 0)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::OrderAndSetPacketDesc, unArraySize == 0\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}

		for (unsigned int unIndex = 0; unIndex < unArraySize; ++unIndex)
		{
			if (tPacketDesc[unIndex].usPacketLen == 0 || tPacketDesc[unIndex].unMaxAllocSize == 0)
			{
				char szError[512];
				sprintf(szError, "LPacketPoolManager::OrderAndSetPacketDesc, Error Len \n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return false;
			}
		}
		//	cun zai  chong fu xiang, na me fan hui false
		for (unsigned int unIndex = 0; unIndex < unArraySize; ++unIndex)
		{
			for (unsigned int unInnerIndex = unIndex + 1; unInnerIndex < unArraySize; ++unInnerIndex)
			{
				if (tPacketDesc[unIndex].usPacketLen == tPacketDesc[unInnerIndex].usPacketLen)
				{
					char szError[512];
					sprintf(szError, "LPacketPoolManager::OrderAndSetPacketDesc, Exist Same Length packet size \n"); 
					g_ErrorWriter.WriteError(szError, strlen(szError));
					return false;
				}
			}
		}
		m_unPacketLenTypeCount = unArraySize;
		//	pai xu, xiao de zai qian mian
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			for (unsigned int unInnerIndex = unIndex + 1; unInnerIndex < m_unPacketLenTypeCount; ++unInnerIndex)
			{
				if (tPacketDesc[unIndex].usPacketLen > tPacketDesc[unInnerIndex].usPacketLen)
				{
					t_Packet_Pool_Desc tppd 	= tPacketDesc[unIndex];
					tPacketDesc[unIndex] 		= tPacketDesc[unInnerIndex];
					tPacketDesc[unInnerIndex] 	= tPacketDesc[unIndex];
				}
			}
		}

		m_parrPacketPoolDesc = new t_Packet_Pool_Desc[m_unPacketLenTypeCount];
		if (m_parrPacketPoolDesc == NULL)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::OrderAndSetPacketDesc, m_parrPacketPoolDesc == NULL\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		memcpy(m_parrPacketPoolDesc, tPacketDesc, sizeof(t_Packet_Pool_Desc) * m_unPacketLenTypeCount);
		
		m_usMinPacketSize 		= m_parrPacketPoolDesc[0].usPacketLen;
		m_usMaxPacketSize 		= m_parrPacketPoolDesc[m_unPacketLenTypeCount - 1].usPacketLen;

		return true;
	}
	int SelectIndexForPacketLen(unsigned short usPacketLen)
	{
		if (usPacketLen > m_usMaxPacketSize)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::SelectIndexForPacketLen, usPacketLen > m_usMaxPacketSize\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return -1;
		}
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			if (usPacketLen <= m_parrPacketPoolDesc[unIndex].usPacketLen)
			{
				return unIndex;
			}
		}
		return -1;
	}
	int SelectIndexForPacketLenEqual(unsigned short usPacketLen)
	{
		if (usPacketLen > m_usMaxPacketSize)
		{
			char szError[512];
			sprintf(szError, "LPacketPoolManager::SelectIndexForPacketLenEqual, usPacketLen > m_usMaxPacketSize\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return -1;
		}
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			if (usPacketLen == m_parrPacketPoolDesc[unIndex].usPacketLen)
			{
				return unIndex;
			}
		}
		return -1;

	}
public: 
	bool CommitAllLocalPacketPoolToGlobalPacketPool()
	{
		if (m_pGlobalPoolManager)
		{
			return false;
		}
		for (unsigned int unIndex = 0; unIndex < m_unPacketLenTypeCount; ++unIndex)
		{
			if (!CommitToAnother(m_pGlobalPoolManager, m_parrPacketPoolDesc[unIndex].usPacketLen))
			{ 
				char szError[512];
				sprintf(szError, "LPacketPoolManager::CommitAllLocalPacketPoolToGlobalPacketPool, CommitToAnother Failed, PacketLen:%hd\n", m_parrPacketPoolDesc[unIndex].usPacketLen); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
			}
		}
		return true;
	}
private:
	unsigned int m_unPacketLenTypeCount;
	unsigned short m_usMinPacketSize;
	unsigned short m_usMaxPacketSize;
	t_Packet_Pool_Desc* m_parrPacketPoolDesc;
	LFixLenCircleBuf* m_parrFixLenCircleBuf;
	LPacketPoolManager<T>* m_pGlobalPoolManager;
	bool m_bEnableAllocBuf;
};

#endif

