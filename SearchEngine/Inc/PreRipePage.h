#ifndef __PRERIPEPAGE_H__
#define __PRERIPEPAGE_H__

#include "../Lib/cppjieba/src/Jieba.hpp"
#include "Page.h"
#include <unordered_set>
#include <vector>
#include <string>
#include <set>


namespace tanfy
{

const int MAXLEN = 110000;
const int TOPN = 20;

class Conf;

class PreRipePage
{
public:
	PreRipePage(Conf &conf, std::vector<Page> &pageVec, cppjieba::Jieba &mycut);

	void createLibs();

private:
	std::string readPage(std::string rawfilePath, int docid);

	std::string getTitle(std::string rawfilePath);
	bool delRepeatPage(std::string rawfilePath,  std::vector<std::set<std::string> >&keywordVec, std::map<std::string, int> &wordfreMap);		
	
	void CutWords(std::string &content, std::vector<std::string> &words);
	void GetTopN(std::vector<std::string> &words, std::map<std::string, int> &wordfreMap, std::set<std::string> &topN, int n);
	bool ComTopN(std::set<std::string> &lhs, std::set<std::string> &rhs);


private:
	std::vector<std::string> rawfilePath_;//包含网页文件目录路径
	const char *pagePath_;
	const char *offsetPath_; 
	const char *rawPath_;
	cppjieba::Jieba &mycut_;
	std::unordered_set<std::string> stopWords_;
	std::vector<Page> &pageVec_;
};


}//end of namespace





#endif
