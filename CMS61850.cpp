#include "CMS61850.h"

#include "SCL/SCLParse.h"
#include "Service/ServiceManager.h"
#include "Event/IEventManager.h"
#include "Config/IConfigManager.h"

namespace cms {

SINGTON_DEFINITION(CCMS61850)

CCMSMessageHandle::CCMSMessageHandle()
    : m_asduSize(65530)
{
    auto *pConfig = base::CComponentManager::instance()->getComponent<base::IConfigManager>("ConfigManager");
    Json::Value cfgValue;
    pConfig->getConfig("CMS61850", cfgValue);
    m_asduSize = cfgValue["associate"]["asduSize"].asInt();
}

CCMSMessageHandle::~CCMSMessageHandle()
{}

void CCMSMessageHandle::handleMessage(int clientId, const char *buf, int len, std::string &response)
{
    #if 1
    infof("recv cms data begin\n");
    for (int i = 0; i < len; i++)
    {
        rawf("0x%.2x, ", buf[i]);
    }
    rawf("\n");
    infof("recv cms data end\n");
    #endif
    if (!packFrame(clientId, buf, len, response))
    {
        dealAPDU(clientId, (const uint8_t *)buf, len, response);
    }
}

void CCMSMessageHandle::clientConnected(const std::string &ip, int port, int clientId)
{
    infof("new client[%s:%d] connect cms, clientId[%d]\n", ip.c_str(), port, clientId);
}

void CCMSMessageHandle::clientDisconnect(const std::string &ip, int port, int clientId)
{
    infof("client[%s:%d] disconnect cms, clientId[%d]\n", ip.c_str(), port, clientId);
    /// 可能客户端会主动断开连接，需要事件通知内部业务
    auto *pEvent = base::CComponentManager::instance()->getComponent<base::IEventManager>("EventManager");
    Json::Value cfgValue;
    cfgValue["state"] = "disconnect";
    cfgValue["clientId"] = clientId;
    pEvent->sendEvent("cmsSocket", cfgValue);
}

bool CCMSMessageHandle::packFrame(int clientId, const char *buf, int len, std::string &response)
{
    uint8_t ctlCode = buf[0];
    int nextFlag = BIT_GET(&ctlCode, 0);
    if (nextFlag != 1)
    {
        infof("complete frame\n");
        if (m_mapFrame.find(clientId) != m_mapFrame.end())
        {
            infof("deal pack full frame\n");
            m_mapFrame[clientId] += buf[6];
            dealAPDU(clientId, (const uint8_t *)m_mapFrame[clientId].c_str(), m_mapFrame[clientId].length(), response);
            m_mapFrame.erase(clientId);
            return true;
        }
        return false;
    }
    infof("part frame\n");
    m_mapFrame[clientId] += buf[6];
    return true;
}

void CCMSMessageHandle::dealAPDU(int clientId, const uint8_t *buf, int len, std::string &response)
{
    int type = buf[1];
    int apduLen = buf[3] * 256 + buf[2];
    infof("type is %d, apduLen is %d\n", type, apduLen);
    std::string content;
    /// tcpServer单线程轮询，内部业务不需要加锁
    int ret = CServiceManager::instance()->dealPDU(type, clientId, buf, apduLen - 2, content);
    if (ret == -2)
    {
        return;
    }
    int length = content.length();
    std::string head;
    if (type != 153)
    {
        head.resize(6);
    }
    else
    {
        head.resize(4);
    }
    if (ret == 0)
    {
        head[0] = 0x41;
    }
    else
    {
        head[0] = 0x61;
    }
    head[1] = type;
    if (type != 153)
    {
        head[2] = (length + 2) & 0xFF;
        head[3] = (length + 2) >> 8;
        head[4] = buf[4];
        head[5] = buf[5];
    }
    else
    {
        head[2] = 0;
        head[3] = 0;
    }
    infof("cms response begin\n");
    /// 不需要分帧时，一帧出去
    if (!spliteASDU(clientId, head, content))
    {
        response = head + content;
    }
    printHex(response.c_str(), response.length());
    infof("cms response end\n");
}

bool CCMSMessageHandle::spliteASDU(int clientId, const std::string &hd, const std::string &ct)
{
    /// 检测要求至少大于95%的长度才分帧
    if (ct.length() < m_asduSize * 0.95)
    {
        return false;
    }
    int num = ct.length() /  m_asduSize;
    int remain = ct.length() % m_asduSize;
    int i = 0;
    infof("start splite frame\n");
    for (; i < num; i++)
    {
        std::string head(hd);
        if (remain != 0)
        {
            head[0] |= 0x80;
        }
        head[2] = (m_asduSize + 2) & 0xff;
        head[3] = (m_asduSize + 2) >> 8;
        const std::string &content = ct.substr(i * m_asduSize, m_asduSize);
        std::string response = head + content;
        printHex(response.c_str(), response.length());
        /// 分帧需要自己独立发送
        send(clientId, response.c_str(), response.length(), 0);
    }
    if (remain > 0)
    {
        std::string head(hd);
        head[2] = (remain + 2) & 0xff;
        head[3] = (remain + 2) >> 8;
        const std::string &content = ct.substr(i * m_asduSize);
        std::string response = head + content;
        printHex(response.c_str(), response.length());
        /// 分帧需要自己独立发送
        send(clientId, response.c_str(), response.length(), 0);
    }
    infof("end splite frame\n");
    return true;
}

CCMS61850::CCMS61850()
{}

CCMS61850::~CCMS61850()
{}

bool CCMS61850::init()
{
    setPrintLevel(getClisd(), DEBUG);
    m_pConfig = base::CComponentManager::instance()->getComponent<base::IConfigManager>("ConfigManager");
    m_pConfig->getConfig("CMS61850", m_cfgValue);
    int connectNum = m_cfgValue["connectNum"].asInt();
    int port = m_cfgValue["port"].asInt();
    m_pHandleMsg.reset(new CCMSMessageHandle());
    m_pTcpServer.reset(new base::CTcpServer("0.0.0.0", port, m_pHandleMsg));
    m_pTcpServer->setConnectNum(connectNum);
    if (!CSCLParse::instance()->init())
    {
        return false;
    }
    if (!CServiceManager::instance()->init())
    {
        return false;
    }
    CServiceManager::instance()->setTcpServer(m_pTcpServer);
    return true;
}

bool CCMS61850::start()
{
    CServiceManager::instance()->start();
    m_pTcpServer->start();
    return true;
}

extern "C"
{
    void initComponent()
    {
        static cms::CCMS61850::CCMS61850ComponentFactory s_cms;
    }
}

}
