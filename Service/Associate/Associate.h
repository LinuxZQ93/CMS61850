/**
 * @file Associate.h
 * @author (linuxzq93@163.com)
 * @brief 协商类服务
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMSTEST_SERVICE_ASSOCIATE_ASSOCIATE_H__
#define __CMSTEST_SERVICE_ASSOCIATE_ASSOCIATE_H__

#include "Service/IService.h"
#include <stdint.h>
#include <string>
#include <map>
#include <mutex>
#include "Net/TcpServer.h"
#include "Thread/Thread.h"
#include "Time/Time.h"
#include "json/json.h"

namespace cms {

class CAssociate : public IService {
enum Status {
    NegotiateFail,
    Negotiate,
    AssociateFail,
    Associate,
    Release,
    Abort,
    Unknown
};

struct CertContent {
    /// 一般证书长度都小于1K吧
    uint8_t buf[1024];
    size_t len;
};

struct ApplicationSec {
    CertContent rootCA;
    CertContent serverCert;
};

struct AssociateInfo {
    base::clientInfo clientInfo;
    Status status;
    int num;
    uint64_t aliveTime;
    AssociateInfo() : status(Unknown), num(0){}
};
public:
    CAssociate();
    virtual ~CAssociate();

public:
    virtual bool init();
    virtual bool start();

private:
    bool beforeAssociate(int clientId, std::string &response);
    void sendAbort(std::string &response);
    void cleanTCP();
    void doSocketEvt(const std::string &code, const Json::Value &cfgValue);
    ServiceError associateNegotiate(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError associate(const std::string &name, const NetMessage &message, std::string &response);
    bool verifySoftSafe(void *ptr);
    bool setSoftSafeAuth(void *ptr);
    ServiceError release(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError abort(const std::string &name, const NetMessage &message, std::string &response);
    ServiceError test(const std::string &name, const NetMessage &message, std::string &response);

private:
    std::string m_stationId;
    std::mutex m_mutex;
    std::map<int, std::shared_ptr<AssociateInfo>> m_mapTcpInfo;
    base::ThreadWrap<void> m_threadRes;
    std::unique_ptr<ApplicationSec> m_pAppSec;
    Json::Value m_cfgValue;
};

}

#endif /* __CMSTEST_SERVICE_ASSOCIATE_ASSOCIATE_H__ */
