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

#ifndef __LINUX_SOCKET_HEADER_DEFINED__
#define __LINUX_SOCKET_HEADER_DEFINED__
#include "IncludeHeader.h"

typedef enum
{
	E_Socket_UnInitialized 	= -1,
	E_Socket_Have_To_Closed = -2,
	E_Socket_No_Recv_Data	= -3,
	E_Socket_Recv_Buf_Null	= -4,
	E_Socket_Send_Buf_Null	= -5,
	E_Socket_Recv_Buf_Zero	= -6,
	E_Socket_Send_Buf_Zero	= -7,
	//	sockopt error
	E_Socket_SetOpt_Buf_Null = -8,
	E_Socket_GetOpt_Buf_Null = -9,
	E_Socket_SetOpt_Buf_Zero = -10,
	E_Socket_GetOpt_Buf_Zero = -11,
	E_Socket_SetOpt_Func_Error = -12,
	E_Socket_GetOpt_Func_Error = -13,
	//	发送套接字的系统发送缓存满了
	E_Socket_Send_System_Buf_Full = -14,	
}E_Socket_Error;


class LSocket
{
public:
	LSocket();
	~LSocket();
public:
	bool SetSocket(int nSocket);
	int GetSocket();
public:
	int Send(const void* pbuf, size_t sbufSize);
	int Recv(void* pbuf, size_t sbufSize);
	int GetPeerName(char* pIpBuf, unsigned short usBufLen, unsigned short& usPort);
	int GetSockName(char* pIpBuf, unsigned short usBuflen, unsigned short& usPort);

	int SetSockOpt(int nLevel, int nOptName, const void* pbuf, socklen_t bufLen);
	int GetSockOpt(int nLevel, int nOptName, void* pbuf, socklen_t* pbufLen);

	bool SetNonBlockSocket();
private:
	int m_Socket;
};
#endif

