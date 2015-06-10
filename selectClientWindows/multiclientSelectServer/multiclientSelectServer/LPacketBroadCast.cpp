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
#ifndef WIN32
	atomic_set(&m_atomicRefCount, 0);
#else
	InterlockedExchange(&m_atomicRefCount, 0);
#endif
}
LPacketBroadCast::~LPacketBroadCast()
{
}

void LPacketBroadCast::IncrementRefCount()
{
#ifndef WIN32
	atomic_inc(&m_atomicRefCount);
#else
	InterlockedIncrement(&m_atomicRefCount);
#endif
}

bool LPacketBroadCast::DecrementRefCountAndResultIsTrue()
{
#ifndef WIN32
	//int nsrc = atomic_read(&m_atomicRefCount);
	return atomic_dec_and_test(&m_atomicRefCount);
	//int nResult = atomic_dec_and_test(&m_atomicRefCount);
	
	//int ndest = atomic_read(&m_atomicRefCount);
	
	//return nResult == 0 ? true : false;
	//	atomic_dec_return(&m_atomicRefCount);
#else
	volatile long lResult = InterlockedDecrement(&m_atomicRefCount);
	if (lResult == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
#endif
}

void LPacketBroadCast::ResetRefCount()
{
	Reset();
#ifndef WIN32
	atomic_set(&m_atomicRefCount, 0); 
#else
	InterlockedExchange(&m_atomicRefCount, 0);
#endif
}

int LPacketBroadCast::GetCurRefCount()
{
#ifndef WIN32
	return atomic_read(&m_atomicRefCount);
#else
	return m_atomicRefCount;
#endif
}
