#ifndef __PAGE_H__
#define __PAGE_H__

#include <map>
#include <string>


namespace tanfy
{


class Page
{
public:
	void setId(int docid);
	void setWordfre(std::map<std::string, int> wordfre);
	void setUrl(std::string &docurl);
	void setTitle(std::string &doctitle);
	void setContent(std::string &docContent);

	int getId();
	std::map<std::string, int>& getWordfre();
	std::string getUrl();
	std::string getTitle();
	std::string getContent();

private:
	int docid_;
	std::map<std::string, int> wordfre_;
	std::string docurl_;
	std::string doctitle_;
	std::string docContent_;
};


}//end of namespace


#endif
