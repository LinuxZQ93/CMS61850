#include "ServerNode.h"

#include "Print/Print.h"

#include "Service/ServiceManager.h"
#include "asn1/GetServerDirectory-RequestPDU.h"
#include "asn1/GetServerDirectory-ResponsePDU.h"
#include "asn1/GetLogicalDeviceDirectory-RequestPDU.h"
#include "asn1/GetLogicalDeviceDirectory-ResponsePDU.h"
#include "asn1/GetLogicalNodeDirectory-RequestPDU.h"
#include "asn1/GetLogicalNodeDirectory-ResponsePDU.h"
#include "asn1/GetAllDataValues-RequestPDU.h"
#include "asn1/GetAllDataValues-ResponsePDU.h"
#include "asn1/GetAllDataDefinition-RequestPDU.h"
#include "asn1/GetAllDataDefinition-ResponsePDU.h"
#include "asn1/GetAllCBValues-RequestPDU.h"
#include "asn1/GetAllCBValues-ResponsePDU.h"
#include "SCL/SCLParse.h"

#include "Common.h"
namespace cms {

SafeDef(GetServerDirectory_RequestPDU)
SafeDef(GetServerDirectory_ResponsePDU)
SafeDef(GetLogicalDeviceDirectory_RequestPDU)
SafeDef(GetLogicalDeviceDirectory_ResponsePDU)
SafeDef(GetLogicalNodeDirectory_RequestPDU)
SafeDef(GetLogicalNodeDirectory_ResponsePDU)
SafeDef(GetAllDataValues_RequestPDU)
SafeDef(GetAllDataValues_ResponsePDU)
SafeDef(GetAllDataDefinition_RequestPDU)
SafeDef(GetAllDataDefinition_ResponsePDU)
SafeDef(GetAllCBValues_RequestPDU)
SafeDef(GetAllCBValues_ResponsePDU)

CServerNode::CServerNode()
{
    CServiceManager::instance()->attachService("ServerNode", this);
}

CServerNode::~CServerNode()
{}

bool CServerNode::init()
{
    CServiceManager::instance()->attachFunc(80, "GetServerDirectory", base::function(&CServerNode::getServerDir, this));
    CServiceManager::instance()->attachFunc(81, "GetLogicalDeviceDirectory", base::function(&CServerNode::getLdDir, this));
    CServiceManager::instance()->attachFunc(82, "GetLogicalNodeDirectory", base::function(&CServerNode::getLnDir, this));
    CServiceManager::instance()->attachFunc(83, "GetAllDataValues", base::function(&CServerNode::getAllDataValues, this));
    CServiceManager::instance()->attachFunc(155, "GetAllDataDefinition", base::function(&CServerNode::getAllDataDef, this));
    CServiceManager::instance()->attachFunc(156, "GetAllCBValues", base::function(&CServerNode::getAllCBValues, this));
    decltype(m_mapAcsiFunc) mapFunc{
        {1, base::function(&CServerNode::getDataObject, this)},
        {2, base::function(&CServerNode::getDataSet, this)},
        {3, base::function(&CServerNode::getBRCB, this)},
        {4, base::function(&CServerNode::getURCB, this)},
        {5, base::function(&CServerNode::getLCB, this)},
        {7, base::function(&CServerNode::getSetting, this)}
    };
    m_mapAcsiFunc.swap(mapFunc);
    return true;
}

ServiceError CServerNode::getServerDir(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetServerDirectory_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    if (reqPtr->objectClass != 1)
    {
        warnf("not support objclass[%ld]\n", reqPtr->objectClass);
        return ServiceError_failed_due_to_server_constraint;
    }
    CSafeStruct<GetServerDirectory_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    const auto &info = m_scl->getSCLInfo();
    for (const auto &iter : info.vecLdInfo)
    {
        infof("domName[%s]\n", iter.complexName.c_str());
        ObjectReference_t *obj = (ObjectReference_t *)calloc(1, sizeof(ObjectReference_t));
        copyStrToOctStr(iter.complexName, obj);
        ASN_SET_ADD(&respPtr->reference.list, obj);
    }
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_server_constraint;
    }
    infof("object class is %ld\n", reqPtr->objectClass);
    return ServiceError_no_error;
}

ServiceError CServerNode::getLdDir(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetLogicalDeviceDirectory_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<GetLogicalDeviceDirectory_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    const auto &info = m_scl->getSCLInfo();
    std::string ldName;
    if (reqPtr->ldName != NULL && reqPtr->ldName->buf != NULL)
    {
        ldName = (char *)reqPtr->ldName->buf;
    }
    if (!ldName.empty() && NULL == m_scl->findDomInfo(ldName))
    {
        warnf("ldName[%s] not find\n", ldName.c_str());
        return ServiceError_instance_not_available;
    }
    std::string strRef;
    if (reqPtr->referenceAfter != NULL && reqPtr->referenceAfter->buf != NULL)
    {
        strRef = (char *)reqPtr->referenceAfter->buf;
    }
    bool bFind = false;
    for (const auto &iter : info.vecLdInfo)
    {
        if (!ldName.empty() && ldName != iter.complexName)
        {
            continue;
        }
        for (const auto &iter1 : iter.vecLnInfo)
        {
            /// refAfter可能未空，这是不是意味着两种情况呢，感觉协议也没有描述清楚，按自己的想法先写一个，逻辑应该差不多
            /// 1、如果ldName未指定，moreFllows回复为true，那么refAfter应该为ldName/lnName?
            /// 2、如果在ldName指定的情况下，moreFllows为true,refAfter应该为lnName?
            if (!strRef.empty() && !bFind)
            {
                if (iter1.complexName == strRef)
                {
                    bFind = true;
                }
                continue;
            }
            std::string lnRef(iter1.complexName);
            /// 如果ld为空，则返回ln为引用
            if (ldName.empty())
            {
                lnRef = iter.complexName + "/" + iter1.complexName;
            }
            ObjectReference_t *obj = CallocPtr(ObjectReference_t);
            copyStrToOctStr(lnRef, obj);
            infof("ln is %s\n", iter1.complexName.c_str());
            ASN_SET_ADD(&respPtr->lnReference.list, obj);
        }
        if (!ldName.empty())
        {
            break;
        }
    }
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    PrintAPER(respPtr);
    return ServiceError_no_error;
}

ServiceError CServerNode::getLnDir(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetLogicalNodeDirectory_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    std::string reName;
    bool bLd = false;
    if (reqPtr->reference.present == GetLogicalNodeDirectory_RequestPDU__reference_PR_ldName)
    {
        reName = (char *)reqPtr->reference.choice.ldName.buf;
        bLd = true;
    }
    else if (reqPtr->reference.present == GetLogicalNodeDirectory_RequestPDU__reference_PR_lnReference)
    {
        reName = (char *)reqPtr->reference.choice.lnReference.buf;
    }
    int acsiClass = reqPtr->acsiClass;
    infof("requset acsiClass[%d], reName[%s]\n", acsiClass, reName.c_str());
    CSafeStruct<GetLogicalNodeDirectory_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    if (bLd)
    {
        if (NULL == m_scl->findDomInfo(reName))
        {
            return ServiceError_instance_not_available;
        }
        if (m_mapAcsiFunc.find(acsiClass) == m_mapAcsiFunc.end())
        {
            warnf("acsi[%d] not support now\n", acsiClass);
            return ServiceError_parameter_value_inappropriate;
        }
        m_mapAcsiFunc[acsiClass](respPtr.get(), reName);
    }
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    PrintAPER(respPtr);
    return ServiceError_no_error;
}

bool CServerNode::getDataObject(void *ptr, const std::string &reName)
{
    auto *respPtr = (GetLogicalNodeDirectory_ResponsePDU *)ptr;
    CSCLParse::LdInfo *ldInfo = m_scl->getLdInfo(reName);
    for (const auto &iter : ldInfo->vecLnInfo)
    {
        for (const auto &iter1 : iter.vecDoInfo)
        {
            SubReference_t *member = CallocPtr(SubReference_t);
            ASN_SEQUENCE_ADD(&respPtr->reference.list, member);
            const std::string &subRef = iter.complexName + "." + iter1.name;
            copyStrToOctStr(subRef, member);
        }
    }
    return true;
}

bool CServerNode::getDataSet(void *ptr, const std::string &reName)
{
    auto *respPtr = (GetLogicalNodeDirectory_ResponsePDU*)ptr;
    CSCLParse::LdInfo *ldInfo = m_scl->getLdInfo(reName);
    for (const auto &iter : ldInfo->vecLnInfo)
    {
        if (iter.complexName != "LLN0")
        {
            continue;
        }
        for (const auto &iter1 : iter.vecDataSet)
        {
            SubReference_t *member = CallocPtr(SubReference_t);
            ASN_SEQUENCE_ADD(&respPtr->reference.list, member);
            const std::string &subRef = iter1.name;
            copyStrToOctStr(subRef, member);
        }
        break;
    }
    return true;
}

bool CServerNode::getBRCB(void *ptr, const std::string &reName)
{
    auto *respPtr = (GetLogicalNodeDirectory_ResponsePDU*)ptr;
    CSCLParse::LdInfo *ldInfo = m_scl->getLdInfo(reName);
    for (const auto &iter : ldInfo->vecLnInfo)
    {
        if (iter.complexName != "LLN0")
        {
            continue;
        }
        for (const auto &iter1 : iter.vecReport)
        {
            if (!iter1.buffered)
            {
                continue;
            }
            SubReference_t *member = CallocPtr(SubReference_t);
            ASN_SEQUENCE_ADD(&respPtr->reference.list, member);
            const std::string &subRef = iter.complexName + "." + iter1.rpName;
            copyStrToOctStr(subRef, member);
        }
        break;
    }
    return true;
}

bool CServerNode::getURCB(void *ptr, const std::string &reName)
{
    auto *respPtr = (GetLogicalNodeDirectory_ResponsePDU*)ptr;
    CSCLParse::LdInfo *ldInfo = m_scl->getLdInfo(reName);
    for (const auto &iter : ldInfo->vecLnInfo)
    {
        if (iter.complexName != "LLN0")
        {
            continue;
        }
        for (const auto &iter1 : iter.vecReport)
        {
            if (iter1.buffered)
            {
                continue;
            }
            SubReference_t *member = CallocPtr(SubReference_t);
            ASN_SEQUENCE_ADD(&respPtr->reference.list, member);
            const std::string &subRef = iter.complexName + "." + iter1.rpName;
            copyStrToOctStr(subRef, member);
        }
        break;
    }
    return true;
}

/// 日志报告块未实现
bool CServerNode::getLCB(void *ptr, const std::string &reName)
{
    return true;
}

bool CServerNode::getSetting(void *ptr, const std::string &reName)
{
    auto *respPtr = (GetLogicalNodeDirectory_ResponsePDU*)ptr;
    CSCLParse::LdInfo *ldInfo = m_scl->getLdInfo(reName);
    for (const auto &iter : ldInfo->vecLnInfo)
    {
        if (iter.complexName != "LLN0")
        {
            continue;
        }
        if (iter.setCtrl == NULL)
        {
            return false;
        }

        SubReference_t *member = CallocPtr(SubReference_t);
        ASN_SEQUENCE_ADD(&respPtr->reference.list, member);
        const std::string &subRef = iter.complexName + ".SGCB";
        copyStrToOctStr(subRef, member);
        break;
    }
    return true;
}

ServiceError CServerNode::getAllDataValues(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetAllDataValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    std::string fc;
    if (reqPtr->fc != NULL && reqPtr->fc->buf != NULL)
    {
        fc = (char *)reqPtr->fc->buf;
    }
    if (!validFC(fc))
    {
        return ServiceError_parameter_value_inappropriate;
    }
    std::string reName;
    bool bLd = false;
    if (reqPtr->reference.present == GetAllDataValues_RequestPDU__reference_PR_ldName)
    {
        reName = (char *)reqPtr->reference.choice.ldName.buf;
        bLd = true;
    }
    else if (reqPtr->reference.present == GetAllDataValues_RequestPDU__reference_PR_lnReference)
    {
        reName = (char *)reqPtr->reference.choice.lnReference.buf;
    }
    if (reName.empty())
    {
        return ServiceError_instance_not_available;
    }
    std::string strRef;
    if (reqPtr->referenceAfter != NULL && reqPtr->referenceAfter->buf != NULL)
    {
        strRef = (char *)reqPtr->referenceAfter->buf;
    }
    infof("requset fc[%s], reName[%s], strRef[%s]\n", fc.c_str(), reName.c_str(), strRef.c_str());
    CSafeStruct<GetAllDataValues_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    if (bLd)
    {
        CSCLParse::DataInfo::DomInfo *domInfo = m_scl->findDomInfo(reName);
        if (NULL == domInfo)
        {
            return ServiceError_instance_not_available;
        }
        parseAllValues(fc, respPtr.get(), domInfo);
    }
    #if 1
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    #endif
    PrintAPER(respPtr);
    return ServiceError_no_error;
}

ServiceError CServerNode::getAllDataDef(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetAllDataDefinition_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    std::string fc;
    if (reqPtr->fc != NULL && reqPtr->fc->buf != NULL)
    {
        fc = (char *)reqPtr->fc->buf;
    }
    if (!validFC(fc))
    {
        return ServiceError_parameter_value_inappropriate;
    }
    std::string reName;
    bool bLd = false;
    if (reqPtr->reference.present == GetAllDataDefinition_RequestPDU__reference_PR_ldName)
    {
        reName = (char *)reqPtr->reference.choice.ldName.buf;
        bLd = true;
    }
    else if (reqPtr->reference.present == GetAllDataDefinition_RequestPDU__reference_PR_lnReference)
    {
        reName = (char *)reqPtr->reference.choice.lnReference.buf;
    }
    if (reName.empty())
    {
        return ServiceError_instance_not_available;
    }
    std::string strRef;
    if (reqPtr->referenceAfter != NULL && reqPtr->referenceAfter->buf != NULL)
    {
        strRef = (char *)reqPtr->referenceAfter->buf;
    }
    infof("requset fc[%s], reName[%s], strRef[%s]\n", fc.c_str(), reName.c_str(), strRef.c_str());
    CSafeStruct<GetAllDataDefinition_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    *flag = 0;
    respPtr->moreFollows = flag;
    if (bLd)
    {
        CSCLParse::LdInfo *ldInfo = m_scl->getLdInfo(reName);
        if (NULL == ldInfo)
        {
            return ServiceError_instance_not_available;
        }
        parseDoAllDef(fc, respPtr.get(), ldInfo->vecLnInfo);
    }
    #if 1
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    #endif
    PrintAPER(respPtr);
    return ServiceError_no_error;
}

ServiceError CServerNode::getAllCBValues(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetAllCBValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    return ServiceError_no_error;
}

bool CServerNode::parseAllValues(const std::string &fc, void *ptr, void *ptr1)
{
    auto *pResponse = (GetAllDataValues_ResponsePDU*)ptr;
    auto *pDomInfo = (CSCLParse::DataInfo::DomInfo*)ptr1;
    for (auto &iter : pDomInfo->leafInfo)
    {
        GetAllDataValues_ResponsePDU__data__Member *member = CallocPtr(GetAllDataValues_ResponsePDU__data__Member);
        ASN_SEQUENCE_ADD(&pResponse->data.list, member);
        const std::string &subRef = iter.name;
        infof("subRef is %s\n", subRef.c_str());
        copyStrToOctStr(subRef, &member->reference);
        member->value.present = getDataPR("Struct");
        if (!parseValueStruct(fc, &member->value.choice.structure, &iter.vecInfo))
        {
            return false;
        }
    }
    return true;
}

bool CServerNode::parseValueStruct(const std::string &fc, void *ptr, void *ptr1)
{
    auto *pStruct = (Data::Data_u::Data__structure *)ptr;
    auto *pVecInfo = (std::vector<CSCLParse::DataInfo::LeafInfo>*)ptr1;
    for (auto &iter : *pVecInfo)
    {
        if (iter.valueType.empty())
        {
            warnf("name[%s] has no type\n", iter.name.c_str());
            return false;
        }
        bool bSet = false;
        if (fc.empty() || (fc != "SG" && fc != "SE"))
        {
            if (iter.fc == "SG" || iter.fc == "SE")
            {
                bSet = true;
            }
        }
        if (bSet)
        {
            continue;
        }
        Data *member = CallocPtr(Data);
        ASN_SEQUENCE_ADD(&pStruct->list, member);
        member->present = getDataPR(iter.valueType);
        debugf("da name is %s, bType is %s, type is %d\n", iter.name.c_str(), iter.valueType.c_str(), member->present);
        if (iter.valueType == "Struct")
        {
            if (!parseValueStruct(fc, &member->choice.structure, &iter.vecInfo))
            {
                return false;
            }
        }
        else
        {
            void *addr = reinterpret_cast<void *>(&member->choice.error);
            copyMem(iter.valueType, addr, iter.data.data, iter.data.len);
        }
    }
    return true;
}

bool CServerNode::parseDoAllDef(const std::string &fc, void *ptr, std::vector<CSCLParse::LnInfo> &lnInfo)
{
    auto *pResponse = (GetAllDataDefinition_ResponsePDU*)ptr;
    // int i = 0;
    // int j = 0;
    for (const auto &iter : lnInfo)
    {
        // i++;
        for (const auto &iter1 : iter.vecDataType)
        {
            // j++;
            GetAllDataDefinition_ResponsePDU__data__Member *member = CallocPtr(GetAllDataDefinition_ResponsePDU__data__Member);
            ASN_SEQUENCE_ADD(&pResponse->data.list, member);
            const std::string &subRef = iter.complexName + "." + iter1.doName;
            infof("subRef is %s\n", subRef.c_str());
            copyStrToOctStr(subRef, &member->reference);
            if (!iter1.cdc.empty())
            {
                member->cdcType = CallocPtr(VisibleString_t);
                copyStrToOctStr(iter1.cdc, member->cdcType);
            }
            member->definition.present = getDataDefPR("Struct");
            if (!parseStruct(fc, &member->definition.choice.structure, iter1.attr))
            {
                return false;
            }
        }
        // if (i * j > 10)
        // {
        //     *pResponse->moreFollows = true;
        //     break;
        // }
    }
    return true;
}

bool CServerNode::parseStruct(const std::string &fc, void *ptr, const std::vector<CSCLParse::DataAttr> &da)
{
    auto *pStruct = (DataDefinition::DataDefinition_u::DataDefinition__structure *)ptr;
    for (const auto &iter : da)
    {
        if (iter.btype.empty())
        {
            warnf("name[%s] has no type\n", iter.name.c_str());
            return false;
        }
        bool bSet = false;
        if (fc.empty() || (fc != "SG" && fc != "SE"))
        {
            if (iter.fc == "SG" || iter.fc == "SE")
            {
                bSet = true;
            }
        }
        if (bSet)
        {
            continue;
        }
        DataDefinition__structure__Member *member = CallocPtr(DataDefinition__structure__Member);
        ASN_SEQUENCE_ADD(&pStruct->list, member);
        member->type = CallocPtr(DataDefinition);
        member->type->present = getDataDefPR(iter.btype);
        debugf1("da name is %s, bType is %s, type is %d\n", iter.name.c_str(), iter.btype.c_str(), member->type->present);
        copyStrToOctStr(iter.name, &member->name);
        if (!iter.fc.empty())
        {
            member->fc = CallocPtr(FunctionalConstraint_t);
            copyStrToOctStr(iter.fc, member->fc);
        }
        if (iter.btype == "Struct")
        {
            if (!parseStruct(fc, &member->type->choice.structure, iter.bda))
            {
                return false;
            }
        }
        else if (iter.btype == "Array")
        {}
        #if 0
        else if (iter.btype == "VisString64")
        {
            member->type->choice.visible_string = 64;
        }
        else if (iter.btype == "VisString255")
        {
            member->type->choice.visible_string = 255;
        }
        else if (iter.btype == "Unicode255")
        {
            member->type->choice.unicode_string = 255;
        }
        #endif
        else
        {
            member->type->choice.int64u = 0;
        }
    }
    return true;
}

static CServerNode s_sNode;

}
