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

#include "LConnectorTest.h"
#include "LErrorWriter.h"

//extern  LErrorWriter g_ErrorWriter;
LErrorWriter g_ErrorWriter;
int main (int nargc, char* argv[])
{
//	struct sigaction action;
//	memset(&action, 0, sizeof(action));
//	action.sa_handler = SIG_IGN;
//	action.sa_flags = 0;
//	sigemptyset(&action.sa_mask);
//	sigaction(SIGPIPE, &action, NULL);	
//	
//	memset(&action, 0, sizeof(action));
//	action.sa_handler = SIG_IGN;
//	action.sa_flags = 0;
//	sigemptyset(&action.sa_mask);
//	sigaction(SIGHUP, &action, NULL);	
//
//	sigset_t sigset;
//	sigemptyset(&sigset);
//	sigaddset(&sigset, SIGPIPE);
//	int n = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
//	if (n != 0)
//	{
//		return -1;
//	}
	char* pErrorFileName = "Error.txt";
	if (!g_ErrorWriter.Initialize(pErrorFileName))
	{
		return -5;
	}

	LConnectorTest ct;
	if (!ct.Initialize("NetConnectorConfig.ini"))
	{
		return -1;
	}
	if (!ct.Start())
	{
		return -1;
	}
	pthread_join(ct.GetThreadHandle(), NULL);
	return 0;
}

