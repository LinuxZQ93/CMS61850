/**
*@file IThreadManager.h
*@brief 线程管理类，只需调用其启动后，即可监测线程状态
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_THREAD_ITHREADMANAGER_H__
#define __FRAMEWORK_INCLUDE_THREAD_ITHREADMANAGER_H__

#include "Component/IUnknown.h"

#include <string>
#include <vector>

#include "Print/Print.h"
#include "Function/Signal.h"

namespace base {

class IThreadManager : public base::IUnknown {
public:
    class IThreadManagerFactory : public base::IUnknown::IFactory
    {
    };
    virtual const std::string getClisd()
    {
        return "ThreadManager";
    }
public:
    virtual bool init(){return true;}
    virtual bool start(){return true;}
    virtual bool stop(){return true;}
    virtual bool destroy(){return true;}
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_THREAD_ITHREADMANAGER_H__ */

