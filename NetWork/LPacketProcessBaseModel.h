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
#include <map>
using namespace std;

#include "IncludeHeader.h"
class LPacketSingle;

typedef void (*PROC_PACKETPROCESS_SINGLE)(uint64_t u64SessionID, LPacketSingle* pPacket);

//	#define REGISTER_PACKETPROCESS_PROC(X) 

class LPacketProcessBaseModel
{
public:
	LPacketProcessBaseModel();
	~LPacketProcessBaseModel();
public:
	//	注册处理函数
	bool RegisterFunction(unsigned int unPacketID, PROC_PACKETPROCESS_SINGLE pFunc);
	//	分发消息包给处理函数
	bool DispatchPacketToProcess(uint64_t u64SessionID, LPacketSingle* pPacket);
private:
	//	消息对应到的消息处理函数
	map<unsigned int, PROC_PACKETPROCESS_SINGLE> m_mapPacketToPacketProcessProc;
};

