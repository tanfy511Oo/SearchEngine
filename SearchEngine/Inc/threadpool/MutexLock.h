#ifndef __MUTEXLOCK_H__
#define __MUTEXLOCK_H__

#include <pthread.h>
#include "Noncopyable.h"

namespace tanfy
{

class MutexLock:Noncopyable
{
public:
	MutexLock();
	~MutexLock();

	void lock();
	void unlock();

	pthread_mutex_t *getmutexptr();

private:
	pthread_mutex_t mutex_;
};

}//end of namespace

#endif
