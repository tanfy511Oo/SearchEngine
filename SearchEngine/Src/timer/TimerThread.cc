#include "TimerThread.h"
#include <functional>

namespace tanfy
{

TimerThread::TimerThread(TimerThrCallback cb, int val, int interval)
	:cb_(cb),
	 timer_(val, interval, cb_),
	 thread_(std::bind(&Timer::start, &timer_)),
	 isRunning_(false)
{}

TimerThread::~TimerThread()
{
	if(isRunning_)
		stop();
}

void TimerThread::start()
{
	isRunning_ = true;
	thread_.start();
}

void TimerThread::stop()
{
	if(isRunning_)
	{
		isRunning_ = false;
		timer_.stop();
		thread_.join();
	}
}

}//end of namespace
