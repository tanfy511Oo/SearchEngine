#include "TcpServer.h"


namespace tanfy
{


TcpServer::TcpServer(unsigned short int port, int clinum)
	:sockfd_(),
	 epfd_(sockfd_.getfd()),
	 addr_(port)
{
	sockfd_.ready(addr_, clinum);	
}


TcpServer::TcpServer(const char *pIp, unsigned short int port, int clinum)
	:sockfd_(),
	 epfd_(sockfd_.getfd()),
	 addr_(pIp, port)
{
	sockfd_.ready(addr_, clinum);
}


TcpServer::~TcpServer()
{
	stop();	
}


void TcpServer::start()
{
	epfd_.loop();
}


void TcpServer::stop()
{
	epfd_.unloop();
}


void TcpServer::setConnectionCb(TcpSerCallback connectionCb)
{
	epfd_.setConnectionCb(connectionCb);
}


void TcpServer::setMessageCb(TcpSerCallback messageCb)
{
	epfd_.setMessageCb(messageCb);
}


void TcpServer::setCloseCb(TcpSerCallback closeCb)
{
	epfd_.setCloseCb(closeCb);
}


}//end of namespace
