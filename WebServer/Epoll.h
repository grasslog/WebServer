#pragma once
#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>

class Epoll
{
public:
	Epoll();
	~Epoll();
	// 对应了epoll_ctl对内核I/O事件监测红黑树的操作
	void epoll_add(SP_Channel request, int timeout);
	void epoll_mod(SP_Channel request, int timeout);
	void epoll_del(SP_Channel request);
	// I/O事件vector集合
	std::vector<std::shared_ptr<Channel>> poll();
	std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
	// 添加请求事件
	void add_timer(std::shared_ptr<Channel> request_data, int timeout);
	int getEpollfd()
	{
		return epollFd_;
	}
	void handleExpired();
private:
	static const int MAXFDS = 100000;
	int epollFd_;
	std::vector<epoll_event> events_;   // 返回的事件结果vector集合
	std::shared_ptr<Channel> fd2chan_[MAXFDS];  // 记录事件
	std::shared_ptr<HttpData> fd2http_[MAXFDS]; // 记录httpdata
	TimerManager timerManager_;
};