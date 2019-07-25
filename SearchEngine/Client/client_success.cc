//使用new的方法生成读取的字符串长度

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <json/json.h>
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
void ReadJsonFromStr(char *str, int count);

int main()
{
	int peerfd = ::socket(AF_INET, SOCK_STREAM, 0);	
	if(-1 == peerfd)
		ERR_EXIT("socket error");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6666);
	addr.sin_addr.s_addr = inet_addr("192.168.200.128");

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
	//0. 接收Json数据
	int readlen;
	::recv(peerfd, &readlen, sizeof(int), MSG_WAITALL);
	//std::cout << "readlen = " << readlen << std::endl;//test	

	char *recvbuf = new char[readlen+1];
	::recv(peerfd, recvbuf, readlen, MSG_WAITALL);
	recvbuf[readlen] = '\0';
	//std::cout << recvbuf << std::endl;//test
	
	//1. 解析Json字符串
	ReadJsonFromStr(recvbuf, strlen(recvbuf));
}


void ReadJsonFromStr(char *str, int count)
{
	Json::Reader reader;
	Json::Value root;
	if(reader.parse(str, root))
	{
		int pagenum = root["pages"].size();
		if(0 == pagenum)
		{
			std::cout << "未查询到相关内容" << std::endl;
			std::cout << std::endl;
			return;
		}

		std::cout << "共查询到" << pagenum << "篇相关内容" << std::endl;
		for(int idx = 0; idx < pagenum; ++idx)
		{
			std::cout << std::endl;
			std::string url = root["pages"][idx]["url"].asString();
			std::string title = root["pages"][idx]["title"].asString();
			std::string abstract = root["pages"][idx]["abstract"].asString();
			
			std::cout << "标题：" << title << std::endl;
			std::cout << "地址：" << url << std::endl;
			std::cout << "摘要：" << std::endl << abstract << std::endl;
		}
	}
}
