/**
 * @file CMS61850.h
 * @author (linuxzq93@163.com)
 * @brief 61850主处理部分，主要实现TcpServer，完成数据的拆解与组装
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_CMS61850_H__
#define __CMS61850_CMS61850_H__

#include "ICMS61850.h"

#include <memory>

#include "Net/TcpServer.h"
#include "Config/IConfigManager.h"

namespace cms {

class CCMSMessageHandle : public base::ITcpServerHandleMessage {
public:
    CCMSMessageHandle();
    virtual ~CCMSMessageHandle();

public:
    virtual void handleMessage(int clientId, const char *buf, int len, std::string &response);
    virtual void clientConnected(const std::string &ip, int port, int clientId);
    virtual void clientDisconnect(const std::string &ip, int port, int clientId);

private:
    void dealAPDU(int clientId, const uint8_t *buf, int len, std::string &response);
    bool packFrame(int clientId, const char *buf, int len, std::string &response);
    bool spliteASDU(int clientId, const std::string &head, const std::string &content);

private:
    int m_asduSize;
    std::string m_packString;
    std::map<int, std::string> m_mapFrame;
};

class CCMS61850 : public ICMS61850 {
SIMPLE_DEF_C(CMS61850, "CMS61850")

public:
    virtual bool init();
    virtual bool start();

private:
    std::shared_ptr<base::CTcpServer> m_pTcpServer;
    std::shared_ptr<CCMSMessageHandle> m_pHandleMsg;
    base::IConfigManager *m_pConfig;
    Json::Value m_cfgValue;
};

}

#endif /* __CMS61850_CMS61850_H__ */
