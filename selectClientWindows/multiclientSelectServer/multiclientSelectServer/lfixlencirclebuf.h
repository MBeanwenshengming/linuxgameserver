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

#ifndef __LINUX_FIX_LEN_CIRCLE_BUF_HEADER_DEFINED__
#define __LINUX_FIX_LEN_CIRCLE_BUF_HEADER_DEFINED__

#include "IncludeHeader.h"
typedef enum
{
	E_Circle_Buf_No_Error = 0,
	E_Circle_Buf_Uninitialized = -1,
	E_Circle_Buf_Can_Not_Contain_Data = -2,
	E_Circle_Buf_Input_Buf_Null = -3,
	E_Circle_Buf_Input_Buf_Not_Enough_Len = -4,
	E_Circle_Buf_Is_Empty = -5,
}E_Circle_Error;

class LFixLenCircleBuf
{
public:
	LFixLenCircleBuf();
	~LFixLenCircleBuf();
public:
	bool Initialize(size_t sItemSize, size_t sMaxItemCount);
	E_Circle_Error AddItems(char* pbuf, size_t sItemCount);
	E_Circle_Error GetOneItem(char* pbuf, size_t sbufSize);
	int RequestItems(LFixLenCircleBuf* pFixCircleBuf, size_t sExpecttedCount);

	bool CopyAllItemsToOtherFixLenCircleBuf(LFixLenCircleBuf* pFixCircleBuf);

	int GetCurrentExistCount();
	
	void GetCurrentReadAndWriteIndex(int& nReadIndex, int& nWriteIndex);
	void ClearContent();
	char* GetBuf();
	void SetReadIndex(int nReadIndex);
	int GetMaxItemCount();

public:
	E_Circle_Error LookUpOneItem(char* pbuf, size_t sbufSize);
	E_Circle_Error DeleteOneItemAtHead();
private:
#ifndef WIN32
	atomic_t m_ReadIndex;
	atomic_t m_WriteIndex;
#else
	volatile long m_ReadIndex;
	volatile long m_WriteIndex;
#endif
	size_t   m_sItemSize;
	char*	 m_pbuf;
	size_t 	 m_sMaxItemCount;
};
#endif


