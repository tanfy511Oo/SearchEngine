#ifndef __SOCKETIO_H__
#define __SOCKETIO_H__

#include "Noncopyable.h"
#include <stdlib.h>


namespace tanfy
{


class SocketIO:Noncopyable
{
public:
	SocketIO(int peerfd);
	
	ssize_t readn(char *buf, size_t count);
	ssize_t writen(const char *buf, size_t count);
	ssize_t readline(char *buf, size_t maxcount);
	
private:
	ssize_t readpeek(char *buf, size_t count);

private:
	int peerfd_;
};


}//end of namespace




#endif
