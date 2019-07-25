//循环读取服务器内容

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#define ERR_EXIT(m) \
		do\
		{\
			perror(m);\
			exit(EXIT_FAILURE);\
		}while(0)


typedef struct msgbuf
{
	int len;
	std::string msgstr;
}Msg;


void InteractWithServer(int peerfd);
void ReadServer(int peerfd);

int main()
{
	int peerfd = ::socket(AF_INET, SOCK_STREAM, 0);	
	if(-1 == peerfd)
		ERR_EXIT("socket error");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6666);
	addr.sin_addr.s_addr = inet_addr("192.168.200.5");

	int ret = ::connect(peerfd, (struct sockaddr *)&addr, sizeof(addr));
	if(-1 == ret)
		ERR_EXIT("connect error");

	//接收服务器欢迎信息
	char welcome[1024];
	memset(welcome, 0, sizeof(welcome));
	::read(peerfd, welcome, sizeof(welcome));
	std::cout << welcome << std::endl;

	//与服务器交互		
	InteractWithServer(peerfd);
}


void InteractWithServer(int peerfd)
{
	char sendbuf[1024];
	
	while(1)
	{
		::fflush(stdin);
		memset(sendbuf, 0, sizeof(sendbuf));
		::fgets(sendbuf, sizeof(sendbuf), stdin);
		if(100 < strlen(sendbuf))
		{
			sendbuf[100] = '\0';
			std::cout << "限制搜索长度为100个字符，后面内容将被忽略" << std::endl;
		}
		::write(peerfd, sendbuf, strlen(sendbuf));

		//接收服务器处理结果
		ReadServer(peerfd);		
	}
}


void ReadServer(int peerfd)
{
	char recvbuf[1024];
	
	while(1)
	{
		memset(recvbuf, 0, sizeof(recvbuf));
		int readlen;
		::recv(peerfd, &readlen, sizeof(int), MSG_WAITALL);
		if(readlen != 0)
		{
			int buflen;
			do
			{
				buflen = ::recv(peerfd, recvbuf, readlen, MSG_WAITALL);
			}while(-1 == buflen && errno == EINTR);

			if(-1 == buflen)
				ERR_EXIT("recv error");		
			else if(0 == buflen)
			{
				std::cout << "server has been closed" << std::endl;
				::close(peerfd);
				exit(EXIT_SUCCESS);
			}
			std::cout << recvbuf;
		}
		else
		{
			std::cout << std::endl;
			break;
		}
	}				
}
