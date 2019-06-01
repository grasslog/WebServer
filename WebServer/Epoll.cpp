#include "Epoll.h"
#include "Util.h"
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <queue>
#include <deque>
#include <assert.h>

#include <arpa/inet.h>
#include <iostream>
using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

typedef shared_ptr<Channel> SP_Channel;

Epoll::Epoll()
: epollFd_(epoll_create1(EPOLL_CLOEXEC)),
events_(EVENTSNUM)
{
	assert(epollFd_ > 0);
}
Epoll::~Epoll()
{ }

// 借助fd2http_[]和fd2chan_[]来记录httpdata和channel

// register fd
void Epoll::epoll_add(SP_Channel request, int timeout)
{
	int fd = request->getFd();
	if(timeout > 0)
	{
		add_timer(request, timeout);
		fd2http_[fd] = request->getHolder();
	}
	struct epoll_event event;
	event.data.fd = fd;
	event.events = request->getEvents();

	request->EqualAndUpdateLastEvents();

	fd2chan_[fd] = request;
	if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		perror("epoll_add error");
		fd2chan_[fd].reset();
	}
}

// mod fd
void Epoll::epoll_mod(SP_Channel request, int timeout)
{
	if(timeout > 0)
		add_timer(request, timeout);
	int fd = request->getFd();
	if(!request->EqualAndUpdateLastEvents())
	{
		struct epoll_event event;
		event.data.fd = fd;
		event.events = request->getEvents();
		if(epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0)
		{
			perror("epoll_mod error");
			fd2chan_[fd].reset();
		}
	}
}

// delete fd
void Epoll::epoll_del(SP_Channel request)
{
	int fd = request->getFd();
	struct epoll_event event;
	event.data.fd = fd;
	event.events = request->getLastEvents();
	if(epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0)
	{
		perror("epoll_del error");
	}
	fd2chan_[fd].reset();
	fd2http_[fd].reset();
}



// return active events
std::vector<SP_Channel> Epoll::poll()
{
	while(true)
	{
		int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
		if(event_count < 0)
			perror("epoll wait error");
		std::vector<SP_Channel> req_data = getEventsRequest(event_count);
		if(req_data.size() > 0)
			return req_data;
	}
}

void Epoll::handleExpired()
{
	timerManager_.handleExpiredEvent();
}

// 根据events_num返回活跃事件数量和缓存的channel和httpdata，最终返回req_data(channel share_ptr)向量集合
std::vector<SP_Channel> Epoll::getEventsRequest(int events_num)
{
	std::vector<SP_Channel> req_data;
	for(int i=0; i<events_num; ++i)
	{
		int fd = events_[i].data.fd;

		SP_Channel cur_req = fd2chan_[fd];

		if(cur_req)
		{
			cur_req->setRevents(events_[i].events); // 根据返回的活跃事件注册添加到结果集合中
			cur_req->setEvents(0);
			req_data.push_back(cur_req);
		}else{
			//LOG << "SP cur_req is invalid";
		}
	}
	return req_data;
}

// 将请求添加到TimerManager_中
void Epoll::add_timer(SP_Channel request_data, int timeout)
{
	shared_ptr<HttpData> t = request_data->getHolder();
	if(t)
		timerManager_.addTimer(t, timeout);
	else
	{
		//LOG << "timer add fail";
	}
}