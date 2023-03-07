/**
 * @file ServiceManager.h
 * @author (linuxzq93@163.com)
 * @brief cms61850服务的主处理部分，完成底层业务函数的回调的注册及调用
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_SERVICE_SERVICEMANAGER_H__
#define __CMS61850_SERVICE_SERVICEMANAGER_H__

#include "SingTon/SingTon.h"
#include "IService.h"
#include <map>
#include <string>

#include "Function/Bind.h"
#include "APER.h"
#include "asn1/ServiceError.h"
#include "Net/TcpServer.h"
#include "json/value.h"

namespace cms {

class CServiceManager {
SINGTON_DECLAR(CServiceManager);

public:
    using Func = TFunction<ServiceError, const std::string&, const NetMessage&, std::string&>;

    enum Status {
        NegotiateFail,
        AssociateFail,
        SUCCESS
    };

public:
    bool init();
    bool start();
    void setTcpServer(const std::shared_ptr<base::CTcpServer> &pServer) { m_tcpServer = pServer; }
    bool getClientInfo(int clientId, base::clientInfo &info) { return m_tcpServer->getClientInfo(clientId, info); }
    bool closeClient(int clientId) { return m_tcpServer->closeClient(clientId); }
    void setStatus(int clientId, Status status) { m_mapStatus[clientId] = status; }
    bool attachService(const std::string &name, IService *pSrv);
    bool attachFunc(int code, const std::string &name, const Func &func);

    /// 0 success -1 failure -2 invalid
    int dealPDU(int code, int clientId, const uint8_t *buf, int len, std::string &response);

private:
    std::map<std::string, IService*> m_mapSrv;
    std::map<int, std::pair<std::string, Func>> m_mapFunc;
    std::map<int, Status> m_mapStatus;
    std::shared_ptr<base::CTcpServer> m_tcpServer;
    int m_errNum;
    int m_apduSize;
    int m_asduSize;
};

}

#endif /* __CMS61850_SERVICE_SERVICEMANAGER_H__ */
