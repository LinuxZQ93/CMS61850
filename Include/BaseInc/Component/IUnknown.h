/**
*@file IUnknown.h
*@brief 组件接口，所有组件必须继承该接口
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_COMPONENT_IUNKNOWN_H__
#define __FRAMEWORK_INCLUDE_COMPONENT_IUNKNOWN_H__

#include <string>
#include <map>

#include "ComponentManager.h"
#include "Print/Print.h"

namespace base {

class IUnknown {
public:
    IUnknown();
    virtual ~IUnknown();
public:
    /// 工厂接口,用于创建组件，用户实现的每个子类必须伴随一个工厂类去创建
    class IFactory {
    public:
        IFactory();
        virtual ~IFactory();
        bool registerComponent();
        bool unRegisterComponent();
    protected:
        IUnknown *m_component;
    };
public:
    /// 组件初始化
    virtual bool init() = 0;
    /// 组件启动
    virtual bool start() = 0;
    /// 组件停止
    virtual bool stop() = 0;
    /// 组件销毁
    virtual bool destroy() = 0;
    /// 标识组件的唯一标识符，由子类定义实现
    virtual const std::string getClisd() = 0;
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_COMPONENT_IUNKNOWN_H__ */

