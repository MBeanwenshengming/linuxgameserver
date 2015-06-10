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

#include "LPacketBase.h"
#include "CRC32.h"

LPacketBase::LPacketBase(unsigned short usBufLen) : m_usPacketBufLen(usBufLen) 
{ 	
	if (usBufLen < sizeof(unsigned short))
	{
		return ;
	}
	m_pData = new char[usBufLen];
	memset(m_pData, 0, m_usPacketBufLen);
	*((unsigned short*)m_pData) = sizeof(unsigned short);		//	设置数据包大小为2字节
	m_nErrorCode				= 0;
	m_usCurrentReadPos		= PACKET_BASE_DATA_OFFSET;
	m_ucPacketType				= 0;
}

LPacketBase::~LPacketBase()
{	
	if (m_pData != NULL)
	{
		delete[] m_pData;
		m_pData = NULL;
	}
}

bool LPacketBase::DecrementRefCountAndResultIsTrue()
{
	return true;
}
//	数据压入
LPacketBase& LPacketBase::AddChar(char cData)
{
	unsigned short usDataLen = sizeof(cData);	
	AddData(&cData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::AddByte(unsigned char ucData)
{
	unsigned short usDataLen = sizeof(ucData);
	AddData(&ucData, usDataLen);	
	return *this;
}
LPacketBase& LPacketBase::AddUShort(unsigned short usData)
{
	unsigned short usDataLen = sizeof(usData);
	unsigned short usTempData = htons(usData);
	AddData(&usTempData, usDataLen);	
	return *this;
}
LPacketBase& LPacketBase::AddShort(short sData)
{
	unsigned short usDataLen = sizeof(sData);
	unsigned short usTempData = htons(sData);
	AddData(&usTempData, usDataLen);	
	return *this;
}
LPacketBase& LPacketBase::AddInt(int nData)
{
	unsigned short usDataLen = sizeof(nData);
	unsigned long unTempData = htonl(nData);
	AddData(&unTempData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::AddUInt(unsigned int unData)
{
	unsigned short usDataLen = sizeof(unData);
	unsigned long unTempData = htonl(unData);
	AddData(&unTempData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::AddLong(long lData)
{
	unsigned short usDataLen = sizeof(lData);
	unsigned long lTempData = htonl(lData);
	AddData(&lTempData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::AddULong(unsigned long ulData)
{
	unsigned short usDataLen = sizeof(ulData);
	unsigned long lTempData = htonl(ulData);
	AddData(&lTempData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::AddLongLong(int64_t n64Data)
{
	unsigned short usDataLen = sizeof(n64Data);
	uint64_t u64TempData = hton64(n64Data);
	AddData(&u64TempData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::AddULongLong(uint64_t u64Data)
{
	unsigned short usDataLen = sizeof(u64Data);
	uint64_t u64TempData = hton64(u64Data);
	AddData(&u64TempData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::AddData(void* pData, size_t sDataLen)
{	
	if (m_pData == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_BUF_NOT_INIT);
		return *this;
	}
	if (pData == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_INPUT_PARAM_WRONG);
		return *this;
	}
	if (sDataLen >= 8 * 1024)			//	最大8k数据包
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
		unsigned short usCurrentDataLen = *((unsigned short*)m_pData);
		memcpy(m_pData + usCurrentDataLen, pData, sDataLen);
		usCurrentDataLen += sDataLen;
		*((unsigned short*)m_pData) = usCurrentDataLen;
	}
	return *this;
}

LPacketBase& LPacketBase::operator<<(unsigned char ucData)
{
	AddByte(ucData);
	return *this;
}
LPacketBase& LPacketBase::operator<<(char cData)
{
	AddChar(cData);
	return *this;
}
LPacketBase& LPacketBase::operator<<(unsigned short usData)
{
	AddUShort(usData);
	return *this;
}
LPacketBase& LPacketBase::operator<<(short sData)
{
	AddShort(sData);
	return *this;
}
LPacketBase& LPacketBase::operator<<(int nData)
{
	AddInt(nData);
	return *this;
}
LPacketBase& LPacketBase::operator<<(unsigned int unData)
{
	AddUInt(unData);
	return *this;
}
LPacketBase& LPacketBase::operator<<(long lData)
{
	AddLong(lData);
	return *this;
}
LPacketBase& LPacketBase::operator<<(unsigned long ulData)
{
	AddULong(ulData);
	return *this;
}
LPacketBase& LPacketBase::operator<<(int64_t n64Data)
{
	AddLongLong(n64Data);
	return *this;
}
LPacketBase& LPacketBase::operator<<(uint64_t un64Data)
{
	AddULongLong(un64Data);
	return *this;
}


//	数据解压
LPacketBase& LPacketBase::GetChar(char& cData)
{
	unsigned short usDataLen = sizeof(cData);
	GetData(&cData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::GetByte(unsigned char& ucData)
{
	unsigned short usDataLen = sizeof(ucData);
	GetData(&ucData, usDataLen);
	return *this;
}
LPacketBase& LPacketBase::GetUShort(unsigned short& usData)
{
	unsigned short usDataLen = sizeof(usData);
	GetData(&usData, usDataLen);
	usData = ntohs(usData);
	return *this;
}
LPacketBase& LPacketBase::GetShort(short& sData)
{
	unsigned short usDataLen = sizeof(sData);
	GetData(&sData, usDataLen);
	sData = ntohs(sData);
	return *this;
}
LPacketBase& LPacketBase::GetInt(int& nData)
{
	unsigned short usDataLen = sizeof(nData);
	GetData(&nData, usDataLen);
	nData = ntohl(nData);
	return *this;
}
LPacketBase& LPacketBase::GetUInt(unsigned int& unData)
{
	unsigned short usDataLen = sizeof(unData);
	GetData(&unData, usDataLen);
	unData = ntohl(unData);
	return *this;
}
LPacketBase& LPacketBase::GetLong(long& lData)
{
	unsigned short usDataLen = sizeof(lData);
	GetData(&lData, usDataLen);
	lData = ntohl(lData);
	return *this;
}
LPacketBase& LPacketBase::GetULong(unsigned long& ulData)
{
	unsigned short usDataLen = sizeof(ulData);
	GetData(&ulData, usDataLen);
	ulData = ntohl(ulData);
	return *this;
}
LPacketBase& LPacketBase::GetLongLong(int64_t& n64Data)
{
	unsigned short usDataLen = sizeof(n64Data);
	GetData(&n64Data, usDataLen);
	n64Data = ntoh64(n64Data);
	return *this;
}
LPacketBase& LPacketBase::GetULongLong(uint64_t& un64Data)
{
	unsigned short usDataLen = sizeof(un64Data);
	GetData(&un64Data, usDataLen);
	un64Data = ntoh64(un64Data);
	return *this;
}
LPacketBase& LPacketBase::GetData(void* pData, size_t sDataLen)
{
	if (m_pData == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_BUF_NOT_INIT);
		return *this;
	}
	if (pData == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_OUTPUT_PARAM_WRONG);
		return *this;
	}
	if (sDataLen >= 10 * 1024)			
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
		memcpy(pData, m_pData + m_usCurrentReadPos, sDataLen);
		m_usCurrentReadPos += sDataLen;
	}
	return *this;
}

LPacketBase& LPacketBase::operator>>(unsigned char& ucData)
{
	GetByte(ucData);
	return *this;
}
LPacketBase& LPacketBase::operator>>(char& cData)
{
	GetChar(cData);
	return *this;
}
LPacketBase& LPacketBase::operator>>(unsigned short& usData)
{
	GetUShort(usData);
	return *this;
}
LPacketBase& LPacketBase::operator>>(short& sData)
{
	GetShort(sData);
	return *this;
}
LPacketBase& LPacketBase::operator>>(int& nData)
{
	GetInt(nData);
	return *this;
}
LPacketBase& LPacketBase::operator>>(unsigned int& unData)
{
	GetUInt(unData);
	return *this;
}
LPacketBase& LPacketBase::operator>>(long& lData)
{
	GetLong(lData);
	return *this;
}
LPacketBase& LPacketBase::operator>>(unsigned long& ulData)
{
	GetULong(ulData);
	return *this;
}
LPacketBase& LPacketBase::operator>>(int64_t& n64Data)
{
	GetLongLong(n64Data);
	return *this;
}
LPacketBase& LPacketBase::operator>>(uint64_t& un64Data)
{
	GetULongLong(un64Data);
	return *this;
}

//	解压或者压缩失败时，设置的错误信息
int LPacketBase::GetErrorCode()
{
	return m_nErrorCode;
}

void LPacketBase::SetErrorCode(int nErrorCode)
{
	m_nErrorCode = nErrorCode;
}

void LPacketBase::Reset()
{ 	
	memset(m_pData, 0, m_usPacketBufLen);
	*((unsigned short*)m_pData) = sizeof(unsigned short);
	m_nErrorCode				= 0;
	m_usCurrentReadPos		= PACKET_BASE_DATA_OFFSET;
	m_ucPacketType 			= 0;
}
unsigned short LPacketBase::GetPacketBufLen()
{
	return m_usPacketBufLen;
}
//	获取缓冲区
char* LPacketBase::GetPacketBuf()
{
	return m_pData;
}

//	获取数据的长度
unsigned short LPacketBase::GetDataLen()
{
	return *((unsigned short*)m_pData) - sizeof(unsigned short);
}
//	获取数据开始位置的char*指针
char* LPacketBase::GetDataBuf()
{
	return m_pData + sizeof(unsigned short);
}

bool LPacketBase::IsBufNull()
{
	if (m_pData == NULL)
	{
		return true;
	}
	return false;
}
unsigned short LPacketBase::GetPacketDataAndHeaderLen()
{
	return *((unsigned short*)m_pData);
}


//	检查是否有足够的缓冲区容纳数据
bool LPacketBase::CheckHaveEnoughBuf(unsigned short usDataLen)
{
	unsigned short usCurrentDataLen = *((unsigned short*)m_pData);
	if (usCurrentDataLen + usDataLen > m_usPacketBufLen)
	{
		return false;
	}
	return true;
}

//	检查是否还有足够的数据供读取
bool LPacketBase::CheckHaveEnoughData(unsigned short usNeedDataLen)
{
	unsigned short usCurrentDataLen = *((unsigned short*)m_pData);
	if (usCurrentDataLen < m_usCurrentReadPos + usNeedDataLen)
	{
		return false;
	}
	return true;
}
//	64位转换为网络字节序
uint64_t LPacketBase::hton64(uint64_t nData)
{
	//	这里要判断机器是否为大端，是大端的话，就不需要转换了
	unsigned int unhostLow = nData & 0xFFFFFFFF;
	unsigned int unhostHigh = (nData >> 32) & 0xFFFFFFFF;

	unsigned int unNetLow = htonl(unhostLow);
	unsigned int unNetHigh = htonl(unhostHigh);

	uint64_t uResult = 0;
	
	uResult = unNetLow;
	uResult = uResult << 32;
	uResult = uResult | unNetHigh;
	return uResult;
}
//	64为转换为本地字节序
uint64_t LPacketBase::ntoh64(uint64_t nData)
{

	unsigned int unNetLow = nData & 0xFFFFFFFF;
	unsigned int unNetHigh = (nData >> 32) & 0xFFFFFFFF;

	unsigned int unHostLow = ntohl(unNetLow);
	unsigned int unHostHigh = ntohl(unNetHigh);

	uint64_t uResult = 0;

	uResult = unHostLow;
	uResult = uResult << 32;
	uResult = uResult | unHostHigh;
	return uResult;
}


unsigned char LPacketBase::GetPacketType()
{
	return m_ucPacketType;
}
void LPacketBase::SetPacketType(unsigned char ucPacketType)
{
	m_ucPacketType = ucPacketType;
}

//	获取消息包的ID，在长度unsigned short后面跟随的就是消息包的ID unsigned int
unsigned int LPacketBase::GetPacketID()
{
	unsigned short usCurrentDataLen = *((unsigned short*)m_pData);
	if (usCurrentDataLen >= sizeof(unsigned short) + sizeof(unsigned int))
	{
		return *((unsigned int*)(m_pData + sizeof(unsigned short)));
	}
	else
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_DATA_NOT_ENOUGH);
		return 0;
	}
}
//	设置消息包的ID
void LPacketBase::SetPacketID(unsigned int unPacketID)
{
	if (m_usPacketBufLen < sizeof(unsigned short) + sizeof(unsigned int))
	{ 
		SetErrorCode(PACKET_BASE_ERROR_CODE_DATA_TOO_LONG); 
	}
	else
	{
		*((unsigned int*)(m_pData + sizeof(unsigned short))) = unPacketID;
	}
}

//	直接设置数据，以前的数据将被丢弃
bool LPacketBase::DirectSetData(char* pData, unsigned short usDataLen)
{
	if (usDataLen > m_usPacketBufLen - sizeof(unsigned short))
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_DATA_TOO_LONG);
		return false;
	}
	if (m_pData == NULL)
	{
		SetErrorCode(PACKET_BASE_ERROR_CODE_BUF_NOT_INIT);
		return false;
	}

	*((unsigned short*)m_pData) = usDataLen + sizeof(unsigned short);
	memcpy(m_pData + sizeof(unsigned short), pData, usDataLen);
	return true;
}

#ifdef __EPOLL_TEST_STATISTIC__
void LPacketBase::FillPacketForTest()
{
	*((unsigned short*)m_pData) = m_usPacketBufLen - 1;
	memset(m_pData + sizeof(unsigned short), 'F',  m_usPacketBufLen - sizeof(unsigned short) - 1);
}
#endif


int LPacketBase::MakeCRC32CodeToPacket()
{
	unsigned short usPacketDataLen = *((unsigned short*)m_pData);
	if (usPacketDataLen + sizeof(unsigned long) > m_usPacketBufLen)
	{
		return -1;
	}
	*((unsigned short*)m_pData) = usPacketDataLen + sizeof(unsigned long);


	unsigned long ulCRC32Code = crc32(0, m_pData, usPacketDataLen);
	memcpy(m_pData + usPacketDataLen, &ulCRC32Code, sizeof(ulCRC32Code));
	return 0;
}
bool LPacketBase::CheckCRC32Code()
{
	unsigned long ulCRC32Code = crc32(0, m_pData, this->GetPacketDataAndHeaderLen() - sizeof(unsigned long));
	unsigned long ulCRC32CodeSource = *((unsigned long*)(m_pData + this->GetPacketDataAndHeaderLen() - sizeof(unsigned long)));
	if (ulCRC32Code != ulCRC32CodeSource)
	{
		printf("Len:%hd, crc32code:%u, source:%u\n", GetPacketDataAndHeaderLen(), ulCRC32Code, ulCRC32CodeSource);
		return false;
	}
	return true;
}
