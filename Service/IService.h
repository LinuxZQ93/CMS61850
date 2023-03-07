/**
 * @file IService.h
 * @author (linuxzq93@163.com)
 * @brief 服务基类，底层业务接口都需要继承此类
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_SERVICE_ISERVICE_H__
#define __CMS61850_SERVICE_ISERVICE_H__

#include "Common.h"

namespace cms {

struct NetMessage {
    int clientId;
    const uint8_t *buf;
    int len;
};

class CSCLParse;

class IService {
public:
    IService();
    virtual ~IService(){}

public:
    virtual bool init() = 0;
    virtual bool start() = 0;

protected:
    CSCLParse *m_scl;
};

}

#endif /* __CMS61850_SERVICE_ISERVICE_H__ */
