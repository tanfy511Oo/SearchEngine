#include "TcpConnection.h"
#include "EpollPoller.h"
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <vector>
#include <map>


namespace tanfy
{

int CreateEpoll()
{
	int epfd = ::epoll_create1(0);
	if(-1 == epfd)
	{		
		perror("create epoll error");
		exit(EXIT_FAILURE);
	}
	return epfd;
}


void AddEpollRead(int epfd, int addfd)
{
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = addfd;

	int ret = ::epoll_ctl(epfd, EPOLL_CTL_ADD, addfd, &ev);
	if(-1 == ret)
	{
		perror("add readfd to epoll error");
		exit(EXIT_FAILURE);
	}
}


void DelEpollRead(int epfd, int delfd)
{
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = delfd;
	
	int ret = ::epoll_ctl(epfd, EPOLL_CTL_DEL, delfd, &ev);
	if(-1 == ret)
	{
		perror("del readfd from epoll error");
		exit(EXIT_FAILURE);
	}
}


ssize_t readpeek(int peerfd)
{
	ssize_t recvnum;
	char buf[1024] = {0};
	do
	{
		recvnum = ::recv(peerfd, buf, sizeof(buf), MSG_PEEK);
	}while(-1 == recvnum && errno == EINTR);
	if(-1 == recvnum)
	{
		perror("recvpeek error");
	}	
	return recvnum;
}


bool ifClose(int peerfd)
{
	ssize_t checknum = tanfy::readpeek(peerfd);
	if(0 == checknum)
		return true;
	else
		return false;	
}




//=====================================
//class EpollPoller
//=====================================
EpollPoller::EpollPoller(int listenfd)
	:listenfd_(listenfd),
	 isLooping_(false),
	 epfd_(tanfy::CreateEpoll())
{
	tanfy::AddEpollRead(epfd_, listenfd_.getfd());
}


EpollPoller::~EpollPoller()
{
	if(isLooping_)
		unloop();
}


void EpollPoller::loop()
{
	isLooping_ = true;
	//std::cout << "start to loop" <<std::endl; //test
	while(isLooping_)
	{
		waitEpoll();
	}
}


void EpollPoller::unloop()
{
	isLooping_ = false;
}


void EpollPoller::waitEpoll()
{
	std::vector<struct epoll_event> epollVec(1024);
	int readynum;
	do
	{
		readynum = ::epoll_wait(epfd_, &(*epollVec.begin()), epollVec.size(), 5000);
	}while(-1 == readynum && errno == EINTR);
	
	//std::cout << "readynum = " << readynum << std::endl; //test

	if(readynum <=0)
		return;	
	else
	{
		if(readynum == epollVec.size())
		{
			epollVec.resize(epollVec.size()*2);
		}

		//std::cout << "readynum = " << readynum << std::endl;//test
		for(int idx = 0; idx < readynum; ++idx)
		{
			if(epollVec[idx].data.fd == listenfd_.getfd() && (epollVec[idx].events & EPOLLIN))
			{
				//std::cout << "handleConnection()" << std::endl;//test	
				handleConnection();
			}	
			else if(epollVec[idx].events & EPOLLIN)
			{
				handleMessage(epollVec[idx].data.fd);
			}
		}
	}
}


void EpollPoller::handleConnection()
{
	//0. 将与客户端连接的fd加入epoll中
	int peerfd = listenfd_.accept();
	tanfy::AddEpollRead(epfd_, peerfd);
	//1. 利用TcpConnection类与客户端交互
	TcpConnection *myconn = new TcpConnection(peerfd);
	std::pair<TcpConnMap::iterator, bool> ret = connectionMap_.insert(std::make_pair(peerfd, myconn));
	if(!ret.second)
	{
		std::cout << "insert error" << std::endl;
	}	

	myconn->setConnectionCb(connectionCb_);
	myconn->setMessageCb(messageCb_);
	myconn->setCloseCb(closeCb_);

	myconn->handleConnection();	
}


void EpollPoller::handleMessage(int peerfd)
{
	//0. 提取该peerfd对应的TcpConnection*
	TcpConnection *myconn = connectionMap_[peerfd];	

	//1. 判断连接是否已断
	bool close = tanfy::ifClose(peerfd);
	if(close)
	{
		myconn->handleClose();

		tanfy::DelEpollRead(epfd_, peerfd);
		delete myconn;
		connectionMap_.erase(peerfd);
	}
	else
	{
		myconn->handleMessage();
	}	
}


void EpollPoller::setConnectionCb(EpollCallback connectionCb)
{
	connectionCb_ = connectionCb;
}


void EpollPoller::setMessageCb(EpollCallback messageCb)
{
	messageCb_ = messageCb;
}


void EpollPoller::setCloseCb(EpollCallback closeCb)
{
	closeCb_ = closeCb;
}


}//end of namespace
