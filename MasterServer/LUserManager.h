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
#include "LUser.h"

class LUser;

class LUserManager
{
public:
	LUserManager();
	~LUserManager();
public:
	bool AddNewUserInfo(LUser* pUser);
	bool IsExistUserInfo(uint64_t u64UserUniqueIDInDB);
	LUser* FindUser(uint64_t u64UserUniqueIDInDB);
	void RemoveUser(uint64_t u64UserUniqueIDInDB);

	bool UpdateUserState(uint64_t u64UserUniqueIDInDB, E_User_State eUserState);
private:
	map<uint64_t, LUser*> m_mapUniqueIDInServerToUser;	//	在数据库中的User唯一ID，映射到User
public:
	bool InitializeUserPool(unsigned int unPoolSize);
	LUser* AllocOneUser();
	void FreeOneUser(LUser* pUser);
private:
	queue<LUser*> m_pUserPool;
};

