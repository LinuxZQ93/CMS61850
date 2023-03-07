/**
 * @file DataSet.h
 * @author (linuxzq93@163.com)
 * @brief 数据集类服务
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_SERVICE_DATA_DATASET_H__
#define __CMS61850_SERVICE_DATA_DATASET_H__

#include "Service/IService.h"
#include <string>
#include <vector>

namespace cms {

class CDataSet : public IService {
public:
    CDataSet();
    ~CDataSet();

public:
    virtual bool init();
    virtual bool start(){return true;}

private:
    ServiceError getDSValueDir(const std::string &funcName, const NetMessage &message, std::string &response);
    bool getDSValueDirBegin(void *ptr, const std::string &ref, const std::vector<std::string> &result);
    bool getDSValueDirSpec(void *ptr, const std::string &ref, const std::string &specName, const std::vector<std::string> &result);
    ServiceError getDSValues(const std::string &funcName, const NetMessage &message, std::string &response);
    void getDSStruct(void *ptr, void *vecInfo);
};

}

#endif /* __CMS61850_SERVICE_DATA_DATASET_H__ */
