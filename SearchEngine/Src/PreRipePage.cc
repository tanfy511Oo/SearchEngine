#include "../Lib/cppjieba/src/Jieba.hpp"
#include "PreRipePage.h"
#include "Conf.h"
#include "Page.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>


namespace tanfy
{


void getSubrawPath(const char *dirPath, std::vector<std::string> &rawfilePath)
{
	//0. 打开目录	
	DIR *pDir = ::opendir(dirPath);
	if(NULL == pDir)
	{
		perror("opendir error");
		exit(EXIT_FAILURE);
	}

	//1.遍历目录
	struct dirent *pDirInfo;
	while((pDirInfo = readdir(pDir))!=NULL)
	{
		if(pDirInfo->d_name[0] == '.')
				continue;	
		
		struct stat filestat;
		char path[1024] = {0};
		sprintf(path, "%s/%s", dirPath, pDirInfo->d_name);
		
		stat(path, &filestat);
		if(S_ISDIR(filestat.st_mode))//如果是个目录，继续遍历
		{
			getSubrawPath(path, rawfilePath);			
		}	
		else if(S_ISREG(filestat.st_mode))//如果为普通文件,将文件名称放入容器中
		{
			rawfilePath.push_back(std::string(path));		
		}	
	} 
  
}


void LoadStopWordDict(const std::string &filePath, std::unordered_set<std::string> &stopWords)
{
	std::ifstream ifs(filePath.c_str());
	if(!ifs.is_open())
	{
		perror("stopwordsdict open error");
		exit(EXIT_FAILURE);
	}
	std::string linestr;
	while(getline(ifs, linestr))
	{
		stopWords.insert(linestr);
	}
	assert(stopWords.size());
}


//=================================================
//class PreRipePage
//=================================================
PreRipePage::PreRipePage(Conf &conf, std::vector<Page> &pageVec, cppjieba::Jieba &mycut)
	:rawPath_(conf.getRawPath().c_str()),
	 pagePath_(conf.getPagePath().c_str()),
	 offsetPath_(conf.getOffsetPath().c_str()),
	 pageVec_(pageVec),
	 mycut_(mycut)
{
	tanfy::getSubrawPath(rawPath_, rawfilePath_);

	std::string stopPath = "./Lib/cppjieba/dict/stop_words.utf8";
	LoadStopWordDict(stopPath, stopWords_);
//	std::cout << "PreRipePage()" << std::endl;//test
}


void PreRipePage::createLibs()
{
	//0. 创建ripepage.lib和offset.lib文件
	FILE *fpage = fopen(pagePath_, "w+");
	if(NULL == fpage)
	{
		perror("fopen ripepage.lib error");
		exit(EXIT_FAILURE);
	}
	std::ofstream offset(offsetPath_);
	if(!offset.good())			
	{
		std::cout << "open offset.lib error" << std::endl;
		exit(EXIT_FAILURE);
	}

	//1. 循环读取文章：判断文章是否重复，不重复则加入lib中
	int docid = 1;
	std::vector<std::string>::iterator ite = rawfilePath_.begin();
	std::vector<std::set<std::string> > keywordVec;//放置文章关键字
	for(; ite != rawfilePath_.end(); ++ite)
	{
		//1.1 文章去重，同时对不重复的文章提取词频
		std::map<std::string, int> wordfreMap;
		bool repeat = delRepeatPage(*ite, keywordVec, wordfreMap);	
		if(repeat) //如果重复跳过
		{
			//std::cout <<"repeat:"  << *ite << std::endl;//test 
			continue;
		}

		//1.2 创建对象,并加入容器中
		Page newPage;
		newPage.setId(docid);
		newPage.setWordfre(wordfreMap);
		pageVec_.push_back(newPage);

		//1.3 读取文章的内容, 获取文章总词数
		std::string filestr = readPage(*ite, docid);
		
	
		//1.4 将string写入ripepage.lib，并记录offset
		long tellpos = ::ftell(fpage);
		::fwrite(filestr.c_str(), filestr.size(), 1, fpage);

		offset << docid << " => " << tellpos << '\t' << filestr.size() << std::endl;
		++ docid;
	}	
	
	//2. 关闭文件
	fclose(fpage);
	offset.close();
}


std::string PreRipePage::readPage(std::string rawfilePath, int docid)
{
	//0. 获取标题
	std::string title = getTitle(rawfilePath);
	
	//1. 拼接xml格式字符串
	std::stringstream sstr;
	std::string idstr;
	sstr << docid;
	sstr >> idstr;
	std::string xmlpage = "<doc>\n<docid>" + idstr;
	xmlpage += "</docid>\n<docurl>\n";
	xmlpage += rawfilePath;
	xmlpage += "\n</docurl>\n<title>\n";
	xmlpage += title;
	xmlpage += "\n</title>\n<content>\n";
	
	std::ifstream rawfile(rawfilePath.c_str());
	char tempbuf[tanfy::MAXLEN] = {0};
	std::string readstr;
	bool firstflag = true;
	while(getline(rawfile, readstr))
	{
		if(firstflag)
		{
			firstflag = false;
			sprintf(tempbuf, "%s", readstr.c_str());
		}
		else
			sprintf(tempbuf, "%s\n%s", tempbuf, readstr.c_str());
	}
	std::string content(tempbuf);
	rawfile.close();
		
	xmlpage += content;
	xmlpage += "\n</content>\n</doc>\n";
	
	return xmlpage;
}


std::string PreRipePage::getTitle(std::string rawfilePath)
{
	//0. 打开文件
	FILE *fp = fopen(rawfilePath.c_str(), "r");
	if(NULL == fp)
	{
		perror("fopen error");
		exit(EXIT_FAILURE);
	}	
	
	//1. 按行读取，寻找:
	int searchNum = 15; //寻找标题的最大行数
	char *linebuf = NULL;
	size_t len;	
	bool findflag = false; //标志是否找到"标题"
	bool first = true; //标志是否为第一行
	std::string firstline;
	std::string title;

	while(searchNum -- && getline(&linebuf, &len, fp)!=-1)
	{
		//1.1 寻找“标  题”
		if(first) // 获取第一行，以防未找到标题
		{
			linebuf[strlen(linebuf)-1] = '\0';
			firstline = linebuf;
			first = false;
		}
		std::string linestr(linebuf);
		int pos = linestr.find("标  题");
		if(pos != std::string::npos)
		{
			findflag = true;
			linebuf[strlen(linebuf)-1] = '\0';
			if(linestr.find(":") != std::string::npos)
				linebuf += (pos + 10);//为“标题:”
			else
				linebuf += (pos + 12);//为“【标题】”
			title = linebuf;
			break;
		}		
	}
	if(!findflag)
	{
		title = firstline;	
	}

	//2. 关闭文件
	fclose(fp);
	return title;	
}


bool PreRipePage::delRepeatPage(std::string rawfilePath, std::vector<std::set<std::string> > &keywordVec, std::map<std::string, int> &wordfreMap)
{
	//0. 获取文章内容
	std::ifstream rawfile(rawfilePath.c_str());
	if(!rawfile.good())
	{
		std::cout << "open rawfile error";
		exit(EXIT_FAILURE);
	}
	char tempbuf[tanfy::MAXLEN] = {0};
	std::string readstr;
	while(getline(rawfile, readstr))
	{
		sprintf(tempbuf, "%s%s", tempbuf, readstr.c_str());
		tempbuf[strlen(tempbuf)-1] = '\0';
	}
	std::string content(tempbuf);
	rawfile.close();
	
	//1. 分词
	std::vector<std::string> words;
	CutWords(content, words);	
	
	//2. 统计词频，获取前TOPN个高频词汇	
	std::set<std::string> topN;
	GetTopN(words, wordfreMap, topN, tanfy::TOPN);	

	//3. 进行比较，>70%n则判断为相同文章
	std::vector<std::set<std::string> >::iterator itv = keywordVec.begin();
	for(; itv != keywordVec.end(); ++itv)
	{
		if(ComTopN(topN, *itv))
			return true;		
	}			
	keywordVec.push_back(topN);
	
	return false;
}


void PreRipePage::CutWords(std::string &content, std::vector<std::string> &words)
{
	//0. 基本分词
	mycut_.Cut(content, words);
	
	//1. 去停用词
	for(std::vector<std::string>::iterator ite = words.begin(); ite!=words.end();)
	{
		if(stopWords_.end()!=stopWords_.find(*ite))
			words.erase(ite);
		else
			ite++;
	}	
}


void PreRipePage::GetTopN(std::vector<std::string> &words, std::map<std::string, int> &wordfreMap, std::set<std::string> &topN, int n)
{
	//0. 统计频率,并保存结果
	std::map<std::string, int> wordfreMapUse;
	std::vector<std::string>::iterator itword = words.begin();
	for(; itword != words.end(); ++itword)
	{
		std::map<std::string, int>::iterator itm = wordfreMapUse.find(*itword);
		if(itm != wordfreMapUse.end())
			itm->second ++;
		else
			wordfreMapUse.insert(std::make_pair(*itword, 1));
	}
	std::copy(wordfreMapUse.begin(), wordfreMapUse.end(), std::inserter(wordfreMap, wordfreMap.begin()));

	//1. 排序
	int topnum = wordfreMapUse.size() < n ? wordfreMapUse.size():n;

#if 0
	//test
	std::map<std::string, int>::iterator itwordfre = wordfreMap.begin();
	for(; itwordfre != wordfreMap.end(); ++itwordfre)
	{
		std::cout << itwordfre->first << "\t" << itwordfre->second << std::endl;
	}
#endif
	for(int i=0; i < topnum; ++i)
	{
		std::map<std::string, int>::iterator max = wordfreMapUse.begin();
		if(wordfreMapUse.size() > 1)
		{
			std::map<std::string, int>::iterator ite = ++max;
			for(; ite != wordfreMapUse.end(); ++ite)
			{
				if(ite->second > max->second)
					max = ite;
			}
		}
		
		topN.insert(max->first);
		wordfreMapUse.erase(max->first);
	}	
#if 0	
	//test
	std::set<std::string>::iterator its = topN.begin();
	for(; its != topN.end(); ++its)
	{
		std::cout << *its << std::endl;
	}
#endif
}


//70%相同则判断为true
bool PreRipePage::ComTopN(std::set<std::string> &lhs, std::set<std::string> &rhs)
{
	int total = lhs.size();
	int samenum = 0;
	for(std::set<std::string>::iterator itl = lhs.begin(); itl != lhs.end(); ++itl)
	{
		if(samenum >= 0.7*total)
			break;
#if 0
		for(std::set<std::string>::iterator itr = rhs.begin(); itr != rhs.end(); ++itr)
		{
			if(*itr > *itl)
				break;
			if(*itr == *itl)
			{
				samenum ++;
				break;
			}
		}
#endif
		samenum += count(rhs.begin(), rhs.end(), *itl);
	}
	if(samenum >= 0.7*total)
		return true;
	else
		return false;		
}


}//end of namespace
