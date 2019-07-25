#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__


#include "Socket.h"
#include "EpollPoller.h"
#include "InetAddress.h"


namespace tanfy
{

class TcpConnection;
class SearchTask;

class TcpServer
{
public:
	typedef std::function<void(TcpConnection *)> TcpSerCallback;

	TcpServer(unsigned short int port, int clinum);
	TcpServer(const char *pIp, unsigned short int port, int clinum);
	~TcpServer();

	void start();
	void stop();

	void setConnectionCb(TcpSerCallback connectionCb);
	void setMessageCb(TcpSerCallback messageCb);
	void setCloseCb(TcpSerCallback closeCb);

private:
	Socket sockfd_;
	EpollPoller epfd_;
	InetAddress addr_;
};


}//end of namespace




#endif
