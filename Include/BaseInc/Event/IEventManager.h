/**
*@file IEventManager.h
*@brief 事件中心组件，支持事件群发，支持不同业务之间交互，为解耦而生
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_EVENT_IEVENTMANAGER_H__
#define __FRAMEWORK_INCLUDE_EVENT_IEVENTMANAGER_H__

#include "Component/IUnknown.h"

#include <map>
#include <string>

#include "json/json.h"

#include "Print/Print.h"
#include "Function/Signal.h"
#include "Data/Any.h"

namespace base {

/// 组件创建方式，可参照此模板
class IEventManager : public base::IUnknown {
public:
    typedef TSignal<void, const std::string&, const Json::Value&> Signal;
    typedef Signal::Proc Proc;
    typedef TSignal<void, const std::string&, const base::any&, base::any&> SignalAny;
    typedef SignalAny::Proc ProcAny;
public:
    class IEventManagerFactory : public base::IUnknown::IFactory
    {
    };
    /// 组件唯一表示符，不可与其它组件重复
    virtual const std::string getClisd()
    {
        return "EventManager";
    }
public:
    virtual bool init(){return true;}
    virtual bool start(){return true;}
    virtual bool stop(){return true;}
    virtual bool destroy(){return true;}
public:
    /**
    *@brief 注册事件
    *注册（订阅）感兴趣的名字为code的事件
    *@param code 名字，唯一标识符
    *@param proc 回调函数
    *@return 注册是否成功
    */
    virtual bool attach(const std::string &code, const Proc &proc, SigEnum::SlotPosition pos = SigEnum::posBack, const unsigned int specPos = 0) = 0;
    virtual bool detach(const std::string &code, const Proc &proc) = 0;

    /**
    *@brief 同步发送事件
    *发送（发布）名字为code的事件，将会同步调用所有监听者，然后返回
    *@param code 名字，唯一标识符
    *@param cfgEvent 待发布的事件
    *@return 发布是否成功
    */
    virtual bool sendEvent(const std::string &code, const Json::Value &cfgEvent) = 0;
    /**
    *@brief 异步发送事件
    *发送（发布）名字为code的事件，异步通知所有监听者，用户调用后，立即返回
    *@param code 名字，唯一标识符
    *@param cfgEvent 待发布的事件
    *@return 发布是否成功
    */
    virtual bool postEvent(const std::string &code, const Json::Value &cfgEvent) = 0;

    /**
    *@brief 注册事件
    *注册（订阅）感兴趣的名字为code的事件
    *@param code 名字，唯一标识符
    *@param proc 回调函数
    *@return 注册是否成功
    */
    virtual bool attach(const std::string &code, const ProcAny &proc, SigEnum::SlotPosition pos = SigEnum::posBack, const unsigned int specPos = 0) = 0;
    virtual bool detach(const std::string &code, const ProcAny &proc) = 0;

    /**
    *@brief 同步发送事件
    *发送（发布）名字为code的事件，将会同步调用所有监听者，然后返回
    *与之前的接口相比，更加抽象化，value可以存储任意值类型，包括Json，int struct等
    *@param code 名字，唯一标识符
    *@param value 发布的事件值
    *@param[out] result 获取的事件结果
    *@return 发布是否成功
    */
    virtual bool sendEvent(const std::string &code, const base::any &value, base::any &result) = 0;
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_EVENT_IEVENTMANAGER_H__ */

