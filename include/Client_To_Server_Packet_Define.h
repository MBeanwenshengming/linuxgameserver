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

#include "Common_Define.h"


enum
{
	PACKET_CS_START_REQ = CLIENT_TO_SERVER_PACKET_ID_START,
	PACKET_SC_START_RES,

	//	发送给LoginServer，验证用户名和密码，暂时使用，正式使用需要加密等措施
	PACKET_CS_2LS_USERID_AND_PASSWORD = CLIENT_TO_SERVER_PACKET_ID_START + 100,
	//	char szUserID[MAX_USER_ID_LEN];
	//	char szPassWord[MAX_PASSWORD_LEN];
	PACKET_SC_USERID_AND_PASSWORD,		//	验证返回结果
	//	int nResult;
	//	uint64_t u64UserUniqueIDInDB;
	//	char szUniqueKeyToLogin[128];	//	唯一登录ID
	//	char szGateServerIp[20];		//	需要连接的GateServerIp地址
	//	unsigned short usGateServerPort;	// gateserver 监听端口
	PACKET_C2LS_CREATE_USER,
	//	char szUserID[MAX_USER_ID_LEN];
	//	char szPassWord[MAX_PASSWORD_LEN];
	PACKET_LS2C_CREATE_USER,
	//	int nResult;
	//	char szUserID[MAX_USER_ID_LEN]; 
	PACKET_C2GA_USER_LOGINKEY,
	//	uint64_t u64UserUniqueIDInDB;
	// 	char szUniqueKeyToLogin[128];
	PACKET_GA2C_USER_LOGIN,
	//	int nResult;
	PACKET_C2GA_ENTER_LOBBYSERVER,
	PACKET_GA2C_ENTER_LOBBYSERVER,
	//	int nResult;
	PACKET_C2GA_GET_ROLE_INFO,
	PACKET_GA2C_ROLE_INFOS,
	//	unsigned char ucRoleCount;
		//	t_Role_Info roleInfo;
	PACKET_C2GA_SELECT_ROLE,
	//	unsigned int unRoleUniqueIDSelected;
	PACKET_GA2C_SELECT_ROLE,
	//	int nResult;
	PACKET_C2GA_ENTER_GAME_WORLD,
	PACKET_GA2C_ENTER_GAME_WORLD,
	//	int nResult;
	//	unsigned int unRoleInstanceIDInGameServer;
	PACKET_C2GA_GET_ROLE_DETAIL_INFO,
	//	unsigned int unRoleInstanceIDInGameServer;
	PACKET_GA2C_ROLE_INFO,
	//	unsigned int unRoleInstanceIDInGameServer;
	//	uint64_t u64UserUniqueIDInDB;
	//	unsigned int unRoleUniqueID;
	//	t_Role_Base_Info tRoleInfo;
	PACKET_GA2C_ROLE_ITEM_INFO,
	//	unsigned int unRoleInstanceIDInGameServer;
	//	uint64_t u64UserUniqueIDInDB;
	//	unsigned int unRoleUniqueID;
	//	unsigned short usItemCount;
		//	t_Item_Info tItemInfo;
	PACKET_GA2C_ROLE_LOAD_COMPLETE,
	//	unsigned int unRoleInstanceIDInGameServer;
	//	int nLoadResult;
	PACKET_C2GA_ENTER_MAP,
	//	unsigned int unRoleInstanceIDInGameServer;
	PACKET_GA2C_ENTER_MAP,
	//	int nEnterResult;
	//	unsigned int unRoleInstanceIDInGameServer;
};



//	协议结构定义
#pragma pack(push,1)

#pragma pack(pop)




