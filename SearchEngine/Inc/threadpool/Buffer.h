#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <functional>
#include <queue>
#include "MutexLock.h"
#include "Condition.h"


namespace tanfy
{

class SearchTask;

class Buffer
{
public:
	Buffer(size_t maxsize);
	
	void push(SearchTask *mytask);
	SearchTask *pop();
	void wakeupall();

private:
	bool ifEmpty();
	bool ifFull();

private:
	std::queue<SearchTask *> que_;
	size_t maxsize_;
	MutexLock mutex_;
	Condition notEmpty_;
	Condition notFull_;
	bool ifwakeup_;
};

}//end of namespace

#endif
