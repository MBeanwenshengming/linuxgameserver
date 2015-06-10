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

#include "LWorkItem.h"
#include "LPacketBase.h"


LWorkItem::LWorkItem()
{ 
	m_pWorkContentBuf = NULL;		//	缓冲区
	m_usWorkContentBufLen = 0;		//	分配的缓冲区的长度
	m_uUniqueID = 0;				//	WorkItem的唯一ID 
	m_nErrorCode = 0;				//	操作该缓冲区时，产生的错误
	m_usReadPos = 0;				//	当前数据的提取位置
	m_usWritePos = 0;				//	当前数据的写入位置
	m_unWorkID = 0;
}
LWorkItem::~LWorkItem()
{
	if (m_pWorkContentBuf != NULL)
	{
		delete[] m_pWorkContentBuf;
	}
}

bool LWorkItem::Initialize(unsigned short usBufLen)
{
	if (usBufLen == 0)
	{
		return false;
	}
	m_usWorkContentBufLen = usBufLen;
	m_pWorkContentBuf = new char[usBufLen];
	if (m_pWorkContentBuf == NULL)
	{
		return false;
	}
	return true;
}

void LWorkItem::Reset()
{ 
	m_uUniqueID = 0;				//	WorkItem的唯一ID 
	m_nErrorCode = 0;				//	操作该缓冲区时，产生的错误
	m_usReadPos = 0;				//	当前数据的提取位置
	m_usWritePos = 0;				//	当前数据的写入位置
	m_unWorkID = 0;
}

unsigned short LWorkItem::GetAllocBufLen()
{
	return m_usWorkContentBufLen;
}

void LWorkItem::SetWorkUniqueID(unsigned int uUniqueID)
{
	m_uUniqueID = uUniqueID;
}

unsigned int LWorkItem::GetWorkUniqueID()
{
	return m_uUniqueID;
}

void LWorkItem::SetWorkID(unsigned int unWorkID)
{
	m_unWorkID = unWorkID;
}
unsigned int LWorkItem::GetWorkID()
{
	return m_unWorkID;
}

void LWorkItem::SetErrorCode(int nErrorCode)
{
	m_nErrorCode = nErrorCode;
}
int LWorkItem::GetErrorCode()
{
	return m_nErrorCode;
}

//	检查是否有足够的缓冲区容纳数据
bool LWorkItem::CheckHaveEnoughBuf(unsigned short usDataLen)
{
	if (m_usWritePos + usDataLen > m_usWorkContentBufLen)
	{
		return false;
	}
	return true;
}

//	检查是否还有足够的数据供读取
bool LWorkItem::CheckHaveEnoughData(unsigned short usNeedDataLen)
{
	if (m_usWritePos < m_usReadPos + usNeedDataLen)
	{
		return false;
	}
	return true;
}

//	数据压入
LWorkItem& LWorkItem::AddChar(char cData)
{
	unsigned short usDataLen = sizeof(cData);	
	AddData(&cData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::AddByte(unsigned char ucData)
{
	unsigned short usDataLen = sizeof(ucData);
	AddData(&ucData, usDataLen);	
	return *this;
}
LWorkItem& LWorkItem::AddUShort(unsigned short usData)
{
	unsigned short usDataLen = sizeof(usData);
	AddData(&usData, usDataLen);	
	return *this;
}
LWorkItem& LWorkItem::AddShort(short sData)
{
	unsigned short usDataLen = sizeof(sData);
	AddData(&sData, usDataLen);	
	return *this;
}
LWorkItem& LWorkItem::AddInt(int nData)
{
	unsigned short usDataLen = sizeof(nData);
	AddData(&nData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::AddUInt(unsigned int unData)
{
	unsigned short usDataLen = sizeof(unData);
	AddData(&unData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::AddLong(long lData)
{
	unsigned short usDataLen = sizeof(lData);
	AddData(&lData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::AddULong(unsigned long ulData)
{
	unsigned short usDataLen = sizeof(ulData);
	AddData(&ulData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::AddLongLong(int64_t n64Data)
{
	unsigned short usDataLen = sizeof(n64Data);
	AddData(&n64Data, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::AddULongLong(uint64_t u64Data)
{
	unsigned short usDataLen = sizeof(u64Data);
	AddData(&u64Data, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::AddData(void* pData, size_t sDataLen)
{	
	if (m_pWorkContentBuf == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_BUF_NOT_INIT);
		return *this;
	}
	if (pData == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_INPUT_PARAM_WRONG);
		return *this;
	}
	if (sDataLen >= m_usWorkContentBufLen)			
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_DATA_TOO_LONG);
		return *this;
	}
	if (!CheckHaveEnoughBuf((unsigned short)sDataLen))
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_DATA_TOO_LONG);
	}
	else
	{
		memcpy(m_pWorkContentBuf + m_usWritePos, pData, sDataLen);
		m_usWritePos += sDataLen;
	}
	return *this;
}

LWorkItem& LWorkItem::operator<<(unsigned char ucData)
{
	AddByte(ucData);
	return *this;
}
LWorkItem& LWorkItem::operator<<(char cData)
{
	AddChar(cData);
	return *this;
}
LWorkItem& LWorkItem::operator<<(unsigned short usData)
{
	AddUShort(usData);
	return *this;
}
LWorkItem& LWorkItem::operator<<(short sData)
{
	AddShort(sData);
	return *this;
}
LWorkItem& LWorkItem::operator<<(int nData)
{
	AddInt(nData);
	return *this;
}
LWorkItem& LWorkItem::operator<<(unsigned int unData)
{
	AddUInt(unData);
	return *this;
}
LWorkItem& LWorkItem::operator<<(long lData)
{
	AddLong(lData);
	return *this;
}
LWorkItem& LWorkItem::operator<<(unsigned long ulData)
{
	AddULong(ulData);
	return *this;
}
LWorkItem& LWorkItem::operator<<(int64_t n64Data)
{
	AddLongLong(n64Data);
	return *this;
}
LWorkItem& LWorkItem::operator<<(uint64_t un64Data)
{
	AddULongLong(un64Data);
	return *this;
}


//	数据解压
LWorkItem& LWorkItem::GetChar(char& cData)
{
	unsigned short usDataLen = sizeof(cData);
	GetData(&cData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetByte(unsigned char& ucData)
{
	unsigned short usDataLen = sizeof(ucData);
	GetData(&ucData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetUShort(unsigned short& usData)
{
	unsigned short usDataLen = sizeof(usData);
	GetData(&usData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetShort(short& sData)
{
	unsigned short usDataLen = sizeof(sData);
	GetData(&sData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetInt(int& nData)
{
	unsigned short usDataLen = sizeof(nData);
	GetData(&nData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetUInt(unsigned int& unData)
{
	unsigned short usDataLen = sizeof(unData);
	GetData(&unData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetLong(long& lData)
{
	unsigned short usDataLen = sizeof(lData);
	GetData(&lData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetULong(unsigned long& ulData)
{
	unsigned short usDataLen = sizeof(ulData);
	GetData(&ulData, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetLongLong(int64_t& n64Data)
{
	unsigned short usDataLen = sizeof(n64Data);
	GetData(&n64Data, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetULongLong(uint64_t& un64Data)
{
	unsigned short usDataLen = sizeof(un64Data);
	GetData(&un64Data, usDataLen);
	return *this;
}
LWorkItem& LWorkItem::GetData(void* pData, size_t sDataLen)
{
	if (m_pWorkContentBuf == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_BUF_NOT_INIT);
		return *this;
	}
	if (pData == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_OUTPUT_PARAM_WRONG);
		return *this;
	}
	if (sDataLen > m_usWorkContentBufLen)			
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_DATA_TOO_LONG);
		return *this;
	}
	if (!CheckHaveEnoughData(sDataLen))
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_DATA_NOT_ENOUGH);
	}
	else
	{
		//	拷贝数据到缓存中
		memcpy(pData, m_pWorkContentBuf + m_usReadPos, sDataLen);
		m_usReadPos += sDataLen;
	}
	return *this;
}

LWorkItem& LWorkItem::operator>>(unsigned char& ucData)
{
	GetByte(ucData);
	return *this;
}
LWorkItem& LWorkItem::operator>>(char& cData)
{
	GetChar(cData);
	return *this;
}
LWorkItem& LWorkItem::operator>>(unsigned short& usData)
{
	GetUShort(usData);
	return *this;
}
LWorkItem& LWorkItem::operator>>(short& sData)
{
	GetShort(sData);
	return *this;
}
LWorkItem& LWorkItem::operator>>(int& nData)
{
	GetInt(nData);
	return *this;
}
LWorkItem& LWorkItem::operator>>(unsigned int& unData)
{
	GetUInt(unData);
	return *this;
}
LWorkItem& LWorkItem::operator>>(long& lData)
{
	GetLong(lData);
	return *this;
}
LWorkItem& LWorkItem::operator>>(unsigned long& ulData)
{
	GetULong(ulData);
	return *this;
}
LWorkItem& LWorkItem::operator>>(int64_t& n64Data)
{
	GetLongLong(n64Data);
	return *this;
}
LWorkItem& LWorkItem::operator>>(uint64_t& un64Data)
{
	GetULongLong(un64Data);
	return *this;
}
