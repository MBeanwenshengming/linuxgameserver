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

#include "LDBServerOpProcessManager.h"
#include "LDBOpThread.h"
#include "../NetWork/LWorkItem.h"

LDBServerOpProcessManager::LDBServerOpProcessManager()
{
}
LDBServerOpProcessManager::~LDBServerOpProcessManager()
{
}

bool LDBServerOpProcessManager::Initialize()
{
	if (m_pDBOpThread == NULL)
	{
		return false;
	}

	//	注册数据库处理函数
	
	return true;
}

bool LDBServerOpProcessManager::Register(unsigned int unWorkID, DBSERVER_DB_OP_PROCESS_PROC pProc)
{ 
	if (pProc == NULL)
	{
		return false;
	}
	map<unsigned int, DBSERVER_DB_OP_PROCESS_PROC>::iterator _ito = m_mapDBOpIDToProcessProc.find(unWorkID);
	if (_ito == m_mapDBOpIDToProcessProc.end())
	{
		return false;
	}
	m_mapDBOpIDToProcessProc[unWorkID] = pProc;
	return true;
}
void LDBServerOpProcessManager::DispatchDBOpToProcessProc(LWorkItem* pWorkItem)
{
	if (pWorkItem == NULL)
	{
		return;
	}
	unsigned int unWorkID = pWorkItem->GetWorkID();
	if (unWorkID == 0)
	{
		return ;
	}

	map<unsigned int, DBSERVER_DB_OP_PROCESS_PROC>::iterator _ito = m_mapDBOpIDToProcessProc.find(unWorkID);
	if (_ito == m_mapDBOpIDToProcessProc.end())
	{
		return ;
	}
	_ito->second(this, pWorkItem);
}



void LDBServerOpProcessManager::SetDBOpThread(LDBOpThread* pDBOpThread)
{
	m_pDBOpThread = pDBOpThread;
}
LDBOpThread* LDBServerOpProcessManager::GetDBOpThread()
{
	return m_pDBOpThread;
}

