#include "DataSet.h"

#include "Print/Print.h"
#include "Service/ServiceManager.h"
#include "SCL/SCLParse.h"

#include "asn1/GetDataSetValues-RequestPDU.h"
#include "asn1/GetDataSetValues-ResponsePDU.h"
#include "asn1/GetDataSetDirectory-RequestPDU.h"
#include "asn1/GetDataSetDirectory-ResponsePDU.h"

#include "Common.h"

namespace cms {

SafeDef(GetDataSetDirectory_RequestPDU)
SafeDef(GetDataSetDirectory_ResponsePDU)
SafeDef(GetDataSetValues_RequestPDU)
SafeDef(GetDataSetValues_ResponsePDU)

CDataSet::CDataSet()
{
    CServiceManager::instance()->attachService("DataSet", this);
}

CDataSet::~CDataSet()
{}

bool CDataSet::init()
{
    CServiceManager::instance()->attachFunc(57, "GetDataSetDirectory", base::function(&CDataSet::getDSValueDir, this));
    CServiceManager::instance()->attachFunc(58, "GetDataSetValues", base::function(&CDataSet::getDSValues, this));
    return true;
}

ServiceError CDataSet::getDSValueDir(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetDataSetDirectory_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    std::string dataRef;
    if (reqPtr->datasetReference.buf != NULL)
    {
        dataRef = (char *)reqPtr->datasetReference.buf;
    }
    std::string specName;
    if (reqPtr->referenceAfter != NULL && reqPtr->referenceAfter->buf != NULL)
    {
        specName = (char *)reqPtr->referenceAfter->buf;
    }
    std::vector<std::string> result;
    if (!splitStr("/", dataRef, result))
    {
        return ServiceError_instance_not_available;
    }
    auto *ldInfo = m_scl->getLdInfo(result[0]);
    if (NULL == ldInfo)
    {
        warnf("dataSetRef[%s] not support\n", dataRef.c_str());
        return ServiceError_instance_not_available;
    }
    CSafeStruct<GetDataSetDirectory_ResponsePDU> respPtr;
    BOOLEAN_t *flag = CallocPtr(BOOLEAN_t);
    respPtr->moreFollows = flag;
    if (specName.empty())
    {
        getDSValueDirBegin(respPtr.get(), dataRef, result);
    }
    else
    {
        getDSValueDirSpec(respPtr.get(), dataRef, specName, result);
    }
    if (!Encode(respPtr, response))
    {
        errorf("encode %s failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    PrintAPER(respPtr);
    return ServiceError_no_error;
}

bool CDataSet::getDSValueDirBegin(void *ptr, const std::string &dataRef, const std::vector<std::string> &result)
{
    GetDataSetDirectory_ResponsePDU *respPtr = (GetDataSetDirectory_ResponsePDU*)ptr;
    auto *lnInfo = m_scl->getLnInfo(result[0], "LLN0");
    auto *dsInfo = m_scl->getDataSetInfo(lnInfo, result[1]);
    if (NULL == dsInfo)
    {
        infof("dsName[%s] not find\n", result[1].c_str());
        return false;
    }
    for (const auto &iter2 : dsInfo->fcda)
    {
        auto *member = CallocPtr(GetDataSetDirectory_ResponsePDU__memberData__Member);
        const std::string &subRef = result[0] + "/" + iter2.prefix + iter2.lnClass + iter2.lnInst + "." + iter2.doName;
        copyStrToOctStr(subRef, &member->reference);
        copyStrToOctStr(iter2.fc, &member->fc);
        ASN_SEQUENCE_ADD(&respPtr->memberData.list, member);
    }
    return true;
}

bool CDataSet::getDSValueDirSpec(void *ptr, const std::string &dataRef, const std::string &specName, const std::vector<std::string> &result)
{
    #if 0
    GetDataSetDirectory_ResponsePDU *respPtr = (GetDataSetDirectory_ResponsePDU*)ptr;
    auto &dataInfo = CSCLParse::instance()->getDataInfo();
    std::vector<std::string> resultSpec;
    if (!splitStr("/", specName, resultSpec))
    {
        return false;
    }
    auto leafInfo = dataInfo.mapLeafInfo[result[0]];
            auto *member = CallocPtr(GetDataSetDirectory_ResponsePDU__memberData__Member);
            std::string subRef = "KHPDFMONT/airGGIO6.hum.mag.f";
            copyStrToOctStr(subRef, &member->reference);
            copyStrToOctStr("MX", &member->fc);
            ASN_SEQUENCE_ADD(&respPtr->memberData.list, member);
            subRef = "q";
            copyStrToOctStr(subRef, &member->reference);
            copyStrToOctStr("MX", &member->fc);
            ASN_SEQUENCE_ADD(&respPtr->memberData.list, member);
            subRef = "t";
            copyStrToOctStr(subRef, &member->reference);
            copyStrToOctStr("MX", &member->fc);
            ASN_SEQUENCE_ADD(&respPtr->memberData.list, member);
    if (leafInfo.find(resultSpec[1]) != leafInfo.end())
    {
    const std::string &leafName = leafInfo[resultSpec[1]]
        return false;
    }
    #endif
    return true;
}

ServiceError CDataSet::getDSValues(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<GetDataSetValues_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    CSafeStruct<GetDataSetValues_ResponsePDU> respPtr;
    std::string strRef;
    if (reqPtr->datasetReference.buf != NULL)
    {
        strRef = (char *)reqPtr->datasetReference.buf;
    }
    if (strRef.empty())
    {
        warnf("datasetRef is NULL\n");
        return ServiceError_instance_not_available;
    }
    std::vector<std::string> result;
    splitStr("/", strRef, result);
    auto *lnInfo = m_scl->getLnInfo(result[0], "LLN0");
    auto *dsInfo = m_scl->getDataSetInfo(lnInfo, result[1]);
    if (strRef.empty())
    {
        warnf("datasetName[%s] not find\n", result[1].c_str());
        return ServiceError_instance_not_available;
    }
    for (auto &iter : dsInfo->fcda)
    {
        const std::string &doName = iter.prefix + iter.lnClass + iter.lnInst + "." + iter.doName;
        auto *leafInfo = m_scl->findLeafInfo(result[0], doName);
        if (NULL == leafInfo)
        {
            warnf("doName[%s] not find\n", doName.c_str());
            return ServiceError_instance_not_available;
        }
        infof("doName is %s, valueType[%s]\n", doName.c_str(), leafInfo->valueType.c_str());
        Data *member = CallocPtr(Data);
        member->present = getDataPR(leafInfo->valueType);
        ASN_SEQUENCE_ADD(&respPtr->value.list, member);
        if (leafInfo->valueType == "Struct")
        {
            getDSStruct(&member->choice.structure, &leafInfo->vecInfo);
        }
        else
        {
            void *addr = reinterpret_cast<void *>(&member->choice.error);
            copyMem(leafInfo->valueType, addr, leafInfo->data.data, leafInfo->data.len);
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

void CDataSet::getDSStruct(void *ptr, void *ptr1)
{
    auto *respPtr = (Data::Data_u::Data__structure*)ptr;
    auto *vecInfo = (std::vector<CSCLParse::DataInfo::LeafInfo>*)ptr1;
    for (auto &iter : *vecInfo)
    {
        Data *member = CallocPtr(Data);
        member->present = getDataPR(iter.valueType);
        ASN_SEQUENCE_ADD(&respPtr->list, member);
        if (iter.valueType == "Struct")
        {
            getDSStruct(&member->choice.structure, &iter.vecInfo);
        }
        else
        {
            infof("doName is %s, valueType[%s]\n", iter.name.c_str(), iter.valueType.c_str());
            void *addr = reinterpret_cast<void *>(&member->choice.error);
            copyMem(iter.valueType, addr, iter.data.data, iter.data.len);
        }
    }
}

static CDataSet s_dataset;

}
