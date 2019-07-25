#include "ChildThread.h"
#include <pthread.h>
#include <stdio.h>

namespace tanfy
{

ChildThread::ChildThread(ChildThreadCallback cb)
	:cb_(cb),
	 isRunning_(false)
{}


ChildThread::~ChildThread()
{
	if(isRunning_)
	{
		pthread_detach(pthid_);
		isRunning_ = false;
	}
}

void ChildThread::start()
{
	isRunning_ = true;
	int ret = pthread_create(&pthid_, NULL, ThreadFunc, this);
	if(0 != ret)
	{
		perror("pthread_create error");
		exit(-1);
	}
}


void ChildThread::join()
{
	if(isRunning_)
	{
		isRunning_ = false;
		int ret = pthread_join(pthid_, NULL);
		if(0 != ret)
		{
			perror("pthread_join error");
			exit(-1);
		}
	}
}

void *ChildThread::ThreadFunc(void *arg)
{ 
	ChildThread *pthread = static_cast<ChildThread*>(arg);
	if(NULL != arg)
	{
		pthread->cb_();
	}
}

}//end of namespace
