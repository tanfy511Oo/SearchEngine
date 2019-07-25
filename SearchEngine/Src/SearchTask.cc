#include "../Lib/cppjieba/src/Jieba.hpp"
#include "SearchTask.h"
#include "TcpConnection.h"
#include "Cache.h"
#include <json/json.h>
#include <iostream>
#include <vector>
#include <string>
#include <queue>


namespace tanfy
{


bool operator <(const PageInf &lhs, const PageInf &rhs)
{
	if(lhs.cosval < rhs.cosval)
		return true;
	return false;
}


//===============================
//class SearchTask
//===============================
SearchTask::SearchTask(TcpConnection *pTcpConn, char *pagestart, IndexMap &indexMap, OffsetMap &offsetMap, cppjieba::Jieba &mycut)
	:pTcpConn_(pTcpConn),
	 pagestart_(pagestart),
	 indexMap_(indexMap),
	 offsetMap_(offsetMap),
	 mycut_(mycut)
{}


void SearchTask::setKeyword(std::string &keyword)
{
	keyword_ = keyword;
}


std::string SearchTask::getResult()
{
	return sendResult_;
}


void SearchTask::process(Cache &cache)
{
	/*****************************************************/
	//在Cache中查找
	/*****************************************************/
	std::map<std::string, std::string> &resultMap = cache.getMap();
		
	std::map<std::string, std::string>::iterator itm = resultMap.find(keyword_);
	if(itm != resultMap.end())
	{
		sendResult_ = itm->second + '\n';
		
		std::string::size_type len = sendResult_.size();
		pTcpConn_->sendmsg((const char *)&len, (size_t)sizeof(int));
		pTcpConn_->sendmsg(sendResult_);

		return;
	}	



	/*****************************************************/
	//Cache中未找到，需要搜索操作
	/*****************************************************/
	//0. 分词+文本纠错（待优化）
	std::vector<std::string> words;	
	mycut_.Cut(keyword_, words);
#if 1
	//服务器输出待查询的分词结果
	std::vector<std::string>::iterator itv = words.begin();
	for(; itv != words.end(); ++itv)
	{
		std::cout << *itv << '\t';
	}
	std::cout << std::endl;
#endif

	//1.利用倒排索引 提取关键字特征,查找包含该关键字的网页
	IndexMap keywordMap;
	std::map<std::string, double> keywordTfidf;
	PageMap keywordinPage;

	calKeyword(words, keywordMap, keywordTfidf);
	int pagenum = getTargetPage(keywordMap, keywordTfidf, keywordinPage);

#if 0
	//test
	PageMap::iterator itpage = keywordinPage.begin();
	for(; itpage != keywordinPage.end(); ++itpage)
	{
		std::ostringstream sstr;
		sstr << itpage->first;
		sendResult_ += sstr.str();
		sendResult_ += '\t';
	}	
	sendResult_ += '\n';
#endif

	//2. 若网页num > 1，对网页进行排序,确定将发送的队列
	if(pagenum > 1)
	{
		sortPage(keywordinPage);
	}
	else
	{
		PageMap::iterator itpage = keywordinPage.begin();
		for(++itpage; itpage != keywordinPage.end(); ++itpage)
		{
			PageInf pagetoque;
			pagetoque.docid = itpage->first;
			pagetoque.cosval = 1;
			getPageInf(pagetoque.docid, pagetoque.docurl, pagetoque.doctitle, pagetoque.abstract);
			sendQue_.push(pagetoque);
		}
	}

#if 0
	//test
	std::cout << "pagenum = " << pagenum << std::endl;
	while(!sendQue_.empty())
	{
		std::cout << "docid = "<<sendQue_.top().docid << "   ";
		std::cout << "cos = "<< sendQue_.top().cosval << "	 ";
		std::cout << "doctitle = " << sendQue_.top().doctitle << "   ";
		std::cout << "abstract = " << sendQue_.top().abstract << std::endl;
		sendQue_.pop();
#if 0
		std::ostringstream sstr;
		sstr << sendQue_.top().docid;
		sendResult_ += sstr.str();
		sendResult_ += '\t';
		sstr.clear();
		sstr << sendQue_.top().cosval;
		sendResult_ += sstr.str();
		sendResult_ += '\t';
#endif
	}	
	sendResult_ += "处理完毕";
	sendResult_ += '\n';

#endif

	//3. 将结果包装成json字符串并返回给客户端
	sendResult_ = writeJsonData();
	cache.addDatatoMap(keyword_, sendResult_);
	
	std::string::size_type len = sendResult_.size();
	pTcpConn_->sendmsg((const char *)&len, (size_t)sizeof(int));
	pTcpConn_->sendmsg(sendResult_);
	
}


void SearchTask::calKeyword(std::vector<std::string> &words, IndexMap &keywordMap, std::map<std::string, double> &tfidf)
{
	//0. 从words中获取每个关键字出现的频率：map<string, int>
	std::map<std::string, int> wordfre;
	for(std::vector<std::string>::iterator itv = words.begin(); itv != words.end(); ++itv)
	{
		std::map<std::string, int>::iterator itm = wordfre.find(*itv);
		if(itm != wordfre.end())
		{
			itm->second ++;
		}	
		else
		{
			wordfre.insert(std::make_pair(*itv, 1));
		}
	}	
	//1. 计算关键字的idf
//	std::map<std::string, double> tfidf;
	int totalfilenum = offsetMap_.size();

//	IndexMap keywordMap;
	for(std::map<std::string, int>::iterator itm = wordfre.begin(); itm != wordfre.end(); ++itm)
	{
		IndexMap::iterator itIndex = indexMap_.find(itm->first);
		if(itIndex != indexMap_.end())//在倒排索引中找到该关键字
		{
			keywordMap.insert(*itIndex);
			tfidf[itm->first] = log10(totalfilenum*1.0 / (itIndex->second.size()+1));		
		}
		else//在倒排索引中未找到该关键字
		{
			tfidf[itm->first] = log10(totalfilenum*1.0);
		}
	}	
	//2. 计算关键字的tf-idf，并作归一化处理
	double powadd = 0; //归一化做准备
	std::map<std::string, double>::iterator itm;
	for(itm = tfidf.begin(); itm != tfidf.end(); ++itm)
	{
		itm->second *= wordfre[itm->first];
		powadd += pow(itm->second,2);
	}
	
	for(itm = tfidf.begin(); itm != tfidf.end(); ++itm)
	{
		itm->second /= sqrt(powadd);
	}	
}



int SearchTask::getTargetPage(IndexMap &keywordMap, std::map<std::string, double> &keywordTfidf, PageMap &keywordinPage)
{
	//0. 从keywordMap中提取出包含所有关键字的网页
	PageMap pagecalMap;
	IndexMap::iterator itm = keywordMap.begin();
	for(; itm != keywordMap.end(); ++itm)
	{
		std::set<std::pair<int, double> >::iterator its = itm->second.begin();
		for(; its != itm->second.end(); ++its)
		{
			PageMap::iterator itpage = pagecalMap.find(its->first);
			if(itpage != pagecalMap.end())
			{
				std::pair<std::set<std::pair<std::string, double> >::iterator, bool> ret;
				ret = (itpage->second).insert(std::make_pair(itm->first, its->second));
				if(!ret.second)
				{
					std::cout << "insert set error" << std::endl;
				}
			}
			else
			{
				std::set<std::pair<std::string, double> > tmpset;
				tmpset.insert(std::make_pair(itm->first, its->second));
				std::pair<PageMap::iterator, bool> ret;
				ret = pagecalMap.insert(std::make_pair(its->first, tmpset));
				if(!ret.second)
				{
					std::cout << "insert map error" << std::endl;
				}
			}
		}
	}	
	
	int keywordNum = keywordTfidf.size();
	PageMap::iterator itpage = pagecalMap.begin();
	for(; itpage != pagecalMap.end(); ++itpage)
	{
		if(itpage->second.size() == keywordNum)
		{
			std::pair<PageMap::iterator , bool> ret;
			ret = keywordinPage.insert(*itpage);	
			if(!ret.second)
			{
				std::cout << "insert to pageMap error" << std::endl;
			}
		}
	}
	
	//1. 将keyword的向量加入keywordinPage中
	std::set<std::pair<std::string, double> > tmpset;
	for(std::map<std::string, double>::iterator itm = keywordTfidf.begin(); itm != keywordTfidf.end(); ++itm)
	{
		tmpset.insert(*itm);
	}
	
	keywordinPage.insert(std::make_pair(0, tmpset));

	return keywordinPage.size()-1;
}


void SearchTask::sortPage(PageMap &keywordinPage)
{
	//0. 计算cos值，值越大，角度越小，越相似
	std::set<std::pair<std::string, double> > recv = keywordinPage[0];
	double recvpowsqrt = 0;
	for(std::set<std::pair<std::string, double> >::iterator its = recv.begin(); its != recv.end(); ++its)
	{
		recvpowsqrt += pow(its->second, 2);
	}
	recvpowsqrt = sqrt(recvpowsqrt);
	int stringNum = recv.size();

	PageMap::iterator itmpage = keywordinPage.begin();
	for(++itmpage; itmpage != keywordinPage.end(); ++itmpage)
	{
		double up = 0;
		double down = 0;
		//up
		std::set<std::pair<std::string, double> >::iterator itspage = itmpage->second.begin();
		std::set<std::pair<std::string, double> >::iterator itsrecv = recv.begin();
		for(; itspage != itmpage->second.end(), itsrecv != recv.end(); ++itspage, ++itsrecv)
		{
			up += (itspage->second * itsrecv->second);
			down += pow(itspage->second, 2);
		}	
		//down
		down = sqrt(down) * recvpowsqrt;
		//cos
		double cos = up/down;

		//1. 将结果加入优先级队列中
		PageInf pagetoque;
		pagetoque.docid = itmpage->first;
		pagetoque.cosval = cos;
		getPageInf(pagetoque.docid, pagetoque.docurl, pagetoque.doctitle, pagetoque.abstract);
		sendQue_.push(pagetoque);
	}	
}


//提取docid篇文章中的标题和摘要
void SearchTask::getPageInf(int docid, std::string &docurl, std::string &doctitle, std::string &abstract)
{
	//0. 获取docid文章的首地址
	off_t offset;
	size_t pagelen;
	OffsetMap::iterator itm = offsetMap_.find(docid);
	if(itm != offsetMap_.end())
	{
		offset = itm->second.first;
		pagelen = itm->second.second;
	}
	else
	{
		std::cout << "docid error" << std::endl;
		return;
	}
	char *filestart = pagestart_ + offset;
	
	//1. 绑定istringstream
	std::string fileinf(filestart, pagelen);
	std::istringstream ssfileInf(fileinf);

	//2. 获取文章信息
	std::string str;
	std::string content;
	bool urlflag = false;
	bool titleflag = false;
	bool contentflag = false;
	while(ssfileInf >> str)
	{
		//url
		if(str == std::string("<docurl>"))
			urlflag = true;
		if(str == std::string("</docurl>"))
			urlflag = false;
		if(urlflag)
		{
			if(str != std::string("<docurl>"))
				docurl += str;
		}
		//title
		if(str == std::string("<title>"))
			titleflag = true;
		if(str == std::string("</title>"))
			titleflag = false;
		if(titleflag)
		{
			if(str != std::string("<title>"))
				doctitle += str;
		}
		//content
		if(str == std::string("<content>"))
			contentflag = true;
		if(str == std::string("</content>"))
			contentflag = false;
		if(contentflag)
		{
			if(str != std::string("<content>"))
				content += str;
		}
	}
	//3. 从文章内容根据关键字提取摘要
	abstract = getPageAbstract(content);
}


std::string SearchTask::getPageAbstract(std::string &content)
{
	std::string abstract;
	//0. 获取关键字，分词
	std::vector<std::string> words;
	mycut_.Cut(keyword_, words);

	//1. 将文章分成句子
	std::vector<std::string> sentences;
	std::string interarr[12] = {"。", ".", "？", "?", "！", "!", "“", "”", "‘", "’", "\'", "\""};	
	//将断句的标点符号全部替换成空格
	std::vector<std::string> inter(interarr, interarr+12);
	for(std::vector<std::string>::iterator itv = inter.begin(); itv != inter.end(); ++itv)
	{
		std::string::size_type pos = 0;
		while((pos = content.find(*itv, pos)) != std::string::npos)
		{
			content.replace(pos, 2, "  ");
			pos += 2;
		}
	}	

	//断句
	std::istringstream sscontent(content);
	std::string tmpstr;
	while(sscontent >> tmpstr)
	{
		sentences.push_back(tmpstr);
	}

	//2. 提取包含关键字最多的句子：multimap<int, string>
	std::multimap<int, std::string, std::greater<int> > keywordInSentences;
	std::vector<std::string>::iterator its = sentences.begin();
	for(; its != sentences.end(); ++its)
	{
		int fre = 0;
		//查找
		for(std::vector<std::string>::iterator itword = words.begin(); itword != words.end(); ++itword)
		{
			std::string::size_type pos = 0;
			while((pos = its->find(*itword, pos)) != std::string::npos)
			{
				fre ++;
				pos ++;
			}
		}
		//统计
		if(0 == fre)
			continue;
		std::multimap<int, std::string>::iterator ret;
		ret = keywordInSentences.insert(std::make_pair(fre, *its));
	}

	//3. 提取topN个作为文章摘要
	int topN = keywordInSentences.size() > ABSTRACTNUM ? ABSTRACTNUM : keywordInSentences.size();
	std::multimap<int, std::string>::iterator itTop = keywordInSentences.begin();
	for(; itTop != keywordInSentences.end(); ++itTop)
	{
		if(topN > 0)
		{
			abstract += itTop->second;
			abstract += '\n';
			topN --;
		}
		else
			break;
	}

	return abstract;
}


//将结果包装成Json字符串
std::string SearchTask::writeJsonData()
{
	Json::Value root;
	
	Json::Value pageArr;//数组：保存所有页的信息
	while(!sendQue_.empty())
	{
		Json::Value newpage;//保存每个网页的信息
		
		newpage["docid"] = sendQue_.top().docid;
		newpage["url"] = sendQue_.top().docurl;
		newpage["title"] = sendQue_.top().doctitle;
		newpage["abstract"] = sendQue_.top().abstract;
		pageArr.append(newpage);
		sendQue_.pop();
	}
	
	root["pages"] = pageArr;
	
	Json::FastWriter writer;
	std::string write = writer.write(root);
	return write;	
}


}//end of namespace
