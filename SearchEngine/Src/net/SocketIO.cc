#include "SocketIO.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


namespace tanfy
{


SocketIO::SocketIO(int peerfd)
	:peerfd_(peerfd)
{}


ssize_t SocketIO::readn(char *buf, size_t count)
{
	memset(buf, 0, count);

	size_t leftnum = count;
	while(leftnum > 0)
	{
		ssize_t readnum = ::read(peerfd_, buf, leftnum);
		if(-1 == readnum)
		{
			if(errno == EINTR)
				continue;
			else
			{
				perror("read error");
				break;	
			}
		}
		else if(0 == readnum)
		{
			std::cout << "there is nothing to read" << std::endl;
			break;
		}
		else
		{
			leftnum -= readnum;
			buf += readnum;
		}
	}
	*buf = '\0';
	return (count - leftnum);
}


ssize_t SocketIO::writen(const char *buf, size_t count)
{
	size_t leftnum = count;
		
	while(leftnum > 0)
	{
		ssize_t writenum = ::write(peerfd_, buf, leftnum);
		if(-1 == writenum)
		{
			if(errno == EINTR)
				continue;
			else
			{
				perror("write error");
				break;
			}
		}
		else if(0 == writenum)
		{
			std::cout << "there is nothing to write" << std::endl;
			break;
		}
		else
		{
			leftnum -= writenum;
			buf += writenum;
		}
	}
	return (count - leftnum);
}


ssize_t SocketIO::readpeek(char *buf, size_t count)
{
	ssize_t recvnum;
	do
	{
		recvnum = ::recv(peerfd_, buf, count, MSG_PEEK);
	}while(-1 == recvnum && errno == EINTR);
	return recvnum;	
}


ssize_t SocketIO::readline(char *buf, size_t maxcount)
{
	memset(buf, 0, maxcount);
	size_t leftnum = maxcount - 1;
	char *pbuf = buf;

	while(leftnum > 0)
	{
		ssize_t readnum = readpeek(pbuf, leftnum);
		if(readnum < 0)
		{
			std::cout << "read error" << std::endl;
			break;
		}
		else if(0 == readnum)
		{
			break;
		}
		//寻找'\n'
		for(int idx = 0; idx < readnum; ++idx)
		{
			if(pbuf[idx] == '\n')
			{
				int lastlen = idx + 1; 
				ssize_t neednum = readn(pbuf, lastlen);
				if(neednum != lastlen)
				{
					leftnum -= neednum;
					pbuf += neednum;
					*pbuf = '\0';
					return (maxcount - leftnum - 1);
				}
				leftnum -= lastlen;
				pbuf[lastlen-1] = '\0';//不包括'\n'
				return (maxcount - leftnum - 1);
			}
		}
		
		ssize_t neednum = readn(pbuf, readnum);
		if(neednum != readnum)
		{
			leftnum -= neednum;
			pbuf += neednum;
			*pbuf = '\0';
			return (maxcount - leftnum - 1);
		}
		leftnum -= readnum;
		pbuf += readnum;
	}
	*pbuf = '\0';
	return (maxcount - leftnum - 1);		
}




}//end of namespace
