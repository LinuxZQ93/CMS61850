/**
 * @file ICMS61850.h
 * @author (linuxzq93@163.com)
 * @brief cms61850组件基类文件
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_ICMS61850_H__
#define __CMS61850_ICMS61850_H__

#include "Component/IUnknown.h"
#include "Component/ComponentMacroDef.h"
#include "json/json.h"

namespace cms {

class ICMS61850 : public base::IUnknown {
SIMPLE_DEF_I(CMS61850, "CMS51850")

public:
    virtual bool init() { return true; }
    virtual bool start() { return true; }
    virtual bool stop() { return true; }
    virtual bool destroy() { return true; }
};

}

#endif /* __CMS61850_ICMS61850_H__ */
