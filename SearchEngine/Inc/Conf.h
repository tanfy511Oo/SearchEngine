#ifndef __CONF_H__
#define __CONF_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include "Page.h"


#include <string>
#include <map>
#include <set>


namespace tanfy
{


class Conf
{
public:
	typedef std::map<std::string, std::string> ConfMap;	
	typedef std::map<std::string, std::set<std::pair<int, double> > > IndexMap;	

	Conf(const char *confpath);
	
	std::string getIp();
	unsigned short getPort();
	
	std::string getRawPath();
	std::string getPagePath();
	std::string getOffsetPath();
	std::string getIndexPath();
	std::string getCachePath();

	std::map<int, std::pair<int, int> > createOffset();
	IndexMap createIndexMap();	

private:
	std::string searchMap(const std::string &key);

private:
	ConfMap confMap_;	
};


}//end of namespace


#endif
