#include "Timer.h"
#include <sys/timerfd.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>


namespace tanfy
{


int Create_Timerfd()
{
	int timerfd = timerfd_create(CLOCK_REALTIME, 0);
	if(-1 == timerfd)
	{
		perror("timerfd_create error");
		exit(-1);
	}
	return timerfd;
}

void Set_Timerfd(int timerfd, int val, int interval)
{
	struct itimerspec new_value;
	new_value.it_interval.tv_sec = interval;
	new_value.it_interval.tv_nsec = 0;
	new_value.it_value.tv_sec = val;
	new_value.it_value.tv_nsec = 0;
	
	int ret = timerfd_settime(timerfd, 0, &new_value, NULL);
	if(-1 == ret)
	{
		perror("timerfd_settime error");
		exit(-1);
	}
}

void Stop_Timerfd(int timerfd)
{
	Set_Timerfd(timerfd, 0, 0);	
}

void Read_Timerfd(int timerfd)
{
	ssize_t howmany;
	ssize_t ret = read(timerfd, &howmany, sizeof(howmany));
	if(ret != sizeof(howmany))
	{
		std::cout << "Read_Timerfd error" << std::endl;
		exit(-1);
	}
}


//class Timer
Timer::Timer(int val, int interval, TimerCallback cb)
	:timerfd_(Create_Timerfd()),
	 val_(val),
	 interval_(interval),
	 cb_(cb),
	 isRunning_(false)
{}

Timer::~Timer()
{
	if(isRunning_)
		stop();
}

void Timer::start()
{
	isRunning_ = true;
	Set_Timerfd(timerfd_, val_, interval_);
				
	struct pollfd pfd;
	pfd.fd = timerfd_;
	pfd.events = POLLIN;
	
	while(isRunning_)
	{
		int readynum;
		do
		{
			readynum = poll(&pfd, 1, 5000);
		}while(-1 == readynum && errno == EINTR);
		
		if(-1 == readynum)
		{
			perror("poll error");
			exit(-1);	
		}
		else if(readynum > 0)
		{
			if(pfd.revents == POLLIN)
			{
				Read_Timerfd(timerfd_);
				cb_();
			}
		}
	}
}

void Timer::stop()
{
	if(isRunning_)
	{
		isRunning_ = false;
		Stop_Timerfd(timerfd_);
	}
}

}//end of namespace
