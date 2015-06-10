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

#include "LAutoReleaseMutex.h"

LAutoReleaseMutex::LAutoReleaseMutex(pthread_mutex_t* pMutex)
{
	m_pMutex = NULL;
	if (pMutex != NULL)
	{
		m_pMutex = pMutex;
		pthread_mutex_lock(m_pMutex);
	}
}

LAutoReleaseMutex::~LAutoReleaseMutex()
{
	if (m_pMutex != NULL)
	{
		pthread_mutex_unlock(m_pMutex);
		m_pMutex = NULL;
	}
}

void LAutoReleaseMutex::LAutoReleaseMutex::Free()
{
	if (m_pMutex != NULL)
	{ 
		pthread_mutex_unlock(m_pMutex);
		m_pMutex = NULL;
	}
}

