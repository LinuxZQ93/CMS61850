#ifndef __FRAMEWORK_INCLUDE_TIMER_TIMERMANAGER_H__
#define __FRAMEWORK_INCLUDE_TIMER_TIMERMANAGER_H__

#include "ITimerManager.h"

#include <chrono>

#include "Thread/Thread.h"
#include "Semaphore/Semaphore.h"
#include "SingTon/SingTon.h"

namespace base{

class ITimer;
/// 运用最小堆管理
class CTimerManager : public ITimerManager {
    SINGTON_DECLAR(CTimerManager);
public:
	class CTimerManagerComponentFactory : public ITimerManager::ITimerManagerFactory
	{
	public:
		CTimerManagerComponentFactory()
		{
			m_component = CTimerManager::instance();
			registerComponent();
		}
		~CTimerManagerComponentFactory()
		{
			unRegisterComponent();
		}
	};
public:
	virtual const std::string getClisd()
	{
		return "TimerManager";
	}
public:
    virtual bool start();
    virtual bool stop();
    /// 添加定时器
    bool addTimer(ITimer *timer);
    /// 移除定时器
    bool removeTimer(ITimer *timer);
private:
    void run();
    unsigned long long onTimer();
    void upHeap(size_t index);
    void downHeap(size_t index);
    void swapHeap(size_t index1, size_t index2);
private:
    std::mutex  m_mutex;
    std::vector<ITimer*> m_minHeapTimer;
    ThreadWrap<void> m_thread;
    std::mutex  m_mutexStart;
    bool m_bStart;
    bool m_bUpdate;
#ifdef WIN32
    std::condition_variable m_cv;
#else
    CConditionVariable m_cv;
#endif
    base::ThreadPool<void> m_pool;
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_TIMER_TIMERMANAGER_H__ */

