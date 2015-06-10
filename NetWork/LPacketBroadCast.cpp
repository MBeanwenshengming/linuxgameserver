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

#include "LPacketBroadCast.h"

LPacketBroadCast::LPacketBroadCast(unsigned short usPacketBufLen) : LPacketBase(usPacketBufLen)
{
	__sync_lock_test_and_set(&m_RefCount, 0);
}
LPacketBroadCast::~LPacketBroadCast()
{
}

void LPacketBroadCast::IncrementRefCount()
{
	__sync_add_and_fetch(&m_RefCount, 1);
}

bool LPacketBroadCast::DecrementRefCountAndResultIsTrue()
{
	bool IsZero = false;
	int nNewValue = __sync_sub_and_fetch(&m_RefCount, 1);
	if (nNewValue == 0)
	{
		IsZero = true;
	}
	if (nNewValue < 0)
	{
		printf("packetRefCount < 0\n");
	}
	return IsZero;
}

void LPacketBroadCast::ResetRefCount()
{
	Reset();
	__sync_lock_test_and_set(&m_RefCount, 0);
}

int LPacketBroadCast::GetCurRefCount()
{
	return __sync_add_and_fetch(&m_RefCount, 0);
}
