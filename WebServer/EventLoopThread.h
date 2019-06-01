#pragma once
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"
#include "EventLoop.h"

// worker thread pool which manage work thread create,join and cancel.
class EventLoopThread : noncopyable
{
public:
	EventLoopThread();
	~EventLoopThread();
	EventLoop* startLoop();

private:
	void threadFunc();
	EventLoop *loop_;
	bool exiting_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
};