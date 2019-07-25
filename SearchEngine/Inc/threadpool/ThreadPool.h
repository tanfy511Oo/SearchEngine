#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "../../Lib/cppjieba/src/Jieba.hpp"
#include "Noncopyable.h"
#include "Buffer.h"
#include <pthread.h>
#include <functional>
#include <vector>
#include <map>
#include <set>

namespace tanfy
{

class Cache;
class Thread;
class SearchTask;

class ThreadPool
{
public:
	typedef std::map<std::string, std::set<std::pair<int, double> > > IndexMap;
	typedef std::map<int, std::pair<int, int> > OffsetMap;

	ThreadPool(size_t threadNum, size_t maxsize, Cache &cache, char *pagestart, IndexMap &indexMap, OffsetMap & offsetMap, cppjieba::Jieba &mycut);
	~ThreadPool();
	
	void start();
	void stop();
	void addTask(SearchTask *mytask);

	void UpdateCache();
public:
	char *pagestart_;
	IndexMap &indexMap_;
	OffsetMap &offsetMap_;
	cppjieba::Jieba &mycut_;

private:
	void ThreadFunc(Cache &cache);
	SearchTask *getTask();		

private:
	size_t threadNum_;
	std::vector<Thread *> threadVec_;	
	Buffer buf_;
	Cache &cache_;
	bool isRunning_;
	bool cacheUpdating_;
};

}//end of namespace


#endif
