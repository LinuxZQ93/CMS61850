/**
*@file Stack.h
*@brief 堆栈回溯类，目前仅支持linux,且编译时不能打开优化，并且需加-rdynamic及-g选项
*得出地址后，可用addr2line工具进行解析，定位到具体的代码。
*
*Version:1.0
*
*Date:2020/09
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_DEBUG_STACK_H__
#define __FRAMEWORK_INCLUDE_DEBUG_STACK_H__

#include "Function/Signal.h"

namespace base {

class CStack {

public:
    CStack();
    ~CStack();

public:
    static void getBacktrace();
    static bool attachGetStackInfo(const TFunction<void, const char*, int> &func);
    static bool detachGetStackInfo(const TFunction<void, const char*, int> &func);

};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_DEBUG_STACK_H__ */

