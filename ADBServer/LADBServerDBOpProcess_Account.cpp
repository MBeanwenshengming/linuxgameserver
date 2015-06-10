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
#include "LADBServerDBOpProcessManager.h"
#include "LDBOpThread.h"
#include "LUserDBOpMessageDefine.h"
#include "LDBServerMainLogicThread.h" 
#include "../DBLib/XMySqlConnector.h"
#include "LWorkQueueManager.h"
#include "LWorkItem.h"
#include "../include/Global_Error_Define.h"

DEFINE_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_READ_USER_INFO)
{
	char szUserID[MAX_USER_ID_LEN + 1];
	char szPassWord[MAX_PASSWORD_LEN + 1];
	uint64_t u64ServerSessionID 	= 0;
	uint64_t u64SessionIDInClient	= 0;			//	玩家在对应的客户端服务器中的SessionID
	memset(szUserID, 0, sizeof(szUserID));
	memset(szPassWord, 0, sizeof(szPassWord));

	pWorkItem->GetData(szUserID, MAX_USER_ID_LEN);
	pWorkItem->GetData(szPassWord, MAX_PASSWORD_LEN);
	pWorkItem->GetULongLong(u64ServerSessionID);
	pWorkItem->GetULongLong(u64SessionIDInClient);
	if (pWorkItem->GetErrorCode() != 0)
	{
		return ;
	}
	uint64_t u64UserUniqueIDInDB = 0;

	LDBOpThread* pDBOpThread = pDBOpManager->GetDBOpThread();
	LDBServerMainLogicThread* pDBServerMainLogic = pDBOpThread->GetDBServerMainLogic();
	
	LWorkQueueManager* pMainLogicWorkQueueManager = pDBServerMainLogic->GetWorkQueueManager();

	XMySqlConnector* pMySqlConnector = pDBOpThread->GetMySqlConnector();
	if (!pMySqlConnector->InitBindParam())
	{
		unsigned int unWorkItemLen = 128;
		LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
		pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_READ_USER_INFO);
		int nResult = Global_Error_Unknown;
		pWorkToMainLogicItem->AddInt(nResult);
		pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
		pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
		pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
		pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
		return ;
	}

	//	暂时不进行数据库操作，以后一起处理
	pMySqlConnector->BindParam(E_Data_Type_String_Fix_Len, szUserID, strlen(szUserID), E_Param_Input);
	pMySqlConnector->BindParam(E_Data_Type_String_Fix_Len, szPassWord, strlen(szPassWord), E_Param_Input);
	int64_t nRetValue = 0;
	pMySqlConnector->BindParam(E_Data_Type_Int64, &nRetValue, sizeof(nRetValue), E_Param_Output);
	pMySqlConnector->SetProcedureName("pc_user_read");

	if (!pMySqlConnector->ExcuteSql())
	{
		unsigned int unWorkItemLen = 128;
		LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
		pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_READ_USER_INFO);
		int nResult = Global_Error_Unknown;
		pWorkToMainLogicItem->AddInt(nResult);
		pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
		pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
		pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
		pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
		return ;
	}
	pMySqlConnector->FreeMoreResult();
	if (!pMySqlConnector->GetOutputResult())
	{
		unsigned int unWorkItemLen = 128;
		LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
		pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_READ_USER_INFO);
		int nResult = Global_Error_Unknown;
		pWorkToMainLogicItem->AddInt(nResult);
		pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
		pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
		pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
		pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
		return ;
	}
	pMySqlConnector->FreeMoreResult();
	if (nRetValue <= 0)
	{
		unsigned int unWorkItemLen = 128;
		LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
		pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_READ_USER_INFO);
		int nResult = -1;
		if (nRetValue == -1)		//	用户不存在
		{
			nResult = Global_Error_User_Not_Exists;
		}
		else if (nRetValue == -2)	//	密码错误
		{
			nResult = Global_Error_User_Password_Error;
		}
		pWorkToMainLogicItem->AddInt(nResult);
		pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
		pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
		pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
		pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
		return ;
	}
	u64UserUniqueIDInDB = (uint64_t)nRetValue;
	unsigned int unWorkItemLen = 128;
	LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
	pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_READ_USER_INFO);
	int nResult = 0;
	pWorkToMainLogicItem->AddInt(nResult);
	pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
	pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
	pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
	pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
	pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
	pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
}
DEFINE_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_DELETE_USER_INFO)
{
}
DEFINE_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_CHANGE_PASSWORD)
{
}

DEFINE_ADBSERVER_DB_OP_PROC(ADB_SERVER_MT2DBT_ADD_USER_INFO)
{
	char szUserID[MAX_USER_ID_LEN + 1];
	char szPassWord[MAX_PASSWORD_LEN + 1];
	uint64_t u64ServerSessionID = 0;
	uint64_t u64SessionIDInClient = 0;			//	玩家在对应的客户端服务器中的SessionID

	memset(szUserID, 0, sizeof(szUserID));
	memset(szPassWord, 0, sizeof(szPassWord));

	pWorkItem->GetData(szUserID, MAX_USER_ID_LEN);
	pWorkItem->GetData(szPassWord, MAX_PASSWORD_LEN);
	pWorkItem->GetULongLong(u64ServerSessionID);
	pWorkItem->GetULongLong(u64SessionIDInClient);

	if (pWorkItem->GetErrorCode() != 0)
	{
		return ;
	}
	uint64_t u64UserUniqueIDInDB = 0;

	LDBOpThread* pDBOpThread = pDBOpManager->GetDBOpThread();
	LDBServerMainLogicThread* pDBServerMainLogic = pDBOpThread->GetDBServerMainLogic();
	
	LWorkQueueManager* pMainLogicWorkQueueManager = pDBServerMainLogic->GetWorkQueueManager();

	XMySqlConnector* pMySqlConnector = pDBOpThread->GetMySqlConnector();
	if (!pMySqlConnector->InitBindParam())
	{
		unsigned int unWorkItemLen = 128;
		LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
		pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_ADD_USER_INFO);
		int nResult = Global_Error_Unknown;
		pWorkToMainLogicItem->AddInt(nResult);
		pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
		pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
		pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
		pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
		return ;
	}

	pMySqlConnector->BindParam(E_Data_Type_String_Fix_Len, szUserID, strlen(szUserID), E_Param_Input);
	pMySqlConnector->BindParam(E_Data_Type_String_Fix_Len, szPassWord, strlen(szPassWord), E_Param_Input);
	int64_t nRetValue = 0;
	pMySqlConnector->BindParam(E_Data_Type_Int64, &nRetValue, sizeof(nRetValue), E_Param_Output);
	pMySqlConnector->SetProcedureName("pc_user_read");

	if (!pMySqlConnector->ExcuteSql())
	{
		unsigned int unWorkItemLen = 128;
		LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
		pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_ADD_USER_INFO);
		int nResult = Global_Error_Unknown;
		pWorkToMainLogicItem->AddInt(nResult);
		pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
		pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
		pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
		pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
		return ;
	}
	pMySqlConnector->FreeMoreResult();
	if (!pMySqlConnector->GetOutputResult())
	{
		unsigned int unWorkItemLen = 128;
		LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
		pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_ADD_USER_INFO);
		int nResult = Global_Error_Unknown;
		pWorkToMainLogicItem->AddInt(nResult);
		pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
		pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
		pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
		pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
		return ;
	}
	pMySqlConnector->FreeMoreResult();
	if (nRetValue <= 0)
	{
		unsigned int unWorkItemLen = 128;
		LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
		pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_ADD_USER_INFO);
		int nResult = -1;
		if (nRetValue == -1)		//	用户不存在
		{
			nResult = Global_Error_User_Not_Exists;
		}
		else if (nRetValue == -2)	//	密码错误
		{
			nResult = Global_Error_User_Password_Error;
		}
		pWorkToMainLogicItem->AddInt(nResult);
		pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
		pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
		pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
		pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
		pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
		pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
		return ;
	}
	u64UserUniqueIDInDB = (uint64_t)nRetValue;
	unsigned int unWorkItemLen = 128;
	LWorkItem* pWorkToMainLogicItem = pMainLogicWorkQueueManager->AllocOneWorkItem(unWorkItemLen);
	pWorkToMainLogicItem->SetWorkID(ADB_SERVER_DBT2MT_ADD_USER_INFO);
	int nResult = 0;
	pWorkToMainLogicItem->AddInt(nResult);
	pWorkToMainLogicItem->AddData(szUserID, MAX_USER_ID_LEN);
	pWorkToMainLogicItem->AddData(szPassWord, MAX_PASSWORD_LEN);
	pWorkToMainLogicItem->AddULongLong(u64UserUniqueIDInDB);
	pWorkToMainLogicItem->AddULongLong(u64ServerSessionID);
	pWorkToMainLogicItem->AddULongLong(u64SessionIDInClient);
	pMainLogicWorkQueueManager->AddWorkItemToGlobalQueue(pWorkItem);
	
}
