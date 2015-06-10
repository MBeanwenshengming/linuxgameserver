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
	E_User_Unknown = 0,
	E_User_Created,
	E_User_Login_Waiting_GateServer_Reponse,	//	正在等待GateServer准备登录的返回
	E_User_Login_Waiting_GateServer_Login,		//	等待GateServer报告玩家登录的消息
	E_User_Connectted_To_GateServer,			//	玩家已经登录了GateServer
}E_User_State;

class LUser
{
public:
	LUser();
	~LUser();
public:
	void Reset();
public:
	void SetUserUniqueIDInDB(uint64_t u64UserUniqueIDInDB);
	uint64_t GetUserUniqueIDInDB();
	void SetUserID(char* pUserID);
	void GetUserID(char* pbuf, unsigned int unbufLen);
	void SetUserState(E_User_State eUserState);
	E_User_State GetUserState();
private:
	uint64_t m_u64UserIDInDB;
	char m_szUserID[MAX_USER_ID_LEN + 1]; 
	E_User_State m_eUserState;
};
