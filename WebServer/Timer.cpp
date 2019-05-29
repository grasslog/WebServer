#include "Timer.h"
#include <sys/time.h>
#include <unistd.h>
#include <queue>


TimerNode::TimerNode(std::shared_ptr<HttpData> requestData, int timeout)
:	deleted_(false),
	SPHttpData(requestData)
	{
		struct timeval now;
		gettimeofday(&now, NULL);
		// 计算超时到期时间
		expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
	}

TimerNode::~TimerNode() // close the binded HttpData when TimerNode was deleted
{
	if(SPHttpData)
		SPHttpData->handleClose();
}

TimerNode::TimerNode(TimerNode &tn)
: SPHttpData(tn.SPHttpData)
{ }

void TimerNode::update(int timeout) // update excally time
{
	struct timeval now;
	gettimeofday(&now, NULL);
	expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

// 判断当前时间是否超时
bool TimerNode::isValid()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
	if(temp < expiredTime_)
		return true;
	else
	{
		this->setDeleted();
		return false;
	}
}

// 清除请求http数据报文
void TimerNode::clearReq()
{
	SPHttpData.reset();
	this->setDeleted();
}


TimerManager::TimerManager()
{ }

TimerManager::~TimerManager()
{ }

// 向时间管理器添加新的节点
void TimerManager::addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout)
{
	SPTimerNode new_node(new TimerNode(SPHttpData, timeout));
	timerNodeQueue.push(new_node);
	SPHttpData->linkTimer(new_node); // HttpData assosiate TimerNode
}

// 
void TimerManager::handleExpiredEvent() // building with least queue, delete old timernode
{	// 时间管理器，通过小根堆的方式来删除堆顶超时的节点
	while(!timerNodeQueue.empty())
	{
		SPTimerNode ptimer_now = timerNodeQueue.top();
		if(ptimer_now->isDeleted())
			timerNodeQueue.pop();
		else if(ptimer_now->isValid()==false)
			timerNodeQueue.pop();
		else break;
	}
}