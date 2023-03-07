/**
*@file IConsole.h
*@brief 窗口打印组件，方便用户与程序实时交流获取状态信息
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_CONSOLE_ICONSOLE_H__
#define __FRAMEWORK_INCLUDE_CONSOLE_ICONSOLE_H__

#include "Component/IUnknown.h"

#include <string>
#include <vector>

#include "Print/Print.h"
#include "Function/Signal.h"

namespace base {

class IConsole : public base::IUnknown {
public:
    typedef TFunction<void, const std::vector<std::string>& > func;
public:
    class IConsoleFactory : public base::IUnknown::IFactory
    {
    };
    virtual const std::string getClisd()
    {
        return "Console";
    }
public:
    virtual bool init(){return true;}
    virtual bool start(){return true;}
    virtual bool stop(){return true;}
    virtual bool destroy(){return true;}
public:
    /**
    *@brief 注册窗口打印函数
    *注册自定义的窗口函数，方便在程序运行时进行实时操作
    *@param cmdName 名字，唯一标识符
    *@param f 回调函数
    *@return 注册是否成功
    */
    virtual bool attach(const std::string &cmdName, const func &f) = 0;
    virtual bool detach(const std::string &cmdName) = 0;
    /**
    *@brief 查询窗口打印函数
    *查询自定义的窗口函数，跟终端交互查询效果一致，此接口提供是为了在程序运行时查询
    *@param cmd 命令数组,cmd[0]是注册命令的名字，对应attach的cmdName
    *@return 查询是否成功
    */
    virtual bool query(std::vector<std::string> cmd) = 0;
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_CONSOLE_ICONSOLE_H__ */

