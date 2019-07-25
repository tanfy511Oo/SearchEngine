#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include "Noncopyable.h"
#include "SearchTask.h"
#include "SocketIO.h"
#include "Socket.h"
#include <functional>
#include <string>



namespace tanfy
{


class TcpConnection:Noncopyable
{
public:
	typedef struct msgbuf
	{
		int len;
		char buf[1024];
	}Msg;

public:
	typedef std::function<void(TcpConnection *)> TcpConnCallback;

	TcpConnection(int peerfd);

	std::string recvmsg();
	void sendmsg(const std::string &sendstr);
	void sendmsg(const char *buf, size_t count);
	void sendmsg(const Msg &sendstruct);
	void shutdown();		
	std::string toString();	

	void setConnectionCb(TcpConnCallback connectionCb);
	void setMessageCb(TcpConnCallback messageCb);
	void setCloseCb(TcpConnCallback closeCb);
	void handleConnection();
	void handleMessage();
	void handleClose();

private:
	Socket peerfd_;
	SocketIO socketIO_;
	TcpConnCallback connectionCb_;
	TcpConnCallback messageCb_;
	TcpConnCallback closeCb_;
	
};


}//end of namespace


#endif
