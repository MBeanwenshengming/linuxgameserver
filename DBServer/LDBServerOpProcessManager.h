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


class LDBServerOpProcessManager;
class LWorkItem;
class LDBOpThread;

#include "LGameDBOpMessageDefine.h"


typedef void (*DBSERVER_DB_OP_PROCESS_PROC)(LDBServerOpProcessManager* pDBOpManager, LWorkItem* pWorkItem);

#define REGISTER_DBSERVER_DB_OP_PROC(X) Register(X, (DBSERVER_DB_OP_PROCESS_PROC)LDBServerOpProcessManager::Proc_##X);

#define DECLARE_DBSERVER_DB_OP_PROC(X) static void Proc_##X(LDBServerOpProcessManager* pDBOpManager, LWorkItem* pWorkItem)

#define DEFINE_DBSERVER_DB_OP_PROC(X) void LDBServerOpProcessManager::Proc_##X(LDBServerOpProcessManager* pDBOpManager, LWorkItem* pWorkItem)

class LDBServerOpProcessManager
{
public:
	LDBServerOpProcessManager();
	~LDBServerOpProcessManager();
public:
	bool Initialize();
	bool Register(unsigned int unWorkID, DBSERVER_DB_OP_PROCESS_PROC pProc);
	void DispatchDBOpToProcessProc(LWorkItem* pWorkItem);
private:
	map<unsigned int, DBSERVER_DB_OP_PROCESS_PROC> m_mapDBOpIDToProcessProc;

public:
	void SetDBOpThread(LDBOpThread* pDBOpThread);
	LDBOpThread* GetDBOpThread();
private:
	LDBOpThread* m_pDBOpThread;

public:

};


