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

#ifndef __WORK_QUEUE_STRUCT_DEFINED__
#define __WORK_QUEUE_STRUCT_DEFINED__

//	���ն���
typedef struct _Recv_Work_Queue_Item
{
	_Recv_Work_Queue_Item* pPreview;		//	����
	_Recv_Work_Queue_Item* pNext;
	unsigned __int64 un64SessionID;
	_Recv_Work_Queue_Item()
	{
		pPreview = NULL;
		pNext	 = NULL;
		un64SessionID = 0;
	}
	void Reset()
	{
		pPreview = NULL;
		pNext	 = NULL;
		un64SessionID = 0;
	}
}t_Recv_Work_Queue_Item;

//	���Ͷ���
typedef struct _Send_Work_Queue_Item
{
	_Send_Work_Queue_Item* pPreview;
	_Send_Work_Queue_Item* pNext;
	_Send_Work_Queue_Item()
	{
		pPreview	= NULL;
		pNext		= NULL;
	}
}t_Send_Work_Queue_Item;
#endif


