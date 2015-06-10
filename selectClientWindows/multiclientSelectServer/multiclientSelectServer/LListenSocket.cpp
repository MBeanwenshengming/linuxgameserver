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

#include "LListenSocket.h"
#include "LErrorWriter.h"


int g_nRecvBufLen = 0;
int g_nSendBufLen = 0; 
extern LErrorWriter g_ErrorWriter;


LListenSocket::LListenSocket()
{
	m_usListenSocket = 0;
	memset(m_szIp, 0, sizeof(m_szIp));
}

LListenSocket::~LListenSocket()
{
}

bool LListenSocket::Initialized(const char* pszIp, unsigned short usListenPort)
{ 
	int nListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (nListenSocket == -1)
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Initialized, nListenSocket == -1\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (!SetSocket(nListenSocket))
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Initialized, !SetSocket(nListenSocket)\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

	if (g_nRecvBufLen == 0)
	{
		g_nRecvBufLen = 8 * 1024; 
	}
	if (g_nSendBufLen == 0)
	{
		g_nSendBufLen = 128 * 1024;
	}
#ifdef _DEBUG
	int nTempRecvBufLen = 0;
	socklen_t nTempRecvLen = sizeof(nTempRecvLen);
	if (GetSockOpt(SOL_SOCKET, SO_RCVBUF, &nTempRecvBufLen, &nTempRecvLen) != 0)
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Initialized, GetSockOpt Failed, SO_RCVBUF\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		//	error
	}
#endif
	if (SetSockOpt(SOL_SOCKET, SO_RCVBUF, &g_nRecvBufLen, sizeof(g_nRecvBufLen)) != 0)
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Initialized, SetSockOpt Failed SO_RCVBUF\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
#ifdef _DEBUG
	int nTempSendBufLen = 0;
	socklen_t nTempSendLen = sizeof(nTempSendBufLen);
	if (GetSockOpt(SOL_SOCKET, SO_SNDBUF, &nTempSendBufLen, &nTempSendLen) != 0)
	{
		//	error
		char szError[512];
		sprintf(szError, "LListenSocket::Initialized, GetSockOpt Failed SO_SNDBUF\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
	}
#endif
	if (SetSockOpt(SOL_SOCKET, SO_SNDBUF, &g_nSendBufLen, sizeof(g_nSendBufLen)) != 0) 
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Initialized, SetSockOpt Failed SO_SNDBUF\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}

#ifndef WIN32
	int nSetNonBlockSuccess = fcntl(GetSocket(), F_SETFL, O_NONBLOCK);
	if (nSetNonBlockSuccess == -1)
	{
		return false;
	}
#else
	unsigned long ul=1;
	int nSetNonBlockSuccess = ioctlsocket(GetSocket(), FIONBIO, (unsigned long *)&ul);
	if (nSetNonBlockSuccess == SOCKET_ERROR)
	{
		return false;
	}
#endif

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(usListenPort);
#ifndef WIN32
	int nConvertSuccess = inet_aton(pszIp, &serverAddr.sin_addr);	
	if (nConvertSuccess == 0)
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Initialized, inet_aton\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	} 
#else
	serverAddr.sin_addr.S_un.S_addr = inet_addr(pszIp);
#endif

	int nBindSuccess = bind(GetSocket(), (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (nBindSuccess == -1)
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Initialized, Bind Failed\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	} 

	m_usListenSocket = usListenPort;
	strncpy(m_szIp, pszIp, MAX_LISTEN_IP_LEN);
	return true;
}

bool LListenSocket::Listen(unsigned int unListenNum)
{
	if (unListenNum == 0)
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Listen, unListenNum == 0\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	if (GetSocket() == -1)
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Listen, GetSocket() == -1\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	int nListenSuccess = listen(GetSocket(), unListenNum);
	if (nListenSuccess == -1)
	{
		char szError[512];
		sprintf(szError, "LListenSocket::Listen, nListenSuccess == -1\n"); 
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}


