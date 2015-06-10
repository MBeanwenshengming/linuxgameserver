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
#include "../include/Common_Define.h"
#include "../NetWork/IncludeHeader.h"
typedef enum 
{
	E_User_State_NULL = 0,
	E_User_State_Creating,					//	正在创建玩家帐号，进行数据库操作之中
	E_User_State_Getting_Info_From_DB,		//	正在从数据库读取玩家信息
	E_User_State_Data_Info_Initialized,		//	玩家信息已初始化完毕
	E_User_State_Data_Info_Changed,			//	玩家数据已经在内存中改变
}E_User_State;


class LUserInfo
{
public:
	LUserInfo();
	~LUserInfo();
public:
	void Reset();
public:
	E_User_State GetUserState();
	void SetUserState(E_User_State eUserState);

	void SetUserID(char* pUserID);
	void SetUserPassWord(char* pPassWord);
	void SetUserUniqueIDInDB(uint64_t u64UserUniqueIDInDB);

	bool GetUserID(char* pbuf, unsigned int unbufLen);
	bool GetUserPassWord(char* pbuf, unsigned int unbufLen);
	uint64_t GetUserUniqueIDInDB();
public:
	bool CheckUserAndPassWord(char* szUserID, char* szPassWord);
private:
	E_User_State m_eUserDataState;
	char m_szUserID[MAX_USER_ID_LEN + 1];
	char m_szUserPassWord[MAX_PASSWORD_LEN + 1];
	uint64_t m_u64UserUniqueIDInDB;
};

