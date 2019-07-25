#ifndef __SEARCHTASK_H__
#define __SEARCHTASK_H__

#include "../Lib/cppjieba/src/Jieba.hpp"
#include <vector>
#include <string>
#include <map>
#include <set>
#include "Page.h"



namespace tanfy
{

typedef struct page_inf
{
	int docid;
	std::string docurl;
	std::string doctitle;
	std::string abstract;
	double cosval;
}PageInf;

class TcpConnection;
class Cache;

class SearchTask
{
public:
	typedef std::map<std::string, std::set<std::pair<int, double> > > IndexMap;
	typedef std::map<int, std::set<std::pair<std::string, double> > > PageMap;
	typedef std::map<int, std::pair<int,int> > OffsetMap;

	SearchTask(TcpConnection *pTcpConn, char *pagestart, IndexMap &indexMap, OffsetMap &offsetMap, cppjieba::Jieba &mycut);
	
	void setKeyword(std::string &keyword);
	std::string getResult();
	void process(Cache &cache);	

public:
	const int ABSTRACTNUM = 5;

private:
	void calKeyword(std::vector<std::string> &words, IndexMap &keywordMap, std::map<std::string, double> &tfidf);
	int getTargetPage(IndexMap &keywordMap, std::map<std::string, double> &keywordTfidf, PageMap &keywordinPage);
	void sortPage(PageMap &keywordinPage);
	void getPageInf(int docid, std::string &docurl, std::string &doctitle, std::string &abstract);
	std::string getPageAbstract(std::string &content);
	std::string writeJsonData();

private:
	char *pagestart_;
	IndexMap &indexMap_;
	OffsetMap &offsetMap_;
	cppjieba::Jieba &mycut_;
	TcpConnection *pTcpConn_;
	std::string keyword_;
	std::string sendResult_;
	std::priority_queue<PageInf> sendQue_;
};


}//end of namespace



#endif
