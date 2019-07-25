#include "ThreadPool.h"
#include "Thread.h"
#include "SearchTask.h"
#include <iostream>

namespace tanfy
{

ThreadPool::ThreadPool(size_t threadNum, size_t maxsize, Cache &cache, char *pagestart, IndexMap &indexMap, OffsetMap &offsetMap, cppjieba::Jieba &mycut)
	:pagestart_(pagestart),
	 indexMap_(indexMap),
	 offsetMap_(offsetMap),
	 cache_(cache),
	 mycut_(mycut),
	 threadNum_(threadNum),
	 buf_(maxsize),
	 isRunning_(false),
	 cacheUpdating_(false)
{}

ThreadPool::~ThreadPool()
{
	if(isRunning_)
		stop();
}

void ThreadPool::start()
{
	isRunning_ = true;
	for(int idx = 0; idx < threadNum_; ++idx)
	{
		Thread *pthread = new Thread(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1), cache_);
		threadVec_.push_back(pthread);
		pthread->start();
	}	
}


void ThreadPool::stop()
{
	if(isRunning_)
	{
		isRunning_ = false;
		buf_.wakeupall();
		
		std::vector<Thread *>::iterator itv = threadVec_.begin();
		for(; itv != threadVec_.end(); ++itv)
		{
			(*itv) -> join();
			delete (*itv);
		}
	#if 0
		for(int idx = 0; idx < threadNum_; ++idx)
		{
			threadVec_[idx]->join();
			delete threadVec_[idx];
		}
	#endif
		threadVec_.clear();
	}
}


void ThreadPool::addTask(SearchTask *mytask)
{
	buf_.push(mytask);
}

tanfy::SearchTask* ThreadPool::getTask()
{
	return buf_.pop();
}

void ThreadPool::ThreadFunc(Cache &cache)
{
	while(isRunning_)
	{
		if(!cacheUpdating_)
		{
			SearchTask *mytask = getTask();	
			if(NULL != mytask)
			{
				mytask->process(cache);
				delete mytask;
			}
		}
	}
}


void ThreadPool::UpdateCache()
{
	std::cout << "//===========================" << std::endl;
	std::cout << "//0.0 正在更新服务器数据..." << std::endl;
	cacheUpdating_ = true;
	
	if(cacheUpdating_)
	{	
		std::vector<Thread *>::iterator itv = threadVec_.begin();
		for(; itv != threadVec_.end(); ++itv)
		{
			cache_.update((*itv)->cache_);
		}
		cache_.writetoFile();

		itv = threadVec_.begin();
		for(; itv != threadVec_.end(); ++itv)
		{
			(*itv)->cache_.setMap(cache_);
		}
	}
	
	cacheUpdating_ = false;
	std::cout << "//0.0 更新完成" << std::endl;
	std::cout << "//===========================" << std::endl;

}


}//end of namespace
