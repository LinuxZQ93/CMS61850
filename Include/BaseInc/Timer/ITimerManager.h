/**
*@file ITimerManager.h
*@brief 定时器管理类,调度管理定时器的执行
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_TIMER_ITIMERMANAGER_H__
#define __FRAMEWORK_INCLUDE_TIMER_ITIMERMANAGER_H__

#include "Component/IUnknown.h"

#include <string>
#include <vector>

#include "Print/Print.h"
#include "Function/Signal.h"

namespace base {

class ITimerManager : public base::IUnknown {
public:
    class ITimerManagerFactory : public base::IUnknown::IFactory
    {
    };
    virtual const std::string getClisd()
    {
        return "TimerManager";
    }
public:
    virtual bool init(){return true;}
    virtual bool start(){return true;}
    virtual bool stop(){return true;}
    virtual bool destroy(){return true;}
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_TIMER_ITIMERMANAGER_H__ */

