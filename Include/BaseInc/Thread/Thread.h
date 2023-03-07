/**
*@file Thread.h
*@brief 线程管理，包含线程类，线程池类
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_THREAD_THREADPOOL_H__
#define __FRAMEWORK_INCLUDE_THREAD_THREADPOOL_H__

#include <assert.h>

#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <future>
#include <chrono>
#include <atomic>

#include "Function/Function.h"
#include "MsgQue/MsgQue.h"
#include "ThreadManager.h"
#include "Print/Print.h"
#include "Semaphore/Semaphore.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif // WIN

namespace base {

/// 线程接口
class ThreadImpl
{
public:
    enum ThreadState
    {
        UNSTART,
        NORMAL,
        SLEEP,
        STOP,
        TIMEOUT,
        DONE
    };
public:
    ThreadImpl(){};
    virtual ~ThreadImpl(){}
    virtual bool bTimeOut(){ return false; }
    virtual ThreadState getThreadState(){ return UNSTART; }
    virtual const std::string &getThreadName(){ return m_threadName; }
    virtual int getTid(){ return -1; }
    virtual bool looping(){ return false; }
    /// 设置是否需要超时机制
    /// 暂用于线程池
    virtual void setNeedTimeOut(bool bOut){}
    /// 可修改线程名称
    virtual void setThreadName(const std::string &name){ m_threadName = name; }
    virtual void stop(){}
    virtual bool sleep(){ return false; }
    virtual bool awake(){ return true; }
private:
    virtual void run() = 0;
protected:
    std::string m_threadName;
};

/// 线程封装，基于TFunctionWrap类，可方便线程的使用
template<typename R, typename ...P>
class ThreadWrap : public ThreadImpl
{
public:
    template<typename FuncWrap>
    ThreadWrap(FuncWrap &&funcWrap, const std::string &name, int timeOut = 60)
        : m_funcWrap(std::forward<FuncWrap>(funcWrap))
        , m_timePoint(std::chrono::steady_clock::time_point())
        , m_bSleep(false)
        , m_bOut(true)
        , m_state(UNSTART)
    {
        m_threadName = name;
        m_threadID = -1;
        //m_mgr = CThreadManager::instance();
        //assert(m_mgr);
        m_timeOut = timeOut;
    }
    ThreadWrap(const std::string &name, int timeOut = 60)
        : m_timePoint(std::chrono::steady_clock::time_point())
        , m_bSleep(false)
        , m_bOut(true)
        , m_state(UNSTART)
    {
        m_threadName = name;
        m_threadID = -1;
        //m_mgr = CThreadManager::instance();
        //assert(m_mgr);
        m_timeOut = timeOut;
    }
    template<typename FuncWrap>
    void setParam(FuncWrap &&funcWrap)
    {
        m_funcWrap = std::forward<FuncWrap>(funcWrap);
    }
    virtual ~ThreadWrap()
    {
        infof("name[%s], id[%d], destruct\n", m_threadName.c_str(), m_threadID);
        if (m_state != STOP)
        {
            stopInternal();
        }
    }
public:
    virtual int getTid()
    {
        return m_threadID;
    }
    virtual bool looping()
    {
        if (m_bSleep)
        {
            m_state = SLEEP;
            infof("threadId[%d], threadName[%s] to sleep\n", m_threadID, m_threadName.c_str());
            m_semphore.pend();
            if (m_state == SLEEP)
            {
                infof("threadId[%d], threadName[%s] to awake\n", m_threadID, m_threadName.c_str());
                m_state = NORMAL;
            }
        }
        m_timePoint = std::chrono::steady_clock::now();
        if (m_state == TIMEOUT)
        {
            m_state = NORMAL;
        }
        return (m_state == NORMAL);
    }
    virtual bool bTimeOut()
    {
        if (!m_bOut || m_state == SLEEP)
        {
            return false;
        }
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        bool ret = (std::chrono::duration_cast<std::chrono::seconds>(now - m_timePoint.load()).count() > m_timeOut) ? true : false;
        if (ret == true)
        {
            m_state = TIMEOUT;
        }
        return ret;
    }
    virtual ThreadState getThreadState()
    {
        return m_state;
    }
    virtual void setNeedTimeOut(bool bOut)
    {
        m_bOut = bOut;
    }

public:
    void operator()()
    {
        if (m_state == NORMAL)
        {
            warnf("already has thread run\n");
            return;
        }
        m_thread = std::thread(&ThreadWrap::run, this);
        m_timePoint = std::chrono::steady_clock::now();
        CThreadManager::instance()->addThreadInfo(this);
        //m_threadOut = std::thread(&ThreadWrap::timeLoop, this);
    }

    /// 线程停止运行，销毁
    virtual void stop()
    {
        stopInternal();
    }
    /// 线程进入休眠状态，并非销毁，唤醒后会继续执行未完成的任务
    virtual bool sleep()
    {
        if (m_state != NORMAL)
        {
            warnf("thread[%d] sleep failed, state is %d\n", m_threadID, m_state.load());
            return false;
        }
        debugf("thread[%d] will sleep\n", m_threadID);
        m_bSleep = true;
        return true;
    }
    /// 线程唤醒
    virtual bool awake()
    {
        if (m_state != SLEEP)
        {
            warnf("thread[%d] awake failed, state is %d\n", m_threadID, m_state.load());
            return false;
        }
        debugf("thread[%d] will awake\n", m_threadID);
        m_bSleep = false;
        m_semphore.post();
        return true;
    }

private:
    virtual void run()
    {
#ifdef WIN32
        m_threadID = GetCurrentThreadId();
#else
        m_threadID = syscall(SYS_gettid);
#endif // WIN32
        m_state = NORMAL;
        m_funcWrap();
        m_state = DONE;
        /// 有可能客户创建线程只走一次,为防止超时，最后设置不超时监控
        m_bOut = false;
    }
    void stopInternal()
    {
        if (m_state == SLEEP)
        {
            awake();
        }
        m_state = STOP;
        if (m_thread.joinable())
        {
            m_thread.join();
        }
        CThreadManager::instance()->delThreadInfo(this);
    }

private:
    std::thread m_thread;
    int m_threadID;
    TFunctionWrap<R, P...> m_funcWrap;
    //CThreadManager *m_mgr;
    int m_timeOut;
    std::atomic<std::chrono::steady_clock::time_point> m_timePoint;
    std::atomic<bool> m_bSleep;
    std::atomic<bool> m_bOut;
    std::atomic<ThreadState> m_state;
    Semaphore m_semphore;
};

/// 线程池类，任务基于TFunctionWrap封装
template<typename R, typename ...P>
class ThreadPool final
{
public:
    struct TaskType
    {
        typedef std::packaged_task<R()> packTask;
        std::shared_ptr<packTask> task;
        /// 任务名称，因线程池不确定是哪个线程执行任务，所以可以用名称固定,比如用于定时器
        std::string name;
    };
private:
    typedef ThreadWrap<void, int> detailThread;
    typedef TFunctionWrap<R, P...> taskFunc;
public:
    /**
    *@brief 线程池构造函数
    *构造线程池
    *@param name 名字，唯一标识符
    *@param threadNum 线程池个数
    *@param taskNum 排队任务数
    *@param prioNum 优先级
    */
    explicit ThreadPool(const std::string &name, int threadNum = 1, int taskNum = 16, int prioNum = 1)
        : m_threadNum(threadNum)
        , m_bStart(false)
        , m_poolName(name)
    {
        m_task.setParam(taskNum, prioNum);
    }
    ~ThreadPool()
    {
        infof("name : %s threadPool will stop\n", m_poolName.c_str());
        stop();
        infof("name : %s threadPool stop done\n", m_poolName.c_str());
    }
public:
    /**
    *@brief 添加同步任务，当前线程会卡住，等待此任务执行完
    *保证线程执行的时序性
    *@param func 任务
    *@param name 线程名
    *@param level 优先级
    */
    R addTaskSync(const taskFunc &func, const std::string &name = "", int level = 0)
    {
        auto pTask = std::make_shared<typename TaskType::packTask>(std::bind(func));
        std::future<R> ret = pTask->get_future();
        if (!addTask(pTask, name, level))
        {
            return R();
        }
        return ret.get();
    }

    /// 添加异步线程，不需要等待任务执行完
    bool addTaskASync(const taskFunc &func, const std::string &name = "", int level = 0)
    {
        auto pTask = std::make_shared<typename TaskType::packTask>(std::bind(func));
        return addTask(pTask, name, level);
    }

    bool bStarted() { return m_bStart; }

    bool start(int timeout = 60)
    {
        if (m_threadNum <= 0)
        {
            errorf("threadNum[%d] must > 0\n", m_threadNum);
            return false;
        }
        if (m_bStart)
        {
            warnf("threadPool name[%s] already start\n", m_poolName.c_str());
            return false;
        }
        m_bStart = true;
        for (int i = 0; i < m_threadNum; i++)
        {
            std::stringstream nameThread;
            // 默认名称
            nameThread << m_poolName << "_" << i;
            TFunction<void, int> func = TFunction<void, int>(&ThreadPool::loopTask, this);
            TFunctionWrap<void, int> funcWrap(func, i);
            std::shared_ptr<detailThread> threadMgr(new detailThread(std::move(funcWrap), nameThread.str(), timeout));
            m_vecThread.push_back(threadMgr);
        }
		for (int i = 0; i < m_threadNum; i++)
		{
			(*m_vecThread[i])();
		}
        return true;
    }
    void stop()
    {
        m_bStart = false;
        std::shared_ptr<TaskType> task = nullptr;
        for (int i = 0; i < m_threadNum; i++)
        {
            debugf("sendMessage\n");
            m_task.sendMessage(task);
        }
        debugf("clear begin\n");
        m_vecThread.clear();
        debugf("clear end\n");
        m_task.clean();
    }

    int getTaskNum()
    {
        return m_task.size();
    }
private:
    void loopTask(int i)
    {
        infof("loop task\n");
        while (m_bStart)
        {
            m_vecThread[i]->setNeedTimeOut(false);
            std::shared_ptr<TaskType> task = takeTask();
            if (task && m_vecThread[i]->looping())
            {
                m_vecThread[i]->setNeedTimeOut(true);
                if (!task->name.empty())
                {
                    m_vecThread[i]->setThreadName(task->name);
                }
                (*(task->task))();
                //debugf("task[%s] do end\n", task->name.c_str());
            }
        }
        infof("loop task end\n");
    }

    bool addTask(const std::shared_ptr<typename TaskType::packTask> &task, const std::string &name, int level = 0)
    {
        if (!m_bStart)
        {
            warnf("poolName : %s not start\n", m_poolName.c_str());
            return false;
        }
        TaskType tmp = { task, name };
        std::shared_ptr<TaskType> taskType = std::make_shared<TaskType>(std::move(tmp));
        if (!m_task.sendMessage(std::move(taskType), level))
        {
            warnf("addTask failed, may be task full\n");
            return false;
        }
        return true;
    }

    std::shared_ptr<TaskType> takeTask()
    {
        std::shared_ptr<TaskType> task;
        if (!m_bStart)
        {
            errorf("poolName[%s] not start\n", m_poolName.c_str());
        }
        m_task.recvMessage(task);
        return task;
    }
private:
    ThreadPool(const ThreadPool &ef);
    const ThreadPool& operator=(const ThreadPool &ef);
private:
    int m_threadNum;
    std::mutex m_mutex;
    std::atomic<bool> m_bStart;
    std::string m_poolName;
    std::vector<std::shared_ptr<detailThread> > m_vecThread;
    TMsgQue<std::shared_ptr<TaskType> > m_task;
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_THREAD_THREADPOOL_H__ */

