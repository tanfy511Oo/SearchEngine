#include "InetAddress.h"
#include "Socket.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>


namespace tanfy
{

int CreateSocket()
{
	int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sockfd)
	{
		perror("create socket error");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}


//===============================
//class Socket
//===============================
Socket::Socket()
	:sockfd_(tanfy::CreateSocket())
{}


Socket::Socket(int sockfd)
	:sockfd_(sockfd)
{}


Socket::~Socket()
{
	::close(sockfd_);
}


void Socket::ready(InetAddress &addr, int clinum)
{
	setReuseAddr(true);
	setReusePort(true);
	bindAddress(addr);
	listen(clinum);
}


void Socket::bindAddress(InetAddress &addr)
{
	int ret = ::bind(sockfd_, (struct sockaddr *)addr.getSockAddr(), sizeof(addr));
	if(-1 == ret)
	{
		perror("bind error");
		exit(EXIT_FAILURE);
	}
}


void Socket::listen(int clinum)
{
	int ret = ::listen(sockfd_, clinum);
	if(-1 == ret)
	{
		perror("listen error");
		exit(EXIT_FAILURE);
	}
}


void Socket::setReuseAddr(bool flag)
{
	int optval = flag ? 1:0;
	int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
	if(-1 == ret)
	{
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}
}


void Socket::setReusePort(bool flag)
{
#ifdef SO_REUSEPORT
	int optval = flag ? 1:0;
	int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
	if(-1 == ret)
	{
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}	
#endif
}


int Socket::accept()
{
	int peerfd = ::accept(sockfd_, NULL, NULL);
	if(-1 == peerfd)
	{
		perror("accept error");
		exit(EXIT_FAILURE);
	}
	return peerfd;
}


tanfy::InetAddress Socket::getLocalAddr()
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	int len = sizeof(addr);

	int ret = ::getsockname(sockfd_, (struct sockaddr *)&addr, (socklen_t *)&len);
	if(-1 == ret)
	{
		perror("getsockname error");
		exit(EXIT_FAILURE);
	}	
	return InetAddress(addr);
}


tanfy::InetAddress Socket::getPeerAddr()
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	int len = sizeof(addr);

	int ret = ::getpeername(sockfd_, (struct sockaddr *)&addr, (socklen_t *)&len);
	if(-1 == ret)
	{
		perror("getpeername error");
		exit(EXIT_FAILURE);
	}
	return InetAddress(addr);
}


void Socket::shutdownWrite()
{
	int ret = ::shutdown(sockfd_, SHUT_WR);
	if(-1 == ret)
	{
		perror("shutdown write error");
		exit(EXIT_FAILURE);
	}	
}


int Socket::getfd()
{	
	return sockfd_;
}


}//end of namespace
