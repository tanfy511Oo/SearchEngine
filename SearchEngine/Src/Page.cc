#include "Page.h"
#include <map>
#include <string>
#include <algorithm>


namespace tanfy
{


void Page::setId(int docid)
{
	docid_ = docid;
}


void Page::setWordfre(std::map<std::string, int> wordfre)
{
	std::copy(wordfre.begin(), wordfre.end(), std::inserter(wordfre_, wordfre_.begin()));
}


void Page::setUrl(std::string &docurl)
{
	docurl_ = docurl;
}


void Page::setTitle(std::string &doctitle)
{
	doctitle_ = doctitle;
}


void Page::setContent(std::string &docContent)
{
	docContent_ = docContent;
}


int Page::getId()
{
	return docid_;
}


std::map<std::string, int>& Page::getWordfre()
{
	return wordfre_;
}


std::string Page::getUrl()
{
	return docurl_;
}


std::string Page::getTitle()
{
	return doctitle_;
}


std::string Page::getContent()
{
	return docContent_;
}


}//end of namespace
