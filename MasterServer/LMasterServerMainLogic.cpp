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

#include "LMasterServerMainLogic.h"
#include "LMainLogicThread.h"
#include "../include/Server_To_Server_Packet_Define.h"
#include "../include/Server_Define.h"
#include "../NetWork/LPacketBroadCast.h"
#include "LServerManager.h"
#include "LUserManager.h"
#include "LUser.h"
#include "../include/Global_Error_Define.h"

LMasterServerMainLogic::LMasterServerMainLogic()
{
}

LMasterServerMainLogic::~LMasterServerMainLogic()
{
}

void LMasterServerMainLogic::BroadCastServerToServers(LMainLogicThread* pMainLogicThread, uint64_t u64RegisterServerUniqueID)
{
	if (pMainLogicThread == NULL)
	{
		return ;
	}
	if (u64RegisterServerUniqueID == 0)
	{
		return ;
	}

	LServerID serverID;
	if (!serverID.SetServerID(u64RegisterServerUniqueID))
	{
		return ;
	}
	//unsigned char ucServerType = serverID.GetServerType();
	unsigned int unServerType = serverID.GetServerType();
	E_Server_Type eServerType = (E_Server_Type)unServerType;

	LPacketBroadCast* pSendPacket = NULL;
	if (eServerType == E_Server_Type_Lobby_Server || eServerType == E_Server_Type_Game_Server || eServerType == E_Server_Type_DB_Server || eServerType == E_Server_Type_Account_DB_Server)
	{
		unsigned short usPacketLen = 100;
		pSendPacket = pMainLogicThread->GetOneSendPacket(usPacketLen);
		if (pSendPacket == NULL)
		{
			return ;
		}
		pSendPacket->SetPacketID(Packet_SS_Server_Register_Broadcast1);
		unsigned char ucServerCount = 1;
		pSendPacket->AddByte(ucServerCount);
		pSendPacket->AddULongLong(u64RegisterServerUniqueID);
	}
	switch (eServerType)
	{
		case E_Server_Type_Lobby_Server:
		case E_Server_Type_Game_Server:
			{
				if (pSendPacket != NULL)
				{
					E_Server_Type eSendToServerType = E_Server_Type_Gate_Server;
					LServerManager::GetServerManagerInstance()->BroadCastToServersByServerType((unsigned char)eSendToServerType, pSendPacket);
				}
			}
			break;
		case E_Server_Type_DB_Server:
			{
				E_Server_Type eSendToServerType = E_Server_Type_Lobby_Server;
				if (pSendPacket != NULL)
				{ 
					LServerManager::GetServerManagerInstance()->BroadCastToServersByServerType((unsigned char)eSendToServerType, pSendPacket);
				}
				eSendToServerType = E_Server_Type_Game_Server;
				if (pSendPacket != NULL)
				{ 
					LServerManager::GetServerManagerInstance()->BroadCastToServersByServerType((unsigned char)eSendToServerType, pSendPacket);
				} 
			}
			break;
		case E_Server_Type_Account_DB_Server:
			{
				E_Server_Type eSendToServerType = E_Server_Type_Login_Server;
				if (pSendPacket != NULL)
				{
					LServerManager::GetServerManagerInstance()->BroadCastToServersByServerType((unsigned char)eSendToServerType, pSendPacket);
				}
			}
			break;
		default:
			break;
	} 
}

void LMasterServerMainLogic::SendConnectableServerInfosToServer(LMainLogicThread* pMainLogicThread, uint64_t u64RegisterServerUniqueID)
{ 
	if (pMainLogicThread == NULL)
	{
		return ;
	}
	if (u64RegisterServerUniqueID == 0)
	{
		return ;
	}

	LServerID serverID;
	if (!serverID.SetServerID(u64RegisterServerUniqueID))
	{
		return ;
	}

	LServerManager* pServerManager = LServerManager::GetServerManagerInstance();
	pServerManager->SendPacketToServerCanConnectToServerInfos(u64RegisterServerUniqueID);
}


int LMasterServerMainLogic::UserOnlineFromLoginServer(LMainLogicThread* pMainLogicThread, char* pszUserID, uint64_t u64UserUniqueIDInDB)
{
	if (pMainLogicThread == NULL || pszUserID == NULL || u64UserUniqueIDInDB == 0)
	{
		return Global_Error_Unknown;
	}
	LUserManager* pUserManager = pMainLogicThread->GetUserManager();
	LUser* pUser = pUserManager->FindUser(u64UserUniqueIDInDB);
	if (pUser != NULL)		//	玩家已经在线，那么登录失败
	{
		return Global_Error_User_Already_Online;
	}
	pUser = pUserManager->AllocOneUser();
	if (pUser == NULL)
	{
		return Global_Error_Unknown;
	}
	pUser->SetUserUniqueIDInDB(u64UserUniqueIDInDB);
	pUser->SetUserID(pszUserID);
	pUser->SetUserState(E_User_Created);
	if (!pUserManager->AddNewUserInfo(pUser))
	{
		pUserManager->FreeOneUser(pUser);
		return Global_Error_Unknown;
	}
	return 0;
}
