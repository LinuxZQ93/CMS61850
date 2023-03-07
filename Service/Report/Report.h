#ifndef __CMS61850_SERVICE_REPORT_REPORT_H__
#define __CMS61850_SERVICE_REPORT_REPORT_H__

#include "Service/IService.h"

#include <map>
#include <mutex>

#include "SCL/SCLParse.h"
#include "Function/Bind.h"
/**
 * @file Report.h
 * @author (linuxzq93@163.com)
 * @brief 报告业务类
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "json/json.h"

namespace cms {

class CReport : public IService {
public:
    CReport();
    virtual ~CReport();

public:
    virtual bool init();
    virtual bool start(){return true;}

struct ReportExt {
    bool resv;
    std::string owner;
};

private:
    ServiceError getURCBValue(const std::string &funcName, const NetMessage &message, std::string &response);
    ServiceError getBRCBValue(const std::string &funcName, const NetMessage &message, std::string &response);
    bool fillURReport(CSCLParse::Report *reportInfo, void *member);
    bool fillBRReport(CSCLParse::Report *reportInfo, void *ptr);
    ServiceError setURCBValue(const std::string &funcName, const NetMessage &message, std::string &response);
    bool setReport(CSCLParse::Report *reportInfo, int fd, void *reqMember, void *member);
    ServiceError bRCBAccess(CSCLParse::Report *reportInfo, int fd, void *reqMember);
    bool setAttr(CSCLParse::Report *reportInfo, int fd, void *reqMember, void *member);

    void socketEvt(const std::string &code, const Json::Value &cfgValue);

private:
    bool writeEnable(CSCLParse::Report *rpInfo, int fd, void *reqMember, void *member);
    bool writeGI(CSCLParse::Report *rpInfo, int fd, void *reqMember, void *member);

private:
    std::mutex m_mutex;
    std::map<std::string, ReportExt> m_mapClient;
    std::vector<TFunction<bool, CSCLParse::Report*, int, void*, void*>> m_vecUrcbFunc;
    std::vector<TFunction<bool, CSCLParse::Report*, int, void*, void*>> m_vecBrcbFunc;
};

}

#endif /* __CMS61850_SERVICE_REPORT_REPORT_H__ */
