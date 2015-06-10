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
//	atomic_set(&m_ReadIndex		, 0);
//	atomic_set(&m_WriteIndex	, 0);
	__sync_lock_test_and_set(&m_ReadIndex, 0);
	__sync_lock_test_and_set(&m_WriteIndex, 0);

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
//		atomic_set(&m_ReadIndex		, 0);
//		atomic_set(&m_WriteIndex	, 0);
		__sync_lock_test_and_set(&m_ReadIndex, 0);
		__sync_lock_test_and_set(&m_WriteIndex, 0);
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
//	int nReadIndex = atomic_read(&m_ReadIndex);
//	int nWriteIndex = atomic_read(&m_WriteIndex);
	int nReadIndex 	= m_ReadIndex;
	int nWriteIndex 	= m_WriteIndex;

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
//		char szError[512];
//		sprintf(szError, "LFixLenCircleBuf::AddItems, Buf is Full, nFreeItemCount <= 1, AddItemCount:%d, MaxCount:%d\n", sItemCount, m_sMaxItemCount);
//		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Can_Not_Contain_Data;
	}
	if (nFreeItemCount - 1 < sItemCount)
	{
//		char szError[512];
//		sprintf(szError, "LFixLenCircleBuf::AddItems, Buf is Full, nFreeItemCount - 1 < sItemCount, AddItemCount:%d, FreeCount:%d, MaxCount:%d\n", sItemCount, nFreeItemCount, m_sMaxItemCount);
//		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Circle_Buf_Can_Not_Contain_Data;
	}
	if (nWriteIndex < nReadIndex)
	{
		memcpy(m_pbuf + nWriteIndex * m_sItemSize, pbuf, sItemCount * m_sItemSize);
		nWriteIndex += sItemCount;
//		atomic_set(&m_WriteIndex, nWriteIndex);
		__sync_lock_test_and_set(&m_WriteIndex, nWriteIndex);
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
//			atomic_set(&m_WriteIndex, nWriteIndex);
			__sync_lock_test_and_set(&m_WriteIndex, nWriteIndex);
		}
		else
		{
			int nFirstWriteSize = m_sMaxItemCount - nWriteIndex;
			int nSecondWriteSize = sItemCount - nFirstWriteSize;
			memcpy(m_pbuf + nWriteIndex * m_sItemSize, pbuf, nFirstWriteSize * m_sItemSize);
			memcpy(m_pbuf, pbuf + nFirstWriteSize * m_sItemSize, nSecondWriteSize * m_sItemSize);
			//atomic_set(&m_WriteIndex, nSecondWriteSize);
			__sync_lock_test_and_set(&m_WriteIndex, nSecondWriteSize);
		}
	}
//	int nTempReadIndex = atomic_read(&m_ReadIndex);
//	int nTempWriteIndex = atomic_read(&m_WriteIndex);
	int nTempReadIndex = m_ReadIndex;
	int nTempWriteIndex = m_WriteIndex;

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
//	int nReadIndex = atomic_read(&m_ReadIndex);
//	int nWriteIndex = atomic_read(&m_WriteIndex);
	int nReadIndex 	= m_ReadIndex;
	int nWriteIndex 	= m_WriteIndex;

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
//	atomic_set(&m_ReadIndex, nReadIndex);
//	int nTempReadIndex = atomic_read(&m_ReadIndex);
//	int nTempWriteIndex = atomic_read(&m_WriteIndex);
	__sync_lock_test_and_set(&m_ReadIndex, nReadIndex);
	int nTempReadIndex = m_ReadIndex;
	int nTempWriteIndex = m_WriteIndex;

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
//	int nReadIndex = atomic_read(&m_ReadIndex);
//	int nWriteIndex = atomic_read(&m_WriteIndex);
	int nReadIndex 	= m_ReadIndex;
	int nWriteIndex 	= m_WriteIndex;
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
//			atomic_set(&m_ReadIndex, nWriteIndex);
			__sync_lock_test_and_set(&m_ReadIndex, nWriteIndex);
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
//			atomic_set(&m_ReadIndex, nReadIndex);
			__sync_lock_test_and_set(&m_ReadIndex, nReadIndex);
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
//			atomic_set(&m_ReadIndex, nReadIndex);
			__sync_lock_test_and_set(&m_ReadIndex, nReadIndex);
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
//			atomic_set(&m_ReadIndex, 0);
			__sync_lock_test_and_set(&m_ReadIndex, 0);
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
//			atomic_set(&m_ReadIndex, 0);
			__sync_lock_test_and_set(&m_ReadIndex, 0);
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
//	int nReadIndex = atomic_read(&m_ReadIndex);
//	int nWriteIndex = atomic_read(&m_WriteIndex);
	int nReadIndex 	= m_ReadIndex;
	int nWriteIndex 	= m_WriteIndex;
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
//			atomic_set(&m_ReadIndex, nWriteIndex);
			__sync_lock_test_and_set(&m_ReadIndex, nWriteIndex);
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
//			atomic_set(&m_ReadIndex, 0);
			__sync_lock_test_and_set(&m_ReadIndex, 0);
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
//			atomic_set(&m_ReadIndex, nWriteIndex);
			__sync_lock_test_and_set(&m_ReadIndex, nWriteIndex);
		}
		return true;
	}
//	int nTempReadIndex = atomic_read(&m_ReadIndex);
//	int nTempWriteIndex = atomic_read(&m_WriteIndex);
	int nTempReadIndex 	= m_ReadIndex;
	int nTempWriteIndex 	= m_WriteIndex;
	return false;
}
void LFixLenCircleBuf::GetCurrentReadAndWriteIndex(int& nReadIndex, int& nWriteIndex)
{
	nReadIndex 		= m_ReadIndex;
	nWriteIndex 	= m_WriteIndex;
}
void LFixLenCircleBuf::ClearContent()
{
	__sync_lock_test_and_set(&m_ReadIndex, 0);
	__sync_lock_test_and_set(&m_WriteIndex, 0);
}

char* LFixLenCircleBuf::GetBuf()
{
	return m_pbuf;
}
void LFixLenCircleBuf::SetReadIndex(int nReadIndex)
{
	__sync_lock_test_and_set(&m_ReadIndex, nReadIndex);
}
int LFixLenCircleBuf::GetMaxItemCount()
{
	return (int)m_sMaxItemCount;
}
int LFixLenCircleBuf::GetCurrentExistCount()
{ 
//	int nReadIndex = atomic_read(&m_ReadIndex);
//	int nWriteIndex = atomic_read(&m_WriteIndex);
	int nReadIndex 	= m_ReadIndex;
	int nWriteIndex	= m_WriteIndex;
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

//	从缓存区中获取一个Item，但是并不删除
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
//	int nReadIndex = atomic_read(&m_ReadIndex);
//	int nWriteIndex = atomic_read(&m_WriteIndex);
	int nReadIndex		= m_ReadIndex;
	int nWriteIndex	= m_WriteIndex;

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
//	查看从当前位置开始的第nAddedReadIndex个数据
E_Circle_Error LFixLenCircleBuf::LookUpNItem(char* pbuf, size_t sbufSize, int nAddedReadIndex)
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
//	int nReadIndex = atomic_read(&m_ReadIndex);
//	int nWriteIndex = atomic_read(&m_WriteIndex);
	int nReadIndex 	= m_ReadIndex;
	int nWriteIndex 	= m_WriteIndex;

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
	if (nAddedReadIndex > nCurrentItemCount)
	{
		return E_Circle_Buf_Is_Empty;
	}
	if (nReadIndex < nWriteIndex)
	{
		memcpy(pbuf, m_pbuf + (nReadIndex + nAddedReadIndex) * m_sItemSize, m_sItemSize);
	}
	else
	{
		if (nReadIndex + nAddedReadIndex >= m_sMaxItemCount)
		{
			memcpy(pbuf, m_pbuf + (nReadIndex + nAddedReadIndex - m_sMaxItemCount) * m_sItemSize, m_sItemSize);
		}
		else
		{
			memcpy(pbuf, m_pbuf + (nReadIndex + nAddedReadIndex) * m_sItemSize, m_sItemSize);
		}
	}
	return E_Circle_Buf_No_Error;
}

//	删除缓存区头部一个Item, 与LookUpOneItem配合使用
E_Circle_Error LFixLenCircleBuf::DeleteOneItemAtHead()
{
//	int nReadIndex = atomic_read(&m_ReadIndex);
//	int nWriteIndex = atomic_read(&m_WriteIndex);
	int nReadIndex 	= m_ReadIndex;
	int nWriteIndex 	= m_WriteIndex;
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
//	atomic_set(&m_ReadIndex, nReadIndex);
	__sync_lock_test_and_set(&m_ReadIndex, nReadIndex);
	return E_Circle_Buf_No_Error;
}
