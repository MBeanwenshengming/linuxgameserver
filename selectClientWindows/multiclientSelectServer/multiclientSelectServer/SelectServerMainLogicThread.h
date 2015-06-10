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


#ifndef SELECTSERVERMAINLOGICTHREAD_H_
#define SELECTSERVERMAINLOGICTHREAD_H_

#include "LThreadBase.h"
#include <map>
#include <time.h>
using namespace std;
class LSelectServer;
class LPacketSingle;

typedef struct _Session_Info
{
	unsigned int unSessionID;
	time_t tLastSendPacketTime;
	int nSendInterval;
	_Session_Info()
	{
		unSessionID = 0;
		tLastSendPacketTime = time(NULL);
		nSendInterval = 5;
	}
}t_Session_Info;

class SelectServerMainLogicThread: public LThreadBase {
public:
	SelectServerMainLogicThread();
	virtual ~SelectServerMainLogicThread();
public:
	virtual int ThreadDoing(void* pParam);
	virtual bool OnStart();
	virtual void OnStop();

public:
	void OnAddSession(unsigned int unSessionID);
	void OnSessionDisconnect(unsigned int unSessionID);
public:
	void SetSelectServer(LSelectServer* pSelectServer);
	void BuildRandomPacket(LPacketSingle* pPacket);
private:
	LSelectServer* m_pSelectServer;
	map<unsigned int, t_Session_Info> m_mapSessionConnected;
};

#endif /* SELECTSERVERMAINLOGICTHREAD_H_ */
