#include "EventLoop.h"
#include "Util.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <iostream>
using namespace std;

__thread EventLoop* t_loopInThisThread = 0;

int createEventfd()
{   // 线程的异步唤醒eventfd
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(evtfd < 0)
	{
		//LOG << "Failed in eventfd";
		abort();
	}
	return evtfd;
}

EventLoop::EventLoop()
: looping_(false),
poller_(new Epoll()),
wakeupFd_(createEventfd()),
quit_(false),
eventHandling_(false),
callingPendingFunctors_(false),
threadId_(CurrentThread::tid()),
pwakeupChannel_(new Channel(this, wakeupFd_))
{
	if(t_loopInThisThread)
	{
		//LOG << "Another EventLoop " << " exists in this thread " <<threadId_;
	}
	else
	{
		t_loopInThisThread = this;
	}
	pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
	pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this)); // 注册读处理回调函数
	pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this)); // 注册连接处理函数
	poller_->epoll_add(pwakeupChannel_, 0); // 注册I/O函数
}

void EventLoop::handleConn()    // 有点玄学
{
	updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop()
{
	close(wakeupFd_);
	t_loopInThisThread = NULL;
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = write(wakeupFd_, (char*)(&one), sizeof one); // write 唤醒任务队列
	if(n != sizeof one)
	{
		//LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
	}
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = readn(wakeupFd_, &one, sizeof one);
	if(n != sizeof one)
	{
		//LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
	}
	pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);  // 设置EPOLLIN 和 EPOLLET
}

// 处理是否在本线程处理
void EventLoop::runInLoop(Functor&& cb)
{
	if(isInLoopThread())
		cb();
	else
		queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb)
{   // 加入到任务队列
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.emplace_back(std::move(cb));
	}
	if(!isInLoopThread() || callingPendingFunctors_)
		wakeup();
}

void EventLoop::loop()
{   // 线程的事件循环函数
	assert(!looping_);
	assert(isInLoopThread());
	looping_ = true;
	quit_ = false;
	std::vector<SP_Channel> ret;
	while(!quit_)
	{
		ret.clear();
		ret = poller_->poll();  // epoll_wait检测事件描述符
		eventHandling_ = true;
		for(auto &it : ret)
			it->handleEvents(); // 根据注册的回调函数处理相应的动作
		eventHandling_ = false;
		doPendingFunctors();    // 处理任务队列中封装的回调函数任务
		poller_->handleExpired();   // 定时器处理超时的请求
	}
	looping_ = false;
}

void EventLoop::doPendingFunctors()
{   // 任务队列的相应的处理流程
	std::vector<Functor> functors;  // 局部缓冲区，避免任务队列调用的死循环
	callingPendingFunctors_ = true;
	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}

	for(size_t i=0; i<functors.size(); ++i)
		functors[i]();  // 通过注册的回调函数完成相应的功能
	callingPendingFunctors_ = false;
}

void EventLoop::quit()
{   // 事件循环的退出
	quit_ = true;
	if(!isInLoopThread())
	{
		wakeup();
	}
}