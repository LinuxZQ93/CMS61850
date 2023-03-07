/**
*@file IConfigManager.h
*@brief 配置管理组件
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_CONFIG_ICONFIGMANAGER_H__
#define __FRAMEWORK_INCLUDE_CONFIG_ICONFIGMANAGER_H__

#include "Component/IUnknown.h"
#include "Component/ComponentMacroDef.h"

#include <map>
#include <string>

#include "json/json.h"

#include "Print/Print.h"
#include "Function/Signal.h"

namespace base {

class IConfigManager : public base::IUnknown {
public:
    typedef TFunction<bool, const Json::Value&> ProcCheck;
    typedef std::list<ProcCheck> SignalCheck;
    typedef TSignal<void, const Json::Value&> SignalChanged;
    typedef SignalChanged::Proc ProcChanged;
    SIMPLE_DEF_I(ConfigManager, "ConfigManager")
public:
    virtual bool init(){return true;}
    virtual bool start(){return true;}
    virtual bool stop(){return true;}
    virtual bool destroy(){return true;}

    /**
    *@brief 下发配置
    *将配置下发至配置中心管理
    *@param key 配置名
    *@param cfg 待配置
    *@return 下发是否成功
    */
    virtual bool setConfig(const std::string &key, const Json::Value &cfg) = 0;
    /**
    *@brief 获取配置
    *从配置中心获取配置
    *@param key 配置名
    *@param[out] cfg 返回配置结果
    *@return 获取是否成功
    */
    virtual bool getConfig(const std::string &key, Json::Value &cfg) = 0;

    /**
    *@brief 下发默认配置
    *将配置下发至默认配置中心管理
    *@param key 配置名
    *@param cfg 待配置
    *@return 下发是否成功
    */
    virtual bool setDefault(const std::string &key, const Json::Value &cfg) = 0;
    /**
    *@brief 获取默认配置
    *从默认配置中心获取配置
    *@param key 配置名
    *@param[out] cfg 返回配置结果
    *@return 获取是否成功
    */
    virtual bool getDefault(const std::string &key, Json::Value &cfg) = 0;

    /// 注册配置更改校验函数，用于校验配置是否正确
    virtual bool attachVerifyConfig(const std::string &key, const ProcCheck &func) = 0;
    virtual bool detachVerifyConfig(const std::string &key, const ProcCheck &func) = 0;
    /// 注册配置更新函数，用于配置更新后需要做的处理
    virtual bool attachChangedConfig(const std::string &key, const ProcChanged &func) = 0;
    virtual bool detachChangedConfig(const std::string &key, const ProcChanged &func) = 0;
    /// 解析json文件
    virtual bool parseFileConfig(const std::string &path, Json::Value &result) = 0;
    /// 注册配置更新函数，用于配置更新后需要做的处理
    virtual bool attachChangedConfig(const std::string &key, const ProcChanged &func, SigEnum::SlotPosition pos, const unsigned int specPos = 0) = 0;
};

}

#endif /* ifndef __FRAMEWORK_INCLUDE_CONFIG_ICONFIGMANAGER_H__ */
