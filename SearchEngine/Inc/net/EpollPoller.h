#ifndef __EPOLLPOLLER_H__
#define __EPOLLPOLLER_H__

#include "Noncopyable.h"
#include "Socket.h"
#include <functional>
#include <map>


namespace tanfy
{

class TcpConnection;
class SearchTask;


class EpollPoller:Noncopyable
{
public:
	typedef std::function<void(TcpConnection *)> EpollCallback;
	
	EpollPoller(int listenfd);
	~EpollPoller();
	
	void loop();
	void unloop();

	void setConnectionCb(EpollCallback connectionCb);
	void setMessageCb(EpollCallback messageCb);
	void setCloseCb(EpollCallback closeCb);

private:
	void waitEpoll();
	void handleConnection();
	void handleMessage(int peerfd);

private:
	int epfd_;
	bool isLooping_;
	Socket listenfd_;
	EpollCallback connectionCb_;
	EpollCallback messageCb_;
	EpollCallback closeCb_;
	
	typedef std::map<int, TcpConnection *> TcpConnMap;
	TcpConnMap connectionMap_;
};


}//end of namespace





#endif
