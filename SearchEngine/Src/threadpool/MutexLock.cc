#include "MutexLock.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

namespace tanfy
{

MutexLock::MutexLock()
{
	int ret = pthread_mutex_init(&mutex_, NULL);
	if(0 != ret)
	{
		perror("pthread_mutex_init error");
		exit(-1);
	}	
}

MutexLock::~MutexLock()
{
	int ret = pthread_mutex_destroy(&mutex_);
	if(0 != ret)
	{
		perror("pthread_mutex_destroy error");
		exit(-1);
	}
}

void MutexLock::lock()
{
	int ret = pthread_mutex_lock(&mutex_);
	if(0 != ret)
	{
		perror("pthread_mutex_lock error");
		exit(-1);
	}
}

void MutexLock::unlock()
{
	int ret = pthread_mutex_unlock(&mutex_);
	if(0 != ret)
	{
		perror("pthread_mutex_unlock error");
		exit(-1);
	}
}

pthread_mutex_t *MutexLock::getmutexptr()
{
	return &mutex_;
}

}//end of namespace
