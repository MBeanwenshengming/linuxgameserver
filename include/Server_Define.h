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

#include "IncludeHeader.h"

typedef  enum
{
	E_Server_Type_Invalid = 0,
	E_Server_Type_Master_Server,
	E_Server_Type_Login_Server,
	E_Server_Type_Gate_Server,
	E_Server_Type_Lobby_Server,
	E_Server_Type_Game_Server,
	E_Server_Type_DB_Server, 
	E_Server_Type_Account_DB_Server,
	E_Server_Type_Max,
}E_Server_Type;


typedef enum
{
	E_Server_State_Unknow,		//	服务器处于未知状态，不可用
	E_Server_State_Registered,	//	服务器已经注册，但是还未初始化数据
	E_Server_State_Initialized,	//	服务器已经注册，已经初始化，但是还有必须信息没有，无法工作
	E_Server_State_Working,		//	服务器所有初始化工作已经完成，正在提供服务
	E_Server_State_Max,
}E_Server_State;

class LServerID
{
public:
	LServerID()
	{
		m_ucServerType 	= 0;
		m_usAreaID		= 0;
		m_usGroupID		= 0;
		m_usServerID	= 0; 
	}
	virtual ~LServerID()
	{
		m_ucServerType 	= 0;
		m_usAreaID		= 0;
		m_usGroupID		= 0;
		m_usServerID	= 0; 
	}
private:
	unsigned char m_ucServerType;		//	服务器类型
	unsigned short m_usAreaID;		//	区ID
	unsigned short m_usGroupID;		//	组ID
	unsigned short m_usServerID;		//	服务器ID
	inline void Reset()
	{
		m_ucServerType 	= 0;
		m_usAreaID		= 0;
		m_usGroupID		= 0;
		m_usServerID	= 0; 
	}
public:
	inline uint64_t GetServerUniqueID()
	{
		uint64_t nServerID = 0;
		uint64_t uTemp = m_ucServerType;
		nServerID = uTemp << (6 * 8);
		
		uTemp = m_usAreaID;
		nServerID |= uTemp << (4 * 8);

		uTemp = m_usGroupID;
		nServerID |= uTemp << (2 * 8);

		nServerID |= m_usServerID;
		return nServerID;
	}
	inline bool SetServerID(uint64_t nServerID)
	{
		uint64_t uTemp = nServerID & 0x00ff000000000000LL;
		uTemp >>= 6 * 8;
		m_ucServerType 	= (unsigned char)uTemp;

		uTemp = nServerID & 0x0000ffff00000000LL;
		uTemp >>= 4 * 8;
		m_usAreaID 		= (unsigned short)uTemp;

		uTemp = nServerID & 0x00000000ffff0000LL; 
		uTemp >>= 2 * 8;
		m_usGroupID		= (unsigned short)uTemp;

		uTemp = nServerID & 0x000000000000ffffLL;
		m_usServerID	= (unsigned short)uTemp;

		if ((m_ucServerType == (unsigned char)E_Server_Type_Invalid) || m_ucServerType >= (unsigned char)E_Server_Type_Max)
		{
			Reset();
			return false;
		}
		return true;
	}
	inline bool SetServerID(E_Server_Type eServerType, unsigned short usAreaID, unsigned short usGroupID, unsigned short usServerID)
	{
		if (eServerType <= E_Server_Type_Invalid || eServerType >= E_Server_Type_Max)
		{
			return false;
		}

		m_ucServerType 	= (unsigned char)eServerType;
		m_usAreaID 		= usAreaID;
		m_usGroupID 	= usGroupID;
		m_usServerID 	= usServerID;
		return true;
	}
	inline E_Server_Type GetServerType()
	{
		return (E_Server_Type)m_ucServerType;
	}
	inline unsigned short GetAreaID()
	{
		return m_usAreaID;
	}
	inline unsigned short GetGroupID()
	{
		return m_usGroupID;
	}
	inline unsigned short GetServerID()
	{
		return m_usServerID;
	}
	static void ToServeTypeString(E_Server_Type eServerType, char* pszBuf, size_t sBufLen)
	{
		if (pszBuf == NULL || sBufLen <= 32)
		{
			return;
		}
		switch(eServerType)
		{
			case E_Server_Type_Master_Server:
			{
				strncpy(pszBuf, "MasterServer", strlen("MasterServer"));
			}
			break;
			case E_Server_Type_Login_Server:
			{
				strncpy(pszBuf, "LoginServer", strlen("LoginServer"));
			}
			break;
			case E_Server_Type_Gate_Server:
			{
				strncpy(pszBuf, "GateServer", strlen("GateServer"));
			}
			break;
			case E_Server_Type_Lobby_Server:
			{
				strncpy(pszBuf, "LobbyServer", strlen("LobbyServer"));
			}
			break;
			case E_Server_Type_Game_Server:
			{
				strncpy(pszBuf, "GameServer", strlen("GameServer"));
			}
			break;
			case E_Server_Type_DB_Server:
			{
				strncpy(pszBuf, "GameDBServer", strlen("GameDBServer"));
			}
			break;
			case E_Server_Type_Account_DB_Server:
			{
				strncpy(pszBuf, "AccountDBServer", strlen("AccountDBServer"));
			}
			break;
			default:
				return ;
		}
	}
};

