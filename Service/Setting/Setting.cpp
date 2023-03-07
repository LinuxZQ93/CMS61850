#include "Setting.h"

#include "Common.h"
#include "Service/ServiceManager.h"
#include "Function/Bind.h"
#include "SCL/SCLParse.h"
#include "asn1/GetSGCBValues-RequestPDU.h"
#include "asn1/GetSGCBValues-ResponsePDU.h"

namespace cms {

SafeDef(GetSGCBValues_RequestPDU)
SafeDef(GetSGCBValues_ResponsePDU)

CSetting::CSetting()
{
    CServiceManager::instance()->attachService("Setting", this);
}

CSetting::~CSetting()
{}

bool CSetting::init()
{
    CServiceManager::instance()->attachFunc(89, "GetSGCBValues", base::function(&CSetting::getSGCBValues, this));
    return true;
}

ServiceError CSetting::getSGCBValues(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetSGCBValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    CSafeStruct<GetSGCBValues_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    for (int i = 0; i < reqPtr->sgcbReference.list.count; i++)
    {
        auto &strRef = reqPtr->sgcbReference.list.array[i];
        if (strRef->buf == NULL)
        {
            continue;
        }
        std::string subRef((char *)strRef->buf);
        std::vector<std::string> result;
        splitStr("/", subRef, result);
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
        auto *setInfo = lnInfo->setCtrl;
        auto *member = CallocPtr(GetSGCBValues_ResponsePDU__sgcb__Member);
        member->present = GetSGCBValues_ResponsePDU__sgcb__Member_PR_value;
        auto &value = member->choice.value;
        value.actSG = setInfo->actSG;
        value.editSG = setInfo->actSG;
        value.numOfSG = setInfo->numOfSGs;
        value.lActTm.buf = (uint8_t *)calloc(1, 8);
        value.lActTm.size = 8;
        ASN_SEQUENCE_ADD(&respPtr->sgcb.list, member);
    }
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    return ServiceError_no_error;
}

static CSetting s_set;

}
