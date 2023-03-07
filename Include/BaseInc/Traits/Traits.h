/**
*@file Traits.h
*@brief 定义一些方便的traits，方便模板的书写
*
*Version:1.0
*
*Date:2022/04
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_TRAITS_TRAITS_H__
#define __FRAMEWORK_INCLUDE_TRAITS_TRAITS_H__

#include <type_traits>
#include <algorithm>

namespace base {

/// 取反
template <typename T>
struct negation : std::integral_constant<bool, !bool(T::value)> {};

/// 结合，当所有条件成立，则最后返回true,否则及时返回false
template <typename...> struct conjunction : std::true_type {};
template <typename B1> struct conjunction<B1> : B1 {};
template <typename B1, typename... Bn>
struct conjunction<B1, Bn...>
    : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};

/// 或者，当其中一个条件满足就返回true,否则返回false
template <typename...> struct either : std::false_type {};
template <typename B1> struct either<B1> : B1 {};
template <typename B1, typename... Bn>
struct either<B1, Bn...>
    : std::conditional<bool(B1::value), B1, either<Bn...>>::type {};

template <typename... T> struct make_void { using type = void; };
template <typename... T> using void_t = typename make_void<T...>::type;

#define HAS_TYPE_MEMBER(typeM) \
template <typename, typename = base::void_t<>> \
struct hasType_##typeM : std::false_type {}; \
template <typename T>\
struct hasType_##typeM<T, typename base::void_t<typename T::typeM>> : std::true_type{};

template <typename Container, typename value>
bool erase(Container &C, const value &P) {
auto iter = std::remove(C.begin(), C.end(), P);
if (iter == C.end()) {
    return false;
}
C.erase(iter, C.end());
return true;
}
template <typename Container, typename UnaryPredicate>
bool erase_if(Container &C, UnaryPredicate P) {
auto iter = std::remove_if(C.begin(), C.end(), P);
if (iter == C.end()) {
    return false;
}
C.erase(iter, C.end());
return true;
}

}
#endif /* __FRAMEWORK_INCLUDE_TRAITS_TRAITS_H__ */
