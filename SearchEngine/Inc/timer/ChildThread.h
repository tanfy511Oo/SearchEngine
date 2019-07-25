#ifndef __CHILDTHREAD_H__
#define __CHILDTHREAD_H__

#include <pthread.h>
#include <functional>
#include "Cache.h"

namespace tanfy
{

class ChildThread
{
public:
	typedef std::function<void()> ChildThreadCallback;
	ChildThread(ChildThreadCallback cb);
	~ChildThread();
	
	void start();
	void join();
	
public:
	Cache cache_;

private:
	static void *ThreadFunc(void *arg);

private:
	pthread_t pthid_;
	ChildThreadCallback cb_;
	bool isRunning_;
};

}//end of namespace









#endif
