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

//	packetid 为unsigned int
//	协议定义
#define SERVER_TO_SERVER_PACKET_ID_START 0
//	服务器之间的协议包范围
#define SERVER_TO_SERVER_PACKET_ID_END   SERVER_TO_SERVER_PACKET_ID_START + 1000000
//	客户端到服务器协议包范围
#define CLIENT_TO_SERVER_PACKET_ID_START SERVER_TO_SERVER_PACKET_ID_END + 1


#define GLOBAL_ERROR_ID int
#define MAX_SERVER_IP	16		//	SERVERIP字符串最大长度
#define MAX_USER_ID_LEN 16		// 玩家用户名最大字节长度
#define MAX_PASSWORD_LEN 16		//	玩家密码最大长度
#define MAX_ROLE_NAME_LEN 16	//	角色名最大长度
