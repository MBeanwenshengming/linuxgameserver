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
//#include "StdAfx.h"
#include "XSqlServerConnector.h"

XSqlServerConnector::XSqlServerConnector(void)
{
	m_hEnv		= SQL_NULL_HANDLE;
	m_hDBCon	= SQL_NULL_HANDLE;
	m_hSTMT		= SQL_NULL_HANDLE;

	memset(m_szConnectString, 0, sizeof(m_szConnectString));
	memset(m_szConnectedString, 0, sizeof(m_szConnectedString));
}

XSqlServerConnector::~XSqlServerConnector(void)
{
}
bool XSqlServerConnector::Initialize()
{
	SQLRETURN sqlReturn = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
	if (!SQL_SUCCEEDED(sqlReturn))
	{
		return false;
	}
	sqlReturn = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if (!SQL_SUCCEEDED(sqlReturn))
	{
		return false;
	}
	sqlReturn = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDBCon);
	if (!SQL_SUCCEEDED(sqlReturn))
	{
		return false;
	}
	return true;
}

bool XSqlServerConnector::Connect()
{
	if (!BuildConnectString())
	{
		return false;
	}
	SQLSMALLINT siReturnConnectStringLen = 0;
	SQLRETURN sqlReturn = SQLDriverConnect(m_hDBCon, NULL, (SQLCHAR*)m_szConnectString, strlen(m_szConnectString), (SQLCHAR*)m_szConnectedString,
		MAX_CONNECT_STRING_LEN * 2, &siReturnConnectStringLen, SQL_DRIVER_COMPLETE);
	if (!SQL_SUCCEEDED(sqlReturn))
	{
		return false;
	}
	if (siReturnConnectStringLen >= MAX_CONNECT_STRING_LEN * 2)
	{
		return false;
	}

	sqlReturn = SQLAllocHandle(SQL_HANDLE_STMT, m_hDBCon, &m_hSTMT);
	if (!SQL_SUCCEEDED(sqlReturn))
	{
		return false;
	}

	return true;
}
bool XSqlServerConnector::DisConnect()
{
	return false;
}
bool XSqlServerConnector::InitBindParam()
{
	m_unBindParamCount = 0;
	memset(&m_dwBindIntAndFlag, 0, sizeof(m_dwBindIntAndFlag));
	return true;
}

bool XSqlServerConnector::BindParam(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen, E_Param_InOut_Type eParamType)
{
	SQLRETURN sqlReturn = SQL_SUCCESS;
	SQLSMALLINT sqlinoutType = SQL_PARAM_INPUT;
	if (E_Param_Input == E_Param_InOut)
	{
		sqlinoutType = SQL_PARAM_INPUT_OUTPUT;
	}
	else if (E_Param_Input == E_Param_Output)
	{
		sqlinoutType = SQL_PARAM_OUTPUT;
	}
	else
	{
		sqlinoutType = SQL_PARAM_INPUT;
	}

	switch(eDataType)
	{
	case E_Data_Type_Char:
		{
			sqlReturn = BindParam((char*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Unsigned_Char:
		{
			sqlReturn = BindParam((unsigned char*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Short:
		{
			sqlReturn = BindParam((short*)pDataAddress, sqlinoutType);
		}
		break;	
	case E_Data_Type_Unsigned_Short:
		{
			sqlReturn = BindParam((unsigned short*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Int:
		{
			sqlReturn = BindParam((int*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Unsigned_Int:
		{
			sqlReturn = BindParam((unsigned int*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Int64:
		{
			sqlReturn = BindParam((unsigned __int64*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Unsigned_Int64:
		{
			sqlReturn = BindParam((unsigned __int64*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Bool:
		{
			sqlReturn = BindParam((bool*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Float:
		{
			sqlReturn = BindParam((float*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_String:
		{
			sqlReturn = BindParam((char*)pDataAddress, (int)sDataLen, sqlinoutType);
		}
		break;
	case E_Data_Type_String_Fix_Len:
		{
			sqlReturn = BindParamVarChar((char*)pDataAddress, (int)sDataLen, sqlinoutType);
		}
		break;
	case E_Data_Type_Date:
		{
			return false;
			sqlReturn = BindParam((DATE_STRUCT*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_Time:
		{
			return false;
			sqlReturn = BindParam((TIME_STRUCT*)pDataAddress, sqlinoutType);
		}
		break;
	case E_Data_Type_DateTime:
		{
			sqlReturn = BindParam((TIMESTAMP_STRUCT*)pDataAddress, sqlinoutType);
		}
		break;
	default:
		return false;
	}
	if (!SUCCEED_ODBC(sqlReturn))
	{
		//	��ӡ������־
		return false;
	}
	return true;
}

bool XSqlServerConnector::InitBindCol()
{
	m_unBindColCount = 0;	
	memset(&m_dwBindIntAndFlag, 0, sizeof(m_dwBindIntAndFlag));
	return true;
}

bool XSqlServerConnector::BindCol(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen)
{
	SQLRETURN sqlReturn = SQL_SUCCESS;

	switch(eDataType)
	{
	case E_Data_Type_Char:
		{
			sqlReturn = BindCol((INT8*)pDataAddress);
		}
		break;
	case E_Data_Type_Unsigned_Char:
		{
			sqlReturn = BindCol((UINT8*)pDataAddress);
		}
		break;
	case E_Data_Type_Short:
		{
			sqlReturn = BindCol((short*)pDataAddress);
		}
		break;	
	case E_Data_Type_Unsigned_Short:
		{
			sqlReturn = BindCol((unsigned short*)pDataAddress);
		}
		break;
	case E_Data_Type_Int:
		{
			sqlReturn = BindCol((int*)pDataAddress);
		}
		break;
	case E_Data_Type_Unsigned_Int:
		{
			sqlReturn = BindCol((unsigned int*)pDataAddress);
		}
		break;
	case E_Data_Type_Int64:
		{
			sqlReturn = BindCol((__int64*)pDataAddress);
		}
		break;
	case E_Data_Type_Unsigned_Int64:
		{
			sqlReturn = BindCol((unsigned __int64*)pDataAddress);
		}
		break;
	case E_Data_Type_Bool:
		{
			sqlReturn = BindCol((bool*)pDataAddress);
		}
		break;
	case E_Data_Type_Float:
		{
			sqlReturn = BindCol((float*)pDataAddress);
		}
		break;
	case E_Data_Type_String:
		{
			sqlReturn = BindCol((LPCTSTR)pDataAddress, sDataLen);
		}
		break;
	case E_Data_Type_String_Fix_Len:
		{
			sqlReturn = BindCol((LPCTSTR)pDataAddress, sDataLen);
		}
		break;
	case E_Data_Type_Date:
		{
			sqlReturn = BindCol((DATE_STRUCT*)pDataAddress);
		}
		break;
	case E_Data_Type_Time:
		{		
			sqlReturn = BindCol((TIME_STRUCT*)pDataAddress);
		}
		break;
	case E_Data_Type_DateTime:
		{
			sqlReturn = BindCol((TIMESTAMP_STRUCT*)pDataAddress);
		}
		break;
	default:
		return false;
	}
	if (!SUCCEED_ODBC(sqlReturn))
	{
		//	��ӡ������־
		return false;
	}
	return true;
}

bool XSqlServerConnector::PrepareSql(TCHAR* pszSql)
{
	SQLRETURN sqlResult = ::SQLPrepare(m_hSTMT, (SQLCHAR*)pszSql, SQL_NTS);
	if (SUCCEED_ODBC(sqlResult))
	{
		return true;
	}
	else
	{
		//	error print
	}
	return false;
}

bool XSqlServerConnector::ExcutePreparedSql()
{
	FreeMoreResult(true);
	SQLRETURN sqlResult = SQL_SUCCESS;
	sqlResult = ::SQLExecute(m_hSTMT);
	if (!SUCCEED_ODBC(sqlResult))
	{
		FreeMoreResult(true);
		
		return false;
	}
	return true;
}

bool XSqlServerConnector::ExcuteSqlDirect(TCHAR* pszSql)
{
	FreeMoreResult(true);

	SQLRETURN sqlResult = SQL_SUCCESS;
	sqlResult = ::SQLExecDirect(m_hSTMT, (SQLCHAR*)pszSql, SQL_NTS);
	if (!SUCCEED_ODBC(sqlResult))
	{
		FreeMoreResult(true);
		//	error print
		return false;
	}
	return true;
}
bool XSqlServerConnector::Fetch()
{
	SQLRETURN sqlResult = SQL_SUCCESS;
	sqlResult = ::SQLFetch(m_hSTMT);
	if(SUCCEED_ODBC(sqlResult)) 
	{
		return true;
	}
	else if(sqlResult == SQL_NO_DATA)
	{
		sqlResult = ::SQLMoreResults(m_hSTMT);
		if(SUCCEED_ODBC(sqlResult))
		{
			return true;
		}
		else
		{
			return false;
		}
		//else if(sqlResult == SQL_NO_DATA) return false;	// escape while loop !!!
	}
	else
	{
		//	error print
		FreeMoreResult(true);
		return false;
	}
}
bool XSqlServerConnector::GetError(char* pszBuf, size_t sBufLen)
{
	return false;
}

bool XSqlServerConnector::FreeMoreResult(bool bFreeCursor)
{
	SQLRETURN sqlResult;
	while ((sqlResult = ::SQLMoreResults(m_hSTMT) ) != SQL_NO_DATA )
      ;

   if(bFreeCursor)
   {
	   sqlResult = ::SQLCloseCursor(m_hSTMT);
   }
	return true;
}


//	=============================protected function====================================
bool XSqlServerConnector::BuildConnectString()
{
	if (_tcslen(GetServerAddress()) == 0 || _tcslen(GetDataBase()) == 0
		|| _tcslen(GetUserName()) == 0 || _tcslen(GetPassWord()) == 0)
	{
		return false;
	}
	sprintf(m_szConnectString, "DRIVER={SQL SERVER};SERVER=%s;DB=%s;UID=%s;PWD=%s", GetServerAddress(), GetDataBase(), GetUserName(), GetPassWord());
	return true;
}

//	=============================Bind Param ===========================================
//	��char unsignedchar
SQLRETURN XSqlServerConnector::BindParam(INT8* pn8Value,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;

	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_TINYINT, SQL_TINYINT, 0, 0, pn8Value, 0, &m_dwBindIntAndFlag[m_unBindParamCount]);
}
SQLRETURN XSqlServerConnector::BindParam(UINT8* pun8Value,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;
	
	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_TINYINT, SQL_TINYINT, 0, 0, pun8Value, 0, &m_dwBindIntAndFlag[m_unBindParamCount]);
}

//	��˫�ֽ����
SQLRETURN XSqlServerConnector::BindParam(short* psValue,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;
	
	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount,inoutType, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, psValue, 0, &m_dwBindIntAndFlag[m_unBindParamCount]);
}
SQLRETURN XSqlServerConnector::BindParam(unsigned short* pusValue,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;
	
	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_USHORT, SQL_SMALLINT, 0, 0, pusValue, 0, &m_dwBindIntAndFlag[m_unBindParamCount]);
}

//	��int
SQLRETURN XSqlServerConnector::BindParam(int* pnValue,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;
	
	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_SLONG, SQL_INTEGER, 0, 0, pnValue, 0, &m_dwBindIntAndFlag[m_unBindParamCount]);
}
SQLRETURN XSqlServerConnector::BindParam(unsigned int* punValue,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;

	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_ULONG, SQL_INTEGER, 0, 0, punValue, 0,  &m_dwBindIntAndFlag[m_unBindParamCount]);
}

//	��int64
SQLRETURN XSqlServerConnector::BindParam(__int64* pn64Value,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;

	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, pn64Value, 0,  &m_dwBindIntAndFlag[m_unBindParamCount]);
}
SQLRETURN XSqlServerConnector::BindParam(unsigned __int64* pun64Value,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;

	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, pun64Value, 0,  &m_dwBindIntAndFlag[m_unBindParamCount]);
}

//	�󶨹̶����ȵ��ַ�
SQLRETURN XSqlServerConnector::BindParam(char* pstr,int dbColumnLen, SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = SQL_NTS;

	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_CHAR, SQL_CHAR, dbColumnLen, 0, pstr, 0, &m_dwBindIntAndFlag[m_unBindParamCount]);

}

//	�󶨱䳤���ַ�
SQLRETURN XSqlServerConnector::BindParamVarChar(char* pstr,int dbColumnLen,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = SQL_NTS;

	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_CHAR, SQL_VARCHAR, dbColumnLen, 0, pstr, 0, &m_dwBindIntAndFlag[m_unBindParamCount]);
}

//	��float
SQLRETURN XSqlServerConnector::BindParam(float* pfValue,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;
	
	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_FLOAT, SQL_FLOAT, 0, 0, pfValue, 0, &m_dwBindIntAndFlag[m_unBindParamCount]);
}

//	��ʱ��
SQLRETURN XSqlServerConnector::BindParam(TIMESTAMP_STRUCT* pdsDateTime,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;

	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, pdsDateTime, 0, &m_dwBindIntAndFlag[m_unBindParamCount]); 
}
SQLRETURN XSqlServerConnector::BindParam(DATE_STRUCT* pdsDate,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;
	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_TYPE_DATE, SQL_TYPE_DATE, SQL_DATE_LEN, 0, pdsDate, 0, &m_dwBindIntAndFlag[m_unBindParamCount]); 
}
SQLRETURN XSqlServerConnector::BindParam(TIME_STRUCT* pdsTime,SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;
	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_TYPE_TIME, SQL_TYPE_TIME, SQL_TIME_LEN, 0, pdsTime, 0, &m_dwBindIntAndFlag[m_unBindParamCount]); 
}

//	��bool
SQLRETURN XSqlServerConnector::BindParam(bool* pbValue, SQLSMALLINT inoutType)
{
	m_unBindParamCount++;	
	assert(m_unBindParamCount < MAX_BIND_PARAM_NUM);
	m_dwBindIntAndFlag[m_unBindParamCount] = 0;

	return ::SQLBindParameter(m_hSTMT, m_unBindParamCount, inoutType, SQL_C_BIT, SQL_BIT, 0, 0, pbValue, 0, &m_dwBindIntAndFlag[m_unBindParamCount]); 
}

//================================================BindCol============================================

SQLRETURN XSqlServerConnector::BindCol(INT8* pn8Value)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_TINYINT, pn8Value, sizeof(INT8), &m_dwBindIntAndFlag[m_unBindColCount]);
}
SQLRETURN XSqlServerConnector::BindCol(UINT8* pun8Value)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_UTINYINT, pun8Value, sizeof(UINT8), &m_dwBindIntAndFlag[m_unBindColCount]);

}

SQLRETURN XSqlServerConnector::BindCol(short* psValue)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_SSHORT, psValue, sizeof(short), &m_dwBindIntAndFlag[m_unBindColCount]);

}
SQLRETURN XSqlServerConnector::BindCol(unsigned short* pusValue)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_USHORT, pusValue, sizeof(unsigned short), &m_dwBindIntAndFlag[m_unBindColCount]);
}

SQLRETURN XSqlServerConnector::BindCol(int* pnValue)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_SLONG, pnValue, sizeof(int), &m_dwBindIntAndFlag[m_unBindColCount]);
}
SQLRETURN XSqlServerConnector::BindCol(unsigned int* punValue)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_ULONG, punValue, sizeof(unsigned int), &m_dwBindIntAndFlag[m_unBindColCount]);
}

SQLRETURN XSqlServerConnector::BindCol(__int64* pn64Value)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_SBIGINT, pn64Value, sizeof(__int64), &m_dwBindIntAndFlag[m_unBindColCount]);
}
SQLRETURN XSqlServerConnector::BindCol(unsigned __int64* pun64Value)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_UBIGINT, pun64Value, sizeof(unsigned __int64), &m_dwBindIntAndFlag[m_unBindColCount]);
}

SQLRETURN XSqlServerConnector::BindCol(bool* pbValue)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_BIT, pbValue, sizeof(SQL_C_BIT), &m_dwBindIntAndFlag[m_unBindColCount]);
}

SQLRETURN XSqlServerConnector::BindCol(LPCTSTR str,int strBufMaxLen)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_TCHAR, (SQLPOINTER*)str, strBufMaxLen, &m_dwBindIntAndFlag[m_unBindColCount]);
}
SQLRETURN XSqlServerConnector::BindCol(BYTE* pstr,int dbColumnLen)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_BINARY, (SQLPOINTER*)pstr, dbColumnLen, &m_dwBindIntAndFlag[m_unBindColCount]);
}

SQLRETURN XSqlServerConnector::BindCol(float* pfValue)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_FLOAT, pfValue, sizeof(float), &m_dwBindIntAndFlag[m_unBindColCount]);
}

SQLRETURN XSqlServerConnector::BindCol(DATE_STRUCT* pdsDate)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_TYPE_DATE, pdsDate, sizeof(DATE_STRUCT), &m_dwBindIntAndFlag[m_unBindColCount]);

}
SQLRETURN XSqlServerConnector::BindCol(TIME_STRUCT* pdsTime)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_TYPE_TIME, pdsTime, sizeof(TIME_STRUCT), &m_dwBindIntAndFlag[m_unBindColCount]);
}

SQLRETURN XSqlServerConnector::BindCol(TIMESTAMP_STRUCT* pdsDateTime)
{
	m_unBindColCount++;
	assert(m_unBindColCount < MAX_BIND_PARAM_NUM);
	
	return	SQLBindCol(m_hSTMT, m_unBindColCount, SQL_C_TIMESTAMP, pdsDateTime, sizeof(SQL_C_TIMESTAMP), &m_dwBindIntAndFlag[m_unBindColCount]);
}
#endif
