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

#include "LDBServerMainLogicThread.h"
#include "LWorkItem.h"

LDBServerMessageProcess::LDBServerMessageProcess()
{
	m_pDBServerMainLogicThread = NULL;
}
LDBServerMessageProcess::~LDBServerMessageProcess()
{
}

//	分发消息包进行处理
void LDBServerMessageProcess::DispatchMessageToProcess(LWorkItem* pWorkItem)
{
	if (pWorkItem == NULL)
	{
		return ;
	}
	unsigned int unWorkID = pWorkItem->GetWorkID();
	switch (unWorkID)
	{
	//	case ADB_SERVER_DBT2MT_READ_USER_INFO:
	//		{
	//			DBMessageProcess_Read_User_Info(pWorkItem);
	//		}
	//		break;
	//	case ADB_SERVER_DBT2MT_ADD_USER_INFO:
	//		{
	//			DBMessageProcess_AddUser_Info(pWorkItem);
	//		}
	//		break;
		case 1:
			{
			}
			break;
		default:
			break;
	}
}

void LDBServerMessageProcess::SetDBServerMainLogicThread(LDBServerMainLogicThread* pdbsmlt)
{
	m_pDBServerMainLogicThread = pdbsmlt;
}
LDBServerMainLogicThread* LDBServerMessageProcess::GetDBServerMainLogicThread()
{
	return m_pDBServerMainLogicThread;
}


