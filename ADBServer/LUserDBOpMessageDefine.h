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

//	MT MainLogicThread  主线程
//	DBT DBThread  数据库线程
//	MT2DBT	主线程到数据库线程的数据包
//	DBT2MT  数据库线程到主线程的数据包

enum
{
	ADB_SERVER_MT2DBT_READ_USER_INFO = 1,		//	读取帐号信息
	//	char szUserID[MAX_USER_ID_LEN];
	//	char szPassWord[MAX_PASSWORD_LEN];
	//	uint64_t u64ServerSessionID;
	//	uint64_t u64SessionIDInClient;			//	玩家在对应的客户端服务器中的SessionID
	
	ADB_SERVER_DBT2MT_READ_USER_INFO,
	//	int nResult;				//	数据库操作是否成功
	//	char szUserID[MAX_USER_ID_LEN];	
	//	char szPassWord[MAX_PASSWORD_LEN]; 
	//	uint64_t u64UniqueIDInDB;
	//	uint64_t u64ServerSessionID;
	//	uint64_t u64SessionIDInClient;
	

	ADB_SERVER_MT2DBT_DELETE_USER_INFO,			//	删除帐号信息
	//	char szUserID[MAX_USER_ID_LEN];
	ADB_SERVER_DBT2MT_DELETE_USER_INFO,
	//	int nResult;
	//	char szUserID[MAX_USER_ID_LEN];
	
	ADB_SERVER_MT2DBT_CHANGE_PASSWORD,			//	修该密码
	//	char szUserID[MAX_USER_ID_LEN];
	//	char szOldPassWord[MAX_PASSWORD_LEN];
	//	char szNewPassWord[MAX_PASSWORD_LEN];
	//	uint64_t u64ServerSessionID;
	//	uint64_t u64SessionIDInClient;			//	玩家在对应的客户端服务器中的SessionID
	ADB_SERVER_DBT2MT_CHANGE_PASSWORD,
	//	int nResult;
	//	char szUserID[MAX_USER_ID_LEN];
	//	uint64_t u64ServerSessionID;
	//	uint64_t u64SessionIDInClient;			//	玩家在对应的客户端服务器中的SessionID
	ADB_SERVER_MT2DBT_ADD_USER_INFO,			//	添加帐号
	//	char szUserID[MAX_USER_ID_LEN];
	//	char szPassWord[MAX_PASSWORD_LEN];
	//	uint64_t u64ServerSessionID;
	//	uint64_t u64SessionIDInClient;			//	玩家在对应的客户端服务器中的SessionID
	ADB_SERVER_DBT2MT_ADD_USER_INFO,
	//	int nResult;
	//	char szUserID[MAX_USER_ID_LEN];
	//	char szPassWord[MAX_PASSWORD_LEN];
	//	uint64_t u64UniqueIDInDB;				//	创建成功时，在数据库中的唯一ID
	//	uint64_t u64ServerSessionID;
	//	uint64_t u64SessionIDInClient;			//	玩家在对应的客户端服务器中的SessionID

};

