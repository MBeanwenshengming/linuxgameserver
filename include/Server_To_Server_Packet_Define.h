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
#include "Server_Define.h"

enum 
{
	//	数据包流开始  发起连接的服务器首先发送该协议包给被连接服务器，被连接服务器应答Res后，连接发起服务器收到的包才有效，不然视为无效数据包丢弃
	Packet_SS_Start_Req = SERVER_TO_SERVER_PACKET_ID_START + 1,
	//	数据包流开始 确认
	Packet_SS_Start_Res,

	//	注册服务器请求
	Packet_SS_Register_Server_Req,
		//	unsigned char ucServerType;
		//	unsigned short usAreaID;
		//	unsigned short usGroupID;
		//	unsigned short usServerID;
	Packet_SS_Register_Server_Req1,
		//  uint64_t u64UniqueServerID;
	//	注册服务器应答
	Packet_SS_Register_Server_Res,
		//	GLOBAL_ERROR_ID errorID;	//	成功返回0。失败返回错误ID
			
	//	广播给对应的服务器，有服务器注册上来
	Packet_SS_Server_Register_Broadcast,		
		//	unsigned char ucServerType;
		//	unsigned short usAreaID;
		//	unsigned short usGroupID;
		//	unsigned short usServerID; 
	Packet_SS_Server_Register_Broadcast1,	//	向服务器广播连接上来的服务器信息
		//	unsigned char ucServerCount;
		//	uint64_t uServerIDs[];
	Packet_SS_Server_Will_Connect_Req,		//	服务器即将连接到指定服务器的请求	
		//	uint64_t u64RequestServerID;	//	请求的服务器
		//	uint64_t u64DestServerID;		//	目标服务器
	Packet_SS_Server_Will_Connect_Res,		//	返回信息
		//	int nResErrorID;				//	请求结果, 为0表示成功
		//	uint64_t u64RequestServerID;
		//	uint64_t u64DestServerID;
		//	char szIp[20];
		//	unsigned short usPort;
	Packet_SS_Server_Can_ConnectTo_Server_Infos,	//	可以连接的服务器信息
		//	uint64_t u64RecvServerID;		//	接收该数据包的服务器的ID
		//	uint64_t u64CanConnectToServerID;
	Packet_SS_Server_Current_Serve_Count,		//	服务器当前服务器的人数，服务器应该在注册完发送，并且每一分中发送数据
		//	unsigned int unServeCount;

	Packet_SS_Server_Current_Serve_Count_BroadCast,	//	Master服务器向对应类型的服务器广播某个服务器当前服务的人数
		//	uint64_t u64ServerID;
		//	unsigned int unServeCount;
	
	Packet_L2ADB_USER_VERIFY = SERVER_TO_SERVER_PACKET_ID_START + 100,
		// char szUserID[MAX_USER_ID_LEN];
		// char szPassWord[MAX_PASSWORD_LEN];
		// uint64_t u64UserSessionID;		//	验证用户的连接信息，返回给AccountServer时，用来查找玩家
	Packet_ADB2L_USER_VERIFY,		//	验证结果
		// int nResultID;
		// uint64_t u64UserSessionID;
		// char szUserID[MAX_USER_ID_LEN];	//	用户帐号名
		// uint64_t u64UserUniqueIDInDB;	//	用户的数字化帐号ID
	Packet_L2ADB_ADD_USER,
		// char szUserID[MAX_USER_ID_LEN];
		// char szPassWord[MAX_PASSWORD_LEN];
		// uint64_t u64UserSessionID;		//	验证用户的连接信息，返回给AccountServer时，用来查找玩家
	Packet_ADB2L_ADD_USER,
		//	int nResult;
		// uint64_t u64UserSessionID;
		// char szUserID[MAX_USER_ID_LEN];	//	用户帐号名
		// uint64_t u64UserUniqueIDInDB;	//	用户的数字化帐号ID
	Packet_L2M_USER_OFFLINE,	//	告诉MasterServer玩家已经从LoginServer断开连接
		//	char szUserID[MAX_USER_ID_LEN];
		//	uint64_t u64UserUniqueIDInDB;
	Packet_L2M_USER_ONLINE,
		// uint64_t u64UserSessionIDInLoginServer;
		// char szUserID[MAX_USER_ID_LEN];
		// uint64_t u64UserUniqueIDInDB;
	Packet_M2L_USER_ONLINE_VERIFY,
		// int nResult;					//	玩家登录结果，为0表示没有错误发生
		// uint64_t u64UserSessionIDInLoginServer;
		// char szUserID[MAX_USER_ID_LEN];
		// uint64_t u64UserUniqueIDInDB;
		// 如果登录没有错误，那么才有如下信息在消息包中
			// char szGateServerIP[MAX_SERVER_IP];
			// unsigned short usGateServerPort;
			// char szConnectToGateServerUniqueKey[128];
	Packet_M2GA_USER_WILL_LOGIN,		//	通知GateServer，有玩家即将登录
		// uint64_t u64UserSessionIDInLoginServer;
		// uint64_t u64LoginServerSessionIDInMasterServer;
		// char szUserID[MAX_USER_ID_LEN];
		// uint64_t u64UserUniqueIDInDB;
	Packet_GA2M_USER_WILL_LOGIN,
		// int nResult;
		// uint64_t u64UserSessionIDInLoginServer;
		// uint64_t u64LoginServerSessionIDInMasterServer;
		// char szUserID[MAX_USER_ID_LEN];
		// uint64_t u64UserUniqueIDInDB;
			// char szConnectToGateServerUniqueKey[128];
			// char szServerIP[MAX_SERVER_IP];
			// unsigned short usPort;
	Packet_GA2M_USER_GATESERVER_ONLINE,
		//	uint64_t u64SessionIDInGateServer;
		// 	uint64_t u64UserUniqueIDInDBOnlineFromGateServer;
	Packet_M2GA_USER_GATESERVER_ONLINE,
		//	int nResult;
		//	uint64_t u64UserUniqueIDInDBOnlineFromGateServer;
		//	uint64_t u64SessionIDInGateServer;
	Packet_GA2M_USER_GATESERVER_OFFLINE,
		//	uint64_t u64UserUniqueIDInDB;
	Packet_GA2M_SELECT_LOBBYSERVER,
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDBOnlineFromGateServer;
	Packet_M2GA_SELECT_LOBBYSERVER,
		//	int nRetult;
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDBOnlineFromGateServer;
		//	uint64_t u64LobbyServerUniqueServerID;
	Packet_M2LB_NOTIFY_USER_WILL_ONLINE,
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDBOnlineFromGateServer;
		//	uint64_t u64GateServerSessionIDInMasterServer;
		//	uint64_t u64GameDBUniqueServerID;		//	被选择的GameDBServer的服务器ID
		//	uint64_t u64LobbyServerUniqueID;		//	被选择的LobbyServer的服务器ID
		//	char szUserID[MAX_USER_ID_LEN];			//	玩家角色名称
	Packet_LB2M_NOTIFY_USER_WILL_ONLINE,
		//	int nResult;
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDBOnlineFromGateServer;
		//	uint64_t u64GateServerSessionIDInMasterServer;
		//	uint64_t u64LobbyServerUniqueID; 
	Packet_LB2GDB_NOTIFY_USER_WILL_ONLINE,		// 通知DB预读玩家角色数据
		//	uint64_t u64UserUniqueIDInDB;
		//	char szUserID[MAX_USER_ID_LEN];
	Packet_GA2LB_USER_ONLINE,
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	char szUserID[MAX_USER_ID_LEN];
	Packet_LB2GA_USER_ONLINE,
		//	int nResult;
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
	Packet_GA2LB_GET_ROLE_INFO,
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	char szUserID[MAX_USER_ID_LEN];
	Packet_LB2GDB_GET_ROLE_INFO,
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	char szUserID[MAX_USER_ID_LEN];
	Packet_GDB2LB_GET_ROLE_INFO,
		//	int nGetResult;
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	char szUserID[MAX_USER_ID_LEN];
		//	unsigned char ucRoleCount;
			//	t_Role_Info roleInfo;
	Packet_LB2GA_ROLE_INFOS, 
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned char ucRoleCount;
			//	t_Role_Info roleInfo;
	Packet_GA2LB_SELECT_ROLE,
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	char szUserID[MAX_USER_ID_LEN];
		//	char szRoleName[MAX_ROLE_NAME_LEN];
		//	unsigned int unRoleUniqueIDSelected;
	Packet_LB2M_SELECT_ROLE_REQUEST_GAMESERVER, //让MasterServer选择一个GameServer服务器
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	char szUserID[MAX_USER_ID_LEN];
		//	char szRoleName[MAX_ROLE_NAME_LEN];
		//	unsigned int unRoleUniqueIDSelected;
		//	unsigned int unStageID;			//	地图组ID
		//	unsigned int unMapID;			//	地图ID	
		//	unsigned int unFBID;			//	副本ID
	Packet_M2GS_NOTIFY_ROLE_WILL_ONLINE,
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueIDSelected;
		//	uint64_t u64GameDBServerUniqueID;
		//	char szUserID[MAX_USER_ID_LEN];
		//	char szRoleName[MAX_ROLE_NAME_LEN];
	Packet_GS2GDB_PRELOAD_ROLE_DATA,
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueIDSelected;
		//	char szRoleName[MAX_ROLE_NAME_LEN];
	Packet_GS2M_NOTIFY_ROLE_WILL_ONLINE,
		//	int nResult;
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueIDSelected;
	Packet_M2LB_SELECT_ROLE_RESPONSE_GAMESERVER,
		//	int nResult;
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueIDSelected;
		//	uint64_t u64GameServerUniqueID;
	Packet_LB2GA_SELECT_ROLE, 
		//	int nSelectResult;
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	uint64_t u64GameServerUniqueID;
	Packet_GA2GS_USER_ONLINE, 
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	char szUserID[MAX_USER_ID_LEN];
		//	char szRoleName[MAX_ROLE_NAME_LEN];
	Packet_GS2GA_USER_ONLINE,
		//	int nResult;
		//	uint64_t u64SessionIDInGateServer;
		//	unsigned int unRoleUniqueID;
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueInstanceID;
	Packet_GA2GS_GET_ROLE_DETAIL_INFO,
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueID;
		//	unsigned int unRoleUniqueInstanceID;
	Packet_GS2GDB_GET_ROLE_DETAIL_INFO,
		//	unsigned int unRoleInstanceIDInGameServer;	//	根据角色在gameserver上的唯一ID来确认数据
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueID;		

	Packet_GDB2GS_Role_Info,
		//	unsigned int unRoleInstanceIDInGameServer;	//	根据角色在gameserver上的唯一ID来确认数据
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueID;
		//	t_Role_Base_Info tRoleInfo;
	Packet_GDB2GS_Role_Item_Info, 
		//	unsigned int unRoleInstanceIDInGameServer;	//	根据角色在gameserver上的唯一ID来确认数据
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueID;
		//	unsigned short usItemCount;
			//	t_Item_Info tItemInfo;
	Packet_GDB2GS_Role_Detail_Info_LoadComplete,
		//	int nSuccessed;
	Packet_GS2GA_Role_Info,
		//	unsigned int unRoleInstanceIDInGameServer;
		//	uint64_t u64SessionIDInGateServer;
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueID;
		//	t_Role_Base_Info tRoleInfo;
	Packet_GS2GA_Role_Item_Info,
		//	unsigned int unRoleInstanceIDInGameServer;
		//	uint64_t u64SessionIDInGateServer; 
		//	uint64_t u64UserUniqueIDInDB;
		//	unsigned int unRoleUniqueID;
		//	unsigned short usItemCount;
			//	t_Item_Info tItemInfo;
	Packet_GS2GA_Role_Detail_Info_Load_Complete,
		//	unsigned int unRoleInstanceIDInGameServer;
		//	int nLoadResult;
	Packet_GA2GS_ENTER_MAP,
		//	unsigned int unRoleInstanceIDInGameServer;
	Packet_GS2GA_ENTER_MAP,
		//	int nEnterResult;
		//	unsigned int unRoleInstanceIDInGameServer;
};	

#pragma pack(push,1)

#pragma pack(pop)



