/**
*@file IRpcServer.h
*@brief json rpc server组件
*
*Version:1.0
*
*Date:2020/08
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_JSONRPC_IRPCSERVER_H__
#define __FRAMEWORK_INCLUDE_JSONRPC_IRPCSERVER_H__

#include "Component/IUnknown.h"

#include <map>
#include <string>

#include "Function/Function.h"
#include "json/json.h"

namespace rpc {

class IRpcServer : public base::IUnknown {
public:
	/// json的类型，分为对象及数组
    typedef enum {OBJECT, ARRAY} JsonType;
	/// json参数的具体类型
    enum JsonParam_t
    {
        JSON_STRING = 1, ///< string
        JSON_BOOLEAN = 2, ///< bool
        JSON_INTEGER = 3, ///< int
        JSON_REAL = 4, ///< double
        JSON_OBJECT = 5, ///< object
        JSON_ARRAY = 6 ///< array
    };
    struct jsonCompleteType
    {
        JsonType jType;
        std::map<std::string, JsonParam_t> mapParamType;
    };
    typedef TFunction<void, const Json::Value&, Json::Value&> Func;
    typedef TFunction<void, const Json::Value&> FuncNotify;
public:
    class IRpcServerFactory : public base::IUnknown::IFactory
    {
    };
    virtual const std::string getClisd()
    {
        return "RPCServer";
    }
public:
    virtual bool init(){return true;}
    virtual bool start(){return true;}
    virtual bool stop(){return true;}
    virtual bool destroy(){return true;}

public:
	/**
	*@brief 向rpcserver注册回调函数
	*web传入相应参数后会调用此函数
	*@param funcName 函数名，web调用的名称
	*@param type 传入json的类型
	*@param func 回调函数
	*@return 注册是否成功
	*/
    virtual bool attachFunc(const std::string &funcName, const jsonCompleteType &type, const Func &func){return true;}
	/// 与前述函数一致，区别该函数为注册通知函数，不需要返回结果给调用端
    virtual bool attachNotifyFunc(const std::string &funcName, const jsonCompleteType &type, const FuncNotify &func){return true;}
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_JSONRPC_IRPCSERVER_H__ */

