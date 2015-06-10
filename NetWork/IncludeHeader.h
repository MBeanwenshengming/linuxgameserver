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

#ifndef __INCLUDE_HEADER_FILE_DEFINED__
#define __INCLUDE_HEADER_FILE_DEFINED__

#ifndef __i386__	//	 ding yi 386 jiqi she ji yuan zi cao zuo 
#define __i386__
#endif

#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <asm/errno.h>
//#include <asm/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <alsa/iatomic.h>
#include <sched.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include <time.h>

//	下面这两个宏，只能定义一个
//	__ADD_SEND_BUF_CHAIN__使用发送数据包的引用计数方法，(这个方法节省内存，但是现在还有bug)
//	__USE_SESSION_BUF_TO_SEND_DATA__ 使用session自带的缓存存放发送的数据
//#define __ADD_SEND_BUF_CHAIN__ 1
#define __USE_SESSION_BUF_TO_SEND_DATA__ 1

#endif

