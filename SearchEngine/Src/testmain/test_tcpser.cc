#include "../Lib/cppjieba/src/Jieba.hpp"
#include "../Inc/TcpConnection.h"
#include "../Inc/EpollPoller.h"
#include "../Inc/InetAddress.h"
#include "../Inc/SearchTask.h"
#include "../Inc/TcpServer.h"
#include "../Inc/Socket.h"
#include "../Inc/Conf.h"
#include <iostream>
#include <unistd.h>
#include <string>


void myConnection(tanfy::TcpConnection *pTcpConn)
{
	std::cout << pTcpConn->toString() << "已连接" << std::endl;
	std::string connmsg = "欢迎使用宇宙无敌超级搜索引擎";	
	pTcpConn->sendmsg(connmsg);
}


void myMessage(tanfy::TcpConnection *pTcpConn)
{
#if 0
	//std::cout << "myMessage()" << std::endl; //test
	std::string recvstr = pTcpConn->recvmsg();
	std::cout << "已接收：" << recvstr << std::endl;
	pTcpConn->sendmsg(recvstr);
#endif
	std::string recvstr = pTcpConn->recvmsg();
	std::cout << "正在处理数据：" << recvstr <<"..." << std::endl;
//	std::cout << "recvstr = " << recvstr << std::endl;//test
	tanfy::SearchTask mytask = pTcpConn->getSearchTask();
	mytask.setKeyword(recvstr);
	mytask.process();
	std::string sendstr = mytask.getResult();
	pTcpConn->sendmsg(sendstr);
}


void myClose(tanfy::TcpConnection *pTcpConn)
{
	std::cout << pTcpConn->toString() << "已断开" << std::endl;
}


int main()
{
	//0. 分词准备
	cppjieba::Jieba mycut("../Lib/cppjieba/dict/jieba.dict.utf8",
						  "../Lib/cppjieba/dict/hmm_model.utf8",
						  "../Lib/cppjieba/dict/user.dict.utf8");
	
	
	std::cout << "========================" << std::endl;
	std::cout << "0.0 服务器准备中..." << std::endl;
 
	tanfy::Conf myconf("/home/fiona/PROJECT@WANGDAO/c++/SearchEngine/Conf/server.conf");
	const char *pIp = myconf.getIp().c_str();
	unsigned short port = myconf.getPort();

	tanfy::SearchTask::IndexMap indexMap = myconf.createIndexMap();
	tanfy::SearchTask::OffsetMap offsetMap = myconf.createOffset();
	tanfy::SearchTask mytask(indexMap, offsetMap, mycut);

	tanfy::TcpServer myser(pIp, port, 10, mytask);
	
	myser.setConnectionCb(myConnection);
	myser.setMessageCb(myMessage);
	myser.setCloseCb(myClose);
	
	std::cout << "0.0 服务器已启动" << std::endl;
	std::cout << "========================" << std::endl;
	myser.start();
}
