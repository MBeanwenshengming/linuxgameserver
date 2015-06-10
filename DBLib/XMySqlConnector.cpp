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

#ifdef _WIN32
#include "StdAfx.h"
#endif
#include "XMySqlConnector.h"

XMySqlConnector::XMySqlConnector(void)
{
	m_pmysql		= NULL;
	m_pMySqlStmt	= NULL;
		
	m_nCurBindParamIndex	= 0;
	memset(&m_bindParam, 0, sizeof(m_bindParam));
	
	m_nCurBindColIndex		= 0;
	memset(&m_bindCol, 0, sizeof(m_bindCol));
	m_bBindCol = false;

	//m_bOutputResultExists = false;		//	���ڴ洢��̵ķ��ؽ��
	//m_nOutputResultFieldNum = 0;
	//memset(m_OutputInfo, 0, sizeof(m_OutputInfo));
	//memset(m_szOutputResultInfoSql, 0, sizeof(m_szOutputResultInfoSql));

	memset(m_ParamDescInfo, 0, sizeof(m_ParamDescInfo));
	m_unParamDescInfoCount = 0;	
	memset(m_szProcedureName, 0, sizeof(m_szProcedureName));

	//	Ҫִ�е����
	m_pSqlStatement = NULL;
	//	Ҫִ�е��������������
	m_pSqlStatementForSetParam = NULL;

	//	ѡ��out, inout����ķ��ؽ������
	m_pSelectOutputResultSqlStatement = NULL;

	//	�Ƿ����inout����
	m_bExistInoutParam = false;
	//	�Ƿ����out����
	m_bExistOutParam = false;
	m_unInoutParamCount = 0;
	memset(m_inoutParamIndex, 0, sizeof(m_inoutParamIndex));		//	inout����out������m_ParamDescInfo�е�λ��
}

XMySqlConnector::~XMySqlConnector(void)
{
	if (m_pSqlStatement != NULL)
	{
		delete[] m_pSqlStatement;
		m_pSqlStatement = NULL;
	}
	if (m_pSqlStatementForSetParam != NULL)
	{
		delete[] m_pSqlStatementForSetParam;
		m_pSqlStatementForSetParam = NULL;
	}
	if (m_pSelectOutputResultSqlStatement != NULL)
	{
		delete[] m_pSelectOutputResultSqlStatement;
		m_pSelectOutputResultSqlStatement = NULL;
	}
}

bool XMySqlConnector::Initialize()
{
	m_pmysql = mysql_init(NULL);
	if (m_pmysql == NULL)
	{
		return false;
	}
	m_pSqlStatement = new char[SQL_STATEMENT_MAN_LEN + 1];
	if (m_pSqlStatement == NULL)
	{
		return false;
	}
	m_pSqlStatementForSetParam = new char[SQL_STATEMENT_MAN_LEN + 1];
	if (m_pSqlStatementForSetParam == NULL)
	{
		return false;
	}
	m_pSelectOutputResultSqlStatement = new char[SQL_STATEMENT_MAN_LEN + 1];
	if (m_pSelectOutputResultSqlStatement == NULL)
	{
		return false;
	}
	return true;
}

bool XMySqlConnector::Connect()
{
	MYSQL* pMysql = mysql_real_connect(m_pmysql, this->GetServerAddress(), this->GetUserName(), this->GetPassWord(), this->GetDataBase(), 3306, NULL, CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS);
	if (pMysql == NULL)
	{	
		return false;
	}

	m_pMySqlStmt = mysql_stmt_init(m_pmysql);
	if (m_pMySqlStmt == NULL)
	{
		return false;
	}
	return true;
}
bool XMySqlConnector::DisConnect()
{
	if (m_pMySqlStmt != NULL)
	{
		my_bool bmySuccess = mysql_stmt_close(m_pMySqlStmt);
		if (bmySuccess)
		{
		}
	}
	if (m_pmysql != NULL)
	{
		mysql_close(m_pmysql);
	}
	return true;
}

bool XMySqlConnector::InitBindParam()
{
	m_nCurBindParamIndex	= 0;
	memset(&m_bindParam[0], 0, sizeof(m_bindParam));
	memset(&m_BindParamEx[0], 0, sizeof(m_BindParamEx));	
	m_unParamDescInfoCount = 0;
	memset(&m_ParamDescInfo[0], 0, sizeof(m_ParamDescInfo));
	memset(m_szProcedureName, 0, sizeof(m_szProcedureName));
	memset(m_pSqlStatement, 0, SQL_STATEMENT_MAN_LEN);
	memset(m_pSqlStatementForSetParam, 0, SQL_STATEMENT_MAN_LEN);
	memset(m_pSelectOutputResultSqlStatement, 0, SQL_STATEMENT_MAN_LEN);
	//	�Ƿ����inout����
	m_bExistInoutParam = false;
	//	�Ƿ����out����
	m_bExistOutParam = false;

	m_unInoutParamCount = 0;
	memset(m_inoutParamIndex, 0, sizeof(m_inoutParamIndex));		//	inout����out������m_ParamDescInfo�е�λ��

	return true;
}
bool XMySqlConnector::SetProcedureName(char* pProcedureName)
{
	if (pProcedureName == NULL)
	{
		return false;
	}
	if (strlen(pProcedureName) > PROCEDURE_NAME_MAX_LEN)
	{
		return false;
	}
	strncpy(m_szProcedureName, pProcedureName, PROCEDURE_NAME_MAX_LEN);
	return true;
}

bool XMySqlConnector::BuildSetParamValue(char* pSqlStatementSetValue, unsigned int unbufLen, t_Param_Desc_Info& tpdi, int nOutputParamCount)
{
	char szTemp[1024 + 1];
	memset(szTemp, 0, sizeof(szTemp));
	switch (tpdi.eDataType)
	{
	case E_Data_Type_Char:
		{
			sprintf(szTemp, "set @ret_%d=\'%c\';", nOutputParamCount, *((char*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_Unsigned_Char:
		{
			sprintf(szTemp, "set @ret_%d=\'%c\';", nOutputParamCount, *((unsigned char*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_Short:
		{
			sprintf(szTemp, "set @ret_%d=%hd;", nOutputParamCount, *((short*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_Unsigned_Short:
		{
			sprintf(szTemp, "set @ret_%d=%hu;", nOutputParamCount, *((unsigned short*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_Int:
		{
			sprintf(szTemp, "set @ret_%d=%d;", nOutputParamCount, *((int*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_Unsigned_Int:
		{
			sprintf(szTemp, "set @ret_%d=%u;", nOutputParamCount, *((unsigned int*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_Int64:
		{
#ifdef _WIN32
			sprintf(szTemp, "set @ret_%d=%I64d;", nOutputParamCount, *((__int64*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
#else
			sprintf(szTemp, "set @ret_%d=%lld;", nOutputParamCount, *((int64_t*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
#endif
		}
		break;
	case E_Data_Type_Unsigned_Int64:
		{
#ifdef _WIN32
			sprintf(szTemp, "set @ret_%d=%I64u;", nOutputParamCount, *((unsigned __int64*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
#else
			sprintf(szTemp, "set @ret_%d=%llu;", nOutputParamCount, *((uint64_t*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
#endif
		}
		break;
	case E_Data_Type_Bool:
		{

			bool b = *((bool*)tpdi.pDataAddress);
			if (b)
			{
				sprintf(szTemp, "set @ret_%d=1;", nOutputParamCount);
			}
			else
			{
				sprintf(szTemp, "set @ret_%d=0;", nOutputParamCount);
			}
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_Float:
		{
			sprintf(szTemp, "set @ret_%d=%f;", nOutputParamCount, *((float*)tpdi.pDataAddress));
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_String_Fix_Len:
		{
			char* pTemp = new char[tpdi.sDataLen * 2 + 2];
			unsigned long ulReturnLen = mysql_real_escape_string(m_pmysql, pTemp, (char*)tpdi.pDataAddress, tpdi.sDataLen);
			sprintf(szTemp, "set @ret_%d=\'%s\';", nOutputParamCount, pTemp);
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
			delete[] pTemp;
		}
		break;
	case E_Data_Type_String:
		{
			char* pTemp = new char[strlen((char*)tpdi.pDataAddress) * 2 + 2];			
			unsigned long ulReturnLen = mysql_real_escape_string(m_pmysql, pTemp, (char*)tpdi.pDataAddress, strlen((char*)tpdi.pDataAddress));
			sprintf(szTemp, "set @ret_%d=\'%s\';", nOutputParamCount, pTemp);
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
			delete[] pTemp;
		}
		break;
	case E_Data_Type_Date:
		{
			MYSQL_TIME* pMysqlTime = (MYSQL_TIME*)tpdi.pDataAddress;
			char szTempDatatime[256];
			sprintf(szTempDatatime, "%4d-%2d-%2d", pMysqlTime->year, pMysqlTime->month, pMysqlTime->day);
			sprintf(szTemp, "set @ret_%d=\'%s\';", nOutputParamCount, szTempDatatime);
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_Time:
		{
			MYSQL_TIME* pMysqlTime = (MYSQL_TIME*)tpdi.pDataAddress;
			char szTempDatatime[256];
			sprintf(szTempDatatime, "%2d:%2d:%2d", pMysqlTime->hour, pMysqlTime->minute, pMysqlTime->second);
			sprintf(szTemp, "set @ret_%d=\'%s\';", nOutputParamCount, szTempDatatime);
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	case E_Data_Type_DateTime:
		{
			MYSQL_TIME* pMysqlTime = (MYSQL_TIME*)tpdi.pDataAddress;
			char szTempDatatime[256];
			sprintf(szTempDatatime, "%4d-%2d-%2d %2d:%2d:%2d", pMysqlTime->year, pMysqlTime->month, pMysqlTime->day, pMysqlTime->hour, pMysqlTime->minute, pMysqlTime->second);
			sprintf(szTemp, "set @ret_%d=\'%s\';", nOutputParamCount, szTempDatatime);
			strncat(pSqlStatementSetValue, szTemp, unbufLen);
		}
		break;
	default:
		return false;
	}
	return true;
}

bool XMySqlConnector::ExcuteBindParamAndPrepareSetValueSqlStatement()
{
	sprintf(m_pSqlStatement, "call %s(", m_szProcedureName);
	sprintf(m_pSelectOutputResultSqlStatement, "select ");
	int nOutputParamCount = 0;
	for (unsigned int unIndex = 0; unIndex < m_unParamDescInfoCount; ++unIndex)
	{
		if (m_ParamDescInfo[unIndex].eParamType == E_Param_Input)
		{
			if (!BindParamExcute(m_ParamDescInfo[unIndex].eDataType, m_ParamDescInfo[unIndex].pDataAddress, m_ParamDescInfo[unIndex].sDataLen, m_ParamDescInfo[unIndex].eParamType))
			{
				return false;
			}
			char szTemp[256];
			if (unIndex + 1 == m_unParamDescInfoCount)
			{
				sprintf(szTemp, "?");
				strcat(m_pSqlStatement, szTemp);
			}
			else
			{
				sprintf(szTemp, "?,");
				strcat(m_pSqlStatement, szTemp);
			}
		}
		else if (m_ParamDescInfo[unIndex].eParamType == E_Param_Output)		//	���������ô��Ҫ
		{	
			m_inoutParamIndex[m_unInoutParamCount] = unIndex;
			m_unInoutParamCount++;

			m_bExistOutParam = true;
			nOutputParamCount++;
			char szTemp[256];
			if (unIndex + 1 == m_unParamDescInfoCount)
			{
				sprintf(szTemp, "@ret_%d", nOutputParamCount);
				strcat(m_pSqlStatement, szTemp);
				strcat(m_pSelectOutputResultSqlStatement, szTemp);
			}
			else
			{
				sprintf(szTemp, "@ret_%d,", nOutputParamCount);
				strcat(m_pSqlStatement, szTemp);
				strcat(m_pSelectOutputResultSqlStatement, szTemp);
			}
		}
		else if (m_ParamDescInfo[unIndex].eParamType == E_Param_InOut)
		{		
			m_inoutParamIndex[m_unInoutParamCount] = unIndex;
			m_unInoutParamCount++;

			m_bExistInoutParam = true;
			nOutputParamCount++;
			char szTemp[256];
			BuildSetParamValue(m_pSqlStatementForSetParam, SQL_STATEMENT_MAN_LEN, m_ParamDescInfo[unIndex], nOutputParamCount);
			if (unIndex + 1 == m_unParamDescInfoCount)
			{				
				sprintf(szTemp, "@ret_%d", nOutputParamCount);
				strcat(m_pSqlStatement, szTemp);
				strcat(m_pSelectOutputResultSqlStatement, szTemp);
			}
			else
			{				
				sprintf(szTemp, "@ret_%d,", nOutputParamCount);
				strcat(m_pSqlStatement, szTemp);
				strcat(m_pSelectOutputResultSqlStatement, szTemp);
			}
		}
		else
		{
			return false;
		}			
	}
	strcat(m_pSqlStatement, ")");	
	return true;
}
bool XMySqlConnector::BindParamExcute(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen, E_Param_InOut_Type eParamType)
{
	switch(eDataType)
	{
	case E_Data_Type_Char:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_TINY;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Unsigned_Char:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_TINY;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)1;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Short:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_SHORT;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Unsigned_Short:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_SHORT;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)1;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Int:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_LONG;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Unsigned_Int:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_LONG;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)1;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Int64:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_LONGLONG;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Unsigned_Int64:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_LONGLONG;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)1;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Bool:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_BIT;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Float:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_FLOAT;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_String_Fix_Len:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}

			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_STRING;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			m_bindParam[m_nCurBindParamIndex].buffer_length = sDataLen;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].ulLength		= (unsigned long)sDataLen;
				m_bindParam[m_nCurBindParamIndex].length			= &m_BindParamEx[m_nCurBindParamIndex].ulLength;
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_String:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_VAR_STRING;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			m_bindParam[m_nCurBindParamIndex].buffer_length = sDataLen;
			if (pDataAddress == NULL)
			{
				m_BindParamEx[m_nCurBindParamIndex].ulLength		= (unsigned long)sDataLen;
				m_bindParam[m_nCurBindParamIndex].length			= &m_BindParamEx[m_nCurBindParamIndex].ulLength;
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Date:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_DATE;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			//	m_bindParam[m_nCurBindParamIndex].buffer_length = sDataLen;
			if (pDataAddress == NULL)
			{
				//m_BindParamEx[m_nCurBindParamIndex].ulLength		= (unsigned long)sDataLen;
				//m_bindParam[m_nCurBindParamIndex].length			= &m_BindParamEx[m_nCurBindParamIndex].ulLength;
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_Time:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_TIME;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			//	m_bindParam[m_nCurBindParamIndex].buffer_length = sDataLen;
			if (pDataAddress == NULL)
			{
				//m_BindParamEx[m_nCurBindParamIndex].ulLength		= (unsigned long)sDataLen;
				//m_bindParam[m_nCurBindParamIndex].length			= &m_BindParamEx[m_nCurBindParamIndex].ulLength;
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	case E_Data_Type_DateTime:
		{
			if (m_nCurBindParamIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindParamIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			//	���ð������е�һ��λ�õİ���Ϣ
			m_bindParam[m_nCurBindParamIndex].buffer_type	= MYSQL_TYPE_DATETIME;
			m_bindParam[m_nCurBindParamIndex].buffer		= pDataAddress;
			m_bindParam[m_nCurBindParamIndex].is_unsigned	= (my_bool)0;
			//	m_bindParam[m_nCurBindParamIndex].buffer_length = sDataLen;
			if (pDataAddress == NULL)
			{
				//m_BindParamEx[m_nCurBindParamIndex].ulLength		= (unsigned long)sDataLen;
				//m_bindParam[m_nCurBindParamIndex].length			= &m_BindParamEx[m_nCurBindParamIndex].ulLength;
				m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL	= (my_bool)1;
				m_bindParam[m_nCurBindParamIndex].is_null			= &m_BindParamEx[m_nCurBindParamIndex].bmysqlIsNULL;
			}
			m_nCurBindParamIndex++;
			return true;
		}
		break;
	default:
		break;
	}
	return true;
}
bool XMySqlConnector::BindParam(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen, E_Param_InOut_Type eParamType)
{
	if (m_unParamDescInfoCount >= MAX_BIND_PARAM_NUM)
	{
		return false;
	}
	m_ParamDescInfo[m_unParamDescInfoCount].eDataType		= eDataType;
	m_ParamDescInfo[m_unParamDescInfoCount].eParamType		= eParamType;
	m_ParamDescInfo[m_unParamDescInfoCount].pDataAddress	= pDataAddress;
	m_ParamDescInfo[m_unParamDescInfoCount].sDataLen		= sDataLen;
	m_unParamDescInfoCount++;
	return true;
}

bool XMySqlConnector::BindParamFinalize()
{
	if (m_pmysql == NULL)
	{
		return false;
	}
	if (m_pMySqlStmt == NULL)
	{
		return false;
	}
	if (!ExcuteBindParamAndPrepareSetValueSqlStatement())
	{
		return false;
	}
	return true;
}
bool XMySqlConnector::InitBindCol()
{
	m_nCurBindColIndex	= 0;
	memset(&m_bindCol, 0, sizeof(m_bindCol));
	memset(&m_BindColEx[0], 0, sizeof(m_BindColEx));
	m_bBindCol			= false;
	return true;
}
bool XMySqlConnector::BindCol(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen)
{
	switch(eDataType)
	{
	case E_Data_Type_Char:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_TINY;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Unsigned_Char:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_TINY;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_unsigned	= (my_bool)1;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Short:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_SHORT;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;	
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Unsigned_Short:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_SHORT;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_unsigned	= (my_bool)1;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Int:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_LONG;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_unsigned	= (my_bool)0;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Unsigned_Int:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_LONG;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_unsigned	= (my_bool)1;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Int64:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_LONGLONG;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_unsigned	= (my_bool)0;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Unsigned_Int64:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_LONGLONG;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_unsigned	= (my_bool)1;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Bool:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_BIT;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;	
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Float:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_FLOAT;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;	
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_String_Fix_Len:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_STRING;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;	
			m_bindCol[m_nCurBindColIndex].buffer_length	= (unsigned long)sDataLen;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_bindCol[m_nCurBindColIndex].length		= &m_BindColEx[m_nCurBindColIndex].ulLength;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_String:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_VAR_STRING;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;	
			m_bindCol[m_nCurBindColIndex].buffer_length	= (unsigned long)sDataLen;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_bindCol[m_nCurBindColIndex].length		= &m_BindColEx[m_nCurBindColIndex].ulLength;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Date:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_DATE;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_Time:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_TIME;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	case E_Data_Type_DateTime:
		{
			if (m_nCurBindColIndex >= MAX_BIND_PARAM_NUM - 1)
			{
				assert(m_nCurBindColIndex < MAX_BIND_PARAM_NUM - 1);
				return false;
			}
			m_bindCol[m_nCurBindColIndex].buffer_type	= MYSQL_TYPE_DATETIME;
			m_bindCol[m_nCurBindColIndex].buffer		= pDataAddress;
			m_bindCol[m_nCurBindColIndex].is_null		= &m_BindColEx[m_nCurBindColIndex].bmysqlIsNULL;
			m_nCurBindColIndex++;
			return true;
		}
		break;
	default:
		break;
	}
	return true;	
}
bool XMySqlConnector::BindOutputCol()
{
	//	��������У���Ϊ�洢��̷��صĽ���Ѿ�ʹ�������
	m_nCurBindColIndex	= 0;
	memset(&m_bindCol, 0, sizeof(m_bindCol));
	memset(&m_BindColEx[0], 0, sizeof(m_BindColEx));

	for (unsigned int unIndex = 0; unIndex < m_unInoutParamCount; ++unIndex)
	{
		int nArrayIndex = m_inoutParamIndex[unIndex];
		if (nArrayIndex >= m_unParamDescInfoCount)
		{
			return false;
		}
		t_Param_Desc_Info tpdi = m_ParamDescInfo[nArrayIndex];
		if (!BindCol(tpdi.eDataType, tpdi.pDataAddress, tpdi.sDataLen))
		{
			return false;
		}
	}
	return true;
}


bool XMySqlConnector::BindColFinalize()
{
	if (m_pmysql == NULL)
	{
		return false;
	}
	if (m_pMySqlStmt == NULL)
	{
		return false;
	}
	if (mysql_stmt_bind_result(m_pMySqlStmt, m_bindCol) != 0)
	{
		return false;
	}
	return true;
}
//bool XMySqlConnector::PrepareSql(TCHAR* pszSql)
//{
//	if (m_pMySqlStmt == NULL)
//	{
//		return false;
//	}
//	int nPrepare = mysql_stmt_prepare(m_pMySqlStmt, pszSql, strlen(pszSql));
//	if (nPrepare != 0)
//	{
//		return false;
//	}
//	return true;
//}
bool XMySqlConnector::ExcuteSql()
{
	if (m_pMySqlStmt == NULL)
	{
		return false;
	}	
	if (!BindParamFinalize())
	{
		return false;
	}

	//	����������������ֵ,�������ֱ�Ӱ���
	if (m_bExistInoutParam)
	{
		if (strlen(m_pSqlStatementForSetParam) == 0)
		{
			return false;
		}
		if (!ExcuteSqlDirect(m_pSqlStatementForSetParam))
		{
			return false;
		}
	}

	//	׼��SQL��Ҫִ�е����
	if (strlen(m_pSqlStatement) == 0)
	{
		return false;
	}
	int nPrepare = mysql_stmt_prepare(m_pMySqlStmt, m_pSqlStatement, strlen(m_pSqlStatement));
	if (nPrepare != 0)
	{
		return false;
	}

	if (m_nCurBindParamIndex != mysql_stmt_param_count(m_pMySqlStmt))
	{
		return false;
	}

	my_bool bmyRes = mysql_stmt_bind_param(m_pMySqlStmt, m_bindParam);
	if (bmyRes != 0)
	{
		return false;
	}

	m_bBindCol			= false;		//	ÿ��ִ��ǰ��������δ�󶨽��
	int nResult = mysql_stmt_execute(m_pMySqlStmt);
	if (nResult != 0)
	{
		return false;
	}
	return true;
}

bool XMySqlConnector::ExcuteSqlDirect(TCHAR* pszSql)
{	
	if (m_pMySqlStmt == NULL)
	{
		return false;
	}
	int nPrepare = mysql_stmt_prepare(m_pMySqlStmt, pszSql, strlen(pszSql));
	if (nPrepare != 0)
	{
		return false;
	}
	int nResult = mysql_stmt_execute(m_pMySqlStmt);
	if (nResult != 0)
	{
		return false;
	}
	return true;
}

//	�ڶ������У�������󶨵��������������ͬ�ļ�¼��
bool XMySqlConnector::FindResult()
{
	do
	{
		int nResult = mysql_stmt_store_result(m_pMySqlStmt);

		if (nResult != 0)
		{
			return false;
		}

		if (mysql_stmt_field_count(m_pMySqlStmt) == m_nCurBindColIndex)				
		{	
			bool bIdentical = true;
			for (unsigned int i = 0; i < m_nCurBindColIndex; ++i)
			{
				if (m_pMySqlStmt->fields[i].type != m_bindCol[i].buffer_type)
				{
					bIdentical = false;
					break;
				}
			}
			if (bIdentical)
			{
				return true;
			}

			mysql_stmt_free_result(m_pMySqlStmt);
		}
		else
		{
			mysql_stmt_free_result(m_pMySqlStmt);
		}
	}while(!mysql_stmt_next_result(m_pMySqlStmt));

	return true;
}
bool XMySqlConnector::Fetch()
{
	if (m_pmysql == NULL)
	{
		return false;
	}
	if (m_pMySqlStmt == NULL)
	{
		return false;
	}

	if (!m_bBindCol)				//	���û�а󶨽����ô���Ȳ�����Ҫ�Ľ��
									//	Ȼ���ٰ��ֶΣ���Ϊ���ô洢���,���ص��Ƕ�������ֱ�Ӱ��ֶ�
									//	ֱ�Ӱ󶨾����ϳ���]
									//	ֻ�ܸ�ݿͻ�ָ�����ֶβ��Ҷ�Ӧ�Ľ��
	{
		if (!FindResult())			//	���ָ���ֶΣ����ҽ��
		{
			return false;
		}
		else
		{
			if (!BindColFinalize())		//	���ҵ�ָ���ֶΣ���ô��
			{
				return false;
			}
			else
			{
				m_bBindCol = true;
			}
		}
	}
	int nResult = mysql_stmt_fetch(m_pMySqlStmt);		//	��ȡ���
	if (nResult != 0)
	{
		return false;
	}
	return true;
}
bool XMySqlConnector::GetOutputResult()
{
	//	�ͷŴ洢��̷��صĽ���
	FreeMoreResult();
	if (m_bExistOutParam == false && m_bExistInoutParam == false)
	{
		return false;
	}
	if (strlen(m_pSelectOutputResultSqlStatement) == 0)
	{
		return false;
	}
	//	��ȡOutput���
	if (!ExcuteSqlDirect(m_pSelectOutputResultSqlStatement))
	{
		return false;
	}
	if (!BindOutputCol())
	{
		return false;
	}
	if (mysql_stmt_bind_result(m_pMySqlStmt, m_bindCol) != 0)
	{
		return false;
	}
	int nResult = mysql_stmt_fetch(m_pMySqlStmt);		//	��ȡ���
	if (nResult != 0)
	{
		return false;
	}
	//	�ͷŷ��ز������Ľ���
	FreeMoreResult();
	return true;
}

bool XMySqlConnector::GetError(char* pszBuf, size_t sBufLen)
{
	if (m_pmysql == NULL)
	{
		return false;
	}
	if (m_pMySqlStmt == NULL)
	{
		return false;
	}
	if (mysql_stmt_errno(m_pMySqlStmt))		//	����errorno�Ŵ��ڴ���
	{
		const char* pszErrorString = mysql_stmt_error(m_pMySqlStmt);
		strncpy(pszBuf, pszErrorString, sBufLen);
	}
	return true;
}
bool XMySqlConnector::FreeMoreResult(bool bFreeCursor)
{
	if (m_pMySqlStmt == NULL)
	{
		return false;
	}
	//	�����ͷŵ�ǰ�Ľ��
	if (mysql_stmt_free_result(m_pMySqlStmt) != 0)
	{
		return false;
	}

	do
	{		
		//	�ٲ鿴�Ƿ����������,�����ڣ����ͷţ��������ˣ���ô�˳�ѭ��
		int nReturn = mysql_stmt_next_result(m_pMySqlStmt);
		if (nReturn == -1)
		{
			return true;
		}
		else if (nReturn  == 0)
		{
			if (mysql_stmt_free_result(m_pMySqlStmt) != 0)
			{
				return false;
			}
		}
		else
		{
			return false;
		}

	}while(1);

	return true;
}



