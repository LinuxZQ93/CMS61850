/**
 * @file ServerNode.h
 * @author (linuxzq93@163.com)
 * @brief 服务节点类
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_SERVICE_NODE_SERVERNODE_H__
#define __CMS61850_SERVICE_NODE_SERVERNODE_H__

#include "Service/IService.h"
#include <string>
#include <map>
#include "SCL/SCLParse.h"
#include "Function/Bind.h"

namespace cms {

class CServerNode : public IService {
public:
    CServerNode();
    ~CServerNode();

public:
    virtual bool init();
    virtual bool start(){return true;}

private:
    ServiceError getServerDir(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError getLdDir(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError getLnDir(const std::string &name, const NetMessage &message, std::string &response);
    bool getDataObject(void *ptr, const std::string &reName);
    bool getDataSet(void *ptr, const std::string &reName);
    bool getBRCB(void *ptr, const std::string &reName);
    bool getURCB(void *ptr, const std::string &reName);
    bool getLCB(void *ptr, const std::string &reName);
    bool getSetting(void *ptr, const std::string &reName);
    ServiceError getAllDataValues(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError getAllDataDef(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError getAllCBValues(const std::string &name, const NetMessage &message, std::string &response);

private:
    bool parseAllValues(const std::string &fc, void *pResponse, void *domInfo);
    bool parseValueStruct(const std::string &fc, void *pStruct, void *vecInfo);
    bool parseDoAllDef(const std::string &fc, void *pResponse, std::vector<CSCLParse::LnInfo> &lnInfo);
    bool parseStruct(const std::string &fc, void *pStruct, const std::vector<CSCLParse::DataAttr> &da);

private:
    std::map<int, TFunction<bool, void*, const std::string&>> m_mapAcsiFunc;
};

}

#endif /* __CMS61850_SERVICE_NODE_SERVERNODE_H__ */
