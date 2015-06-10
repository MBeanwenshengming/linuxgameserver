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

#pragma once

#include "IncludeHeader.h"

#define MAX_FILE_SIZE 1024*16
#define LEFT_BRACE '['
#define RIGHT_BRACE ']'

class LIniFileReadAndWrite
{
public:
	LIniFileReadAndWrite(void);
	~LIniFileReadAndWrite(void);
public:
	bool OpenIniFile(const char *file);
public:
	int read_profile_string(const char *section, const char *key,char *value, int size,const char *default_value);
	int read_profile_int(const char *section, const char *key, int default_value);
	int write_profile_string(const char *section, const char *key,const char *value, const char* outfile);
private:
	char m_buf[MAX_FILE_SIZE];
	FILE* m_pFile;

protected:
	inline int newline(char c)
	{
		return ('\n' == c ||  '\r' == c )? 1 : 0;
	}
	inline int end_of_string(char c)
	{
		return '\0'==c? 1 : 0;
	}
	inline int left_barce(char c)
	{
		return LEFT_BRACE == c? 1 : 0;
	}
	inline int isright_brace(char c )
	{
		return RIGHT_BRACE == c? 1 : 0;
	}
	int parse_file(const char *section, const char *key, const char *buf,int *sec_s,int *sec_e,
					  int *key_s,int *key_e, int *value_s, int *value_e);
};
