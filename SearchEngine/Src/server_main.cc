#include "head.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <functional>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>


static tanfy::ThreadPool *pthreadpool = NULL;

char *myMmap(tanfy::Conf &conf, size_t &filelen);
void myConnection(tanfy::TcpConnection *pTcpConn);
void myMessage(tanfy::TcpConnection *pTcpConn);
void myClose(tanfy::TcpConnection *pTcpConn);


int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		std::cout << "请输入参数：server.conf的地址" << std::endl;
		exit(-1);
	}
	/***************************************************************/
	//0. 分词/配置文件准备
	/***************************************************************/
	//此处的路径根据server.sh所在的位置而定	
	cppjieba::Jieba mycut("./Lib/cppjieba/dict/jieba.dict.utf8",
						  "./Lib/cppjieba/dict/hmm_model.utf8",
						  "./Lib/cppjieba/dict/user.dict.utf8");

	tanfy::Conf myconf(argv[1]);
//	tanfy::Conf myconf("/home/fiona/PROJECT@BUAA/c++/SearchEngine/Conf/server.conf");
	std::vector<tanfy::Page> mypage;

	/***************************************************************/
	//1. 选择是否更新数据（离线）
	/***************************************************************/
	std::cout << "===========================" << std::endl;
	std::cout << "0.0 是否需要更新数据?(y/n)" << std::endl;
	std::cout << "   （更新时间约20min）" << std::endl;

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
		std::cout << "===========================" << std::endl;
		exit(EXIT_FAILURE);
	}	
	else
	{
		if(answer == 'y')
		{
			std::cout << "正在创建 网页库..." << std::endl;
			tanfy::PreRipePage myripe(myconf, mypage, mycut);
			myripe.createLibs();

			std::cout << "正在创建 索引文件..." << std::endl;
			tanfy::Index myindex(myconf, mypage);
			myindex.createIndex();
			std::cout << "0.0 更新完毕 感谢耐心的你" << std::endl;
			std::cout << "===========================" << std::endl;
		}
		else
		{
			std::cout << "===========================" << std::endl;
		}
	}

	/***************************************************************/
	//2. 服务器与客户端交互（在线）
	/***************************************************************/
	std::cout << "===========================" << std::endl;
	std::cout << "0.0 服务器准备中..." << std::endl;
 
	const char *pIp = myconf.getIp().c_str();
	unsigned short port = myconf.getPort();

	size_t filelen;
	char *pagestart = myMmap(myconf, filelen);	

	tanfy::ThreadPool::IndexMap indexMap = myconf.createIndexMap();
	tanfy::ThreadPool::OffsetMap offsetMap = myconf.createOffset();

	tanfy::Cache mycache(myconf);

	//2.1 线程池
	tanfy::ThreadPool thrpool(10, 10, mycache, pagestart, indexMap, offsetMap, mycut);
	pthreadpool = &thrpool;	

	//2.2 定时器
	tanfy::TimerThread mytimer(std::bind(&tanfy::ThreadPool::UpdateCache, pthreadpool), 60, 120);//第一次60s，后面间隔120s	

	//2.3 服务器-客户端
	tanfy::TcpServer myser(pIp, port, 10);
	
	myser.setConnectionCb(myConnection);
	myser.setMessageCb(myMessage);
	myser.setCloseCb(myClose);
	
	std::cout << "0.0 服务器就绪" << std::endl;
	std::cout << "===========================" << std::endl;

	//2.4 依次启动
	pthreadpool->start();		
	mytimer.start();
	myser.start();
}


char *myMmap(tanfy::Conf &conf, size_t &filelen)
{
	//0.打开文件，获取文件大小
	const char *filepath = conf.getPagePath().c_str();
	int fd = ::open(filepath, O_RDONLY);
	if(-1 == fd)
	{
		perror("open ripepage.lib error");
		exit(EXIT_FAILURE);
	}	

	struct stat statbuf;
	int ret = ::stat(filepath, &statbuf);
	if(-1 == ret)
	{
		perror("stat error");
		exit(EXIT_FAILURE);
	}
	filelen = statbuf.st_size;
	
	//1. 映射
	char *filestart;
	filestart = (char *)mmap(NULL, filelen, PROT_READ, MAP_PRIVATE, fd, 0);
	if(filestart == (char *)-1)
	{
		perror("mmap error");
		exit(EXIT_FAILURE);
	}
	
	::close(fd);
	return filestart;
}


void myConnection(tanfy::TcpConnection *pTcpConn)
{
	std::cout << pTcpConn->toString() << "已连接" << std::endl;
	std::string connmsg = "欢迎使用宇宙无敌超级搜索引擎";	
	pTcpConn->sendmsg(connmsg);
}


void myMessage(tanfy::TcpConnection *pTcpConn)
{
	std::string recvstr = pTcpConn->recvmsg();
	std::cout << "正在处理数据：" << recvstr <<"..." << std::endl;

	tanfy::SearchTask *mytask = new tanfy::SearchTask(pTcpConn, pthreadpool->pagestart_, pthreadpool->indexMap_, pthreadpool->offsetMap_, pthreadpool->mycut_);
	mytask->setKeyword(recvstr);
	//将任务丢到线程池中
	pthreadpool->addTask(mytask);
#if 0
	std::string sendstr = mytask.getResult();

	//处理待传输数据
	sendMsg(pTcpConn, sendstr);
	//std::cout << "处理完毕" << std::endl << std::endl;
#endif
}


void myClose(tanfy::TcpConnection *pTcpConn)
{
	std::cout << pTcpConn->toString() << "已断开" << std::endl;
}

