#include "Report.h"

#include <algorithm>
#include "Print/Print.h"
#include "Event/IEventManager.h"

#include "Service/ServiceManager.h"
#include "asn1/GetURCBValues-RequestPDU.h"
#include "asn1/GetURCBValues-ResponsePDU.h"
#include "asn1/SetURCBValues-RequestPDU.h"
#include "asn1/SetURCBValues-ResponsePDU.h"
#include "asn1/SetURCBValues-ErrorPDU.h"
#include "asn1/GetBRCBValues-RequestPDU.h"
#include "asn1/GetBRCBValues-ResponsePDU.h"

#include "Common.h"

namespace cms {

SafeDef(GetURCBValues_RequestPDU)
SafeDef(GetURCBValues_ResponsePDU)
SafeDef(SetURCBValues_RequestPDU)
SafeDef(SetURCBValues_ErrorPDU)
SafeDef(GetBRCBValues_RequestPDU)
SafeDef(GetBRCBValues_ResponsePDU)

CReport::CReport()
{
    CServiceManager::instance()->attachService("Report", this);
}

CReport::~CReport()
{}

bool CReport::init()
{
    CServiceManager::instance()->attachFunc(91, "GetBRCBValues", base::function(&CReport::getBRCBValue, this));
    CServiceManager::instance()->attachFunc(93, "GetURCBValues", base::function(&CReport::getURCBValue, this));
    CServiceManager::instance()->attachFunc(94, "SetURCBValues", base::function(&CReport::setURCBValue, this));
    auto *pEvent = base::CComponentManager::instance()->getComponent<base::IEventManager>("EventManager");
    pEvent->attach("cmsSocket", base::IEventManager::Proc(&CReport::socketEvt, this));
    m_vecUrcbFunc.emplace_back(&CReport::writeGI, this);
    return true;
}

ServiceError CReport::getURCBValue(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetURCBValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<GetURCBValues_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    for (int i = 0; i < reqPtr->reference.list.count; i++)
    {
        auto *member = CallocPtr(GetURCBValues_ResponsePDU__urcb__Member);
        ASN_SEQUENCE_ADD(&respPtr->urcb.list, member);
        auto &reqRef = reqPtr->reference.list.array[i];
        std::string strRef;
        if (reqRef->buf != NULL)
        {
            strRef = (char *)reqRef->buf;
        }
        std::vector<std::string> result;
        splitStr("/", strRef, result);
        auto *ldInfo = m_scl->getLdInfo(result[0]);
        if (NULL == ldInfo)
        {
            warnf("ldName[%s] not find\n", result[0].c_str());
            member->present = GetURCBValues_ResponsePDU__urcb__Member_PR_error;
            member->choice.error = ServiceError_instance_not_available;
            continue;
        }
        std::vector<std::string> result1;
        splitStr(".", result[1], result1);
        auto *lnInfo = m_scl->getLnInfo(ldInfo, result1[0]);
        if (NULL == lnInfo)
        {
            warnf("lnName[%s] not find\n", result1[0].c_str());
            continue;
        }
        auto *rpInfo = m_scl->getReportInfo(lnInfo, result1[1]);
        if (NULL == rpInfo)
        {
            continue;
        }
        if (!fillURReport(rpInfo, member))
        {
            warnf("fill report[%s] failed\n", result1[1].c_str());
            continue;
        }
    }
    PrintAPER(respPtr);
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    return ServiceError_no_error;
}

ServiceError CReport::getBRCBValue(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetBRCBValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<GetBRCBValues_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    for (int i = 0; i < reqPtr->reference.list.count; i++)
    {
        auto &reqRef = reqPtr->reference.list.array[i];
        std::string strRef;
        if (reqRef->buf != NULL)
        {
            strRef = (char *)reqRef->buf;
        }
        std::vector<std::string> result;
        splitStr("/", strRef, result);
        auto *ldInfo = m_scl->getLdInfo(result[0]);
        if (NULL == ldInfo)
        {
            warnf("ldName[%s] not find\n", result[0].c_str());
            continue;
        }
        std::vector<std::string> result1;
        splitStr(".", result[1], result1);
        auto *lnInfo = m_scl->getLnInfo(ldInfo, result1[0]);
        if (NULL == lnInfo)
        {
            warnf("lnName[%s] not find\n", result1[0].c_str());
            continue;
        }
        auto *rpInfo = m_scl->getReportInfo(lnInfo, result1[1]);
        if (NULL == rpInfo)
        {
            continue;
        }
        if (!fillBRReport(rpInfo, respPtr.get()))
        {
            warnf("fill report[%s] failed\n", result1[1].c_str());
            continue;
        }
    }
    PrintAPER(respPtr);
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    return ServiceError_no_error;
}

bool CReport::fillURReport(CSCLParse::Report *reportInfo, void *ptr)
{
    auto *member = (GetURCBValues_ResponsePDU__urcb__Member*)ptr;
    member->present = GetURCBValues_ResponsePDU__urcb__Member_PR_value;
    auto &value = member->choice.value;
    value.rptEna = reportInfo->rptEna;
    value.confRev = reportInfo->confRev;
    copyStrToOctStr(reportInfo->datSet, &value.datSet);
    value.intgPd = reportInfo->intgPd;
    copyBitPtr(&value.optFlds, reportInfo->optFields, 1);
    copyStrToOctStr(reportInfo->rptId, &value.rptID);
    copyBitPtr(&value.trgOps, reportInfo->trgOps, 1);
    std::lock_guard<std::mutex> Guard(m_mutex);
    if (m_mapClient.find(reportInfo->rpName) != m_mapClient.end())
    {
        auto &tmp = m_mapClient[reportInfo->rpName];
        value.resv = tmp.resv;
        value.owner = CallocPtr(OCTET_STRING_t);
        copyStrToOctStr(tmp.owner, value.owner);
    }
    return true;
}

bool CReport::fillBRReport(CSCLParse::Report *reportInfo, void *ptr)
{
    auto *respPtr = (GetBRCBValues_ResponsePDU*)ptr;
    auto *member = CallocPtr(GetBRCBValues_ResponsePDU__brcb__Member);
    member->present = GetBRCBValues_ResponsePDU__brcb__Member_PR_value;
    auto &value = member->choice.value;
    value.rptEna = reportInfo->rptEna;
    value.confRev = reportInfo->confRev;
    copyStrToOctStr(reportInfo->datSet, &value.datSet);
    value.intgPd = reportInfo->intgPd;
    copyBitPtr(&value.optFlds, reportInfo->optFields, 1);
    copyStrToOctStr(reportInfo->rptId, &value.rptID);
    copyBitPtr(&value.trgOps, reportInfo->trgOps, 1);
    uint8_t *entry = (uint8_t *)calloc(1, 8);
    value.entryID.buf = entry;
    value.entryID.size = 8;
    uint8_t *bTime = (uint8_t *)calloc(1, 6);
    value.timeOfEntry.buf = bTime;
    value.timeOfEntry.size = 6;
    ASN_SEQUENCE_ADD(&respPtr->brcb.list, member);
    return true;
}

ServiceError CReport::setURCBValue(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<SetURCBValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<SetURCBValues_ErrorPDU> respPtr;
    bool bErr = false;
    for (int i = 0; i < reqPtr->urcb.list.count; i++)
    {
        auto *member = CallocPtr(SetURCBValues_ErrorPDU__result__Member);
        ASN_SEQUENCE_ADD(&respPtr->result.list, member);
        auto &reqRef = reqPtr->urcb.list.array[i];
        std::string strRef;
        if (reqRef->reference.buf != NULL)
        {
            strRef = (char *)reqRef->reference.buf;
        }
        std::vector<std::string> result;
        splitStr("/", strRef, result);
        auto *ldInfo = m_scl->getLdInfo(result[0]);
        if (NULL == ldInfo)
        {
            warnf("ldName[%s] not find\n", result[0].c_str());
            member->error = CallocPtr(ServiceError_t);
            *member->error = ServiceError_instance_not_available;
            continue;
        }
        std::vector<std::string> result1;
        splitStr(".", result[1], result1);
        auto *lnInfo = m_scl->getLnInfo(ldInfo, result1[0]);
        if (NULL == lnInfo)
        {
            warnf("lnName[%s] not find\n", result1[0].c_str());
            member->error = CallocPtr(ServiceError_t);
            *member->error = ServiceError_instance_not_available;
            continue;
        }
        auto *rpInfo = m_scl->getReportInfo(lnInfo, result1[1]);
        if (NULL == rpInfo)
        {
            warnf("not find rpInfo[%s]\n", result1[1].c_str());
            member->error = CallocPtr(ServiceError_t);
            *member->error = ServiceError_parameter_value_inappropriate;
            continue;
        }
        if (!setReport(rpInfo, message.clientId, reqRef, member))
        {
            bErr = true;
        }
    }
    if (bErr)
    {
        if (!Encode(respPtr, response))
        {
            errorf("encode %s failed\n", funcName.c_str());
            return ServiceError_failed_due_to_communications_constraint;
        }
        PrintAPER(respPtr);
    }
    return ServiceError_no_error;
}

bool CReport::setReport(CSCLParse::Report *reportInfo, int fd, void *ptr, void *ptr1)
{
    auto *reqPtr = (SetURCBValues_RequestPDU__urcb__Member*)ptr;
    auto *member = (SetURCBValues_ErrorPDU__result__Member*)ptr1;
    bool bEnable = *reqPtr->rptEna;
    if (!bEnable)
    {
        bool ret = writeEnable(reportInfo, fd, ptr, ptr1);
        ret &= setAttr(reportInfo, fd, ptr, ptr1);
        if (!ret)
        {
            return false;
        }
    }
    else
    {
        if (setAttr(reportInfo, fd, ptr, ptr1))
        {
            writeEnable(reportInfo, fd, ptr, ptr1);
        }
        else
        {
            warnf("set urcb report attr failed\n");
            member->rptEna = CallocPtr(ServiceError_t);
            *member->rptEna = ServiceError_parameter_value_inappropriate;
            return false;
        }
    }
    return true;
}

ServiceError CReport::bRCBAccess(CSCLParse::Report *reportInfo, int fd, void *ptr)
{
    auto *reqPtr = (SetURCBValues_RequestPDU__urcb__Member*)ptr;
    if (reportInfo->rptEna)
    {
        if (reqPtr->rptEna == NULL || reqPtr->gi == NULL)
        {
            warnf("set urcb report failed, miss rptEnable\n");
            return ServiceError_access_not_allowed_in_current_state;
        }
    }
    base::clientInfo info;
    CServiceManager::instance()->getClientInfo(fd, info);
    const std::string &cStr = info.ip + ":" + std::to_string(info.port);
    m_mutex.lock();
    if (m_mapClient.find(reportInfo->rpName) == m_mapClient.end())
    {
        m_mutex.unlock();
        return ServiceError_no_error;
    }
    auto repExt = m_mapClient[reportInfo->rpName];
    m_mutex.unlock();
    if (repExt.resv && repExt.owner != cStr)
    {
        warnf("report used by dest[%s], fobid\n", repExt.owner.c_str());
        return ServiceError_instance_locked_by_other_client;
    }
    return ServiceError_no_error;
}

/// 这里的具体每一个属性应该怎么设置，有相应的限制条件，这里先简单处理了
bool CReport::setAttr(CSCLParse::Report *reportInfo, int fd, void *ptr, void *ptr1)
{
    auto *reqPtr = (SetURCBValues_RequestPDU__urcb__Member*)ptr;
    bool ret = true;
    for (const auto &iter : m_vecUrcbFunc)
    {
        ret &= iter(reportInfo, fd, ptr, ptr1);
    }
    if (!ret)
    {
        return false;
    }
    if (reqPtr->rptID != NULL)
    {
        reportInfo->rptId = (char *)reqPtr->rptID->buf;
    }
    if (reqPtr->datSet != NULL)
    {
        reportInfo->datSet = (char *)reqPtr->datSet->buf;
    }

    return true;
}

void CReport::socketEvt(const std::string &code, const Json::Value &cfgValue)
{
    const std::string &state = cfgValue["state"].asString();
    if (state == "disconnect")
    {
        int fd = cfgValue["clientId"].asInt();
        base::clientInfo info;
        CServiceManager::instance()->getClientInfo(fd, info);
        const std::string &cStr = info.ip + ":" + std::to_string(info.port);
        std::lock_guard<std::mutex> Guard(m_mutex);
        for (auto &iter : m_mapClient)
        {
            if (iter.second.owner == cStr)
            {
                m_mapClient.erase(iter.first);
                return;
            }
        }
    }
}

bool CReport::writeEnable(CSCLParse::Report *rpInfo, int fd, void *ptr, void *ptr1)
{
    /// 实际上这个函数urcb和brcb公用，后面很多元素函数都是同样设计
    /// 此处取巧，在resv元素之前，urcb与brcb的内存表示是一样的，所以统一用urcb的内存来转换
    auto *reqPtr = (SetURCBValues_RequestPDU__urcb__Member*)ptr;
    auto *member = (SetURCBValues_ErrorPDU__result__Member*)ptr1;
    auto code = bRCBAccess(rpInfo, fd, ptr);
    if (code != ServiceError_no_error)
    {
        member->rptEna = CallocPtr(ServiceError_t);
        *member->rptEna = code;
        return false;
    }
    rpInfo->rptEna = *reqPtr->rptEna;
    /// 更新resv及owner
    base::clientInfo info;
    CServiceManager::instance()->getClientInfo(fd, info);
    const std::string &cStr = info.ip + ":" + std::to_string(info.port);
    m_mutex.lock();
    if (rpInfo->rptEna)
    {
        ReportExt &ext = m_mapClient[rpInfo->rpName];
        ext.owner = cStr;
        ext.resv = true;
    }
    else
    {
        m_mutex.lock();
        if (m_mapClient.find(rpInfo->rpName) == m_mapClient.end())
        {
            m_mutex.unlock();
            return false;
        }
        m_mapClient.erase(rpInfo->rpName);
        m_mutex.unlock();
    }
    m_mutex.unlock();
    return true;
}

bool CReport::writeGI(CSCLParse::Report *rpInfo, int fd, void *ptr, void *ptr1)
{
    auto *reqPtr = (SetURCBValues_RequestPDU__urcb__Member*)ptr;
    auto *member = (SetURCBValues_ErrorPDU__result__Member*)ptr1;
    auto code = bRCBAccess(rpInfo, fd, ptr);
    if (code != ServiceError_no_error)
    {
        member->gi = CallocPtr(ServiceError_t);
        *member->gi = code;
        return false;
    }
    if (reqPtr->gi == NULL)
    {
        return true;
    }
    if (*(reqPtr->gi) == 0)
    {
        BIT_SET_OFF(rpInfo->trgOps->buf, TriggerConditions_general_interrogation);
    }
    else
    {
        BIT_SET_ON(rpInfo->trgOps->buf, TriggerConditions_general_interrogation);
    }
    return true;
}

static CReport s_report;

}
