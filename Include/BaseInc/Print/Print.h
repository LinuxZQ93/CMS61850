/**
*@file Print.h
*@brief 打印库，支持函数名，模块名，线程名等的打印显示,终端下支持不同颜色的输出
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_PRINT_PRINT_H__
#define __FRAMEWORK_INCLUDE_PRINT_PRINT_H__

#include <cstring>
#include "Function/Signal.h"
#include "Export/Export.h"

#ifdef __cplusplus
extern "C"
{
#endif

#undef nonef
#undef fatalf
#undef errorf
#undef warnf
#undef infof
#undef debugf
#undef debugf1
#undef debugf2

enum LogLevel
{
    NONE = 0,
    FATAL,
#ifdef WIN32
    ERROR_W,
#else
    ERROR,
#endif
    WARN,
    INFO,
    DEBUG,
    DEBUG1,
    DEBUG2,
    ALL
};

#ifndef LibName
/**
* @def LibName
* @brief 库名称
*/
#define LibName "Unknown"
#endif

#ifndef LibVersion
/**
* @def LibVersion
* @brief 库版本
*/
#define LibVersion "0"
#endif

#ifdef WIN32
#define __func__ __FUNCTION__
#endif

/// 设置打印级别，精确到组件，componentName为all，设置所有组件
DLL_EXPORT void setPrintLevel(const std::string &componentName, LogLevel level);

/// 注册数据流流向，可用于日志保存及网络传输，不带颜色
DLL_EXPORT bool attachDataSinkWithoutColor(const TFunction<void, const char *, int> &func);
/// 卸载数据流
DLL_EXPORT bool detachDataSinkWithoutColor(const TFunction<void, const char *, int> &func);

/// 注册数据流流向，可用于日志保存及网络传输，带颜色（仅支持终端查看）
DLL_EXPORT bool attachDataSinkWithColor(const TFunction<void, const char *, int> &func);
/// 卸载数据流
DLL_EXPORT bool detachDataSinkWithColor(const TFunction<void, const char *, int> &func);

#ifdef WIN32
DLL_EXPORT int LogPrint(int level, const char *file, const char *func, int line,
	const char *libName, const char* libVersion, const char *fmt, ...);
#else
int LogPrint(int level, const char *file, const char *func, int line,
                const char *libName, const char *libVersion, const char *fmt, ...) __attribute__((format(printf, 7, 8)));
#endif

inline const char * baseName(const char *filePath)
{
    const char* pBase = strrchr(filePath, '/');
#ifdef WIN32
    if (!pBase)
    {
       pBase = strrchr(filePath, '\\');
    }
#endif
    return pBase ? (pBase+1) : filePath;
}

/**
* @def nonef
* @brief 裸正常打印，可取代printf
*/

#define rawf(fmt, ...) LogPrint(NONE, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)
/**
* @def fatalf
* @brief 致命错误打印
*/
#define fatalf(fmt, ...) LogPrint(FATAL, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)
/**
* @def errorf
* @brief 错误打印
*/
#ifdef WIN32
#define errorf(fmt, ...) LogPrint(ERROR_W, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)
#else
#define errorf(fmt, ...) LogPrint(ERROR, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)
#endif
/**
* @def warnf
* @brief 警告打印
*/
#define warnf(fmt, ...) LogPrint(WARN, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)
/**
* @def infof
* @brief 正常打印
*/
#define infof(fmt, ...) LogPrint(INFO, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)
/**
* @def debugf
* @brief 重要调试打印，可用于函数出入口打印
*/
#define debugf(fmt, ...) LogPrint(DEBUG, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)
/**
* @def debugf1
* @brief 次重要调试打印，可用于函数内部重要函数调用时打印
*/
#define debugf1(fmt, ...) LogPrint(DEBUG1, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)
/**
* @def debugf2
* @brief 一般调试打印，可用于一般调试信息
*/
#define debugf2(fmt, ...) LogPrint(DEBUG2, baseName(__FILE__), __func__, __LINE__, LibName, LibVersion, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* ifndef __FRAMEWORK_INCLUDE_PRINT_PRINT_H__ */

