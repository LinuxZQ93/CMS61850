/**
*@file Export.h
*@brief 解析动态库常用宏定义
*
*Version:1.0
*
*Date:2020/09
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_EXPORT_EXPORT_H__
#define __FRAMEWORK_INCLUDE_EXPORT_EXPORT_H__

#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport)
#define API_CALL __stdcall
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#define API_CALL
#endif

#endif /* ifndef __FRAMEWORK_INCLUDE_EXPORT_EXPORT_H__ */

