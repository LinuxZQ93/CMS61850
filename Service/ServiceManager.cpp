#include "ServiceManager.h"

#include "Print/Print.h"

#include "Common.h"
#include "Config/IConfigManager.h"

namespace cms {

SINGTON_DEFINITION(CServiceManager)

CServiceManager::CServiceManager()
    : m_errNum(0)
    , m_apduSize(65535)
    , m_asduSize(65530)
{}

CServiceManager::~CServiceManager()
{}

bool CServiceManager::init()
{
    for (const auto &iter : m_mapSrv)
    {
        if (!iter.second->init())
        {
            errorf("%s service init failed\n", iter.first.c_str());
            return false;
        }
    }
    base::IConfigManager *pConfig = base::CComponentManager::instance()->getComponent<base::IConfigManager>("ConfigManager");
    Json::Value cfgValue;
    pConfig->getConfig("CMS61850", cfgValue);
    m_errNum = cfgValue["errorNum"].asInt();
    m_apduSize = cfgValue["associate"]["apduSize"].asInt();
    m_asduSize = cfgValue["associate"]["asduSize"].asInt();
    return true;
}

bool CServiceManager::start()
{
    for (const auto &iter : m_mapSrv)
    {
        if (!iter.second->start())
        {
            errorf("%s service init failed\n", iter.first.c_str());
            return false;
        }
    }
    return true;
}

bool CServiceManager::attachService(const std::string &name, IService *pSrv)
{
    if (m_mapSrv.find(name) != m_mapSrv.end())
    {
        errorf("service[%s] already attach\n", name.c_str());
        return false;
    }
    m_mapSrv[name] = pSrv;
    return true;
}

bool CServiceManager::attachFunc(int code, const std::string &name, const Func &func)
{
    if (m_mapFunc.find(code) != m_mapFunc.end())
    {
        errorf("code[%d] already attach\n", code);
        return false;
    }
    m_mapFunc[code].first = name;
    m_mapFunc[code].second = func;
    return true;
}

int CServiceManager::dealPDU(int code, int clientId, const uint8_t *buf, int len, std::string &response)
{
    static std::map<int, int> s_mapNum;
    if (s_mapNum.find(clientId) == s_mapNum.end())
    {
        s_mapNum[clientId] = 0;
    }
    else
    {
        if (s_mapNum[clientId] > m_errNum)
        {
            warnf("errnum max %d, close\n", m_errNum);
            CServiceManager::instance()->closeClient(clientId);
            return -2;
        }
    }
    if (m_mapFunc.find(code) != m_mapFunc.end())
    {
        if (m_mapStatus.find(clientId) != m_mapStatus.end())
        {
            Status status = m_mapStatus[clientId];
            if (status != SUCCESS && code != 154 && code != 1)
            {
                warnf("please negotiate or associate first\n");
                return -2;
            }
            else if (status == SUCCESS)
            {
                if (len > m_apduSize || len > m_asduSize)
                {
                    return -1;
                }
            }
        }
        if (code == 2)
        {
            int reqId = buf[5] * 256 + buf[4];
            if (reqId != 0)
            {
                return -1;
            }
        }
        int proType = buf[0] & 0x0f;
        auto &tmp = m_mapFunc[code];
        NetMessage message;
        message.buf = &buf[6];
        message.clientId = clientId;
        message.len = len;
        infof("enter %s\n", tmp.first.c_str());
        ServiceError ret = tmp.second(tmp.first, message, response);
        infof("leave %s, ret is %d\n", tmp.first.c_str(), ret);
        if (ret != ServiceError_no_error && ret != ServiceError_other && ret != ServiceError_decode_error)
        {
            encodeErr(ret, response);
        }
        if (ret == ServiceError_decode_error || proType != 0x01 || len > 65531 || len < -2)
        {
            s_mapNum[clientId]++;
        }
        else
        {
            s_mapNum[clientId] = 0;
        }
        return ret == ServiceError_no_error ? 0 : -1;
    }
    else
    {
        s_mapNum[clientId]++;
    }
    warnf("service code[%d] not support\n", code);
    return -2;
}

}
