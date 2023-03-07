/**
*@file MutexRw.h
*@brief 读写锁类
*
*Version:1.0
*
*Date:2020/08
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_MUTEX_MUTEXRW_H__
#define __FRAMEWORK_INCLUDE_MUTEX_MUTEXRW_H__

#include <mutex>
#include <condition_variable>

namespace base {
class CMutexRW
{
public:
     CMutexRW();
     ~CMutexRW();

public:
    void lockRead();
    void unlockRead();
    void lockWrite();
    void unlockWrite();

private:
    unsigned int m_readCount;
    unsigned int m_writeCount;
    bool m_bWrite;
    std::mutex m_mutex;
    std::condition_variable m_condW;
    std::condition_variable m_condR;
};

class CLockReadGuard
{
public:
    explicit CLockReadGuard(CMutexRW &readLock)
        : m_readLock(readLock)
    {
        m_readLock.lockRead();
    }
    ~CLockReadGuard()
    {
        m_readLock.unlockRead();
    }
private:
    CLockReadGuard();
    CLockReadGuard(const CLockReadGuard&);
    CLockReadGuard &operator=(const CLockReadGuard&);
private:
    CMutexRW &m_readLock;
};

class CLockWriteGuard
{
public:
    explicit CLockWriteGuard(CMutexRW &writeLock)
        : m_writeLock(writeLock)
    {
        m_writeLock.lockWrite();
    }
    ~CLockWriteGuard()
    {
        m_writeLock.unlockWrite();
    }
private:
    CLockWriteGuard();
    CLockWriteGuard(const CLockWriteGuard&);
    CLockWriteGuard &operator=(const CLockWriteGuard&);
private:
    CMutexRW &m_writeLock;
};
}

#endif /* ifndef __FRAMEWORK_INCLUDE_MUTEX_MUTEXRW_H__ */

