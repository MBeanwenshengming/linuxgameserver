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

#include "IncludeHeader.h"
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include "LMainLogicThread.h"

static struct termios oldt;
unsigned int g_unServerID = 0;

void RestoreTerminalState()
{
	tcsetattr(0, TCSANOW, &oldt);
}

void EnableTerminalEnterString()
{
	struct termios newt;


	tcgetattr(0, &oldt);

	newt = oldt;

	newt.c_iflag &= ~(ICANON | ECHO);

	tcsetattr(0, TCSANOW, &newt);

	atexit(RestoreTerminalState);
}

int main(int nargc, char* argv[])
{ 
	char* pOptString = "s:";
	int nch = -1;
	extern char* optarg;
	extern int optopt;
	while ((nch = getopt(nargc, argv, pOptString)) != -1)
	{
		switch (nch)
		{
			case 's':
				{
					printf("argname:%c, value:%s\n", nch, optarg);
				}
				break;
			case '?':
				{
					printf("unknown arg:%c\n", optopt);
				}
				break;
			default:
				break;
		}
	}

	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGPIPE, &action, NULL);	
	
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGHUP, &action, NULL);	

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGPIPE);
	int n = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	if (n != 0)
	{
		return -1;
	}

	int ch;

	EnableTerminalEnterString();

	printf("Hello World, MasterServer\n");
	printf("Master Server Running, you can input command\n");
	LMainLogicThread MainLogicThread;

	//	初始化逻辑线程
	if (!MainLogicThread.Initialize("MasterServerConfig.ini", "MasterServerConfig.ini"))
	{
		return -1;
	}
	if (!MainLogicThread.Start())
	{
		return -1;
	}
	while (1)
	{
		ch = getchar();
		if (ch == 'Q')
		{
			MainLogicThread.StopMainLogicThread();
			return 0;
		}
		printf("Enter char is:%c\n", ch);
	}

	return 0;
}
