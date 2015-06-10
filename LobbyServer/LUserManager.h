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
#include <queue>
using namespace std;
#include "stdint.h"
#include "time.h"

typedef struct _Will_Connect_Info
{
	uint64_t uUserUniqueIDInDB;
	time_t tKickOutTime;
}t_Will_Connect_Info;

class LUser;

class LUserManager
{
public:
	LUserManager();
	~LUserManager();
public:
	bool AddUser(LUser* pUser);
	LUser* FineUser(uint64_t u64UniqueUserIDInDB);
	void RemoveUser(uint64_t u64UniqueUserIDInDB);

	unsigned int GetServeCount();
private:
	map<uint64_t, LUser*> m_mapUserUniqueIDInDBToUser;


public:
	bool InitializeUserPool(unsigned int unUserPoolSize);
	LUser* AllocUserFromPool();
	void FreeUserToPool(LUser* pUser);
private:
	queue<LUser*> m_queueUserPool;

public:
	bool AddNewWillConnectUser(uint64_t u64UniqueUserIDInDB);
	bool ExistsWillConnectUser(uint64_t u64UniqueUserIDInDB);
	void ProcessWillConnectUser();
private:
	map<uint64_t, uint64_t> m_mapWillConnectUser;
	queue<t_Will_Connect_Info> m_queueKickOutConUser;
};


