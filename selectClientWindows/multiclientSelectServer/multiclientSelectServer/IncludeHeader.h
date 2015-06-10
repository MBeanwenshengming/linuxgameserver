#pragma once

#define FD_SETSIZE  1024

#include "windows.h"
#include <process.h>
#include "stdio.h"
#include <assert.h>


typedef int socklen_t;
typedef __int64 int64_t; 
typedef unsigned __int64 uint64_t;

#define __USE_SESSION_BUF_TO_SEND_DATA__ 1