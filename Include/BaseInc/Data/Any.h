/**
*@file Any.h
*@brief 可绑定任意数据类型，更方便抽象化数据的编写。参考llvm编写
*
*Version:1.0
*
*Date:2022/04
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_TRAITS_H__
#define __FRAMEWORK_INCLUDE_TRAITS_H__

#include <memory>
#include <typeinfo>
#include <string>
#include <cassert>
#include "Traits/Traits.h"
#include "Print/Print.h"

namespace base {

class any {
    struct StorageBase
    {
        virtual ~StorageBase() = default;
        virtual std::unique_ptr<StorageBase> clone() const = 0;
        virtual std::string typeName() const = 0;
    };

    template <typename T>
    struct StorageImpl : public StorageBase
    {
        explicit StorageImpl(const T &Value) : Value(Value) {}

        explicit StorageImpl(T &&Value) : Value(std::move(Value)) {}

        std::unique_ptr<StorageBase> clone() const override
        {
            return std::unique_ptr<StorageImpl<T>>(new StorageImpl<T>(Value));
        }

        std::string typeName() const override
        {
            return typeid(T).name();
        }

        T Value;

    private:
        StorageImpl &operator=(const StorageImpl &Other) = delete;
        StorageImpl(const StorageImpl &Other) = delete;
    };
public:
    // 为兼容TFunction,加类型检测
    typedef void anyData;
    any() = default;

    any(const any &Other)
        : Storage(Other.Storage ? Other.Storage->clone() : nullptr) {}

    // When T is Any or T is not copy-constructible we need to explicitly disable
    // the forwarding constructor so that the copy constructor gets selected
    // instead.
    template <
        typename T,
        typename = typename std::enable_if<
            base::conjunction<
                base::negation<std::is_same<typename std::decay<T>::type, any>>,
                // We also disable this overload when an `Any` object can be
                // converted to the parameter type because in that case, this
                // constructor may combine with that conversion during overload
                // resolution for determining copy constructibility, and then
                // when we try to determine copy constructibility below we may
                // infinitely recurse. This is being evaluated by the standards
                // committee as a potential DR in `std::any` as well, but we're
                // going ahead and adopting it to work-around usage of `Any` with
                // types that need to be implicitly convertible from an `Any`.
                base::negation<std::is_convertible<any, typename std::decay<T>::type>>,
                std::is_copy_constructible<typename std::decay<T>::type>>::value>::type>
    any(T &&Value)
    {
        using U = typename std::decay<T>::type;
        Storage.reset(new StorageImpl<U>(std::forward<T>(Value)));
    }

    any(any &&Other) : Storage(std::move(Other.Storage)) {}

    any &swap(any &Other)
    {
        std::swap(Storage, Other.Storage);
        return *this;
    }

    any &operator=(any Other)
    {
        Storage = std::move(Other.Storage);
        return *this;
    }

    bool empty() const { return Storage == nullptr; }

    std::string typeName() const { return (Storage ? Storage->typeName() : typeid(void).name());}

    template<typename T>
    bool checkType() const
    {
        const std::string &info = typeName();
        const std::string &castInfo = typeid(T).name();
        bool ret = (info == castInfo);
        if (!ret)
        {
            errorf("any type[%s], cast type[%s], not same!!!\n", info.c_str(), castInfo.c_str());
        }
        return ret;
    }

    void reset() { Storage.reset(); }

    /// this不为const时调用，可对内部数据进行修改等操作，慎用
    /// 考虑性能问题，不利用RTTI检查类型，本来C++就是强类型语言，转成的类型正确性让用户保证
    /// 转错误的类型，可能会引发死机或其它未知问题，如有需要用户可自行调用checkType进行类型校验
    template<typename T>
    T cast() {
        if (empty()){
            errorf("bad cast, empty value\n");
            assert(0);
        }
#if 0
        if (!checkType<T>()) {
            assert(0);
        }
#endif
        using U = typename std::decay<T>::type;
        return static_cast<any::StorageImpl<U> *>(Storage.get())->Value;
    }

    /// 都可以调用，对内部存储进行保护，建议传const T&提高效率。传T时也可以，只是多了层拷贝
    template<typename T, typename = typename std::enable_if<
        base::either<std::is_const<typename std::remove_reference<T>::type>,
        base::negation<std::is_reference<T>>>::value>::type>
    T castConst() const {
        if (empty()){
            errorf("bad cast, empty value\n");
            assert(0);
        }
#if 0
        if (!checkType<T>()) {
            assert(0);
        }
#endif
        using U = typename std::decay<T>::type;
        return static_cast<any::StorageImpl<U> *>(Storage.get())->Value;
    }

private:
    std::unique_ptr<StorageBase> Storage;
};

}

#endif /* __FRAMEWORK_INCLUDE_TRAITS_H__ */
