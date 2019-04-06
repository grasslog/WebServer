#pragma once
#include "HttpData.h"
#include "base/noncopyable.h"
#include "base/MutexLock.h"
#include <unistd.h>
#include <memory>
#include <queue>
#include <deque>

class HttpData;  // http request bind with time manager

class TimerNode // record time message bind with httpdata
{
public:
	TimerNode(std::shared_ptr<HttpData> requestData, int timeout);
	~TimerNode();
	TimerNode(TimerNode &tn);
	void update(int timeout); // updata the excally time
	bool isValid();
	void clearReq(); // clear the request which is binded with this TimerNode
	void setDeleted() { deleted_ = true; }
	bool isDeleted() const { return deleted_; }
	size_t getExptime() const { return expiredTime_; } // expect time

private:
	bool deleted_;
	size_t expiredTime_;
	std::shared_ptr<HttpData> SPHttpData;
};

struct TimerCmp
{
	bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const
	{
		return a->getExptime() > b->getExptime();
	}
};

class TimerManager // manage TimerNode
{
public:
	TimerManager();
	~TimerManager();
	void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
	void handleExpiredEvent(); // delete old TimerNode with priority queue

private:
	typedef std::shared_ptr<TimerNode> SPTimerNode;
	std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp> timerNodeQueue;
};