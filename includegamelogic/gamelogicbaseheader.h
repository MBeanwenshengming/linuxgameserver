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

#include "../NetWork/IncludeHeader.h"

#define MAX_ROLE_NAME_LEN 20

typedef struct _User_Role_Base_Info			//	玩家角色基本信息
{
	uint64_t u64UniqueIDInDB;
	unsigned int unRoleIDInDB;
	char szRoleName[MAX_ROLE_NAME_LEN + 1];
	unsigned short usLv;
	unsigned int unStageID;
	unsigned int unMapID;
	float fx;
	float fy;
	float fz;
}t_User_Role_Base_Info;


typedef struct _User_Role_Item_Info			//	玩家物品信息基本信息
{ 
	uint64_t u64UniqueIDInDB;
	unsigned int unRoleIDInDB;
	uint64_t u64ItemUniqueIDInDB;
	unsigned int unItemID; 
}t_User_Role_Item_Info;

