#ifndef __INETADDRESS_H__
#define __INETADDRESS_H__

#include <string>
#include <netinet/in.h>


namespace tanfy
{


class InetAddress
{
public:
	InetAddress(const struct sockaddr_in &addr);
	InetAddress(unsigned short int port);
	InetAddress(const char *pIp, unsigned short int port);
	
	struct sockaddr_in *getSockAddr();
	std::string getIp();
	unsigned short int getPort();

private:
	struct sockaddr_in addr_;
};


}//end of namespace







#endif
