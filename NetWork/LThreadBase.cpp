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

#include "LThreadBase.h"

LThreadBase::LThreadBase(void)
{
	m_ThreadHandle 	= 0;
	m_bStop			= false;
}

LThreadBase::~LThreadBase(void)
{
}


bool LThreadBase::Start()
{
	if (!OnStart())
	{
		return false;
	}
	m_bStop = false;
	int nResult = pthread_mutex_init(&m_StopMutex, NULL);
	if (nResult == -1)
	{
		return false;
	}
	nResult = pthread_create(&m_ThreadHandle, NULL, LThreadBase::ThreadProc, this);
	if (nResult == -1)
	{
		return false;
	}
	return true;
}
void* LThreadBase::ThreadProc(void* pParam)
{
	LThreadBase* pThread = (LThreadBase*)pParam;
	if (pThread == NULL)
	{
		return NULL;
	}
	pThread->ThreadDoing(NULL);

	pThread->OnStop();
	return NULL;
}

int LThreadBase::ThreadDoing(void* pParam)
{
	sleep(1);
	return 0;
}

void LThreadBase::Stop()
{
	pthread_mutex_lock(&m_StopMutex);
	m_bStop = true;
	pthread_mutex_unlock(&m_StopMutex);
}
pthread_t LThreadBase::GetThreadHandle()
{
	return m_ThreadHandle;
}


bool LThreadBase::CheckForStop()
{
	bool bStop = false;
	pthread_mutex_lock(&m_StopMutex);
	bStop = m_bStop;
	pthread_mutex_unlock(&m_StopMutex);
	return bStop;
}


bool LThreadBase::OnStart()
{
	return true;
}
void LThreadBase::OnStop()
{
}

