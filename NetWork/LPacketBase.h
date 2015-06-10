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


#define PACKET_BASE_ERROR_CODE_DATA_TOO_LONG   -1			//	压包时，数据太长
#define PACKET_BASE_ERROR_CODE_DATA_NOT_ENOUGH	-2		//	解包时，数据太短
#define PACKET_BASE_ERROR_CODE_BUF_NOT_INIT -3
#define PACKET_BASE_ERROR_CODE_INPUT_PARAM_WRONG -4
#define PACKET_BASE_ERROR_CODE_OUTPUT_PARAM_WRONG -5

#define PACKET_BASE_DATA_OFFSET 2			//	数据开始的偏移位置


class LPacketBase
{
public:
	LPacketBase(unsigned short usBufLen);
	virtual ~LPacketBase();

public:			// pack data
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


public:			// unpack data
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

	//	解压或者压缩失败时，设置的错误信息
	int GetErrorCode();
	void SetErrorCode(int nErrorCode);

	//	直接设置数据，以前的数据将被丢弃
	bool DirectSetData(char* pData, unsigned short usDataLen);
private:
	int m_nErrorCode;		//	错误信息保存，数据包初始化时，设置为0
public:
	void Reset();
	bool IsBufNull();

	//	数据包格式
	//	unsigned short usPacketDataLen			2字节指示整个数据包的大小
	//	unsigned int unPacketID						4字节指示数据包的ID
	//	data														跟随数据
public: 
	//	获取缓冲区的长度
	unsigned short GetPacketBufLen();
	//	获取缓冲区
	char* GetPacketBuf();

	//	获取数据的长度
	unsigned short GetDataLen();
	//	获取数据开始位置的char*指针
	char* GetDataBuf();

	//	获取数据和头的总长度	
	unsigned short GetPacketDataAndHeaderLen();

	//	获取数据包ID
	//unsigned int GetP
	//	bool SetPacketID(unsigned int unPacketID);	
	//	CRC32
	int MakeCRC32CodeToPacket();
	bool CheckCRC32Code();
private:
	const unsigned short m_usPacketBufLen;	
	char* m_pData;

protected:
	//	检查是否有足够的缓冲区容纳数据
	bool CheckHaveEnoughBuf(unsigned short usDataLen);
	//	检查是否还有足够的数据供读取
	bool CheckHaveEnoughData(unsigned short usNeedDataLen);

	//	64位转换为网络字节序
	uint64_t hton64(uint64_t nData);	
	//	64为转换为本地字节序
	uint64_t ntoh64(uint64_t nData);
private:
	//	当前读取的数据的位置，在解压时，标志已经解压的数据 
	unsigned short m_usCurrentReadPos;
public:
	unsigned char GetPacketType();
	void SetPacketType(unsigned char ucPacketType);
	virtual bool DecrementRefCountAndResultIsTrue();
private:
	unsigned char m_ucPacketType;	//	数据包的类型，一般为0,为1为接收连接的数据包
public:
	//	获取消息包的ID，在长度unsigned short后面跟随的就是消息包的ID unsigned int
	unsigned int GetPacketID();
	//	设置消息包的ID
	void SetPacketID(unsigned int unPacketID);
#ifdef __EPOLL_TEST_STATISTIC__
	void FillPacketForTest();
#endif
private:
};
#endif

