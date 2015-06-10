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

//#include "StdAfx.h"
#include "XDBBase.h"

XDBBase::XDBBase(void)
{
	memset(m_szServerAddress,	0, sizeof(m_szServerAddress));
	memset(m_szDataBase,		0, sizeof(m_szDataBase));
	memset(m_szUser,			0, sizeof(m_szUser));
	memset(m_szPassWord,		0, sizeof(m_szPassWord));
	m_bDBConnectted = false;
}

XDBBase::~XDBBase(void)
{
}
bool XDBBase::Initialize()
{
	return false;
}
bool XDBBase::SetDBServerAddress(TCHAR* pszServerAddress, TCHAR* pszDataBase, TCHAR* pszUser, TCHAR* pszPassWord)
{
	if (pszServerAddress == NULL || pszDataBase == NULL || pszUser == NULL || pszPassWord == NULL)
	{
		return false;
	}
	//_tcsncpy(m_szServerAddress, pszServerAddress, MAX_SERVER_ADDRESS_LEN);
	strncpy(m_szServerAddress, pszServerAddress, MAX_SERVER_ADDRESS_LEN);
	//_tcsncpy(m_szDataBase, pszDataBase, MAX_DATABASE_NAME_LEN);
	strncpy(m_szDataBase, pszDataBase, MAX_DATABASE_NAME_LEN);
	//_tcsncpy(m_szUser, pszUser, MAX_USER_NAME_LEN);
	strncpy(m_szUser, pszUser, MAX_USER_NAME_LEN);
	//_tcsncpy(m_szPassWord, pszPassWord, MAX_USER_PASSWORD_LEN);
	strncpy(m_szPassWord, pszPassWord, MAX_USER_PASSWORD_LEN);
	return true;
}
bool XDBBase::Connect()
{
	return false;
}
bool XDBBase::DisConnect()
{
	return false;
}
bool XDBBase::InitBindParam()
{
	return false;
}
bool XDBBase::BindParam(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen, E_Param_InOut_Type eParamType)
{
	return false;
}
bool XDBBase::InitBindCol()
{
	return false;
}

bool XDBBase::BindCol(E_Data_Type eDataType, void* pDataAddress, size_t sDataLen)
{
	return false;
}

bool XDBBase::PrepareSql(TCHAR* pszSql)
{
	return false;
}

bool XDBBase::ExcutePreparedSql()
{
	return false;
}

bool XDBBase::ExcuteSqlDirect(TCHAR* pszSql)
{
	return false;
}
bool XDBBase::Fetch()
{
	return false;
}
bool XDBBase::GetError(char* pszBuf, size_t sBufLen)
{
	return false;
}
bool XDBBase::FreeMoreResult(bool bFreeCursor)
{
	return false;
}
