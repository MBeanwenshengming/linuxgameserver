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

#pragma once

#include "IncludeHeader.h"

class LWorkItem
{
public:
	LWorkItem();
	~LWorkItem();
public:
	bool Initialize(unsigned short usBufLen);
public:
	void SetWorkUniqueID(unsigned int uUniqueID);
	unsigned int GetWorkUniqueID();
	int GetErrorCode();
	void Reset();
	unsigned short GetAllocBufLen();
	void SetWorkID(unsigned int unWorkID);
	unsigned int GetWorkID();
private:
	char* m_pWorkContentBuf;				//	缓冲区
	unsigned short m_usWorkContentBufLen;	//	分配的缓冲区的长度
	unsigned int m_uUniqueID;				//	WorkItem的唯一ID
	unsigned int m_unWorkID;				//	工作编号
private:
	void SetErrorCode(int nErrorCode); 
	//	检查是否有足够的缓冲区容纳数据
	bool CheckHaveEnoughBuf(unsigned short usDataLen);
	//	检查是否还有足够的数据供读取
	bool CheckHaveEnoughData(unsigned short usNeedDataLen);

	int m_nErrorCode;						//	操作该缓冲区时，产生的错误
private:
	unsigned short m_usReadPos;				//	当前数据的提取位置
	unsigned short m_usWritePos;			//	当前数据的写入位置
public:			// pack data
	LWorkItem& AddChar(char cData);
	LWorkItem& AddByte(unsigned char ucData);
	LWorkItem& AddUShort(unsigned short usData);
	LWorkItem& AddShort(short sData);
	LWorkItem& AddInt(int nData);
	LWorkItem& AddUInt(unsigned int unData);
	LWorkItem& AddLong(long lData);
	LWorkItem& AddULong(unsigned long ulData);
	LWorkItem& AddLongLong(int64_t n64Data);
	LWorkItem& AddULongLong(uint64_t u64Data);
	LWorkItem& AddData(void* pData, size_t sDataLen);

	LWorkItem& operator<<(unsigned char ucData);
	LWorkItem& operator<<(char cData);
	LWorkItem& operator<<(unsigned short usData);
	LWorkItem& operator<<(short sData);
	LWorkItem& operator<<(int nData);
	LWorkItem& operator<<(unsigned int unData);
	LWorkItem& operator<<(long lData);
	LWorkItem& operator<<(unsigned long ulData);
	LWorkItem& operator<<(int64_t n64Data);
	LWorkItem& operator<<(uint64_t un64Data);


public:			// unpack data
	LWorkItem& GetChar(char& cData);
	LWorkItem& GetByte(unsigned char& ucData);
	LWorkItem& GetUShort(unsigned short& usData);
	LWorkItem& GetShort(short& sData);
	LWorkItem& GetInt(int& nData);
	LWorkItem& GetUInt(unsigned int& unData);
	LWorkItem& GetLong(long& lData);
	LWorkItem& GetULong(unsigned long& ulData);
	LWorkItem& GetLongLong(int64_t& n64Data);
	LWorkItem& GetULongLong(uint64_t& un64Data);
	LWorkItem& GetData(void* pData, size_t sDataLen);

	LWorkItem& operator>>(unsigned char& ucData);
	LWorkItem& operator>>(char& cData);
	LWorkItem& operator>>(unsigned short& usData);
	LWorkItem& operator>>(short& sData);
	LWorkItem& operator>>(int& nData);
	LWorkItem& operator>>(unsigned int& unData);
	LWorkItem& operator>>(long& lData);
	LWorkItem& operator>>(unsigned long& ulData);
	LWorkItem& operator>>(int64_t& n64Data);
	LWorkItem& operator>>(uint64_t& un64Data);
};


