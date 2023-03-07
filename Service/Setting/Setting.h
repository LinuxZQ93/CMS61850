/**
 * @file Setting.h
 * @author (linuxzq93@163.com)
 * @brief 定值组类业务
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_SERVICE_SETTING_H__
#define __CMS61850_SERVICE_SETTING_H__

#include "Service/IService.h"

namespace cms {

class CSetting : public IService {
public:
    CSetting();
    virtual ~CSetting();

public:
    virtual bool init();
    virtual bool start(){return true;}

private:
    ServiceError getSGCBValues(const std::string &funcName, const NetMessage &message, std::string &response);
};

}

#endif /* __CMS61850_SERVICE_SETTING_H__ */
