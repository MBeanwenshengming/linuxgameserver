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
#include "XDBBase.h"
#include "mysql.h"

#define PROCEDURE_NAME_MAX_LEN 256		//	���洢������
#define SQL_STATEMENT_MAN_LEN 4 * 1024	//	�Sql���,4K����

typedef struct _Bind_Data_Ex
{
	unsigned long ulLength;
	my_bool       bmysqlIsNULL;
}t_Bind_Data_Ex;

typedef struct _Output_Result_Info
{
	E_Param_InOut_Type eParamInOUtType;		//	ΪInout����out
	E_Data_Type eDataType;
	void* pOutputValueBuf;
	unsigned int unBufLen;
}t_Output_Result_Info;

typedef struct _Param_Desc_Info
{
	E_Data_Type eDataType;
	void* pDataAddress;
	size_t sDataLen;
	E_Param_InOut_Type eParamType;
}t_Param_Desc_Info;			//	�󶨲�������

class XMySqlConnector :
	public XDBBase
{
public:
	XMySqlConnector(void);
	virtual ~XMySqlConnector(void);
public:
	virtual bool Initialize();	
	virtual bool Connect();
	virtual bool DisConnect();
protected:
	bool ExcuteBindParamAndPrepareSetValueSqlStatement();
	bool BindParamExcute(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen, E_Param_InOut_Type eParamType);
	bool BuildSetParamValue(char* pSqlStatementSetValue, unsigned int unbufLen, t_Param_Desc_Info& tpdi, int nOutputParamCount);
	bool BindOutputCol();
public:
	virtual bool InitBindParam();
	virtual bool BindParam(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen, E_Param_InOut_Type eParamType);
	virtual bool InitBindCol();
	virtual bool BindCol(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen);

	bool SetProcedureName(char* pProcedureName);
public:
	virtual bool ExcuteSql();
	virtual bool ExcuteSqlDirect(TCHAR* pszSql);

public:
	bool GetOutputResult();
public:
	virtual bool Fetch();
	virtual bool GetError(char* pszBuf, size_t sBufLen);
public:
	virtual bool FreeMoreResult(bool bFreeCursor = false);

private:		//	�ڶ������У�������󶨵��������������ͬ�ļ�¼��
	bool FindResult();
private:
	bool BindParamFinalize();
	bool BindColFinalize();
private:
	MYSQL* m_pmysql;
	MYSQL_STMT* m_pMySqlStmt;
	//	�洢������
	char m_szProcedureName[PROCEDURE_NAME_MAX_LEN + 1];
	//	Ҫִ�е����
	char* m_pSqlStatement;
	//	Ҫִ�е��������������
	char* m_pSqlStatementForSetParam;
	//	ѡ��out, inout����ķ��ؽ������
	char* m_pSelectOutputResultSqlStatement;
	//	��ǰ�󶨵Ĳ���ĸ���
	int m_nCurBindParamIndex;
	//	�Ƿ����inout����
	bool m_bExistInoutParam;
	//	�Ƿ����out����
	bool m_bExistOutParam;
	//	�󶨲���
	MYSQL_BIND m_bindParam[MAX_BIND_PARAM_NUM];
	t_Bind_Data_Ex m_BindParamEx[MAX_BIND_PARAM_NUM];

	unsigned int m_unInoutParamCount;
	int m_inoutParamIndex[MAX_BIND_PARAM_NUM];		//	inout����out������m_ParamDescInfo�е�λ��,����ȡ����ֵʱ�����

	//	�󶨲���������Ϣ����������󶨲���
	unsigned int m_unParamDescInfoCount;
	t_Param_Desc_Info m_ParamDescInfo[MAX_BIND_PARAM_NUM];
	//	��ǰ�󶨵��еĸ���
	int m_nCurBindColIndex;
	//	�󶨷��ص���
	MYSQL_BIND m_bindCol[MAX_BIND_PARAM_NUM];
	t_Bind_Data_Ex m_BindColEx[MAX_BIND_PARAM_NUM];
	bool m_bBindCol;
};
