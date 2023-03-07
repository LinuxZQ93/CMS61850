#ifndef __FRAMEWORK_INCLUDE_LOG_LOGBASE_H__
#define __FRAMEWORK_INCLUDE_LOG_LOGBASE_H__

#include "json/json.h"

namespace base {

class CLogBase {
public:
    CLogBase(){}
    virtual ~CLogBase(){}

public:
    virtual bool init();
    virtual bool destroy(){ return true; };

public:
    virtual void saveLog(const char *buf, int len) = 0;

private:
    bool parseConfigFile(const std::string &path);

protected:
    Json::Value m_logCfg;
};

}

#endif /* __FRAMEWORK_INCLUDE_LOG_LOGBASE_H__ */
