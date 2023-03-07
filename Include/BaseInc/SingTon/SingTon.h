/**
*@file SingTon.h
*@brief 单例宏，方便定义单例类
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_SINGTON_SINGTON_H__
#define __FRAMEWORK_INCLUDE_SINGTON_SINGTON_H__

#include <mutex>

#undef SINGTON_DECLAR
#define SINGTON_DECLAR(className) \
public:\
    static className *instance();\
private:\
    className();\
    ~className();\

#undef SINGTON_DEFINITION
#define SINGTON_DEFINITION(className) \
className *className::instance() \
{\
    static std::mutex s_mutex;\
    static className *s_instance;\
    if (nullptr == s_instance) \
    {\
        std::lock_guard<std::mutex> Guard(s_mutex);\
        if (nullptr == s_instance) \
        {\
            s_instance = new className;\
        }\
    } \
    return s_instance;\
}

#endif /* ifndef __FRAMEWORK_INCLUDE_SINGTON_SINGTON_H__ */

