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

#include "../NetWork/IncludeHeader.h"

#define MAX_BIND_PARAM_NUM 150
#define TCHAR char

typedef enum 
{
	E_Data_Type_NULL = 0,
	E_Data_Type_Char,
	E_Data_Type_Unsigned_Char,
	E_Data_Type_Short,
	E_Data_Type_Unsigned_Short,
	E_Data_Type_Int,
	E_Data_Type_Unsigned_Int,
	E_Data_Type_Int64,
	E_Data_Type_Unsigned_Int64,
	E_Data_Type_Bool,
	E_Data_Type_Float,
	E_Data_Type_String_Fix_Len,		//	�ֶζ���Ϊ�������ַ��ֶΣ���(char(num) ��:char(20))
	E_Data_Type_String,				//	�ֶζ���Ϊ�䳤���ַ��ֶΣ���(varchar(num) ��:varchar(20))
	E_Data_Type_Date,
	E_Data_Type_Time,
	E_Data_Type_DateTime,
}E_Data_Type;

typedef enum
{
	E_Param_Input,
	E_Param_Output,
	E_Param_InOut,
}E_Param_InOut_Type;

#define MAX_SERVER_ADDRESS_LEN  32
#define	MAX_DATABASE_NAME_LEN	64
#define MAX_USER_NAME_LEN		128
#define MAX_USER_PASSWORD_LEN	128

class XDBBase
{
public:
	XDBBase(void);
	virtual ~XDBBase(void);
public:
	virtual bool Initialize();
	bool SetDBServerAddress(TCHAR* pszServerAddress, TCHAR* pszDataBase, TCHAR* pszUser, TCHAR* pszPassWord);
	virtual bool Connect();
	virtual bool DisConnect();
public:
	virtual bool InitBindParam();
	virtual bool BindParam(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen, E_Param_InOut_Type eParamType);
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

public:
	bool GetDBConnectted()
	{
		return m_bDBConnectted;
	}
	TCHAR* GetServerAddress()
	{
		return m_szServerAddress;
	}
	TCHAR* GetDataBase()
	{
		return m_szDataBase;
	}
	TCHAR* GetUserName()
	{
		return m_szUser;
	}
	TCHAR* GetPassWord()
	{
		return m_szPassWord;
	}
private:
	TCHAR m_szServerAddress[MAX_SERVER_ADDRESS_LEN + 1];
	TCHAR m_szDataBase[MAX_DATABASE_NAME_LEN + 1];
	TCHAR m_szUser[MAX_USER_NAME_LEN + 1];
	TCHAR m_szPassWord[MAX_USER_PASSWORD_LEN + 1];

	bool m_bDBConnectted;
};
