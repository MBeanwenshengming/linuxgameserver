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

#include "LWorkItemPoolManager.h"
#include "LWorkItem.h" 
#include "LFixLenCircleBuf.h"


class LWorkQueueManagerConfigReader
{
public: 
	LWorkQueueManagerConfigReader();
	~LWorkQueueManagerConfigReader();
public:
	bool ReadWorkQueueConfig(char* pConfigFile, char* pHeader);
	void GetInitParam(t_WorkItem_Pool_Init_Param** pwpip, unsigned short& usArrayCount, unsigned int& unGlobalWorkQueueItemSize);
private:
	t_WorkItem_Pool_Init_Param* m_pArrayWipip;
	unsigned short m_usArrayCount;
	unsigned int m_unGlobalWorkQueueItemSize;
};

class LWorkQueueManager
{
public:
	LWorkQueueManager();
	~LWorkQueueManager();
public:
	bool InitializePool(t_WorkItem_Pool_Init_Param* wpip, unsigned int unTypeCount, unsigned int unGlobalWorkQueueItemSize); 
	void Release(); 

public:		//	工作队列的缓冲池
	LWorkItem* AllocOneWorkItem(unsigned short usBufLen);
	void FreeOneWorkItem(LWorkItem* pWorkItem);
private:
	LWorkItemPoolManager m_WorkItemPoolManager;

public:
	//本地保存的工作信息，等待处理返回时需要的信息，但是不需要发送到其它线程的，存放在这里
	//等其它线程处理返回后，从这里取出信息来辅助处理
	bool AddWorkItemToLocal(LWorkItem* pWorkItem);
	LWorkItem* FindLocalWorkItem(unsigned int unWorkItemID);
	void RemoveLocalWorkItem(unsigned int unWorkItemID);
private:
	//	WorkItem的唯一ID号对应到WorkItem
	map<unsigned int, LWorkItem*> m_mapLocalWorkQueue;
	
public:		//	与外界交互的工作队列
	bool AddWorkItemToGlobalQueue(LWorkItem* pWorkItem);
	LWorkItem* GetOneWorkItemFromGlobalQueue();
protected:
	void ReleaseWorkItemInGlobalQueue();
private:
	//	工作队列中存放的最大的工作项数目
	unsigned int m_unGlobalWorkQueueItemSize;
	LFixLenCircleBuf m_FixLenCircleBufWorkQueue;
};

