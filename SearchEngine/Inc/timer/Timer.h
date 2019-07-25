#ifndef __TIMER_H__
#define __TIMER_H__

#include <functional>

namespace tanfy
{

class Timer
{
public:
	typedef std::function<void()> TimerCallback;
	Timer(int val, int interval, TimerCallback cb);
	~Timer();

	void start();
	void stop();

private:
	int timerfd_;
	int val_;
	int interval_;
	TimerCallback cb_;
	bool isRunning_;
};

}//end of namespace
#endif
