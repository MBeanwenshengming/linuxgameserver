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

#include "LErrorWriter.h"




LErrorWriter::LErrorWriter()
{
	m_pErrorFile = NULL;
#ifndef WIN32
	pthread_mutex_init(&m_mutexForWrite, NULL);
#else
	InitializeCriticalSectionAndSpinCount(&m_mutexForWrite, 4000);
#endif
}
LErrorWriter::~LErrorWriter()
{
#ifndef WIN32 
	pthread_mutex_destroy(&m_mutexForWrite);
#else
	DeleteCriticalSection(&m_mutexForWrite);
#endif
	if (m_pErrorFile != NULL)
	{
		fflush(m_pErrorFile);
		fclose(m_pErrorFile);
		m_pErrorFile = NULL;
	}
}

bool LErrorWriter::Initialize(char* pFileName)
{
	if (pFileName == NULL)
	{
		return false;
	}
	m_pErrorFile = fopen(pFileName, "a+");
	if (m_pErrorFile == NULL)
	{
		return false;
	}
	return true;
}
void LErrorWriter::WriteError(char* pErrorString, size_t sErrorStringLen)
{
	if (pErrorString == NULL)
	{
		return ;
	}
	if (strlen(pErrorString) != sErrorStringLen)
	{
		return ;
	}
	fputs(pErrorString, m_pErrorFile);
	fflush(m_pErrorFile);
}
