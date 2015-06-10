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

#include "LSocket.h"

#include "LErrorWriter.h" 
extern LErrorWriter g_ErrorWriter;
extern int g_narrLen[]; 
extern int g_globalCount;

LSocket::LSocket()
{
	m_Socket = -1;
}
LSocket::~LSocket()
{
}

bool LSocket::SetSocket(int nSocket)
{
	if (nSocket <= 0)
	{
		return false;
	}
	m_Socket = nSocket;
	return true;
}
int LSocket::GetSocket()
{
	return m_Socket;
}
//	
int LSocket::Send(const void* pbuf, size_t sbufSize)
{
	if (m_Socket == -1)
	{
		char szError[512];
		sprintf(szError, "LSocket::Send, m_Socket == -1\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));

		return E_Socket_UnInitialized;
	}
	if (pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LSocket::Send, pbuf == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_Send_Buf_Null;
	}
	if (sbufSize == 0)
	{
		char szError[512];
		sprintf(szError, "LSocket::Send, sbufSize == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_Send_Buf_Zero;
	}
	//unsigned short usPacketDataLen = *((unsigned short*)pbuf);
	//if (usPacketDataLen != sbufSize)
	//{
	//	char szError[512];
	//	sprintf(szError, "LSocket::Send, usPacketDataLen != sbufSize\n");
	//	g_ErrorWriter.WriteError(szError, strlen(szError));
	//}
		
	//bool bFinded = false;
	//for (int i = 0; i < g_globalCount; ++i)
	//{
	//	if (usPacketDataLen - sizeof(unsigned short) == g_narrLen[i])
	//	{
	//		bFinded = true;
	//		break;
	//	}
	//}
	//if (!bFinded)
	//{ 
	//	char szError[512];
	//	sprintf(szError, "LSocket::Send Error Packet Found!!, bFinded\n");
	//	g_ErrorWriter.WriteError(szError, strlen(szError));
	//}
	ssize_t sSended = send(m_Socket, pbuf, sbufSize, 0);
	if (sSended == -1)
	{ 
		if (errno == EAGAIN)
		{
			return E_Socket_Send_System_Buf_Full;
		}
		char szError[512];
		sprintf(szError, "LSocket::Send, sSended == -1, ErrorID:%d\n",errno);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_Have_To_Closed; 
	}
	return sSended;
}
int LSocket::Recv(void* pbuf, size_t sbufSize)
{
	if (m_Socket == -1)
	{
		char szError[512];
		sprintf(szError, "LSocket::Recv, m_Socket == -1\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_UnInitialized;
	}
	if (pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LSocket::Recv, pbuf == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_Recv_Buf_Null;
	}
	if (sbufSize == 0)
	{
		char szError[512];
		sprintf(szError, "LSocket::Recv, sbufSize == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_Recv_Buf_Zero;
	}
	ssize_t sRecved = recv(m_Socket, pbuf, sbufSize, 0);
	if (sRecved == -1)
	{
		if (errno == EAGAIN)
		{
			return E_Socket_No_Recv_Data; 
		}
		else
		{
			char szError[512];
			sprintf(szError, "LSocket::Recv, Recv Failed, errorCode:%d\n", errno);
			g_ErrorWriter.WriteError(szError, strlen(szError));
			return E_Socket_Have_To_Closed;
		}
	}
	return sRecved;
}

int LSocket::GetPeerName(char* pIpBuf, unsigned short usBufLen, unsigned short& usPort)
{ 
	if (pIpBuf == NULL || usBufLen == 0)
	{
		return -1;
	}
	if (m_Socket == -1)
	{
		return -2;
	}
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	socklen_t sockLen = sizeof(sockAddr);
	int nResult = getpeername(m_Socket, (sockaddr*)&sockAddr, &sockLen);
	if (nResult == -1)
	{
		return -3;
	}
	char* pIp = inet_ntoa(sockAddr.sin_addr);
	if (usBufLen <= strlen(pIp))
	{
		return -4;
	}
	strncpy(pIpBuf, pIp, usBufLen);
	usPort = sockAddr.sin_port; 
	return 0;
}
int LSocket::GetSockName(char* pIpBuf, unsigned short usBufLen, unsigned short& usPort)
{	
	if (pIpBuf == NULL || usBufLen == 0)
	{
		return -1;
	}
	if (m_Socket == -1)
	{
		return -2;
	}
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	socklen_t sockLen = sizeof(sockAddr);
	int nResult = getsockname(m_Socket, (sockaddr*)&sockAddr, &sockLen);
	if (nResult == -1)
	{
		return -3;
	}
	char* pIp = inet_ntoa(sockAddr.sin_addr);
	if (usBufLen <= strlen(pIp))
	{
		return -4;
	}
	strncpy(pIpBuf, pIp, usBufLen);
	usPort = sockAddr.sin_port; 
	return 0;
}
int LSocket::SetSockOpt(int nLevel, int nOptName, const void* pbuf, socklen_t bufLen)
{
	if (m_Socket == -1)
	{
		char szError[512];
		sprintf(szError, "LSocket::SetSockOpt, m_Socket == -1\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_UnInitialized;
	}
	if (pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LSocket::SetSockOpt, pbuf == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_SetOpt_Buf_Null;
	}
	if (bufLen == 0)
	{
		char szError[512];
		sprintf(szError, "LSocket::SetSockOpt, bufLen == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_SetOpt_Buf_Zero;
	}
	int nSetResult = setsockopt(m_Socket, nLevel, nOptName, pbuf, bufLen);
	if (nSetResult == -1)
	{
		char szError[512];
		sprintf(szError, "LSocket::SetSockOpt, nSetResult == -1 ,errorCode:%d\n", errno);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_SetOpt_Func_Error;
	}
	return 0;
}
int LSocket::GetSockOpt(int nLevel, int nOptName, void* pbuf, socklen_t* pbufLen)
{
	if (m_Socket == -1)
	{
		char szError[512];
		sprintf(szError, "LSocket::GetSockOpt, m_Socket == -1\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_UnInitialized; 
	}
	if (pbuf == NULL)
	{
		char szError[512];
		sprintf(szError, "LSocket::GetSockOpt, pbuf == NULL\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_GetOpt_Buf_Null;
	}
	if (*pbufLen == 0)
	{
		char szError[512];
		sprintf(szError, "LSocket::GetSockOpt, *pbufLen == 0\n");
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_GetOpt_Buf_Zero;
	}
	int nGetResult = getsockopt(m_Socket, nLevel, nOptName, pbuf, pbufLen);
	if (nGetResult == -1)
	{
		char szError[512];
		sprintf(szError, "LSocket::GetSockOpt, getsockopt Failed, errorCode:%d\n", errno);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return E_Socket_GetOpt_Func_Error;
	}
	return 0;
}

bool LSocket::SetNonBlockSocket()
{
	int nSetNonBlockSuccess = fcntl(GetSocket(), F_SETFL, O_NONBLOCK);
	if (nSetNonBlockSuccess == -1)
	{
		char szError[512];
		sprintf(szError, "LSocket::SetNonBlockSocket, SetNonBlockSocket Failed, errorCode:%d\n", errno);
		g_ErrorWriter.WriteError(szError, strlen(szError));
		return false;
	}
	return true;
}
