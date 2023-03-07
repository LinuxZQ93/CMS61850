/**
*@file Bind.h
*@brief 函数模态绑定函数，可自动推断类型，不用去写TFunction等工具的具体类型
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_FUNCTION_BIND_H__
#define __FRAMEWORK_INCLUDE_FUNCTION_BIND_H__

#include "Function.h"

namespace base {

/// 闭包类型，可由base::bind绑定函数后，隐式转换得到
typedef TFunction<void> Closure;

template<typename T, typename ...P>
inline TFunctionWrap<typename T::RetType, typename std::decay<P>::type...> bind(T &&func, P&&... args)
{
    return TFunctionWrap<typename T::RetType, typename std::decay<P>::type...>(std::forward<T>(func), std::forward<P>(args)...);
}

template<typename R, typename ...P>
inline TFunction<R, P...> function(R(*pf)(P...))
{
    return TFunction<R, P...>(pf);
}

template<typename R, typename O, typename ...P>
inline TFunction<R, P...> function(R(O::*mf)(P...), O *p)
{
    return TFunction<R, P...>(mf, p);
}

template<typename R, typename O, typename ...P>
inline TFunction<R, P...> function(R(O::*mf)(P...) const, O *p)
{
    return TFunction<R, P...>(mf, p);
}

template<typename R, typename O, typename ...P>
inline TFunction<R, P...> function(R(O::*mf)(P...), std::shared_ptr<O> p)
{
    return TFunction<R, P...>(mf, p);
}

template<typename R, typename O, typename ...P>
inline TFunction<R, P...> function(R(O::*mf)(P...) const, std::shared_ptr<O> p)
{
    return TFunction<R, P...>(mf, p);
}

}

#endif /* ifndef __FRAMEWORK_INCLUDE_FUNCTION_BIND_H__ */

