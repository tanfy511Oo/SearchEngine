#ifndef __INDEX_H__
#define __INDEX_H__

#include "Page.h"
#include <unordered_map>
#include <vector>
#include <string>


namespace tanfy
{

class Conf;

class Index
{
public:
	Index(Conf &conf, std::vector<Page> &pageVec);
	
	void createIndex();

private:
	void getWordtfToMap();

private:
	const char *indexPath_;
	std::vector<Page> &pageVec_;

	typedef std::unordered_map<std::string, std::vector<std::pair<int, double> > > IndexMap;
	IndexMap indexMap_;
};

}//end of namespace



#endif
