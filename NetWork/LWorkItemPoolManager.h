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
#include "LFixLenCircleBuf.h"
#include <queue>
#include <map>
using namespace std;

class LWorkItem;

typedef struct _WorkItem_Pool_Init_Param
{
	_WorkItem_Pool_Init_Param()
	{
		usPoolBufSize 	= 0;	
		unPoolInitCount = 0;
		unMaxPoolSize 	= 0;;	
	}
	unsigned short usPoolBufSize;		//	WorkItem分配的缓冲区大小
	unsigned int unPoolInitCount; 	//	WorkItem初始分配的个数
	unsigned int unMaxPoolSize;		//	缓冲区中最多留下多少个WorkItem，多于的删除
}t_WorkItem_Pool_Init_Param;

class LWorkItemPool
{
public:
	LWorkItemPool(); 
	~LWorkItemPool();
public:
	bool InitializePool(t_WorkItem_Pool_Init_Param twipip);
	LWorkItem* AllocOneWorkItem();
	void FreeOneWorkItem(LWorkItem* pWorkItem);
	void Release();
private:
	//queue<LWorkItem*> m_queueWorkItemPool;
	LFixLenCircleBuf m_FixLenCircleBuf;
	unsigned short m_usWorkItemBufSize;
	unsigned int m_unMaxWorkItemPoolSize;
};


class LWorkItemPoolManager
{
public:
	LWorkItemPoolManager();
	~LWorkItemPoolManager();
public:
	bool Initialize(t_WorkItem_Pool_Init_Param* wpip, unsigned int unTypeCount);
public:
	LWorkItem* AllocOneWorkItem(unsigned short usBufSize);
	void FreeOneWorkItem(LWorkItem* pWorkItem);
	void ReleaseAllPool();
private:
	map<unsigned short, LWorkItemPool*> m_mapSizeToWorkItemPool; 
};

