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

#include "LIniFileReadAndWrite.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <assert.h>
//#include <string.h>
//#include <ctype.h>

LIniFileReadAndWrite::LIniFileReadAndWrite(void)
{
	memset(m_buf, 0, MAX_FILE_SIZE);
	m_pFile = NULL;
}

LIniFileReadAndWrite::~LIniFileReadAndWrite(void)
{
	if (m_pFile != NULL)
	{
		fclose(m_pFile);
	}
}

bool LIniFileReadAndWrite::OpenIniFile(const char *file)
{
	m_pFile = NULL;
	int i=0;

	assert(file !=NULL);

	m_pFile = fopen(file,"r");
	if( NULL == m_pFile)
	{
		return false;
	}

	m_buf[i]=fgetc(m_pFile);
	
	//load initialization file
	while (m_buf[i]!= (char)EOF)
	{
		i++;
		assert( i < MAX_FILE_SIZE ); //file too big, you can redefine MAX_FILE_SIZE to fit the big file
		if (i >= MAX_FILE_SIZE)
		{
			return false;
		}
		m_buf[i] = fgetc(m_pFile);
	}	
	m_buf[i]='\0';

	return true;
}


int LIniFileReadAndWrite::read_profile_string(const char *section, const char *key,char *value, int size,const char *default_value)
{
	int sec_s,sec_e,key_s,key_e, value_s, value_e;

	//check parameters
	assert(section != NULL && strlen(section));
	assert(key != NULL && strlen(key));
	assert(value != NULL);
	assert(size > 0);
	assert(strlen(key));

	//if(!load_ini_file(file,buf,&file_size))
	if (m_pFile == NULL)
	{
		if(default_value!=NULL)
		{
			strncpy(value, default_value, size);
		}
		return 0;
	}

	if(!parse_file(section, key, m_buf, &sec_s, &sec_e, &key_s, &key_e, &value_s, &value_e))
	{
		if(default_value!=NULL)
		{
			strncpy(value, default_value, size);
		}
		return 0; //not find the key
	}
	else
	{
		int cpcount = value_e -value_s;

		if( size-1 < cpcount)
		{
			cpcount =  size-1;
		}
	
		memset(value, 0, size);
		memcpy(value, m_buf + value_s, cpcount );
		value[cpcount] = '\0';

		return 1;
	}
}
int LIniFileReadAndWrite::read_profile_int(const char *section, const char *key, int default_value)
{
	char value[32] = {0};
	if(!read_profile_string(section, key, value, sizeof(value), NULL))
	{
		return default_value;
	}
	else
	{
		return atoi(value);
	}
}
int LIniFileReadAndWrite::write_profile_string(const char *section, const char *key,const char *value, const char* outfile)
{
	//char buf[MAX_FILE_SIZE]={0};
	char w_buf[MAX_FILE_SIZE]={0};
	int sec_s,sec_e,key_s,key_e, value_s, value_e;
	int value_len = (int)strlen(value);
	int file_size = 0;
	FILE *out = NULL;

	//check parameters
	assert(section != NULL && strlen(section));
	assert(key != NULL && strlen(key));
	assert(value != NULL);
	assert(outfile !=NULL &&strlen(key));

	//if(!load_ini_file(file,buf,&file_size))
	if (m_pFile == NULL)
	{
		sec_s = -1;
	}
	else
	{
		parse_file(section, key, m_buf, &sec_s, &sec_e, &key_s, &key_e, &value_s, &value_e);
	}

	if( -1 == sec_s)
	{
		if(0==file_size)
		{
			sprintf(w_buf + file_size, "[%s]\n%s=%s\n", section, key, value);
		}
		else
		{
			//not find the section, then add the new section at end of the file
			memcpy(w_buf,m_buf,file_size);
			sprintf(w_buf+file_size,"\n[%s]\n%s=%s\n",section,key,value);
		}
	}
	else if(-1 == key_s)
	{
		//not find the key, then add the new key=value at end of the section
		memcpy(w_buf,m_buf,sec_e);
		sprintf(w_buf+sec_e,"%s=%s\n",key,value);
		sprintf(w_buf+sec_e+strlen(key)+strlen(value)+2,m_buf+sec_e, file_size - sec_e);
	}
	else
	{
		//update value with new value
		memcpy(w_buf,m_buf,value_s);
		memcpy(w_buf+value_s,value, value_len);
		memcpy(w_buf+value_s+value_len, m_buf+value_e, file_size - value_e);
	}
	
	out = fopen(outfile,"w");
	if(NULL == out)
	{
		return 0;
	}
	
	if(-1 == fputs(w_buf,out) )
	{
		fclose(out);
		return 0;
	}

	fclose(out);
	return 1;
}

int LIniFileReadAndWrite::parse_file(const char *section, const char *key, const char *buf,int *sec_s,int *sec_e,
					  int *key_s,int *key_e, int *value_s, int *value_e)
{
	const char *p = buf;
	int i=0;

	assert(buf!=NULL);
	assert(section != NULL && strlen(section));
	assert(key != NULL && strlen(key));

	*sec_e = *sec_s = *key_e = *key_s = *value_s = *value_e = -1;

	while( !end_of_string(p[i]) )
	{
		//find the section
		if( ( 0==i ||  newline(p[i-1]) ) && left_barce(p[i]) )
		{
			int section_start=i+1;

			//find the ']'
			do
			{
				i++;
			} while( !isright_brace(p[i]) && !end_of_string(p[i]));

			//if( 0 == strncmp(p + section_start, section, i - section_start))	//	bug
			if( 0 == strncmp(p + section_start, section, strlen(section)))
			{
				int newline_start=0;

				i++;

				//Skip over space char after ']'
				while(isspace(p[i]))
				{
					i++;
				}

				//find the section
				*sec_s = section_start;
				*sec_e = i;

				while(!(newline(p[i-1]) && left_barce(p[i])) 
				&& !end_of_string(p[i]))
				{
					int j=0;
					//get a new line
					newline_start = i;

					while( !newline(p[i]) &&  !end_of_string(p[i]) ) 
					{
						i++;
					}
					
					//now i  is equal to end of the line
					j = newline_start;

					if(';' != p[j]) //skip over comment
					{
						while(j < i && p[j]!='=') 
						{
							j++;
							if('=' == p[j]) 
							{
								//if(strncmp(key, p + newline_start, j - newline_start)==0)
								if(strncmp(key, p + newline_start, strlen(key))==0)
								{
									//find the key ok
									*key_s = newline_start;
									*key_e = j-1;

									*value_s = j+1;
									//	�������ע��
									int m = j + 1;
									while (m < i)
									{
										if (';' == p[m])
										{
											break;
										}
										else
										{
											m++;
										}
									}
									*value_e = m;

									return 1;
								}
							}
						}
					}
					i++;
				}
			}
		}
		else
		{
			i++;
		}
	}
	return 0;
}
