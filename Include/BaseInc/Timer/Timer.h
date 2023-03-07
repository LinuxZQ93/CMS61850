/**
*@file Timer.h
*@brief 定时器类,用于定时触发的任务
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_TIMER_TIMER_H__
#define __FRAMEWORK_INCLUDE_TIMER_TIMER_H__

#include <queue>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

#include "Function/Function.h"
#include "Print/Print.h"
#include "Thread/Thread.h"
#include "Timer/TimerManager.h"

namespace base {

class ITimer{
public:
    ITimer()
        : m_interval(0)
        , m_expries(std::chrono::steady_clock::time_point())
        , m_index(0)
        , m_bRepeat(false)
    {}
    virtual ~ITimer(){}
    friend class CTimerManager;
private:
    virtual void run(std::chrono::steady_clock::time_point now) = 0;
protected:
    unsigned int m_interval;
    std::atomic<std::chrono::steady_clock::time_point> m_expries;
    unsigned int m_index;
    std::atomic<bool> m_bRepeat;
    std::string m_timerName;
};

/// 后期可考虑与task一样的设计，从接口调用，但不一定必要。
/// 因timer一般不会存在短暂即时的调用，一般与调用对象同周期，且此调用方式看起来会更为清晰
template<typename R, typename ...P>
class CTimer : public ITimer{
    typedef TFunctionWrap<R, P...> funcWrap;
public:
    /// 可以一开始就设置函数，也可以后期指定
    explicit CTimer(const std::string &name)
        : m_bStart(false)
    {
        m_timerName = name;
    }
    virtual ~CTimer(){ if (m_bStart){stop();} }

public:
    /**
    *@brief 开始定时器
    *delayMs后启动之前创建的定时器
    *@attention 注意，定时器的任务一般为轻量级快速任务，不要将执行时间长，大量休眠的函数塞进来。如果由这样的需要，用线程实现
    *@param func 待执行的任务
    *@param delayMs 执行间隔
    *@param bRepeat 是否重复执行
    *@return 启动是否成功
    */
    bool start(const funcWrap &func, int delayMs, bool bRepeat)
    {
        if (m_bStart)
        {
            warnf("the timer[%s] already start\n", m_timerName.c_str());
            return false;
        }
        std::chrono::steady_clock::time_point nowTime(std::chrono::steady_clock::now());
        m_expries = nowTime + std::chrono::milliseconds(delayMs);
        m_interval = delayMs;
        m_bRepeat = bRepeat;
        m_funcWrap = func;
        CTimerManager::instance()->addTimer(this);
        m_bStart = true;
        infof("timer[%s] start success\n", m_timerName.c_str());
        return true;
    }
    /// 暂停定时器
    bool stop()
    {
        if (!m_bStart)
        {
            warnf("the timer[%s] already stop\n", m_timerName.c_str());
            return false;
        }
        CTimerManager::instance()->removeTimer(this);
        m_bStart = false;
        infof("timer[%s] stop success\n", m_timerName.c_str());
        return true;
    }

    bool bStarted()
    {
        return m_bStart;
    }
private:
    CTimer(const CTimer&);
    const CTimer& operator=(const CTimer&);

private:
    virtual void run(std::chrono::steady_clock::time_point now)
    {
        if (m_bRepeat)
        {
            m_expries = std::chrono::milliseconds(m_interval) + now;
            CTimerManager::instance()->addTimer(this);
        }
        else
        {
            m_bStart = false;
        }
        m_funcWrap();
    }

private:
    funcWrap m_funcWrap;
    std::atomic<bool> m_bStart;
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_TIMER_TIMER_H__ */
