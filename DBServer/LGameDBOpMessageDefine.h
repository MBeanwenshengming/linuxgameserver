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

//	MT mainthread
//	DBT DB Thread
//	MT2DBT  MT====>DBT message
//	DBT2MT  DBT===>MT message
enum
{
	DBSERVER_MT2DBT_GET_ROLE_INFO,			//	获取玩家的角色信息
	//	uint64_t u64UniqueIDInDB;			//	玩家ID
	//	char szUserID[MAX_USER_ID_LEN];		//	玩家角色名 
	//	int nMaxReadRoleInfo;				//	最多读取多少个角色
	DBSERVER_DBT2MT_GET_ROLE_INFO,			//	
	//	int nResult;						//	获取结果
	//	uint64_t u64UniqueIDInDB;			//	玩家ID
	//	char szUserID[MAX_USER_ID_LEN];		//	玩家用户名
	//	int nRoleInfoCount;					//	角色数量
	//	int nCurRoleInfo;					//	当前传送的角色数量
	//	t_Role_Info roleInfo;				//	玩家的角色信息， 一个数据包传递一个角色信息
	DBSERVER_MT2DBT_GET_ROLE_ITEM_INFO,
	//	uint64_t u64UniqueIDInDB;			//	玩家帐户唯一ID
	//	unsigned int unRoleIDInDB;			//	玩家但个角色唯一ID
	DBSERVER_DBT2MT_GET_ROLE_ITEM_INFO,
	//	int nResult;						//	读取是否成功
	//	uint64_t u64UniqueIDInDB;			//	
	//	unsigned int unRoleIDInDB;
	//	unsigned char ucItemSendCompleted;	//	角色的道具是否发送完成，0为正在发送，后续还有道具，1为所有道具发送完成
	//	unsigned char ucItemCount;			//	本数据包中包含的道具信息数量
		//	t_Role_Item_Info tRoleItemInfo;		//	单个道具的信息
};


