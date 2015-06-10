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

#include "LFixLenCircleBuf.h"
#include "LErrorWriter.h"


extern LErrorWriter g_ErrorWriter;

LFixLenCircleBuf::LFixLenCircleBuf()
{
#ifndef WIN32
	atomic_set(&m_ReadIndex		, 0);
	atomic_set(&m_WriteIndex	, 0);
#else
	InterlockedExchange(&m_ReadIndex, 0);
	InterlockedExchange(&m_WriteIndex, 0);
#endif
	m_sItemSize 	= 0;
	m_pbuf 			= NULL;
	m_sMaxItemCount = 0; 
}
LFixLenCircleBuf::~LFixLenCircleBuf()
{
	if (m_pbuf != NULL)
	{
		delete[] m_pbuf;
	}
}
bool LFixLenCircleBuf::Initialize(size_t sItemSize, size_t sMaxItemCount)
{
	if (sItemSize == 0)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::Initialize, sItemSize == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (sMaxItemCount == 0)
	{
		sMaxItemCount = 10;
	}
	if (m_pbuf != NULL)
	{
#ifndef	WIN32
		atomic_set(&m_ReadIndex		, 0);
		atomic_set(&m_WriteIndex	, 0); 
#else
		InterlockedExchange(&m_ReadIndex, 0);
		InterlockedExchange(&m_WriteIndex, 0);
#endif
		delete[] m_pbuf;
	}
	m_pbuf = new char[sMaxItemCount * sItemSize];
	if (m_pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::Initialize, m_pbuf = NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	m_sItemSize 	= sItemSize;
	m_sMaxItemCount = sMaxItemCount;
	return true;
}
E_Circle_Error LFixLenCircleBuf::AddItems(char* pbuf, size_t sItemCount)
{
	if (m_pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::AddItems, Buf Not Initialized, AddItemCount:%d\n", sItemCount); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Uninitialized;
	}
	if (pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::AddItems, pbuf == NULL, AddItemCount:%d\n", sItemCount); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Input_Buf_Null;
	}
	if (sItemCount > m_sMaxItemCount - 1)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::AddItems, Buf is Full, AddItemCount:%d\n", sItemCount); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Can_Not_Contain_Data;
	}
#ifndef WIN32
	int nReadIndex = atomic_read(&m_ReadIndex);
	int nWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nReadIndex	= m_ReadIndex;
	int nWriteIndex = m_WriteIndex;
#endif
	int nFreeItemCount = 0;
	if (nWriteIndex < nReadIndex)
	{
		nFreeItemCount = nReadIndex - nWriteIndex;
	}
	else
	{
		nFreeItemCount = m_sMaxItemCount + nReadIndex - nWriteIndex; 
	}
	if (nFreeItemCount <= 1)
	{
		//char szError[512];
		//sprintf(szError, "LFixLenCircleBuf::AddItems, Buf is Full, nFreeItemCount <= 1, AddItemCount:%d\n", sItemCount); 
		//g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Can_Not_Contain_Data;
	}
	if (nFreeItemCount - 1 < sItemCount)
	{
		//char szError[512];
		//sprintf(szError, "LFixLenCircleBuf::AddItems, Buf is Full, nFreeItemCount - 1 < sItemCount, AddItemCount:%d\n", sItemCount); 
		//g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Can_Not_Contain_Data;
	}
	if (nWriteIndex < nReadIndex)
	{
		memcpy(m_pbuf + nWriteIndex * m_sItemSize, pbuf, sItemCount * m_sItemSize);
		nWriteIndex += sItemCount;
#ifndef WIN32
		atomic_set(&m_WriteIndex, nWriteIndex);
#else
		InterlockedExchange(&m_WriteIndex, nWriteIndex);
#endif
	}
	else
	{
		if (nWriteIndex + sItemCount <= m_sMaxItemCount)
		{
			memcpy(m_pbuf + nWriteIndex * m_sItemSize, pbuf, sItemCount * m_sItemSize);
			nWriteIndex += sItemCount;
			if (nWriteIndex == m_sMaxItemCount)
			{
				nWriteIndex = 0;
			}
#ifndef WIN32
			atomic_set(&m_WriteIndex, nWriteIndex); 
#else
			InterlockedExchange(&m_WriteIndex, nWriteIndex);
#endif
		}
		else
		{
			int nFirstWriteSize = m_sMaxItemCount - nWriteIndex;
			int nSecondWriteSize = sItemCount - nFirstWriteSize;
			memcpy(m_pbuf + nWriteIndex * m_sItemSize, pbuf, nFirstWriteSize * m_sItemSize);
			memcpy(m_pbuf, pbuf + nFirstWriteSize * m_sItemSize, nSecondWriteSize * m_sItemSize);
#ifndef WIN32
			atomic_set(&m_WriteIndex, nSecondWriteSize);
#else
			InterlockedExchange(&m_WriteIndex, nSecondWriteSize);
#endif
		}
	}
#ifndef WIN32
	int nTempReadIndex	= atomic_read(&m_ReadIndex);
	int nTempWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nTempReadIndex	= m_ReadIndex;
	int nTempWriteIndex = m_WriteIndex;
#endif
	return E_Circle_Buf_No_Error;
}
E_Circle_Error LFixLenCircleBuf::GetOneItem(char* pbuf, size_t sbufSize)
{
	if (m_pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::GetOneItem, Buf Not Initialized\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Uninitialized;
	}
	if (pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::GetOneItem, pbuf == NULL, \n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		return E_Circle_Buf_Input_Buf_Null;
	}
	if (sbufSize < m_sItemSize)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::GetOneItem, sbufSize < m_sItemSize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Input_Buf_Not_Enough_Len;
	}
#ifndef WIN32
	int nReadIndex = atomic_read(&m_ReadIndex);
	int nWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nReadIndex	= m_ReadIndex;
	int nWriteIndex = m_WriteIndex;
#endif
	if (nReadIndex == nWriteIndex)
	{
		return E_Circle_Buf_Is_Empty;
	}
	int nCurrentItemCount  = 0;
	if (nReadIndex < nWriteIndex)
	{
		nCurrentItemCount = nWriteIndex - nReadIndex;
	}
	else
	{
		nCurrentItemCount = m_sMaxItemCount + nWriteIndex - nReadIndex;
	}
	if (nCurrentItemCount == 0)
	{
		return E_Circle_Buf_Is_Empty;
	}
	memcpy(pbuf, m_pbuf + nReadIndex * m_sItemSize, m_sItemSize);
	nReadIndex++;
	if (nReadIndex == m_sMaxItemCount)
	{
		nReadIndex = 0;
	}
#ifndef WIN32
	atomic_set(&m_ReadIndex, nReadIndex); 
	int nTempReadIndex = atomic_read(&m_ReadIndex);
	int nTempWriteIndex = atomic_read(&m_WriteIndex);
#else
	InterlockedExchange(&m_ReadIndex, nReadIndex); 
	int nTempReadIndex	= m_ReadIndex;
	int nTempWriteIndex = m_WriteIndex;
#endif
	return E_Circle_Buf_No_Error;
}

int LFixLenCircleBuf::RequestItems(LFixLenCircleBuf* pFixCircleBuf, size_t sExpecttedCount)
{
	if (m_pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::RequestItems, Buf Not Initialized\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return 0;
	}
	if (pFixCircleBuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::RequestItems, pFixCircleBuf == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return 0;
	}
#ifndef WIN32
	int nReadIndex = atomic_read(&m_ReadIndex);
	int nWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nReadIndex = m_ReadIndex;
	int nWriteIndex =m_WriteIndex;
#endif
	if (nReadIndex == nWriteIndex)
	{
		return 0;
	}
	if (nReadIndex < nWriteIndex)
	{
		int nExisttedBufCount = nWriteIndex - nReadIndex;
		if (nExisttedBufCount <= sExpecttedCount)
		{
			if (pFixCircleBuf->AddItems(m_pbuf + nReadIndex * m_sItemSize, nExisttedBufCount) != E_Circle_Buf_No_Error)
			{ 
				char szError[512];
				sprintf(szError, "LFixLenCircleBuf::RequestItems, pFixCircleBuf->AddItems Failed pos1\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return 0;
			}
#ifndef WIN32
			atomic_set(&m_ReadIndex, nWriteIndex);
#else
			InterlockedExchange(&m_ReadIndex, nWriteIndex);
#endif
			return nExisttedBufCount;
		}
		else
		{ 
			if (pFixCircleBuf->AddItems(m_pbuf + nReadIndex * m_sItemSize, sExpecttedCount) != E_Circle_Buf_No_Error)
			{
				char szError[512];
				sprintf(szError, "LFixLenCircleBuf::RequestItems, pFixCircleBuf->AddItems Failed pos2\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return 0;
			}
			nReadIndex += sExpecttedCount;
#ifndef WIN32
			atomic_set(&m_ReadIndex, nReadIndex);
#else
			InterlockedExchange(&m_ReadIndex, nReadIndex);
#endif
			return sExpecttedCount;
		}
	}
	else
	{ 
		int nCountFromCurrentReadIndexToEnd = m_sMaxItemCount - nReadIndex;
		if (nCountFromCurrentReadIndexToEnd > sExpecttedCount)
		{
			if (pFixCircleBuf->AddItems(m_pbuf + nReadIndex * m_sItemSize, sExpecttedCount) != E_Circle_Buf_No_Error)
			{
				char szError[512];
				sprintf(szError, "LFixLenCircleBuf::RequestItems, pFixCircleBuf->AddItems Failed pos3\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return 0;
			}
			nReadIndex += sExpecttedCount;
#ifndef WIN32
			atomic_set(&m_ReadIndex, nReadIndex);
#else
			InterlockedExchange(&m_ReadIndex, nReadIndex);
#endif
			return sExpecttedCount; 
		}
		else if (nCountFromCurrentReadIndexToEnd == sExpecttedCount)
		{ 
			if (pFixCircleBuf->AddItems(m_pbuf + nReadIndex * m_sItemSize, sExpecttedCount) != E_Circle_Buf_No_Error)
			{
				char szError[512];
				sprintf(szError, "LFixLenCircleBuf::RequestItems, pFixCircleBuf->AddItems Failed pos4\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return 0;
			}
#ifndef WIN32
			atomic_set(&m_ReadIndex, 0);
#else
			InterlockedExchange(&m_ReadIndex, 0);
#endif
			return sExpecttedCount; 
		}
		else
		{ 
			if (pFixCircleBuf->AddItems(m_pbuf + nReadIndex * m_sItemSize, nCountFromCurrentReadIndexToEnd) != E_Circle_Buf_No_Error)
			{
				char szError[512];
				sprintf(szError, "LFixLenCircleBuf::RequestItems, pFixCircleBuf->AddItems Failed pos5\n"); 
				g_ErrorWriter.WriteError(szError, strlen(szError));
				return 0;
			}
#ifndef WIN32
			atomic_set(&m_ReadIndex, 0);
#else
			InterlockedExchange(&m_ReadIndex, 0);
#endif
			return nCountFromCurrentReadIndexToEnd; 
		}
	}
}

bool LFixLenCircleBuf::CopyAllItemsToOtherFixLenCircleBuf(LFixLenCircleBuf* pFixCircleBuf)
{
	if (m_pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::CopyAllItemsToOtherFixLenCircleBuf, Buf Not Initialized\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (pFixCircleBuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::CopyAllItemsToOtherFixLenCircleBuf, pFixCircleBuf == NULL\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#ifndef WIN32
	int nReadIndex = atomic_read(&m_ReadIndex);
	int nWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nReadIndex = m_ReadIndex;
	int nWriteIndex = m_WriteIndex;
#endif
	if (nReadIndex == nWriteIndex)
	{
		return true;
	}
	if (nReadIndex < nWriteIndex)
	{
		if (pFixCircleBuf->AddItems(m_pbuf + nReadIndex * m_sItemSize, nWriteIndex - nReadIndex) != E_Circle_Buf_No_Error)
		{ 
			char szError[512];
			sprintf(szError, "LFixLenCircleBuf::CopyAllItemsToOtherFixLenCircleBuf, AddItems Failed pos 1\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
		}
		else
		{ 
#ifndef WIN32
			atomic_set(&m_ReadIndex, nWriteIndex); 
#else
			InterlockedExchange(&m_ReadIndex, nWriteIndex); 
#endif
			return true;
		}
	}
	else
	{
		if (pFixCircleBuf->AddItems(m_pbuf + nReadIndex * m_sItemSize, m_sMaxItemCount - nReadIndex) != E_Circle_Buf_No_Error)
		{
			char szError[512];
			sprintf(szError, "LFixLenCircleBuf::CopyAllItemsToOtherFixLenCircleBuf, AddItems Failed\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		else
		{
#ifndef WIN32
			atomic_set(&m_ReadIndex, 0);
#else
			InterlockedExchange(&m_ReadIndex, 0);
#endif
		}
		if (pFixCircleBuf->AddItems(m_pbuf, nWriteIndex) != E_Circle_Buf_No_Error)
		{
			char szError[512];
			sprintf(szError, "LFixLenCircleBuf::CopyAllItemsToOtherFixLenCircleBuf, AddItems(m_pbuf, nWriteIndex) != E_Circle_Buf_No_Error\n"); 
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return false;
		}
		else
		{
#ifndef WIN32
			atomic_set(&m_ReadIndex, nWriteIndex);
#else
			InterlockedExchange(&m_ReadIndex, nWriteIndex);
#endif
		}
		return true;
	}
#ifndef WIN32
	int nTempReadIndex = atomic_read(&m_ReadIndex);
	int nTempWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nTempReadIndex = m_ReadIndex;
	int nTempWriteIndex = m_WriteIndex;
#endif
	return false;
}


int LFixLenCircleBuf::GetCurrentExistCount()
{ 
#ifndef WIN32
	int nReadIndex = atomic_read(&m_ReadIndex);
	int nWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nReadIndex = m_ReadIndex;
	int nWriteIndex = m_WriteIndex;
#endif
	if (nWriteIndex == nReadIndex)
	{
		return 0;
	}
	if (nWriteIndex > nReadIndex)
	{
		return nWriteIndex - nReadIndex;
	}
	else
	{
		return m_sMaxItemCount - nReadIndex + nWriteIndex;
	}
}
void LFixLenCircleBuf::GetCurrentReadAndWriteIndex(int& nReadIndex, int& nWriteIndex)
{
	nReadIndex 		= m_ReadIndex;
	nWriteIndex 	= m_WriteIndex;
}
void LFixLenCircleBuf::ClearContent()
{
#ifndef WIN32
	__sync_lock_test_and_set(&m_ReadIndex, 0);
	__sync_lock_test_and_set(&m_WriteIndex, 0);
#else
	InterlockedExchange(&m_ReadIndex, 0); 
	InterlockedExchange(&m_WriteIndex, 0); 
#endif
	
}

char* LFixLenCircleBuf::GetBuf()
{
	return m_pbuf;
}
void LFixLenCircleBuf::SetReadIndex(int nReadIndex)
{
#ifndef WIN32
	__sync_lock_test_and_set(&m_ReadIndex, nReadIndex);
#else
	InterlockedExchange(&m_ReadIndex, nReadIndex); 
#endif
}
int LFixLenCircleBuf::GetMaxItemCount()
{
	return (int)m_sMaxItemCount;
}

E_Circle_Error LFixLenCircleBuf::LookUpOneItem(char* pbuf, size_t sbufSize)
{
	if (m_pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::LookUpOneItem, Buf Not Initialized\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Uninitialized;
	}
	if (pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::GetOneItem, pbuf == NULL, \n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));

		return E_Circle_Buf_Input_Buf_Null;
	}
	if (sbufSize < m_sItemSize)
	{
		char szError[512];
		sprintf(szError, "LFixLenCircleBuf::GetOneItem, sbufSize < m_sItemSize\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Input_Buf_Not_Enough_Len;
	}
#ifndef WIN32
	int nReadIndex = atomic_read(&m_ReadIndex);
	int nWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nReadIndex = m_ReadIndex;
	int nWriteIndex = m_WriteIndex;
#endif
	if (nReadIndex == nWriteIndex)
	{
		return E_Circle_Buf_Is_Empty;
	}
	int nCurrentItemCount  = 0;
	if (nReadIndex < nWriteIndex)
	{
		nCurrentItemCount = nWriteIndex - nReadIndex;
	}
	else
	{
		nCurrentItemCount = m_sMaxItemCount + nWriteIndex - nReadIndex;
	}
	if (nCurrentItemCount == 0)
	{
		return E_Circle_Buf_Is_Empty;
	}
	memcpy(pbuf, m_pbuf + nReadIndex * m_sItemSize, m_sItemSize);
	return E_Circle_Buf_No_Error;
}
E_Circle_Error LFixLenCircleBuf::DeleteOneItemAtHead()
{
#ifndef WIN32
	int nReadIndex = atomic_read(&m_ReadIndex);
	int nWriteIndex = atomic_read(&m_WriteIndex);
#else
	int nReadIndex	= m_ReadIndex;
	int nWriteIndex = m_WriteIndex;
#endif
	if (nReadIndex == nWriteIndex)
	{
		return E_Circle_Buf_Is_Empty;
	}
	int nCurrentItemCount  = 0;
	if (nReadIndex < nWriteIndex)
	{
		nCurrentItemCount = nWriteIndex - nReadIndex;
	}
	else
	{
		nCurrentItemCount = m_sMaxItemCount + nWriteIndex - nReadIndex;
	}
	if (nCurrentItemCount == 0)
	{
		return E_Circle_Buf_Is_Empty;
	}
	nReadIndex++;
	if (nReadIndex == m_sMaxItemCount)
	{
		nReadIndex = 0;
	}
#ifndef WIN32
	atomic_set(&m_ReadIndex, nReadIndex); 
#else
	InterlockedExchange(&m_ReadIndex, nReadIndex); 
#endif
	return E_Circle_Buf_No_Error;
}
