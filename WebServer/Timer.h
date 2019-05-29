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
	void clearReq(); // 清除连接请求以及与之关联的时间节点
	void setDeleted() { deleted_ = true; }
	bool isDeleted() const { return deleted_; }
	size_t getExptime() const { return expiredTime_; } // expect time

private:
	bool deleted_;
	size_t expiredTime_;
	std::shared_ptr<HttpData> SPHttpData;	// TimerNode关联的HttpData报文类
};

struct TimerCmp	// 时间比较函数
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
	void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);	// 将HttpData新添加到时间管理器
	void handleExpiredEvent(); // 通过小根堆来管理请求的释放

private:
	typedef std::shared_ptr<TimerNode> SPTimerNode;
	std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp> timerNodeQueue;	// 时间管理器
};