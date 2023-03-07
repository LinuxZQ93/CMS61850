/**
*@file ComponentManager.h
*@brief 组件管理类，承接组件的注册管理以及获取
*
*Version:1.0
*
*Date:2020/07
*
*@author linuxzq93@163.com
*/
#ifndef __FRAMEWORK_INCLUDE_COMPONENT_COMPONENTMANAGER_H__
#define __FRAMEWORK_INCLUDE_COMPONENT_COMPONENTMANAGER_H__

#include <map>
#include <mutex>

#include "SingTon/SingTon.h"
#include "Print/Print.h"

namespace base {

class IUnknown;
class CComponentManager {
    SINGTON_DECLAR(CComponentManager);
    friend class IUnknown;
public:
    bool init();
    bool start();
    bool stop();
    bool destroy();

    /// 获取组件,用户调用
	template<typename T>
	T * getComponent(const std::string &clisd)
    {
        std::lock_guard<std::mutex> Guard(m_mutex);
        if (m_mapComponent.find(clisd) != m_mapComponent.end())
        {
            return dynamic_cast<T*>(m_mapComponent[clisd]);
        }
        return nullptr;
    }

    std::list<std::string> getAllComponentClisd();
private:
    /// 注册组件,用户不需要直接调用
	bool registerComponent(const std::string &clisd, IUnknown *iu);
    /// 卸载组件,用户不需要直接调用
	bool unRegisterComponent(const std::string &clisd);

private:
	std::mutex m_mutex;
	std::map<std::string, IUnknown*> m_mapComponent;
    /// 存储有效需要启动的组件
	std::map<int, IUnknown*> m_mapComponentValid;
    int m_minPrio;
    int m_maxPrio;
    std::map<int, std::string> m_componentPrio;
};
}

#endif /* ifndef __FRAMEWORK_INCLUDE_COMPONENT_COMPONENTMANAGER_H__ */

