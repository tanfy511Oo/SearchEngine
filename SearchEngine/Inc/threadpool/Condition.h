#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <pthread.h>
#include "Noncopyable.h"

namespace tanfy
{

class MutexLock;

class Condition:Noncopyable
{
public:
	Condition(MutexLock &mutex);
	~Condition();

	void wait();
	void notify();
	void notifyall();

private:
	pthread_cond_t cond_;
	MutexLock &mutex_;
};

}//end of namespace

#endif
