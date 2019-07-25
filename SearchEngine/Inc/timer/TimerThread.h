#ifndef __TIMERTHREAD_H__
#define __TIMERTHREAD_H__

#include "ChildThread.h"
#include "Timer.h"
#include <functional>

namespace tanfy
{

class TimerThread
{
public:
	typedef std::function<void()> TimerThrCallback;
	TimerThread(TimerThrCallback cb, int val, int interval);
	~TimerThread();

	void start();
	void stop();
	
private:
	TimerThrCallback cb_;
	ChildThread thread_;
	Timer timer_;
	bool isRunning_;
};

}//end of namespace


#endif
