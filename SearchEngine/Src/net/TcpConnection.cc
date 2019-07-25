#include "TcpConnection.h"
#include "InetAddress.h"
#include "SocketIO.h"
#include "Socket.h"
#include <iostream>
#include <unistd.h>

namespace tanfy
{


TcpConnection::TcpConnection(int peerfd)
	:peerfd_(peerfd),
	 socketIO_(peerfd)
{}


std::string TcpConnection::recvmsg()
{
	char buf[1024] = {0};
	ssize_t readnum = socketIO_.readline(buf, sizeof(buf));
	if(readnum <= 0)
	{
		//std::cout << "recvmsg error" << std::endl;
		return std::string();
	}
	return std::string(buf);
}


void TcpConnection::sendmsg(const std::string &sendstr)
{
	ssize_t writenum = socketIO_.writen(sendstr.c_str(), sendstr.size());
	if(writenum != sendstr.size())
	{
		std::cout << "sendmsg error" << std::endl;
	}	
}


void TcpConnection::sendmsg(const char *buf, size_t count)
{
	ssize_t writenum = socketIO_.writen(buf, count);
	if(writenum != count)
	{
		std::cout << "sendmsg error" << std::endl;
	}	
	
}

void TcpConnection::sendmsg(const Msg &sendstruct)
{
	ssize_t writenum = ::write(peerfd_.getfd(), &sendstruct.len, sizeof(int));
	if(writenum != sizeof(int))
	{
		std::cout << "sendmsg error" << std::endl;
	}
	writenum = socketIO_.writen(sendstruct.buf, sendstruct.len);
	if(writenum != sendstruct.len)
	{
		std::cout << "sendmsg error" << std::endl;
	}	
}


void TcpConnection::shutdown()
{
	peerfd_.shutdownWrite();
}


std::string TcpConnection::toString()
{
	InetAddress localaddr = peerfd_.getLocalAddr();
	InetAddress peeraddr = peerfd_.getPeerAddr();
	char buf[1024] = {0};
	sprintf(buf, "%s:%u -> %s:%u", localaddr.getIp().c_str(), localaddr.getPort(),\
								   peeraddr.getIp().c_str(), peeraddr.getPort());
	return std::string(buf);	
}


void TcpConnection::setConnectionCb(TcpConnCallback connectionCb)
{
	connectionCb_ = connectionCb;
}


void TcpConnection::setMessageCb(TcpConnCallback messageCb)
{
	messageCb_ = messageCb;
}


void TcpConnection::setCloseCb(TcpConnCallback closeCb)
{
	closeCb_ = closeCb;
}


void TcpConnection::handleConnection()
{
	connectionCb_(this);
}

void TcpConnection::handleMessage()
{
	messageCb_(this);
}


void TcpConnection::handleClose()
{
	closeCb_(this);
}
}//end of namespace
