/**
*@file Library.h
*@brief 加载动态库通用函数
*
*Version:1.0
*
*Date:2020/09
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_LIBRARY_LIBRARY_H__
#define __FRAMEWORK_INCLUDE_LIBRARY_LIBRARY_H__

#include <string>

#include "Export/Export.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace base {

#ifdef WIN32
typedef HINSTANCE LibraryPtr;
#else
typedef void* LibraryPtr;
#endif  // OS_*

/**
*@brief 加载动态库
*@param libPath 动态库路径
*@param[out] error 当函数返回为空时，error存储具体的错误信息
*@return 返回动态库句柄
*/
DLL_EXPORT LibraryPtr loadLibraryPtr(const std::string &libPath, std::string &error);

/// 卸载动态库
DLL_EXPORT void unloadLibraryPtr(LibraryPtr lib);

/**
*@brief 获取动态库内的指定函数指针
*@param lib 动态库句柄
*@param name 需要获取函数的具体名称
*@return 返回函数指针
*/
DLL_EXPORT void *getFuncPtrFormLib(LibraryPtr lib, const char *name);

/// 获取动态库的具体名称，根据平台不同，名称不同，比如linux为so,windows为dll
DLL_EXPORT std::string getLibName(const std::string &name);

}

#endif /* ifndef __FRAMEWORK_INCLUDE_LIBRARY_LIBRARY_H__ */

