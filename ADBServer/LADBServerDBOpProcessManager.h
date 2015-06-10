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

#include <map>
using namespace std;

class LADBServerDBOpProcessManager;
class LWorkItem;
class LDBOpThread;

#include "LUserDBOpMessageDefine.h"


typedef void (*ADBSERVER_DB_OP_PROCESS_PROC)(LADBServerDBOpProcessManager* pDBOpManager, LWorkItem* pWorkItem);

#define REGISTER_ADBSERVER_DB_OP_PROC(X) Register(X, (ADBSERVER_DB_OP_PROCESS_PROC)LADBServerDBOpProcessManager::Proc_##X);

#define DECLARE_ADBSERVER_DB_OP_PROC(X) static void Proc_##X(LADBServerDBOpProcessManager* pDBOpManager, LWorkItem* pWorkItem)

#define DEFINE_ADBSERVER_DB_OP_PROC(X) void LADBServerDBOpProcessManager::Proc_##X(LADBServerDBOpProcessManager* pDBOpManager, LWorkItem* pWorkItem)


class LADBServerDBOpProcessManager
{
public:
	LADBServerDBOpProcessManager();		
	~LADBServerDBOpProcessManager();
public:
	bool Initialize();
	bool Register(unsigned int unWorkID, ADBSERVER_DB_OP_PROCESS_PROC pProc);
	void DispatchDBOpToProcessProc(LWorkItem* pWorkItem);
private:
	map<unsigned int, ADBSERVER_DB_OP_PROCESS_PROC> m_mapDBOpIDToProcessProc;

public:
	void SetDBOpThread(LDBOpThread* pDBOpThread);
	LDBOpThread* GetDBOpThread();
private:
	LDBOpThread* m_pDBOpThread;
public:
	DECLARE_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_READ_USER_INFO);
	DECLARE_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_DELETE_USER_INFO);
	DECLARE_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_CHANGE_PASSWORD);
	DECLARE_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_ADD_USER_INFO);
};

