#pragma once
#include "base/Thread.h"
#include "Epoll.h"
#include "Channel.h"
#include "base/CurrentThread.h"
#include "Util.h"
#include <vector>
#include <memory>
#include <functional>
#include <assert.h>
#include <iostream>
using namespace std;

class EventLoop
{
public:
	typedef std::function<void()> Functor; // 异步回调函数
	EventLoop();
	~EventLoop();
	void loop();    // 线程的事件循环
	void quit();    // 退出事件循环
	void runInLoop(Functor&& cb);   // 主要是解决非本线程问题，在当前线程则执行回调函数否则queueInLoop
	void queueInLoop(Functor&& cb); // 将回调函数加入到工作队列上面
	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); } // 判断是否在本线程运行
	void assertInLoopThread()
	{
		assert(isInLoopThread());
	}
	void shutdown(shared_ptr<Channel> channel)
	{
		shutDownWR(channel->getFd());
	}
	// epoll关于事件的一些操作
	void removeFromPoller(shared_ptr<Channel> channel)
	{
		poller_->epoll_del(channel);
	}
	void updatePoller(shared_ptr<Channel> channel, int timeout = 0)
	{
		poller_->epoll_mod(channel, timeout);
	}
	void addToPoller(shared_ptr<Channel> channel, int timeout = 0)
	{
		poller_->epoll_add(channel, timeout);
	}

private:
	bool looping_;
	shared_ptr<Epoll> poller_;  // epoll类封装了一系列的功能
	int wakeupFd_;  // 线程间异步唤醒eventfd
	bool quit_;
	bool eventHandling_;
	mutable MutexLock mutex_;
	std::vector<Functor> pendingFunctors_;  // 任务队列
	bool callingPendingFunctors_;   // 互斥访问任务队列的布尔量
	const pid_t threadId_;  // 线程的ID
	shared_ptr<Channel> pwakeupChannel_;    // 用于唤醒工作的事件类

	void wakeup();  // 唤醒函数，发送eventfd唤醒阻塞的任务队列
	void handleRead();  // 处理读
	void doPendingFunctors();   // 任务队列函数的执行
	void handleConn();  // 处理连接
};