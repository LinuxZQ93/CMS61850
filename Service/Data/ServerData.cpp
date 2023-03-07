#include "ServerData.h"

#include <sstream>
#include <iomanip>
#include "Service/ServiceManager.h"
#include "asn1/GetDataValues-RequestPDU.h"
#include "asn1/GetDataValues-ResponsePDU.h"
#include "asn1/SetDataValues-ErrorPDU.h"
#include "asn1/SetDataValues-RequestPDU.h"
#include "asn1/SetDataValues-ResponsePDU.h"
#include "asn1/GetDataDirectory-RequestPDU.h"
#include "asn1/GetDataDirectory-ResponsePDU.h"
#include "asn1/GetDataDefinition-RequestPDU.h"
#include "asn1/GetDataDefinition-ResponsePDU.h"
#include "Time/Time.h"
#include "Common.h"

namespace cms {

#define MAX_DATA_VALUES_RESP 20

SafeDef(GetDataValues_RequestPDU)
SafeDef(GetDataValues_ResponsePDU)
SafeDef(GetDataDefinition_RequestPDU)
SafeDef(GetDataDefinition_ResponsePDU)
SafeDef(GetDataDirectory_RequestPDU)
SafeDef(GetDataDirectory_ResponsePDU)
SafeDef(SetDataValues_RequestPDU)
SafeDef(SetDataValues_ErrorPDU)
SafeDef(Data)

CServerData::CServerData()
{
    CServiceManager::instance()->attachService("ServerData", this);
}

CServerData::~CServerData()
{}

bool CServerData::init()
{
    CServiceManager::instance()->attachFunc(48, "GetDataValues", base::function(&CServerData::getDataValue, this));
    CServiceManager::instance()->attachFunc(49, "SetDataValues", base::function(&CServerData::setDataValue, this));
    CServiceManager::instance()->attachFunc(50, "GetDataDirectory", base::function(&CServerData::getDataDir, this));
    CServiceManager::instance()->attachFunc(51, "GetDataDefinition", base::function(&CServerData::getDataDef, this));
    return true;
}

void CServerData::doNormal(void *ptr, void *ptr1)
{
    auto *respPtr = (GetDataValues_ResponsePDU*)ptr;
    auto *leafInfo = (CSCLParse::DataInfo::LeafInfo*)ptr1;

    Data *pData = CallocPtr(Data);
    ASN_SEQUENCE_ADD(&respPtr->value.list, pData);
    void *addr = reinterpret_cast<void *>(&pData->choice.error);
    infof("name[%s] fc[%s], type[%s]\n", leafInfo->name.c_str(), leafInfo->fc.c_str(), leafInfo->valueType.c_str());
    pData->present = getDataPR(leafInfo->valueType);
    copyMem(leafInfo->valueType, addr, leafInfo->data.data, leafInfo->data.len);
}

void CServerData::doStruct(void *ptr, void *ptr1)
{
    auto *respPtr = (Data::Data_u::Data__structure*)ptr;
    auto *vecInfo = (std::vector<CSCLParse::DataInfo::LeafInfo>*)ptr1;
    for (uint32_t i = 0; i < vecInfo->size(); i++)
    {
        auto &leafInfo = (*vecInfo)[i];
        if (leafInfo.valueType == "Struct")
        {
            Data *pData = CallocPtr(Data);
            ASN_SEQUENCE_ADD(&respPtr->list, pData);
            pData->present = Data_PR_structure;
            doStruct(&pData->choice.structure, &leafInfo.vecInfo);
        }
        else
        {
            Data *pData = CallocPtr(Data);
            ASN_SEQUENCE_ADD(&respPtr->list, pData);
            pData->present = getDataPR(leafInfo.valueType);
            void *addr = reinterpret_cast<void *>(&pData->choice.error);
            infof("name[%s] fc[%s], type[%s]\n", leafInfo.name.c_str(), leafInfo.fc.c_str(), leafInfo.valueType.c_str());
            copyMem(leafInfo.valueType, addr, leafInfo.data.data, leafInfo.data.len);
        }
    }
}

void CServerData::doAllValue(void *ptr, void *ptr1)
{
    auto *respPtr = (GetDataValues_ResponsePDU*)ptr;
    auto *leafInfo = (CSCLParse::DataInfo::LeafInfo*)ptr1;
    if (leafInfo->valueType != "Struct")
    {
        doNormal(ptr, ptr1);
    }
    else
    {
        Data *pData = CallocPtr(Data);
        ASN_SEQUENCE_ADD(&respPtr->value.list, pData);
        pData->present = Data_PR_structure;
        doStruct(&pData->choice.structure, &leafInfo->vecInfo);
    }
}

ServiceError CServerData::getDataValue(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetDataValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<GetDataValues_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    for (int i = 0; i < reqPtr->data.list.count; i++)
    {
        auto *ptr = reqPtr->data.list.array[i];
        std::string name;
        std::string fc;
        // memset(addr, 0, sizeof(pData->choice));
        if (ptr->reference.buf != NULL)
        {
            name = (char *)ptr->reference.buf;
        }
        if (name.empty())
        {
            Data *pData = CallocPtr(Data);
            ASN_SEQUENCE_ADD(&respPtr->value.list, pData);
            pData->present = Data_PR_error;
            pData->choice.error = ServiceError_instance_not_available;
        }
        if (ptr->fc != NULL && ptr->fc->buf != NULL)
        {
            fc = (char *)ptr->fc->buf;
        }
        if (!validFC(fc))
        {
            Data *pData = CallocPtr(Data);
            ASN_SEQUENCE_ADD(&respPtr->value.list, pData);
            pData->present = Data_PR_error;
            pData->choice.error = ServiceError_parameter_value_inappropriate;
            continue;
        }
        std::vector<std::string> result;
        if (!splitStr("/", name, result))
        {
            Data *pData = CallocPtr(Data);
            ASN_SEQUENCE_ADD(&respPtr->value.list, pData);
            pData->present = Data_PR_error;
            pData->choice.error = ServiceError_instance_not_available;
            continue;
        }
        CSCLParse::DataInfo::DomInfo *domInfo = m_scl->findDomInfo(result[0]);
        if (NULL == domInfo)
        {
            Data *pData = CallocPtr(Data);
            ASN_SEQUENCE_ADD(&respPtr->value.list, pData);
            pData->present = Data_PR_error;
            pData->choice.error = ServiceError_instance_not_available;
            warnf("name[%s], fc[%s] not support\n", name.c_str(), fc.c_str());
            continue;
        }
        CSCLParse::DataInfo::LeafInfo *leafInfo = m_scl->findLeafInfo(domInfo, result[1]);
        if (NULL == leafInfo)
        {
            Data *pData = CallocPtr(Data);
            ASN_SEQUENCE_ADD(&respPtr->value.list, pData);
            pData->present = Data_PR_error;
            pData->choice.error = ServiceError_instance_not_available;
            warnf("name[%s], fc[%s] not support\n", name.c_str(), fc.c_str());
            continue;
        }
        doAllValue(respPtr.get(), leafInfo);
        if (i >= MAX_DATA_VALUES_RESP)
        {
            *respPtr->moreFollows = true;
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

ServiceError CServerData::setDataValue(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<SetDataValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<SetDataValues_ErrorPDU> respPtr;
    bool bErr = false;
    for (int i = 0; i < reqPtr->data.list.count; i++)
    {
        auto &reqData = reqPtr->data.list.array[i];
        std::string subRef;
        CSafeStruct<Data> tmp(&reqData->value);
        auto *err = CallocPtr(ServiceError);
        ASN_SEQUENCE_ADD(&respPtr->result.list, err);
        if (!Check(tmp))
        {
            *err = ServiceError_parameter_value_inappropriate;
            warnf("value over limit\n");
            bErr = true;
            continue;
        }
        if (reqData->reference.buf == NULL)
        {
            continue;
        }
        subRef = (char *)reqData->reference.buf;
        std::string fc;
        if (reqData->fc != NULL && reqData->fc->buf != NULL)
        {
            fc = (char *)reqData->fc->buf;
        }
        std::vector<std::string> result;
        splitStr("/", subRef, result);
        auto *leafInfo = m_scl->findLeafInfo(result[0], result[1]);
        if (!leafInfo->fc.empty() && leafInfo->fc != fc)
        {
            *err = ServiceError_parameter_value_inappropriate;
            warnf("value type not same\n");
            bErr = true;
            continue;
        }
        auto dataType = getDataPR(leafInfo->valueType);
        if (reqData->value.present != dataType)
        {
            *err = ServiceError_type_conflict;
            warnf("value type not same\n");
            bErr = true;
            continue;
        }
        /**
         * TODO 数据只读情况
         *
         */
        void *addr = reinterpret_cast<void *>(&reqData->value.choice.error);
        copyMem(leafInfo->valueType, leafInfo->data.data, addr, leafInfo->data.len);
        infof("subRef[%s], fc[%s], dataType[%d]\n", subRef.c_str(), fc.c_str(), reqData->value.present);
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

ServiceError CServerData::getDataDir(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetDataDirectory_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<GetDataDirectory_ResponsePDU> respPtr;
    std::string strRef;
    if (reqPtr->dataReference.buf != NULL)
    {
        strRef = (char *)reqPtr->dataReference.buf;
    }
    std::vector<std::string> result;
    splitStr("/", strRef, result);
    auto *leafInfo = m_scl->findLeafInfo(result[0], result[1]);
    const auto &f = [&](CSCLParse::DataInfo::LeafInfo *leafInfo)
    {
        auto *member = CallocPtr(GetDataDirectory_ResponsePDU__dataAttribute__Member);
        ASN_SEQUENCE_ADD(&respPtr->dataAttribute.list, member);
        copyStrToOctStr(leafInfo->name, &member->reference);
        if (!leafInfo->fc.empty())
        {
            member->fc = CallocPtr(FunctionalConstraint_t);
            copyStrToOctStr(leafInfo->fc, member->fc);
        }
    };
    f(leafInfo);
    for (auto &iter : leafInfo->vecInfo)
    {
        f(&iter);
    }
    PrintAPER(respPtr);
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    return ServiceError_no_error;
}

void CServerData::doDefStruct(const std::string &fc, void *ptr, void *ptr1)
{
    auto *respPtr = (DataDefinition::DataDefinition_u::DataDefinition__structure*)ptr;
    auto *vecInfo = (std::vector<CSCLParse::DataInfo::LeafInfo>*)ptr1;
    for (uint32_t i = 0; i < vecInfo->size(); i++)
    {
        auto &leafInfo = (*vecInfo)[i];
        bool bSet = false;
        if (fc == "SG" || fc == "SE")
        {
            if (leafInfo.fc == "SG" || leafInfo.fc == "SE")
            {
                bSet = true;
            }
        }
        if (!leafInfo.fc.empty() && !fc.empty() && !bSet && leafInfo.fc != fc)
        {
            continue;
        }
        std::vector<std::string> result;
        splitStr(".", leafInfo.name, result);
        if (leafInfo.valueType == "Struct")
        {
            DataDefinition__structure__Member *member = CallocPtr(DataDefinition__structure__Member);
            ASN_SEQUENCE_ADD(&respPtr->list, member);
            if (!leafInfo.fc.empty())
            {
                member->fc = CallocPtr(FunctionalConstraint_t);
                copyStrToOctStr(leafInfo.fc, member->fc);
            }
            copyStrToOctStr(*result.rbegin(), &member->name);
            member->type = CallocPtr(DataDefinition);
            member->type->present = getDataDefPR(leafInfo.valueType);
            doDefStruct(fc, &member->type->choice.structure, &leafInfo.vecInfo);
        }
        else
        {
            DataDefinition__structure__Member *member = CallocPtr(DataDefinition__structure__Member);
            ASN_SEQUENCE_ADD(&respPtr->list, member);
            if (!leafInfo.fc.empty())
            {
                member->fc = CallocPtr(FunctionalConstraint_t);
                copyStrToOctStr(leafInfo.fc, member->fc);
            }
            copyStrToOctStr(*result.rbegin(), &member->name);
            member->type = CallocPtr(DataDefinition);
            member->type->present = getDataDefPR(leafInfo.valueType);
            auto dataType = member->type->present;
            if (dataType == DataDefinition_PR_bit_string || dataType == DataDefinition_PR_octet_string
                || dataType == DataDefinition_PR_visible_string || dataType == DataDefinition_PR_unicode_string)
            {
                OCTET_STRING_t *addr = reinterpret_cast<OCTET_STRING_t *>(&member->type->choice.error);
                OCTET_STRING_t *src = (OCTET_STRING_t *)leafInfo.data.data;
                addr->size = src->size;
            }
            infof("name[%s] fc[%s], type[%s]\n", leafInfo.name.c_str(), leafInfo.fc.c_str(), leafInfo.valueType.c_str());
        }
    }
}

void CServerData::doAllDef(const std::string &fc, void *ptr, void *ptr1)
{
    auto *respPtr = (GetDataDefinition_ResponsePDU*)ptr;
    auto *leafInfo = (CSCLParse::DataInfo::LeafInfo*)ptr1;
    GetDataDefinition_ResponsePDU__data__Member *member = CallocPtr(GetDataDefinition_ResponsePDU__data__Member);
    ASN_SEQUENCE_ADD(&respPtr->data.list, member);
    member->definition.present = getDataDefPR(leafInfo->valueType);
    member->cdcType = CallocPtr(VisibleString_t);
    if (!leafInfo->cdcType.empty())
    {
        copyStrToOctStr(leafInfo->cdcType, member->cdcType);
    }
    /// 获取数据定义应该都为struct
    if (leafInfo->valueType == "Struct")
    {
        doDefStruct(fc, &member->definition.choice.structure, &leafInfo->vecInfo);
    }
    else
    {
        auto dataType = member->definition.present;
        if (dataType == DataDefinition_PR_bit_string || dataType == DataDefinition_PR_octet_string
            || dataType == DataDefinition_PR_visible_string || dataType == DataDefinition_PR_unicode_string)
        {
            OCTET_STRING_t *addr = reinterpret_cast<OCTET_STRING_t *>(&member->definition.choice.error);
            OCTET_STRING_t *src = (OCTET_STRING_t*)leafInfo->data.data;
            addr->size = src->size;
        }
        infof("name[%s] fc[%s], type[%s]\n", leafInfo->name.c_str(), leafInfo->fc.c_str(), leafInfo->valueType.c_str());
    }
}

ServiceError CServerData::getDataDef(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetDataDefinition_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<GetDataDefinition_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    for (int i = 0; i < reqPtr->data.list.count; i++)
    {
        auto &reqData = reqPtr->data.list.array[i];
        std::string subRef;
        if (reqData->reference.buf == NULL)
        {
            auto *pData = CallocPtr(GetDataDefinition_ResponsePDU__data__Member);
            ASN_SEQUENCE_ADD(&respPtr->data.list, pData);
            pData->definition.present = DataDefinition_PR_error;
            pData->definition.choice.error = ServiceError_instance_not_available;
            continue;
        }
        subRef = (char *)reqData->reference.buf;
        std::string fc;
        if (reqData->fc != NULL && reqData->fc->buf != NULL)
        {
            fc = (char *)reqData->fc->buf;
        }
        if (!validFC(fc))
        {
            auto *pData = CallocPtr(GetDataDefinition_ResponsePDU__data__Member);
            ASN_SEQUENCE_ADD(&respPtr->data.list, pData);
            pData->definition.present = DataDefinition_PR_error;
            pData->definition.choice.error = ServiceError_instance_not_available;
            continue;
        }
        std::vector<std::string> result;
        splitStr("/", subRef, result);
        CSCLParse::DataInfo::DomInfo *domInfo = m_scl->findDomInfo(result[0]);
        if (NULL == domInfo)
        {
            auto *pData = CallocPtr(GetDataDefinition_ResponsePDU__data__Member);
            ASN_SEQUENCE_ADD(&respPtr->data.list, pData);
            pData->definition.present = DataDefinition_PR_error;
            pData->definition.choice.error = ServiceError_instance_not_available;
            warnf("name[%s], fc[%s] not support\n", subRef.c_str(), fc.c_str());
            continue;
        }
        CSCLParse::DataInfo::LeafInfo *leafInfo = m_scl->findLeafInfo(domInfo, result[1]);
        if (NULL == leafInfo)
        {
            auto *pData = CallocPtr(GetDataDefinition_ResponsePDU__data__Member);
            ASN_SEQUENCE_ADD(&respPtr->data.list, pData);
            pData->definition.present = DataDefinition_PR_error;
            pData->definition.choice.error = ServiceError_instance_not_available;
            warnf("name[%s], fc[%s] not support\n", subRef.c_str(), fc.c_str());
            continue;
        }
        doAllDef(fc, respPtr.get(), leafInfo);
    }
    PrintAPER(respPtr);
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    return ServiceError_no_error;
}

static CServerData s_data;

}
