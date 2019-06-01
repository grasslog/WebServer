#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread()
: loop_(NULL),
exiting_(false),
thread_(bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
mutex_(),
cond_(mutex_)
{ }

EventLoopThread::~EventLoopThread()
{ // when the thread exit do loop_ quit and thread_ join.
	exiting_ = true;
	if(loop_ != NULL)
	{
		loop_->quit();
		thread_.join();
	}
}

// That is the ThreadFunc really does the work eventLoop 
EventLoop* EventLoopThread::startLoop()
{
	assert(!thread_.started());
	thread_.start();
	{
		MutexLockGuard lock(mutex_);
		while(loop_ == NULL)	// make sure the callback function has been maked.
			cond_.wait();
	}
	return loop_;
}

// That is the thread which really runs event loop
void EventLoopThread::threadFunc()
{
	EventLoop loop;

	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		cond_.notify();		// as to cond_.wait
	}

	loop.loop();
	loop_ = NULL;
}