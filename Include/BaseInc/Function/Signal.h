/**
*@file Signal.h
*@brief 信号槽类，基于TFunction,触发注册的回调函数
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_FUNCTION_SIGNAL__
#define __FRAMEWORK_INCLUDE_FUNCTION_SIGNAL__

#include <list>
#include <mutex>
#include <thread>

#include "Function.h"

struct SigEnum {
    enum SlotPosition
    {
        posAny,
        posBack,
        posFront,
    };
    enum ErrorCode
    {
        codeNoFound = -1,
        codeExist = -2,
        codeFull = -3,
        codeEmpty = -4,
    };
};

template<typename R, typename ...P>
class TSignal
{
    template<typename, typename...> friend class TSignal;
public:
    typedef TFunction<R, P...> Proc;
private:
    struct SignalSlot
    {
        Proc proc;
        SignalSlot &operator=(const SignalSlot &ef)
        {
            proc = ef.proc;
            return *this;
        }
        SignalSlot &operator=(SignalSlot &&ef) noexcept
        {
            proc = std::move(ef.proc);
            return *this;
        }
        SignalSlot(const SignalSlot &ef)
        {
            *this = ef;
        }
        SignalSlot(SignalSlot &&ef) noexcept
        {
            *this = ef;
        }
        SignalSlot(){}
    };

public:
    explicit TSignal(int maxNum = 16) :m_numberMax(maxNum){}

    /// 设置槽的最大容量
    void setMaxSigNum(int num){m_numberMax = num;}
    /**
    *@brief 注册回调
    *一般参数默认即可
    *@param proc 回调函数
    *@param pos 存放位置
    *@param specPos 特定的存放位置，当pos为posAny时有效
    *@return 注册是否成功
    */
    int attach(const Proc &proc, SigEnum::SlotPosition pos = SigEnum::posBack, const unsigned int specPos = 0)
    {
        if (!proc)
        {
            return SigEnum::codeEmpty;
        }
        if (isAttached(proc))
        {
            return SigEnum::codeExist;
        }
        std::lock_guard<std::mutex> Guard(m_mutex);
        if (m_lstSlots.size() >= m_numberMax)
        {
            return SigEnum::codeFull;
        }
        SignalSlot slot;
        slot.proc = proc;
        switch (pos)
        {
        case SigEnum::posBack:
            m_lstSlots.push_back(slot);
            break;
        case SigEnum::posFront:
            m_lstSlots.push_front(slot);
            break;
        case SigEnum::posAny:
            if (specPos >= m_numberMax)
            {
                return SigEnum::codeNoFound;
            }
            else if (specPos == 0)
            {
                return SigEnum::codeNoFound;
            }
            else if (specPos > m_lstSlots.size())
            {
                m_lstSlots.push_back(slot);
            }
            else
            {
                unsigned int i = 0;
                for (typename std::list<SignalSlot>::iterator iter = m_lstSlots.begin(); iter != m_lstSlots.end(); ++iter)
                {
                    i++;
                    if (specPos == i)
                    {
                        m_lstSlots.insert(iter, slot);
                        break;
                    }
                }
            }
            break;
        default:
            break;
        }
        return m_lstSlots.size();
    }

    /// 卸载槽中的某个回调
    int detach(const Proc &proc)
    {
        if (!proc)
        {
            return SigEnum::codeEmpty;
        }
        std::unique_lock<std::mutex> Guard(m_mutex);
        for (typename std::list<SignalSlot>::iterator iter = m_lstSlots.begin(); iter != m_lstSlots.end(); ++iter)
        {
            if (proc == iter->proc)
            {
                m_lstSlots.erase(iter);
                return m_lstSlots.size();
            }
        }
        return SigEnum::codeNoFound;
    }

    bool isAttached(const Proc &proc)
    {
        std::lock_guard<std::mutex> Guard(m_mutex);
        for (typename std::list<SignalSlot>::iterator iter = m_lstSlots.begin(); iter != m_lstSlots.end(); ++iter)
        {
            if (proc == iter->proc)
            {
                return true;
            }
        }
        return false;
    }

    /// 触发信号，会将调用槽中的所有函数
    void operator()(P...args)
    {
        m_mutex.lock();
        if (m_lstSlots.empty())
        {
            m_mutex.unlock();
            return;
        }
        auto tmp = m_lstSlots;
        m_mutex.unlock();
        for (typename std::list<SignalSlot>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter)
        {
            Proc proc = iter->proc;
            proc(args...);
        }
    }

    uint32_t getSlotSize(){ std::lock_guard<std::mutex> Guard(m_mutex); return m_lstSlots.size();}

    bool empty() {std::lock_guard<std::mutex> Guard(m_mutex); return m_lstSlots.empty();}

    TSignal &operator=(const TSignal &ef)
    {
        if (this == &ef)
        {
            return *this;
        }
        m_numberMax = ef.m_numberMax;
        for (const auto &iter : ef.m_lstSlots)
        {
            m_lstSlots.push_back(iter);
        }
        return *this;
    }
    TSignal &operator=(TSignal &&ef) noexcept
    {
        if (this == &ef)
        {
            return *this;
        }
        m_numberMax = ef.m_numberMax;
        m_lstSlots =std::move(ef);
        return *this;
    }
    TSignal(const TSignal &ef)
    {
        *this = ef;
    }
    TSignal(TSignal &&ef) noexcept
    {
        *this = ef;
    }

private:
    size_t m_numberMax;
    std::list<SignalSlot> m_lstSlots;
    std::mutex m_mutex;
};
#endif /* ifndef __FRAMEWORK_INCLUDE_FUNCTION_SIGNAL_H__ */

