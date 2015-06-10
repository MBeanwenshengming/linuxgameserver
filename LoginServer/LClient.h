
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
#include "../NetWork/LServerBaseNetWork.h"
#include "../include/Common_Define.h"

class LClient 
{
public:
	LClient(); 
	~LClient();
public:
	void Reset();
	void SetClientInfo(uint64_t u64SesssionID, t_Session_Accepted& tas);
	uint64_t GetSessionID();
	void GetSessionInfo(t_Session_Accepted& tSAInfo);
	int GetSendThreadID();
	int GetRecvThreadID();
	void GetClientIpAndPort(char* pbuf, unsigned int unbufLen, unsigned short& usPort);
private: 
	uint64_t m_u64SessionID;	
	t_Session_Accepted m_SA;

public:
	void SetClientUniqueIDInADB(uint64_t u64ClientUniqueIDInADB);
	uint64_t GetClientUniqueIDInADB();
	void SetUserID(char* pUserID);
	void GetUserID(char* pbuf, unsigned int unbufLen);
private:
	uint64_t m_u64ClientUniqueIDInADB;
	char m_szUserID[MAX_USER_ID_LEN + 1];
};


