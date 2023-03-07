/**
*@file MsgQue.h
*@brief 消息队列模板，支持绑定任意类型，支持消息优先级
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_MSGQUE_MSGQUE_H__
#define __FRAMEWORK_INCLUDE_MSGQUE_MSGQUE_H__

#include <list>
#include <mutex>
#include <memory>
#include <iostream>
#include <vector>

#include "Print/Print.h"
#include "Semaphore/Semaphore.h"
#include "Function/Function.h"

namespace base {

template<typename T>
class TMsgQue
{
    typedef std::list<T> Queue;
    typedef typename Queue::iterator QueIter;
public:
    TMsgQue()
        : m_queSize(0)
        , m_prioLevel(1)
    {
    }
    /**
    *@brief 队列构造函数
    *@param queueSize 队列大小
    *@param priLevel 支持几个优先级范围
    */
    TMsgQue(int queueSize, int priLevel)
    {
        setParam(queueSize, priLevel);
    };

    ~TMsgQue(){}

    void setParam(int queueSize, int priLevel)
    {
        m_queSize = queueSize;
        m_prioLevel = priLevel;
        m_prioIterPtr = std::move(std::unique_ptr<QueIter[]>(new QueIter[priLevel]));
        for (int i = 0; i < priLevel; i++)
        {
            m_prioIterPtr[i] = m_queue.begin();
        }
    }

    /// 发送消息
    template<typename M, typename = typename std::enable_if<std::is_same<typename std::decay<M>::type, T>::value>::type>
    bool sendMessage(M&& message, int priority = 0)
    {
        if (priority < 0 || priority >= m_prioLevel)
        {
            errorf("priority[%d] wrong\n", priority);
            return false;
        }

        std::lock_guard<std::mutex> Guard(m_mutex);
        if ((int)m_queue.size() >= m_queSize)
        {
            // warnf("queue size[%zu] over initSize[%d]\n", m_queue.size(), m_queSize);
            return false;
        }

        QueIter oldIter = m_prioIterPtr[priority];
        QueIter newIter = m_queue.insert(oldIter, std::forward<M>(message));
        for (int i = priority + 1; i < m_prioLevel; i++)
        {
            if (m_prioIterPtr[i] == oldIter)
            {
                m_prioIterPtr[i] = newIter;
            }
            else
            {
                break;
            }
        }
        m_semphore.post();
        return true;
    }

    /**
    *@brief 接收消息
    *@param message 消息引用
    *@param timeWait 最大的等待时间，在wait为true的情况下有效,默认为0，单位毫秒
    *@param wait 是否等待,与timeWait结合，若timeWait为0，则一直阻塞等待，若timeWait大于0，则最多等待timeWait时间后，释放
    *@param bDeque 是否弹出该元素
    *@return bool
    */
    bool recvMessage(T &message, int timeWait = 0, bool wait = true, bool bDeque = true)
    {
        if (timeWait < 0)
        {
            errorf("invaild timeWait, must > 0\n");
            return false;
        }
        std::unique_lock<std::mutex> Guard(m_mutex);
        if (!m_semphore.tryPend())
        {
            if (wait)
            {
                if (0 == timeWait)
                {
                    m_mutex.unlock();
                    m_semphore.pend();
                    m_mutex.lock();
                }
                else
                {
                    m_mutex.unlock();
                    bool ret = m_semphore.pendFor(timeWait);
                    m_mutex.lock();
                    if (!ret)
                    {
                        return false;
                    }
                }

            }
            else
            {
                return false;
            }
        }

        message = m_queue.front();

        if (bDeque)
        {
            for (int i = m_prioLevel - 1; i >= 0; i--)
            {
                if (m_prioIterPtr[i] == m_queue.begin())
                {
                    m_prioIterPtr[i]++;
                }
                else
                {
                    break;
                }
            }
            m_queue.pop_front();
        }
        else
        {
            m_semphore.post();
        }
        return true;
    }

    void clean()
    {
        std::lock_guard<std::mutex> Guard(m_mutex);
        while(!m_queue.empty())
        {
            if (m_semphore.tryPend())
            {
                m_queue.pop_back();
            }
            else
            {
                break;
            }
        }
    }

    int size()
    {
        std::lock_guard<std::mutex> Guard(m_mutex);
        return m_queue.size();
    }

    int getMaxSize()
    {
        std::lock_guard<std::mutex> Guard(m_mutex);
        return m_queSize;
    }

    /**
    *@brief 拷贝队列数据至vector
    *@param func 有些元素拷贝需要做一些定制操作，留此回调，可用户定制
    *@param vec 保存数据的数组
    *@return void
    */
    void getPktVec(const TFunction<T, T> &func, std::vector<T> &vec)
    {
        std::lock_guard<std::mutex> Guard(m_mutex);
        for (const auto &iter : m_queue)
        {
            const T& value = func(iter);
            vec.push_back(value);
        }
    }

private:
    Queue m_queue;
    std::unique_ptr<QueIter[]> m_prioIterPtr;
    std::mutex m_mutex;
    Semaphore m_semphore;
    int m_queSize;
    int m_prioLevel;
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_MSGQUE_MSGQUE_H__ */

