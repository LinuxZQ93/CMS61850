#ifndef __FRAMEWORK_INCLUDE_AUTHENTICATION_AUTHENTICATION_H__
#define __FRAMEWORK_INCLUDE_AUTHENTICATION_AUTHENTICATION_H__

#include <string>
#include <map>
#include "json/json.h"

#include "SingTon/SingTon.h"
#include "Timer/Timer.h"
#include "Config/IConfigManager.h"

namespace base {

class CAesEncryptor {
public:
    static std::string encryptString(const std::string &key, const std::string &data);
    static std::string decryptString(const std::string &key, const std::string &encData);
};

class CRsaEncryptor {
public:
    static std::string encryptString(const std::string &pubKey, const std::string &data);
    static std::string decryptString(const std::string &priKey, const std::string &encData);
};

class CAuthentication {
SINGTON_DECLAR(CAuthentication)
public:
    bool init(const std::string &gatewayId);

    int setLicense(const std::string &content);

public:
    /// 判断规约是否激活
    bool bAuth(const std::string &name);

private:
    void parseProtocol(const std::string &gatewayId);
    std::string decode(const char *data, int dataByte, int &outByte);
    void checkLicense();

private:
    std::map<std::string, bool> m_mapValid;
    Json::Value m_cfg;
    base::IConfigManager *m_pConfig;
    std::string m_gatewayId;
    base::CTimer<void> m_timer;

};
}

#endif /* __FRAMEWORK_INCLUDE_AUTHENTICATION_AUTHENTICATION_H__ */
