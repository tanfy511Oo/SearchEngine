#include "../Inc/PreRipePage.h"
#include "../Inc/Conf.h"

int main()
{
	tanfy::Conf myconf("/home/fiona/PROJECT@WANGDAO/c++/SearchEngine/Conf/server.conf");
	tanfy::PreRipePage myripe(myconf);
	myripe.createLibs();
}
