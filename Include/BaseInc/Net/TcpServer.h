/// 这个功能就准备为网络打印设计，简单写写，不写跨平台了
/// 后期要是有udp，以及跨平台的需求在本框架开发的话，再考虑
#ifndef __FRAMEWORK_INCLUDE_NET_TCPSERVER_H__
#define __FRAMEWORK_INCLUDE_NET_TCPSERVER_H__

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>

#include "Thread/Thread.h"
#include "IHandleMessage.h"

namespace base {

struct clientInfo {
    std::string ip;
    int port;
};

class CTcpServer {

    static const int sm_maxRecv;
public:
    CTcpServer(const std::string &ip, int port, const std::shared_ptr<ITcpServerHandleMessage> &Ihandle);
    ~CTcpServer();

public:
    void setConnectNum(int num);
    bool start();
    bool stop();
    bool notifyAll(const char *buf, int len);
    bool notify(int clientId, const char *buf, int len);
    bool closeClient(int clientId);
    bool getClientInfo(int clientId, clientInfo &info);

private:
    bool initServer();
    void listenLoop();
    void handleRequest(fd_set &fs, char *buf);
    void checkNewClient(fd_set &fs, int &maxFd);
    void onConsole(const std::vector<std::string> &argList);

private:
    int m_maxConnectNum;
    std::string m_strIP;
    int m_port;
    int m_socketFd;
    struct sockaddr_in m_addrIn;
    bool m_bStart;
    base::ThreadWrap<void> m_thread;
    std::mutex m_mutexClient;
    std::map<int, clientInfo> m_mapClient;
    std::shared_ptr<ITcpServerHandleMessage> m_pHandle;
    std::string m_strCmdName;
};

}

#endif /* __FRAMEWORK_INCLUDE_NET_SERVER_H__ */
