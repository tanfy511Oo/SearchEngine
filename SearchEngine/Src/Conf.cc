#include "Conf.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <string>
#include <set>
#include <map>


namespace tanfy
{

extern FILE *openFileRead(const char *confpath)
{
	FILE *fp = fopen(confpath, "r");
	if(NULL == fp)
	{
		perror("fopen error");
		exit(EXIT_FAILURE);
	}	
	return fp;
}

void extractDataToMap(const char *buf, tanfy::Conf::ConfMap &confMap)
{
	char key[27] = {0};
	char word[1024] = {0};
	//0. 提取数据
	int idx;
	int keyidx = 0;
	int wordidx = 0;
	bool ifkey = true;
		
	for(idx = 0; idx < strlen(buf); ++idx)
	{
		if(ifkey) //key
		{
			if('=' == buf[idx])
			{
				ifkey = false;
				key[keyidx] = '\0';
				continue;			
			}
			key[keyidx++] = buf[idx];	
		}	
		else //word
		{
			word[wordidx++] = buf[idx];				
		}
	}
	word[wordidx-1] = '\0';//可能有问题：不知道getline是否会读取\n
	
//	std::cout <<"key = " << key << std::endl; //test
//	std::cout << "word = " << word << std::endl; //test

	//1. 将key-word放入map中
	std::pair<tanfy::Conf::ConfMap::iterator, bool> ret = confMap.insert(std::make_pair(std::string(key), std::string(word)));
	if(!ret.second)
	{
		std::cout << "confMap insert error" << std::endl;;
		exit(EXIT_FAILURE);
	}	
}


void readConf(const char *confpath, tanfy::Conf::ConfMap &confMap)
{
	//0. 打开配置文件
	FILE *fp = tanfy::openFileRead(confpath);
	
	//1. 从配置文件中读取信息至map中
	char *linebuf = NULL;
	size_t len = 0;
	
	//1.1 读取配置信息
	while(-1 != ::getline(&linebuf, &len, fp))
	{
	//1.2 将配置信息分解，并填入map中
		tanfy::extractDataToMap(linebuf, confMap);		
	}
	//bug:line free
}

//========================================
//class Conf
//========================================
Conf::Conf(const char *confpath)
{
	tanfy::readConf(confpath, confMap_);
}

std::string Conf::searchMap(const std::string &key)
{
	ConfMap::iterator ite = confMap_.find(key);
	if(ite != confMap_.end())
		return (ite->second);
	return std::string();
}

std::string Conf::getIp()
{
	std::string ip = searchMap(std::string("ip"));
	return ip;		
}

unsigned short Conf::getPort()
{
	unsigned short uport;
	std::string port = searchMap(std::string("port"));
	
	std::stringstream ss_stream;
	ss_stream << port;
	ss_stream >> uport;
	return uport;
}

std::string Conf::getRawPath()
{
	std::string rawpath = searchMap(std::string("raw"));
	return rawpath;
}

std::string Conf::getPagePath()
{
	std::string ripepath = searchMap(std::string("ripepage"));
	return ripepath;
}

std::string Conf::getOffsetPath()
{
	std::string offsetpath = searchMap(std::string("offset"));
	return offsetpath;
}

std::string Conf::getIndexPath()
{
	std::string indexpath = searchMap(std::string("index"));
	return indexpath;
}


std::string Conf::getCachePath()
{
	std::string cachepath = searchMap(std::string("cache"));
	return cachepath;
}


std::map<int, std::pair<int, int> > Conf::createOffset()
{
	//0. 准备
	std::map<int, std::pair<int, int> > offset;
	const char *offsetpath = getOffsetPath().c_str();
	//1. 打开文件
	FILE *fp = fopen(offsetpath, "r");
	if(NULL == fp)
	{
		perror("fopen offset.lib error");
		exit(EXIT_FAILURE);
	}
	//2. 提取
	char *linebuf = NULL;
	size_t len = 0;
	std::string discard;
	while(-1 != ::getline(&linebuf, &len, fp))
	{
		std::istringstream linesstr(linebuf);
		std::pair<int, std::pair<int, int> > tmppair;
		linesstr >> tmppair.first;
		linesstr >> discard;
		linesstr >> tmppair.second.first;
		linesstr >> tmppair.second.second;

		std::pair<std::map<int, std::pair<int,int> >::iterator, bool> ret;
		ret = offset.insert(tmppair);
		if(!ret.second)
		{
			std::cout << "insert error" << std::endl;
		}
	}
	//3. 关闭文件
	fclose(fp);	
	
	return offset;
}


Conf::IndexMap Conf::createIndexMap()
{
	//0. 准备
	const char *indexpath = getIndexPath().c_str();
	Conf::IndexMap index;
	
	//1. 打开文件
	FILE *fp = fopen(indexpath, "r");
	if(NULL == fp)
	{
		perror("fopen index.lib error");
		exit(EXIT_FAILURE);
	}

	//2. 提取
	char *linebuf = NULL;
	size_t len = 0;
	std::string discard;
	
	while(-1 != ::getline(&linebuf, &len, fp))
	{
		std::istringstream linesstr(linebuf);
		std::pair<std::string, std::set<std::pair<int, double> > > tmppair;
	
		linesstr >> tmppair.first;
		linesstr >> discard;
	
		int docid;
		double tfidf;
		while(linesstr >> docid)
		{
			linesstr >> tfidf;
			std::pair<std::set<std::pair<int, double> >::iterator, bool> ret;
			ret = (tmppair.second).insert(std::make_pair(docid, tfidf));
			if(!ret.second)
			{
				std::cout << "insert set error" << std::endl;
			}
		}	
		std::pair<Conf::IndexMap::iterator, bool> retm;
		retm = index.insert(tmppair);
		if(!retm.second)
		{
			std::cout << "insert map error" << std::endl;
		}
	}
	//3. 关闭文件
	fclose(fp);

	return index;
}


}//end of namespace
