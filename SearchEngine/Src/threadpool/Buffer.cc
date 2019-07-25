#include "Buffer.h"
#include <iostream>

namespace tanfy
{

Buffer::Buffer(size_t maxsize)
	:maxsize_(maxsize),
	 mutex_(),
	 notEmpty_(mutex_),
	 notFull_(mutex_),
	 ifwakeup_(false)
{}

bool Buffer::ifEmpty()
{
	return (0 == que_.size())?true : false;
}

bool Buffer::ifFull()
{
	return (maxsize_ == que_.size())?true:false;
}

void Buffer::push(SearchTask *mytask)
{
	mutex_.lock();

	while(ifFull())
		notFull_.wait();
	que_.push(mytask);
	notEmpty_.notify();
	
	mutex_.unlock();		
}

tanfy::SearchTask* Buffer::pop()
{
	mutex_.lock();
	
	while(ifEmpty() && !ifwakeup_)
		notEmpty_.wait();
	SearchTask *mytask = NULL;
	if(!ifwakeup_)
	{
		mytask = que_.front();
		que_.pop();
		notFull_.notify();
	}
	mutex_.unlock();

	return mytask;
}

void Buffer::wakeupall()
{
	ifwakeup_ = true;
	notEmpty_.notifyall();
}

}//end of namespace
