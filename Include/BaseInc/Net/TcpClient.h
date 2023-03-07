#ifndef __FRAMEWORK_INCLUDE_NET_TCPCLIENT_H__
#define __FRAMEWORK_INCLUDE_NET_TCPCLIENT_H__

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include <memory>

#include "IHandleMessage.h"
#include "Thread/Thread.h"

namespace base {

class CTcpClient {
    static const int sm_maxRecv;
public:
    CTcpClient(const std::string &ip, int port, const std::shared_ptr<IHandleMessage> &Ihandle);
    ~CTcpClient();

public:
    bool start();
    bool stop();

private:
    bool connect();
    void loop();
    void handleRequest(fd_set &fs, char *buf);

private:
    std::string m_strIP;
    int m_port;
    int m_socketFd;
    std::shared_ptr<IHandleMessage> m_pHandle;
    base::ThreadWrap<void> m_thread;
    bool m_bStart;
};

}

#endif /* __FRAMEWORK_INCLUDE_NET_CLIENT_H__ */
