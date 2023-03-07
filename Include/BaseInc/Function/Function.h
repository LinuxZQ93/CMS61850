/**
*@file Function.h
*@brief 回调函数模板类，可封装任意函数类型，包括C函数，C++类函数，lambda等
*
*sample:void test(int);
*TFunction<void, int>(&test);
*class CTest{
*public:
*void test(int);
*}
*CTest a;
*TFunction<void, int>(&CTest::test, &a);
*第一个参数为返回值，其余为参数，需保证函数类型一致才可以绑定

*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_FUNCTION_FUNCTION__
#define __FRAMEWORK_INCLUDE_FUNCTION_FUNCTION__

#include <functional>
#include <iostream>
#include <tuple>
#include <memory>

#include "Traits/Traits.h"

HAS_TYPE_MEMBER(anyData)

template<typename R, typename ...P>
class TFunction {
    template<typename, typename...> friend class TFunction;
    class X{};
    typedef R(X::*mf)(P...);
    typedef R(*const *pf)(P...);
    typedef R(*pfn)(P...);
    struct MemberFunc
    {
        MemberFunc() :proc(nullptr), obj(nullptr){}
        mf proc;
        X *obj;
        /// 仅仅为了增加计数，使一些临时类的生命周期与TFunction对象一样长
        std::shared_ptr<void> objShared;
    }memFunc;
public:
    typedef R RetType;
public:
    TFunction() :m_func(nullptr), m_bMemFunc(false){}

    TFunction(const pfn &f) : m_func(f), m_bMemFunc(false)
    {}
    /// 支持隐式转换，函数对象可隐式转为TFunction类型
    /// TFunctionWrap可自动转为Closure类型
    template<typename F, typename = typename std::enable_if<
        base::conjunction<base::negation<std::is_function<F>>,
        base::negation<hasType_anyData<F>>>::value>::type>
    TFunction(const F &f) : m_func(f), m_bMemFunc(false)
    {
    }

    template<typename O>
    TFunction(R(O::*f)(P... args), O *p) : m_func(nullptr), m_bMemFunc(true)
    {
        memFunc.proc = reinterpret_cast<mf>(f);
        memFunc.obj = reinterpret_cast<X*>(p);
    }
    template<typename O>
    TFunction(R(O::*f)(P... args)const, O *p) : m_func(nullptr), m_bMemFunc(true)
    {
        memFunc.proc = reinterpret_cast<mf>(f);
        memFunc.obj = reinterpret_cast<X*>(p);
    }

    template<typename O>
    TFunction(R(O::*f)(P... args), std::shared_ptr<O> p) : m_func(nullptr), m_bMemFunc(true)
    {
        memFunc.proc = reinterpret_cast<mf>(f);
        memFunc.obj = reinterpret_cast<X*>(p.get());
        memFunc.objShared = p;
    }
    template<typename O>
    TFunction(R(O::*f)(P... args)const, std::shared_ptr<O> p) : m_func(nullptr), m_bMemFunc(true)
    {
        memFunc.proc = reinterpret_cast<mf>(f);
        memFunc.obj = reinterpret_cast<X*>(p.get());
        memFunc.objShared = p;
    }

    ~TFunction(){}

    /// 函数调用，按正常函数调用传参即可
    R operator() (P... args) const
    {
        if (m_bMemFunc)
        {
            return (memFunc.obj->*memFunc.proc)(std::forward<P>(args)...);
        }
        else
        {
            return m_func(std::forward<P>(args)...);
        }
    }
    bool operator==(const TFunction &ef) const
    {
        if (m_bMemFunc)
        {
            if (memFunc.proc == ef.memFunc.proc && memFunc.obj == ef.memFunc.obj)
            {
                return true;
            }
            return false;
        }
        else
        {
            auto a1 = m_func.template target<R(*)(P...)>();
            auto a2 = ef.getFunc1();
            if (a1 == nullptr && a2 == nullptr)
            {
                if (m_func.target_type() == ef.m_func.target_type())
                {
                    return true;
                }
                return false;
            }
            if (a1 == nullptr || a2 == nullptr)
            {
                return false;
            }
            if (*a1 == *a2)
            {
                return true;
            }
            return false;
        }
    }
    bool operator!=(const TFunction &ef) const
    {
        return !(*this == ef);
    }
    TFunction &operator=(const TFunction &ef)
    {
        if (this == &ef)
        {
            return *this;
        }
        if (ef.m_bMemFunc)
        {
            memFunc.proc = ef.memFunc.proc;
            memFunc.obj = ef.memFunc.obj;
            memFunc.objShared = ef.memFunc.objShared;
            m_bMemFunc = true;
        }
        else
        {
            m_func = ef.m_func;
            m_bMemFunc = false;
        }
        return *this;
    }
    TFunction &operator=(TFunction &&ef) noexcept
    {
        if (this == &ef)
        {
            return *this;
        }
        if (ef.m_bMemFunc)
        {
            memFunc.proc = ef.memFunc.proc;
            memFunc.obj = ef.memFunc.obj;
            memFunc.objShared = std::move(ef.memFunc.objShared);
            m_bMemFunc = true;
            ef.memFunc.proc = nullptr;
            ef.memFunc.obj = nullptr;
        }
        else
        {
            m_func = std::move(ef.m_func);
            m_bMemFunc = false;
        }
        return *this;
    }
    TFunction(const TFunction &ef)
    {
        *this = ef;
    }
    TFunction(TFunction &&ef) noexcept
    {
        *this = ef;
    }
    const pf getFunc1() const
    {
        return m_func.template target<R(*)(P...)>();
    }
    explicit operator bool() const
    {
        if (m_bMemFunc)
        {
            if (nullptr == memFunc.proc || nullptr == memFunc.obj)
            {
                return false;
            }
        }
        else
        {
            if (nullptr == m_func)
            {
                return false;
            }
        }
        return true;
    }
    bool clean()
    {
        if (m_bMemFunc)
        {
            memFunc.proc = nullptr;
            memFunc.obj = nullptr;
            memFunc.objShared.reset();
        }
        else
        {
            m_func = nullptr;
        }
        return true;
    }

private:
    std::function<R(P...args)> m_func;
    bool m_bMemFunc;
};

template<int ...>
struct TIndexTuple{};

template<int N, int... Indexes>
struct MakeIndexes : MakeIndexes<N - 1, N - 1, Indexes...>{};

/// 递归继承结束条件, 同时也是对上面模板的一个偏特化
template<int... indexes>
struct MakeIndexes<0, indexes...>
{
    typedef TIndexTuple<indexes...> type;
};

/// TFunction封装类，TFunction只封装函数，并没有封装参数，调用时需传参执行
/// TFunctionWrap将TFunction与相应参数进行了封装，调用时不需要传参
/// 底层不支持右值为参数的函数
template<typename R, typename ...P>
class TFunctionWrap {
    typedef typename MakeIndexes<sizeof...(P)>::type Type;
    template<typename, typename...> friend class TFunctionWrap;
public:
    TFunctionWrap(){}
    template<typename F>
    TFunctionWrap(F &&func, const P& ...args)
        : m_args(args...)
        , m_func(std::forward<F>(func))
    {
    }
    ~TFunctionWrap(){}
    TFunctionWrap(const TFunctionWrap&) = default;
    TFunctionWrap(TFunctionWrap && rf) noexcept
        : m_args(std::move(rf.m_args))
        , m_func(std::move(rf.m_func))
    {
    }
    TFunctionWrap& operator=(const TFunctionWrap&) = default;
    TFunctionWrap& operator=(TFunctionWrap && rf) noexcept
    {
        if (this != &rf)
        {
            m_func = std::move(rf.m_func);
            m_args = std::move(rf.m_args);
        }
        return *this;
    }
    /// 函数调用
    R operator()()
    {
        return run(Type());
    }
    /// 设置参数
    template<typename F>
    void setArgs(F &&func, const P& ... args){m_func = std::forward<F>(func); m_args = std::move(std::make_tuple(args...));}
    const TFunction<R, P...> &getTFunc()
    {
        return m_func;
    }
private:
    template <int ...index>
    R run(TIndexTuple<index...>) const
    {
        return m_func(std::get<index>(m_args)...);
    }
private:
    std::tuple<P...> m_args;
    TFunction<R, P...> m_func;
};

#endif /* ifndef __FRAMEWORK_INCLUDE_FUNCTION_FUNCTION_H__ */
