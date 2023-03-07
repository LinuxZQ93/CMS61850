/**
 * @file ServerData.h
 * @author (linuxzq93@163.com)
 * @brief 服务数据类
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_SERVICE_DATA_SERVERDATA_H__
#define __CMS61850_SERVICE_DATA_SERVERDATA_H__

#include "Service/IService.h"

#include "SCL/SCLParse.h"

namespace cms {

class CServerData : public IService {
public:
    CServerData();
    virtual ~CServerData();

public:
    virtual bool init();
    virtual bool start(){return true;}

private:
    void doNormal(void *strPtr, void *leafInfo);
    void doStruct(void *strPtr, void *vecInfo);
    void doAllValue(void *respPtr, void *leafInfo);
    ServiceError getDataValue(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError setDataValue(const std::string &name, const NetMessage &message, std::string &response);
    void doDefStruct(const std::string &fc, void *strPtr, void *vecInfo);
    void doAllDef(const std::string &fc, void *respPtr, void *leafInfo);
    ServiceError getDataDir(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError getDataDef(const std::string &name, const NetMessage &message, std::string &response);

};

}

#endif /* __CMS61850_SERVICE_DATA_SERVERDATA_H__ */
