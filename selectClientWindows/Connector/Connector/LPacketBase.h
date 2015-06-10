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

#ifndef __LINUX_PAKCET_BASE_HEADER_DEFINED__
#define __LINUX_PAKCET_BASE_HEADER_DEFINED__
#include "IncludeHeader.h"


#define PACKET_BASE_ERROR_CODE_DATA_TOO_LONG   -1
#define PACKET_BASE_ERROR_CODE_DATA_NOT_ENOUGH	-2
#define PACKET_BASE_ERROR_CODE_BUF_NOT_INIT -3
#define PACKET_BASE_ERROR_CODE_INPUT_PARAM_WRONG -4
#define PACKET_BASE_ERROR_CODE_OUTPUT_PARAM_WRONG -5

#define PACKET_BASE_DATA_OFFSET 2 + 4


class LPacketBase
{
public:
	LPacketBase(unsigned short usBufLen);
	~LPacketBase();

public:
	LPacketBase& AddChar(char cData);
	LPacketBase& AddByte(unsigned char ucData);
	LPacketBase& AddUShort(unsigned short usData);
	LPacketBase& AddShort(short sData);
	LPacketBase& AddInt(int nData);
	LPacketBase& AddUInt(unsigned int unData);
	LPacketBase& AddLong(long lData);
	LPacketBase& AddULong(unsigned long ulData);
	LPacketBase& AddLongLong(int64_t n64Data);
	LPacketBase& AddULongLong(uint64_t u64Data);
	LPacketBase& AddData(void* pData, size_t sDataLen);

	LPacketBase& operator<<(unsigned char ucData);
	LPacketBase& operator<<(char cData);
	LPacketBase& operator<<(unsigned short usData);
	LPacketBase& operator<<(short sData);
	LPacketBase& operator<<(int nData);
	LPacketBase& operator<<(unsigned int unData);
	LPacketBase& operator<<(long lData);
	LPacketBase& operator<<(unsigned long ulData);
	LPacketBase& operator<<(int64_t n64Data);
	LPacketBase& operator<<(uint64_t un64Data);


public:
	LPacketBase& GetChar(char& cData);
	LPacketBase& GetByte(unsigned char& ucData);
	LPacketBase& GetUShort(unsigned short& usData);
	LPacketBase& GetShort(short& sData);
	LPacketBase& GetInt(int& nData);
	LPacketBase& GetUInt(unsigned int& unData);
	LPacketBase& GetLong(long& lData);
	LPacketBase& GetULong(unsigned long& ulData);
	LPacketBase& GetLongLong(int64_t& n64Data);
	LPacketBase& GetULongLong(uint64_t& un64Data);
	LPacketBase& GetData(void* pData, size_t sDataLen);

	LPacketBase& operator>>(unsigned char& ucData);
	LPacketBase& operator>>(char& cData);
	LPacketBase& operator>>(unsigned short& usData);
	LPacketBase& operator>>(short& sData);
	LPacketBase& operator>>(int& nData);
	LPacketBase& operator>>(unsigned int& unData);
	LPacketBase& operator>>(long& lData);
	LPacketBase& operator>>(unsigned long& ulData);
	LPacketBase& operator>>(int64_t& n64Data);
	LPacketBase& operator>>(uint64_t& un64Data);

	int GetErrorCode();
	void SetErrorCode(int nErrorCode);

	bool DirectSetData(char* pData, unsigned short usDataLen);
private:
	int m_nErrorCode;
public:
	void Reset();
	bool IsBufNull();

public: 
	const unsigned short GetPacketBufLen();
	char* GetPacketBuf();

	unsigned short GetDataLen();
	char* GetDataBuf();

	unsigned short GetPacketDataAndHeaderLen();


private:
	const unsigned short m_usPacketBufLen;	
	char* m_pData;

protected:
	bool CheckHaveEnoughBuf(unsigned short usDataLen);
	bool CheckHaveEnoughData(unsigned short usNeedDataLen);

	uint64_t hton64(uint64_t nData);	
	uint64_t ntoh64(uint64_t nData);
private:
	unsigned short m_usCurrentReadPos;
public:
	unsigned char GetPacketType();
	void SetPacketType(unsigned char ucPacketType);
	virtual bool DecrementRefCountAndResultIsTrue();
private:
	unsigned char m_ucPacketType;
public:
	unsigned int GetPacketID();
	void SetPacketID(unsigned int unPacketID);
#ifdef __EPOLL_TEST_STATISTIC__
	void FillPacketForTest();
#endif
private:
};
#endif

