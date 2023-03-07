/**
*@file ComponentMacroDef.h
*@brief 生成组件接口与实现的简单宏
*
*Version:1.0
*
*Date:2020/11
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_COMPONENT_COMPONENTMACRODEF_H__
#define __FRAMEWORK_INCLUDE_COMPONENT_COMPONENTMACRODEF_H__

#include "IUnknown.h"
#include "SingTon/SingTon.h"

#undef SIMPLE_DEF_I
#undef SIMPLE_DEF_C

/**
 *@brief 生成接口组件
 *@param name 组件名，单纯名称，也就是去掉I的名称
 *比如IConfigManager组件，那么name就需要填写ConfigManager
 *@param clisd 组件名称
 *Sample
 *SIMPLE_DEF_I(ConfigManager, "ConfigManager")

 *NOTE 末尾不需要加分号
*/
#define SIMPLE_DEF_I(name, clisd) \
public:\
    class I##name##Factory : public base::IUnknown::IFactory \
    {\
    };\
    virtual const std::string getClisd()\
    {\
        return clisd;\
    }

/**
 *@brief 生成实现组件
 *@param name 组件名，单纯名称，也就是去掉C的名称
 *比如CConfigManager组件，那么name就需要填写ConfigManager
 *@param clisd 组件名称
 *Sample
 *SIMPLE_DEF_C(ConfigManager, "ConfigManager")

 *NOTE 末尾不需要加分号
*/
#define SIMPLE_DEF_C(name, clisd) \
    SINGTON_DECLAR(C##name);\
public:\
    class C##name##ComponentFactory : public I##name::I##name##Factory\
    {\
    public:\
        C##name##ComponentFactory()\
        {\
            m_component = C##name::instance();\
            if (!registerComponent())\
            {\
                errorf("component[%s] register failed\n", C##name::name().c_str());\
            }\
        }\
        ~C##name##ComponentFactory()\
        {\
            unRegisterComponent();\
        }\
    };\
\
public:\
    static const std::string name()\
    {\
        return clisd;\
    }\
    virtual const std::string getClisd()\
    {\
        return name();\
    }

#endif /* ifndef __FRAMEWORK_INCLUDE_COMPONENT_COMPONENTMACRODEF_H__ */

