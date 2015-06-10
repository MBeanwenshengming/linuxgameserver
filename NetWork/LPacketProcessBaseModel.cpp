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

#include "LPacketProcessBaseModel.h"
#include "LPacketSingle.h"

LPacketProcessBaseModel::LPacketProcessBaseModel()
{
}

LPacketProcessBaseModel::~LPacketProcessBaseModel()
{
}

//	注册处理函数
bool LPacketProcessBaseModel::RegisterFunction(unsigned int unPacketID, PROC_PACKETPROCESS_SINGLE pFunc)
{
	if (pFunc == NULL)
	{
		return false;
	}
	map<unsigned int, PROC_PACKETPROCESS_SINGLE>::iterator _ito = m_mapPacketToPacketProcessProc.find(unPacketID);
	if (_ito != m_mapPacketToPacketProcessProc.end())
	{
		return false;
	}
	m_mapPacketToPacketProcessProc[unPacketID] = pFunc;
	return true;
}

//	分发消息包给处理函数
bool LPacketProcessBaseModel::DispatchPacketToProcess(uint64_t u64SessionID, LPacketSingle* pPacket)
{
	if (pPacket == NULL)
	{
		return false;
	}
	unsigned int unPacketID  = 0; // pPacket->GetPacketID();
	if (unPacketID == 0)
	{
		return false;
	}

	map<unsigned int, PROC_PACKETPROCESS_SINGLE>::iterator _ito = m_mapPacketToPacketProcessProc.find(unPacketID);
	if (_ito == m_mapPacketToPacketProcessProc.end())
	{
		return false;
	}
	_ito->second(u64SessionID, pPacket);
	return true;
}
