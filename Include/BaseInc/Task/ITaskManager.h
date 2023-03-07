/**
*@file ITaskManager.h
*@brief 框架任务管理类，用于执行提交给框架的任务Task，减少业务线程的申请
*
*Version:1.0
*
*Date:2020/12
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_TASK_ITASKMANAGER_H__
#define __FRAMEWORK_INCLUDE_TASK_ITASKMANAGER_H__

#include "Component/IUnknown.h"
#include "Component/ComponentMacroDef.h"

#include <memory>

#include "Function/Bind.h"

namespace base {

class ITaskManager : public base::IUnknown {
    SIMPLE_DEF_I(TaskManager, "TaskManager")
public:
    virtual bool init(){return true;}
    virtual bool start(){return true;}
    virtual bool stop(){return true;}
    virtual bool destroy(){return true;}

public:
    /**
    *@brief 添加系统异步任务，由框架执行。
    *@attention 注意，task可为耗时任务但不允许独占，原则上也不要过于耗时，塞大量休眠等。如果有这样的需要，自己创建线程实现
    *@param Closure 抽象闭包函数对象，可由base::bind绑定任意类型函数实现
    *@param name 任务名称
    *@return 是否添加成功
    */
    virtual bool addTask(const Closure &funcWrap, const std::string &name){ return true; }
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_TASK_ITASKMANAGER_H__ */

