#include "Thread.h"
#include <pthread.h>
#include <stdio.h>

namespace tanfy
{

Thread::Thread(ThreadCallback cb, Cache &cache)
	:cb_(cb),
	 cache_(cache),
	 isRunning_(false)
{}


Thread::~Thread()
{
	if(isRunning_)
	{
		pthread_detach(pthid_);
		isRunning_ = false;
	}
}

void Thread::start()
{
	isRunning_ = true;
	int ret = pthread_create(&pthid_, NULL, ThreadFunc, this);
	if(0 != ret)
	{
		perror("pthread_create error");
		exit(-1);
	}
}


void Thread::join()
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

void *Thread::ThreadFunc(void *arg)
{ 
	Thread *pthread = static_cast<Thread*>(arg);
	if(NULL != arg)
	{
		pthread->cb_(pthread->cache_);
	}
}

}//end of namespace
