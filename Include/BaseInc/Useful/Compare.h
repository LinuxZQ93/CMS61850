/**
*@file Compare.h
*@brief 一些方便的定义
*
*Version:1.0
*
*Date:2022/05
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_USEFUL_COMPARE_H__
#define __FRAMEWORK_INCLUDE_USEFUL_COMPARE_H__

namespace base {

/// crtp，节省一些符号的定义
template<typename Derived>
class EqualityComparable {
public:
    friend bool operator!=(Derived const &x1, Derived const &x2)
    {
        return !(x1 == x2);
    }
};

}

#endif /* __FRAMEWORK_INCLUDE_USEFUL_COMPARE_H__ */
