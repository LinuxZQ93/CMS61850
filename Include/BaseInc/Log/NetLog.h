/// TCP服务端，暂且适配网络日志的
#ifndef __FRAMEWORK_INCLUDE_LOG_NETLOG_H__
#define __FRAMEWORK_INCLUDE_LOG_NETLOG_H__

#include "LogBase.h"

#include "SingTon/SingTon.h"
#include "Net/IHandleMessage.h"
#include "Net/TcpServer.h"
#include "Thread/Thread.h"

#include "Config/IConfigManager.h"

namespace base {

class CNetLogMessageHandle : public ITcpServerHandleMessage {
public:
    CNetLogMessageHandle();
    virtual ~CNetLogMessageHandle();

public:
    virtual void handleMessage(int clientId, const char *buf, int len, std::string &respons);
    virtual void setFirstMessage(std::string &respons);
    virtual void stop();

private:
    int m_attachNum;
    base::IConfigManager *m_pConfig;
};

class CNetLogMessageHandleColor : public ITcpServerHandleMessage {
public:
    CNetLogMessageHandleColor();
    virtual ~CNetLogMessageHandleColor();

public:
    virtual void handleMessage(int clientId, const char *buf, int len, std::string &respons);
    virtual void setFirstMessage(std::string &respons);
    virtual void stop();

private:
    int m_attachNum;
};

class CNetLog : public CLogBase {
SINGTON_DECLAR(CNetLog)
public:
    virtual bool init();
    bool destroy();

public:
    virtual void saveLog(const char *buf, int len);
    void saveLogWithoutColor(const char *buf, int len);

private:
    void saveLogInternal(const char *buf, int len);
    void saveLogWithoutColorInternal(std::string buf);
    bool createNet();

private:
    std::shared_ptr<CTcpServer> m_pServerColor;
    std::shared_ptr<CTcpServer> m_pServer;
    base::ThreadPool<void> m_threadPool;

};

}

#endif /* __FRAMEWORK_INCLUDE_LOG_NETLOG_H__ */
