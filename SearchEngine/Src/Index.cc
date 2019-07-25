#include "Index.h"
#include "Page.h"
#include "Conf.h"
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <map>


namespace tanfy
{


Index::Index(Conf &conf, std::vector<Page> &pageVec)
	:indexPath_(conf.getIndexPath().c_str()),
	 pageVec_(pageVec)
{}


void Index::createIndex()
{
	//0. 从pageVec中的内容写入unordered_map中,并计算tf值
	getWordtfToMap();
//	exit(EXIT_FAILURE);//test

	//1. 计算idf
	std::unordered_map<std::string, double> idfMap;
	IndexMap::iterator ite = indexMap_.begin();
	int totalfilenum = pageVec_.size();//文档总数

	for(; ite != indexMap_.end(); ++ite)
	{
		idfMap[ite->first] = log10(totalfilenum*1.0 / (ite->second.size()+1));
	}
#if 0	
	//test
	std::unordered_map<std::string, double>::iterator ittest = idfMap.begin();
	for(; ittest != idfMap.end(); ++ittest)
	{
		std::cout << ittest->first << "\t" << ittest->second << std::endl;
	}
	exit(EXIT_FAILURE);
#endif
//	std::cout << "idf size() = " << idfMap.size() << std::endl;//test
//	std::cout << "indexMap size() = " << indexMap_.size() << std::endl; //test
//	exit(EXIT_FAILURE);

	//2. 计算tf-idf；同时将结果放入std::map<int, double>中，为归一化做准备
	std::map<int, double> tfidf;

	for(ite = indexMap_.begin(); ite != indexMap_.end(); ++ite)
	{
		double idf = idfMap[ite->first];
		std::vector<std::pair<int, double> >::iterator its = ite->second.begin();
		for(; its != ite->second.end(); ++its)
		{
			its->second = idf * (its->second);

			std::map<int, double>::iterator itfind = tfidf.find(its->first);
			if(itfind != tfidf.end())
			{
				itfind->second += pow(its->second, 2);
			}			
			else 
				tfidf.insert(std::make_pair(its->first, pow(its->second, 2)));
		}
	}

	//3. 归一化处理
	for(ite = indexMap_.begin(); ite != indexMap_.end(); ++ite)
	{
		std::vector<std::pair<int, double> >::iterator its = ite->second.begin();
		for(; its != ite->second.end(); ++its)
		{
			its->second /= sqrt(tfidf[its->first]);
		}
	}
	
	
#if 0
	//test
	IndexMap::iterator itm = indexMap_.begin();
	std::cout.flags(std::ios::left);
	for(; itm != indexMap_.end(); ++itm)
	{
		std::cout << std::setw(10) <<itm->first << "=";
		std::vector<std::pair<int, double> >::iterator ite = itm->second.begin();
		for(; ite != itm->second.end(); ++ite)
		{
			std::cout << std::setw(8) << ite->first << '\t' << std::setw(8)<<ite->second << '\t';
		}
		std::cout << std::endl;
	}
	exit(EXIT_FAILURE);	
#endif

	//4. 将unordered_map内容写入lib中
	std::ofstream index(indexPath_);
	index.flags(std::ios::left);
	if(!index.good())
	{
		std::cout << "open inverted_index.lib error" << std::endl;
		exit(EXIT_FAILURE);
	}	
	for(ite = indexMap_.begin(); ite != indexMap_.end(); ++ite)
	{	
		index << std::setw(10) << ite->first << "  =  ";
		std::vector<std::pair<int, double> >::iterator its = ite->second.begin();
		for(; its != ite->second.end(); ++its)
		{
			
			index << std::setw(8)<< its->first << '\t' << std::setw(8)<< its->second << '\t';
		}
		index << std::endl;
	}

	index.close();	
}


void Index::getWordtfToMap()
{
	std::vector<Page>::iterator itv = pageVec_.begin();
	for(; itv != pageVec_.end(); ++itv)
	{
		//0. 获取每个Page中的id 和 map<string, int>wordfre
		int docid = itv->getId();
		std::map<std::string, int> wordfreMap = itv->getWordfre();

		//1. 遍历wordfreMap中的内容, 加入hash_map中
		std::map<std::string, int>::iterator itm = wordfreMap.begin();
		for(; itm != wordfreMap.end(); ++itm)
		{
			//std::cout << itm->first << "\t" << itm->second << std::endl;//test
			//在unordered_map中更新word的内容
			indexMap_[itm->first].push_back(std::make_pair(docid, itm->second));
		}	
	}
#if 0	 
	//test
	IndexMap::iterator itm = indexMap_.begin();
	for(; itm != indexMap_.end(); ++itm)
	{
		std::cout << itm->first << "=";
		std::vector<std::pair<int, double> >::iterator ite = itm->second.begin();
		for(; ite != itm->second.end(); ++ite)
		{
			std::cout << ite->first << '\t' << ite->second << '\t';
		}
		std::cout << std::endl;
	}
#endif
}



}//end of namespace
