#include "Condition.h"
#include "MutexLock.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

namespace tanfy
{	

Condition::Condition(MutexLock &mutex)
	:mutex_(mutex)
{
	int ret = pthread_cond_init(&cond_, NULL);
	if(0 != ret)
	{
		perror("pthread_cond_init error");
		exit(-1);
	}
}

Condition::~Condition()
{
	int ret = pthread_cond_destroy(&cond_);
	if(0 != ret)
	{
		perror("pthread_cond_destroy error");
		exit(-1);
	}	
}

void Condition::wait()
{
	pthread_cond_wait(&cond_, mutex_.getmutexptr());
}

void Condition::notify()
{
	pthread_cond_signal(&cond_);
}

void Condition::notifyall()
{
	pthread_cond_broadcast(&cond_);
}

}//end of namespace
