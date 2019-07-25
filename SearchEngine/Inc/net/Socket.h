#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "Noncopyable.h"


namespace tanfy
{

class InetAddress;

class Socket:Noncopyable
{
public:
	Socket();
	Socket(int sockfd);
	~Socket();

	void ready(InetAddress &addr, int clinum);	
	int accept();
	InetAddress getLocalAddr();
	InetAddress getPeerAddr();
	void shutdownWrite();
	
	int getfd();

private:
	void bindAddress(InetAddress &addr);
	void listen(int clinum);
	void setReuseAddr(bool flag);
	void setReusePort(bool flag);

private:
	int sockfd_;
};


}//end of namespace





#endif
