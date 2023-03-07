#ifndef __FRAMEWORK_INCLUDE_LOG_LOG_H__
#define __FRAMEWORK_INCLUDE_LOG_LOG_H__

#include "Log/LogBase.h"

#include <queue>
#include <vector>

#include "json/json.h"

#include "SingTon/SingTon.h"

namespace base {

class CLog : public CLogBase {
SINGTON_DECLAR(CLog)
struct strCmp {
    bool operator()(const std::string &a, const std::string &b)
    {
        return a > b;
    }
};
public:
    virtual bool init();
    bool destroy();

public:
    virtual void saveLog(const char *buf, int len);

private:
    bool createFile();
    void setHead();
    void closeFile();
    void updateLogFileNum();

private:
    std::mutex m_mutex;
    FILE    *m_fLog;
    unsigned int m_fLength;
    unsigned int m_maxLogSize;
    unsigned int m_maxLogNum;
    std::string m_curLogName;
    std::priority_queue<std::string, std::vector<std::string>, strCmp> m_minFName;
};

}

#endif /* __FRAMEWORK_INCLUDE_LOG_LOG_H__ */
