/**
*@file Runtime.h
*@brief 运行时类，用于获取动态库组件的注册接口并调用
*
*Version:1.0
*
*Date:2020/09
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_RUNTIME_RUNTIME_H__
#define __FRAMEWORK_INCLUDE_RUNTIME_RUNTIME_H__

#include <vector>

#include "SingTon/SingTon.h"
#include "Library/Library.h"

namespace base {

class CRuntime {

typedef void(*FuncPtr)();
SINGTON_DECLAR(CRuntime);

public:
    bool registerAllComponent();
    bool unRegisterAllComponent();
    bool init();
    bool start();
    bool stop();
    bool destory();

    static std::string version();
private:
    void onConsole(const std::vector<std::string> &argList);

private:
    std::vector<LibraryPtr> m_vecFuncPtr;

};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_RUNTIME_RUNTIME_H__ */

