#pragma once
#include "Timer.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <sys/epoll.h>

class EventLoop;	// 核心的事件循环函数，one thread per loop
class HttpData;		// HttpData类封装了http报文的解析以及读写等

// I/O注册函数，I/O多路复用，用来分发主线程的I/O事件；通过异步回调的机制正确的处理I/O事件。
// 设置一个通用的Channel注册相对应的事件处理回调函数，当然Channel也有事件处理方法来统一处理事件
class Channel
{
private:
	typedef std::function<void()> CallBack;	// 异步回调函数
	EventLoop *loop_;	// 关键的事件循环
	int fd_;	// 事件相关的文件描述符
	__uint32_t events_;	// 相关的事件
	__uint32_t revents_;
	__uint32_t lastEvents_;

	std::weak_ptr<HttpData> holder_;	// 虚指针

private:
	int parse_URI();
	int parse_Headers();
	int analysisRequest();

// 各回调函数机制
	CallBack readHandler_;
	CallBack writeHandler_;
	CallBack errorHandler_;
	CallBack connHandler_;

public:
	Channel(EventLoop *loop);
	Channel(EventLoop *loop, int fd);
	~Channel();
	int getFd();
	void setFd(int fd);

	void setHolder(std::shared_ptr<HttpData> holder)
	{
		holder_ = holder;
	}
	std::shared_ptr<HttpData> getHolder()
	{
		std::shared_ptr<HttpData> ret(holder_.lock());
		return ret;
	}

// 设置相应的回调函数
	void setReadHandler(CallBack &&readHandler)
	{
		readHandler_ = readHandler;
	}
	void setWriteHandler(CallBack &&writeHandler)
	{
		writeHandler_ = writeHandler;
	}
	void setErrorHandler(CallBack &&errorHandler)
	{
		errorHandler_ = errorHandler;
	}
	void setConnHandler(CallBack &&connHandler)
	{
		connHandler_ = connHandler;
	}

// Channel的事件处理方法
	void handleEvents()
	{
		events_ = 0;
		if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
		{
			events_ = 0;
			return ;
		}
		if(revents_ & EPOLLERR)
		{
			if(errorHandler_) errorHandler_();
			events_ = 0;
			return;
		}
		if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLHUP))
		{
			handleRead();
		}
		if(revents_ & EPOLLOUT)
		{
			handleWrite();
		}
		handleConn();
	}
	void handleRead();
	void handleWrite();
	void handError(int fd, int err_num, std::string short_msg);
	void handleConn();

	void setRevents(__uint32_t ev)
	{
		revents_ = ev;
	}

	void setEvents(__uint32_t ev)
	{
		events_ = ev;
	}
	__uint32_t& getEvents()
	{
		return events_;
	}

	bool EqualAndUpdateLastEvents()
	{
		bool ret = (lastEvents_ == events_);
		lastEvents_ = events_;
		return ret;
	}

	__uint32_t getLastEvents()
	{
		return lastEvents_;
	}

};

typedef std::shared_ptr<Channel> SP_Channel;