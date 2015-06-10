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

#include "LADBServerDBOpProcessManager.h"
#include "LDBOpThread.h"
#include "../NetWork/LWorkItem.h"

LADBServerDBOpProcessManager::LADBServerDBOpProcessManager()
{
	m_pDBOpThread = NULL;
}
LADBServerDBOpProcessManager::~LADBServerDBOpProcessManager()
{
}
bool LADBServerDBOpProcessManager::Initialize()
{
	if (m_pDBOpThread == NULL)
	{
		return false;
	}
	REGISTER_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_READ_USER_INFO);
	REGISTER_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_DELETE_USER_INFO);
	REGISTER_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_CHANGE_PASSWORD);
	REGISTER_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_ADD_USER_INFO);
	return true;
}
bool LADBServerDBOpProcessManager::Register(unsigned int unWorkID, ADBSERVER_DB_OP_PROCESS_PROC pProc)
{ 
	if (pProc == NULL)
	{
		return false;
	}
	map<unsigned int, ADBSERVER_DB_OP_PROCESS_PROC>::iterator _ito = m_mapDBOpIDToProcessProc.find(unWorkID);
	if (_ito == m_mapDBOpIDToProcessProc.end())
	{
		return false;
	}
	m_mapDBOpIDToProcessProc[unWorkID] = pProc;
	return true;
}
void LADBServerDBOpProcessManager::DispatchDBOpToProcessProc(LWorkItem* pWorkItem)
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

	map<unsigned int, ADBSERVER_DB_OP_PROCESS_PROC>::iterator _ito = m_mapDBOpIDToProcessProc.find(unWorkID);
	if (_ito == m_mapDBOpIDToProcessProc.end())
	{
		return ;
	}
	_ito->second(this, pWorkItem);
}



void LADBServerDBOpProcessManager::SetDBOpThread(LDBOpThread* pDBOpThread)
{
	m_pDBOpThread = pDBOpThread;
}
LDBOpThread* LADBServerDBOpProcessManager::GetDBOpThread()
{
	return m_pDBOpThread;
}
