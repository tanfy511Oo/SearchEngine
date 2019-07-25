#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <functional>
#include "Cache.h"

namespace tanfy
{

class Thread
{
public:
	typedef std::function<void(Cache &)> ThreadCallback;
	Thread(ThreadCallback cb, Cache &cache);
	~Thread();
	
	void start();
	void join();
	
public:
	Cache cache_;

private:
	static void *ThreadFunc(void *arg);

private:
	pthread_t pthid_;
	ThreadCallback cb_;
	bool isRunning_;
};

}//end of namespace









#endif
