#include "../Inc/TcpConnection.h"
#include "../Inc/EpollPoller.h"
#include "../Inc/InetAddress.h"
#include "../Inc/Socket.h"
#include "../Inc/Conf.h"
#include <iostream>
#include <string>


void myConnection(tanfy::TcpConnection *pTcpConn)
{
	std::cout << pTcpConn->toString() << "已连接" << std::endl;
	std::string connmsg = "welcome to server";	
	pTcpConn->sendmsg(connmsg);
}


void myMessage(tanfy::TcpConnection *pTcpConn)
{
	std::string recvstr = pTcpConn->recvmsg();
	std::cout << "已接收：" << recvstr << std::endl;
	pTcpConn->sendmsg(recvstr);
}


void myClose(tanfy::TcpConnection *pTcpConn)
{
	std::cout << pTcpConn->toString() << "已断开" << std::endl;
}


int main()
{
	tanfy::Conf myconf("/home/fiona/PROJECT@WANGDAO/c++/SearchEngine/Conf/server.conf");
	const char *pIp = myconf.getIp().c_str();
	unsigned short port = myconf.getPort();

	tanfy::InetAddress myaddr(pIp, port);
	tanfy::Socket mysocket;
	mysocket.ready(myaddr, 10);	
	
	tanfy::EpollPoller myepoll(mysocket.getfd());
	myepoll.setConnectionCb(myConnection);
	myepoll.setMessageCb(myMessage);
	myepoll.setCloseCb(myClose);

	myepoll.loop();
}
