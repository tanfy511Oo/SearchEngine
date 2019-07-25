#include "../Inc/PreRipePage.h"
#include "../Inc/Conf.h"
#include "../Inc/Page.h"
#include "../Inc/Index.h"
#include <vector>
#include <unistd.h>

int main()
{
	tanfy::Conf myconf("/home/fiona/PROJECT@WANGDAO/c++/SearchEngine/Conf/server.conf");
	std::vector<tanfy::Page> mypage;

	//0. 选择是否更新数据
	std::cout <<"============================" << std::endl;
	std::cout << "0.0 是否需要更新数据?(y/n)" << std::endl;

	char answer;
	int index = 3;
	bool flag = false;
	while(index --)
	{
		std::cin >> answer;
		if(answer != 'y' && answer != 'n')
			std::cout << "请选择y/n：" << std::endl;
		else
		{
			flag = true;
			break;
		}
	}
	if(!flag)
	{
		std::cout << "0.0 输入错误 将退出程序" << std::endl;
		::sleep(1);
		std::cout <<"============================" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		if(answer == 'y')
		{
			std::cout << "正在创建 网页库..." << std::endl;
			tanfy::PreRipePage myripe(myconf, mypage);
			myripe.createLibs();

			std::cout << "正在创建 索引文件..." << std::endl;
			tanfy::Index myindex(myconf, mypage);
			myindex.createIndex();
			std::cout << "0.0 更新完毕 感谢耐心的你" << std::endl;
			std::cout <<"============================" << std::endl;
		}
		else
		{
			std::cout <<"============================" << std::endl;
		}
	}


}
