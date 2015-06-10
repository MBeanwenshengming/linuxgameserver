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

#ifdef _WIN32_
#pragma once
#include "xdbbase.h"
#include <sql.h>
#include <sqlext.h>

#define MAX_CONNECT_STRING_LEN 1024

#define SUCCEED_ODBC(X)		((SQL_SUCCESS == X)||(SQL_SUCCESS_WITH_INFO == X))
#define FAILED_ODBC(X)		((SQL_ERROR == X)||(SQL_INVALID_HANDLE == X))

class XSqlServerConnector :
	public XDBBase
{
public:
	XSqlServerConnector(void);
	virtual ~XSqlServerConnector(void);
public:
	virtual bool Initialize();	
	virtual bool Connect();
	virtual bool DisConnect();
public:
	virtual bool InitBindParam();
	virtual bool BindParam(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen, E_Param_InOut_Type eParamType = E_Param_Input);
	virtual bool InitBindCol();
	virtual bool BindCol(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen);

public:
	virtual bool PrepareSql(TCHAR* pszSql);
	virtual bool ExcutePreparedSql();
	virtual bool ExcuteSqlDirect(TCHAR* pszSql);

public:
	virtual bool Fetch();
	virtual bool GetError(char* pszBuf, size_t sBufLen);
public:
	virtual bool FreeMoreResult(bool bFreeCursor = false);

protected:
	bool BuildConnectString();
private:
	SQLHANDLE m_hEnv;
	SQLHANDLE m_hDBCon;
	SQLHANDLE m_hSTMT;

private:
	char m_szConnectString[MAX_CONNECT_STRING_LEN + 1];
	char m_szConnectedString[MAX_CONNECT_STRING_LEN * 2 + 1];

private:
	unsigned int m_unBindParamCount;
	unsigned int m_unBindColCount;
	SQLINTEGER m_dwBindIntAndFlag[MAX_BIND_PARAM_NUM];

protected:		//	���������

	//	��char unsignedchar
	SQLRETURN BindParam(INT8* pn8Value,SQLSMALLINT inoutType = SQL_PARAM_INPUT);
	SQLRETURN BindParam(UINT8* pun8Value,SQLSMALLINT inoutType = SQL_PARAM_INPUT);

	//	��˫�ֽ����
	SQLRETURN BindParam(short* psValue,SQLSMALLINT inoutType = SQL_PARAM_INPUT);
	SQLRETURN BindParam(unsigned short* pusValue,SQLSMALLINT inoutType = SQL_PARAM_INPUT);

	//	��int
	SQLRETURN BindParam(int* pnValue,SQLSMALLINT inoutType = SQL_PARAM_INPUT);
	SQLRETURN BindParam(unsigned int* punValue,SQLSMALLINT inoutType = SQL_PARAM_INPUT);

	//	��int64
	SQLRETURN BindParam(__int64* pn64Value,SQLSMALLINT inoutType = SQL_PARAM_INPUT);
	SQLRETURN BindParam(unsigned __int64* pun64Value,SQLSMALLINT inoutType = SQL_PARAM_INPUT); 

	//	�󶨹̶����ȵ��ַ�
	SQLRETURN BindParam(char* pstr,int dbColumnLen,SQLSMALLINT inoutType = SQL_PARAM_INPUT);
	
	//	�󶨱䳤���ַ�
	SQLRETURN BindParamVarChar(char* pstr,int dbColumnLen,SQLSMALLINT inoutType = SQL_PARAM_INPUT);	

	//	��float
	SQLRETURN BindParam(float* pfValue,SQLSMALLINT inoutType = SQL_PARAM_INPUT);
	
	//	��ʱ��
	SQLRETURN BindParam(TIMESTAMP_STRUCT* pdsDateTime,SQLSMALLINT inoutType = SQL_PARAM_INPUT);

	SQLRETURN BindParam(DATE_STRUCT* pdsDate,SQLSMALLINT inoutType = SQL_PARAM_INPUT);
	SQLRETURN BindParam(TIME_STRUCT* pdsTime,SQLSMALLINT inoutType = SQL_PARAM_INPUT);				

	//	��bool
	SQLRETURN BindParam(bool* pbValue, SQLSMALLINT inoutType = SQL_PARAM_INPUT);

protected:		//	���������
	SQLRETURN BindCol(INT8* pn8Value);	
	SQLRETURN BindCol(UINT8* pun8Value);

	SQLRETURN BindCol(short* psValue);
	SQLRETURN BindCol(unsigned short* pusValue);

	SQLRETURN BindCol(int* pnValue);
	SQLRETURN BindCol(unsigned int* punValue);
	
	SQLRETURN BindCol(__int64* pn64Value);
	SQLRETURN BindCol(unsigned __int64* pun64Value);

	SQLRETURN BindCol(bool* pbValue);	

	SQLRETURN BindCol(LPCTSTR str,int strBufMaxLen);
	SQLRETURN BindCol(BYTE* pstr,int dbColumnLen);		

	SQLRETURN BindCol(float* pfValue);

	SQLRETURN BindCol(DATE_STRUCT* pdsDate);
	SQLRETURN BindCol(TIME_STRUCT* pdsTime);				
	SQLRETURN BindCol(TIMESTAMP_STRUCT* pdsDateTime);
};
#endif
