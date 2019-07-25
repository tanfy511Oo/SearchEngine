#include "InetAddress.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


namespace tanfy
{


InetAddress::InetAddress(const struct sockaddr_in &addr)
	:addr_(addr)
{}


InetAddress::InetAddress(unsigned short int port)
{
	memset(&addr_, 0, sizeof(addr_));

	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(port);
	addr_.sin_addr.s_addr = INADDR_ANY;	
}


InetAddress::InetAddress(const char *pIp, unsigned short int port)
{
	memset(&addr_, 0, sizeof(addr_));

	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(port);
	addr_.sin_addr.s_addr = inet_addr(pIp);
}


struct sockaddr_in *InetAddress::getSockAddr()
{
	return &addr_;
}


std::string InetAddress::getIp()
{
	char *pIp = inet_ntoa(addr_.sin_addr);
	return std::string(pIp);
}


unsigned short int InetAddress::getPort()
{
	unsigned short int port = ntohs(addr_.sin_port);
	return port;
}


}//end of namespace
