/**
*@file Semaphore.h
*@brief 信号量类
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_SEMAPHORE_SEMAPHORE_H__
#define __FRAMEWORK_INCLUDE_SEMAPHORE_SEMAPHORE_H__

#include <mutex>
#ifndef WIN32
#include <pthread.h>
#else
#include <condition_variable>
#endif
// #include "condition_variable_modify"

/**
 * 因为std的condition_variable不支持相对时间，导致系统对时出现时间跳变时，产生的线程超时。
 * 若直接修改std的，风险比较大，且不一定会成功，所以模仿其实现所需的相对等待时间。
 * 此类用户无特殊情况，不需要调用，可调用封装好的信号量。并且此类仅使用于posix，若后期有windows的需求再考虑扩展
*/
#ifndef WIN32
class CConditionVariable {
  enum class cv_status { no_timeout, timeout };
  typedef std::chrono::steady_clock	__clock_t;
public:
    CConditionVariable() noexcept;
    ~CConditionVariable() noexcept;

    CConditionVariable(const CConditionVariable&) = delete;
    CConditionVariable& operator=(const CConditionVariable&) = delete;

    void
    notify_one() noexcept;

    void
    notify_all() noexcept;

public:
    void
    wait(std::unique_lock<std::mutex>& __lock) noexcept;

    template<typename _Predicate>
      void
      wait(std::unique_lock<std::mutex>& __lock, _Predicate __p)
      {
    while (!__p())
      wait(__lock);
      }

    template<typename _Duration>
      cv_status
      wait_until(std::unique_lock<std::mutex>& __lock,
         const std::chrono::time_point<__clock_t, _Duration>& __atime)
      { return __wait_until_impl(__lock, __atime); }

    template<typename _Clock, typename _Duration>
      cv_status
      wait_until(std::unique_lock<std::mutex>& __lock,
         const std::chrono::time_point<_Clock, _Duration>& __atime)
      {
    // DR 887 - Sync unknown clock to known clock.
    const typename _Clock::time_point __c_entry = _Clock::now();
    const typename _Clock::time_point __s_entry = _Clock::now();
    const auto __delta = __atime - __c_entry;
    const auto __s_atime = __s_entry + __delta;

    return __wait_until_impl(__lock, __s_atime);
      }

    template<typename _Clock, typename _Duration, typename _Predicate>
      bool
      wait_until(std::unique_lock<std::mutex>& __lock,
         const std::chrono::time_point<_Clock, _Duration>& __atime,
         _Predicate __p)
      {
    while (!__p())
      if (wait_until(__lock, __atime) == cv_status::timeout)
        return __p();
    return true;
      }

    template<typename _Rep, typename _Period>
      cv_status
      wait_for(std::unique_lock<std::mutex>& __lock,
           const std::chrono::duration<_Rep, _Period>& __rtime)
      {
    using __dur = typename __clock_t::duration;
    auto __reltime = std::chrono::duration_cast<__dur>(__rtime);
    if (__reltime < __rtime)
      ++__reltime;
    return wait_until(__lock, __clock_t::now() + __reltime);
      }

    template<typename _Rep, typename _Period, typename _Predicate>
      bool
      wait_for(std::unique_lock<std::mutex>& __lock,
           const std::chrono::duration<_Rep, _Period>& __rtime,
           _Predicate __p)
      {
    using __dur = typename __clock_t::duration;
    auto __reltime = std::chrono::duration_cast<__dur>(__rtime);
    if (__reltime < __rtime)
      ++__reltime;
    return wait_until(__lock, __clock_t::now() + __reltime, std::move(__p));
      }

  private:
    template<typename _Clock, typename _Dur>
      cv_status
      __wait_until_impl(std::unique_lock<std::mutex>& __lock,
            const std::chrono::time_point<_Clock, _Dur>& __atime)
      {
    auto __s = std::chrono::time_point_cast<std::chrono::seconds>(__atime);
    auto __ns = std::chrono::duration_cast<std::chrono::nanoseconds>(__atime - __s);

    __gthread_time_t __ts =
      {
        static_cast<std::time_t>(__s.time_since_epoch().count()),
        static_cast<long>(__ns.count())
      };

    __gthread_cond_timedwait(&condition_, __lock.mutex()->native_handle(),
                 &__ts);

    return (_Clock::now() < __atime
        ? cv_status::no_timeout : cv_status::timeout);
      }

private:
    pthread_cond_t condition_;
};
#endif

class Semaphore
{
public:
    explicit Semaphore(unsigned int count = 0);
    ~Semaphore(){}

    void pend();
    bool tryPend();
    bool pendFor(unsigned int milliseconds);
    void post();

private:
    unsigned int m_count;
    std::mutex m_mutex;
#ifdef WIN32
    std::condition_variable m_cv;
#else
    CConditionVariable m_cv;
#endif
};

#endif /* ifndef __FRAMEWORK_INCLUDE_SEMAPHORE_SEMAPHORE_H__ */

