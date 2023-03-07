#include "Associate.h"

#include <cmath>

#include "Service/ServiceManager.h"
#include "Print/Print.h"
#include "Event/IEventManager.h"
#include "Config/IConfigManager.h"

#include "asn1/AssociateNegotiate-RequestPDU.h"
#include "asn1/AssociateNegotiate-ResponsePDU.h"
#include "asn1/Associate-RequestPDU.h"
#include "asn1/Associate-ResponsePDU.h"
#include "asn1/Release-RequestPDU.h"
#include "asn1/Release-ResponsePDU.h"
#include "asn1/Abort-RequestPDU.h"

#include "gmssl/sm2.h"
#include "gmssl/x509.h"

#include "Common.h"

#include "SCL/SCLParse.h"

namespace cms {

SafeDef(AssociateNegotiate_RequestPDU)
SafeDef(AssociateNegotiate_ResponsePDU)
SafeDef(Associate_RequestPDU)
SafeDef(Associate_ResponsePDU)
SafeDef(Release_RequestPDU)
SafeDef(Release_ResponsePDU)
SafeDef(Abort_RequestPDU)

CAssociate::CAssociate()
    : m_stationId("linuxzq")
    , m_threadRes("AssociateManage")
{
    CServiceManager::instance()->attachService("Associate", this);
}

CAssociate::~CAssociate()
{}

bool CAssociate::init()
{
    CServiceManager::instance()->attachFunc(1, "Associate", base::function(&CAssociate::associate, this));
    CServiceManager::instance()->attachFunc(2, "Abort", base::function(&CAssociate::abort, this));
    CServiceManager::instance()->attachFunc(3, "Release", base::function(&CAssociate::release, this));
    CServiceManager::instance()->attachFunc(153, "Test", base::function(&CAssociate::test, this));
    CServiceManager::instance()->attachFunc(154, "AssociateNegotiate", base::function(&CAssociate::associateNegotiate, this));
    auto *pEvent = base::CComponentManager::instance()->getComponent<base::IEventManager>("EventManager");
    pEvent->attach("cmsSocket", base::IEventManager::Proc(&CAssociate::doSocketEvt, this));
    auto *pConfig = base::CComponentManager::instance()->getComponent<base::IConfigManager>("ConfigManager");
    Json::Value cfgValue;
    pConfig->getConfig("CMS61850", cfgValue);
    if (cfgValue["associate"]["timeDiff"].asInt() < 0)
    {
        warnf("timeDiff invaild, use default 60\n");
        cfgValue["associate"]["timeDiff"] = 60;
        pConfig->setConfig("CMS61850", cfgValue);
    }
    m_cfgValue = cfgValue["associate"];
    const Json::Value &appSec = m_cfgValue["safe"]["application"];
    if (appSec["enable"].asBool())
    {
        m_pAppSec.reset(new ApplicationSec());
        const std::string &rootCAPath = appSec["rootCA"].asString();
        const std::string &serverCertPath = appSec["serverCert"].asString();
        FILE *fpRoot = fopen(rootCAPath.c_str(), "r");
        FILE *fpSvr = fopen(serverCertPath.c_str(), "r");
        if (NULL == fpRoot || NULL == fpSvr)
        {
            warnf("rootCA or server cert path set failed\n");
            return false;
        }
        if (x509_cert_from_pem(m_pAppSec->rootCA.buf, &m_pAppSec->rootCA.len, sizeof(m_pAppSec->rootCA.buf), fpRoot) != 1)
        {
            errorf("load CA cert failed\n");
            return false;
        }
        if (x509_cert_from_pem(m_pAppSec->serverCert.buf, &m_pAppSec->serverCert.len, sizeof(m_pAppSec->serverCert.buf), fpSvr) != 1)
        {
            errorf("load server cert failed\n");
            return false;
        }
    }
    return true;
}

bool CAssociate::start()
{
    m_threadRes.setParam(base::function(&CAssociate::cleanTCP, this));
    m_threadRes();
    return true;
}

bool CAssociate::beforeAssociate(int clientId, std::string &response)
{
    base::clientInfo info;
    CServiceManager::instance()->getClientInfo(clientId, info);
    bool ret = true;
    std::lock_guard<std::mutex> Guard(m_mutex);
    for (const auto &iter : m_mapTcpInfo)
    {
        const auto &tmp = iter.second;
        if (tmp->status == Associate && tmp->clientInfo.ip == info.ip && tmp->clientInfo.port == info.port)
        {
            sendAbort(response);
            m_mapTcpInfo.erase(iter.first);
            break;
        }
        else if (tmp->status == Negotiate && tmp->clientInfo.ip == info.ip && tmp->clientInfo.port == info.port)
        {
            warnf("recv same ip[%s], port[%d] associate\n", info.ip.c_str(), info.port);
            CServiceManager::instance()->closeClient(clientId);
            break;
        }
        else if (tmp->clientInfo.ip == info.ip)
        {
            warnf("recv same ip[%s], associate\n", info.ip.c_str());
            CServiceManager::instance()->closeClient(iter.first);
            m_mapTcpInfo.erase(iter.first);
            break;
        }
        else if (tmp->status == Release && tmp->clientInfo.ip == info.ip && tmp->clientInfo.port == info.port)
        {
            ret = false;
            break;
        }
    }
    return ret;
}

void CAssociate::sendAbort(std::string &response)
{
    CSafeStruct<Abort_RequestPDU> respPtr;
    copyStrToOctStr(m_stationId, &respPtr->associationId);
    respPtr->reason = Abort_RequestPDU__reason_other;
    Encode(respPtr, response);
}

void CAssociate::cleanTCP()
{
    while (m_threadRes.looping())
    {
        auto now = base::CTime::getCurrentTime();
        m_mutex.lock();
        for (auto iter = m_mapTcpInfo.begin(); iter != m_mapTcpInfo.end();)
        {
            const auto &tmp = iter->second;
            if (tmp->status == Release || tmp->status == NegotiateFail || tmp->status == AssociateFail)
            {
                if (tmp->num > 60)
                {
                    base::clientInfo info;
                    CServiceManager::instance()->getClientInfo(iter->first, info);
                    warnf("ip[%s], port[%d], status[%d] timeout close connect\n", info.ip.c_str(), info.port, tmp->status);
                    CServiceManager::instance()->closeClient(iter->first);
                    m_mapTcpInfo.erase(iter++);
                    continue;
                }
                else
                {
                    tmp->num++;
                }
            }

            if (tmp->status == Associate && now - tmp->aliveTime > 3 * 60 * 1000)
            {
                #if 0
                base::clientInfo info;
                CServiceManager::instance()->getClientInfo(iter->first, info);
                warnf("ip[%s], port[%d], status[%d] recv test timeout\n", info.ip.c_str(), info.port, tmp->status);
                CServiceManager::instance()->closeClient(iter->first);
                m_mapTcpInfo.erase(iter++);
                continue;
                #endif
            }
            ++iter;
        }
        m_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void CAssociate::doSocketEvt(const std::string &code, const Json::Value &cfgValue)
{
    const std::string &state = cfgValue["state"].asString();
    if (state == "disconnect")
    {
        int clientId = cfgValue["clientId"].asInt();
        std::lock_guard<std::mutex> Guard(m_mutex);
        m_mapTcpInfo.erase(clientId);
    }
}

ServiceError CAssociate::associateNegotiate(const std::string &funcName, const NetMessage &message, std::string &response)
{
    if (!beforeAssociate(message.clientId, response))
    {
        return ServiceError_failed_due_to_communications_constraint;
    }
    auto info = std::make_shared<AssociateInfo>();
    CServiceManager::instance()->getClientInfo(message.clientId, info->clientInfo);
    info->status = NegotiateFail;
    info->aliveTime = base::CTime::getCurrentTime();
    m_mutex.lock();
    m_mapTcpInfo[message.clientId] = info;
    m_mutex.unlock();
    CServiceManager::instance()->setStatus(message.clientId, CServiceManager::NegotiateFail);
    CSafeStruct<AssociateNegotiate_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s request failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    PrintAPER(reqPtr);
    uint32_t apduSize = m_cfgValue["apduSize"].asInt();
    uint32_t asduSize = m_cfgValue["asduSize"].asInt();
    uint32_t protocol = m_cfgValue["protocol"].asInt();
    if (reqPtr->apduSize < apduSize || reqPtr->asduSize < asduSize)
    {
        warnf("client apduSize and asduSize must not small[%d], [%d]\n", apduSize, asduSize);
        return ServiceError_parameter_value_inappropriate;
    }
    if (reqPtr->protocolVersion != protocol)
    {
        warnf("client protocol[%lu] not support\n", reqPtr->protocolVersion);
        return ServiceError_failed_due_to_communications_constraint;
    }
    CSafeStruct<AssociateNegotiate_ResponsePDU> respPtr;
    /// 协商参数以客户端配置为准
    respPtr->apduSize = apduSize;
    respPtr->asduSize = asduSize;
    respPtr->protocolVersion = protocol;
    const auto &sclInfo = CSCLParse::instance()->getSCLInfo();
    copyStrToOctStr(sclInfo.configVersion, &respPtr->modelVersion);
    if (!Encode(respPtr, response))
    {
        errorf("encode %s response failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    PrintAPER(respPtr);
    info->status = Negotiate;
    CServiceManager::instance()->setStatus(message.clientId, CServiceManager::SUCCESS);
    return ServiceError_no_error;
}

ServiceError CAssociate::associate(const std::string &funcName, const NetMessage &message, std::string &response)
{
    m_mutex.lock();
    auto info = m_mapTcpInfo[message.clientId];
    m_mutex.unlock();
    info->status = AssociateFail;
    /// 协商一开始状态假定为Fail
    CServiceManager::instance()->setStatus(message.clientId, CServiceManager::AssociateFail);

    CSafeStruct<Associate_RequestPDU> reqPtr;
    /// 解码协商请求,填充到Associate_RequestPDU中
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("deocde %s failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    /// 打印请求内容
    PrintAPER(reqPtr);
    auto &sclInfo = CSCLParse::instance()->getSCLInfo();
    if (reqPtr->serverAccessPointReference != NULL && reqPtr->serverAccessPointReference->buf != NULL)
    {
        std::string serverPoint = (char *)reqPtr->serverAccessPointReference->buf;
        if (serverPoint != sclInfo.accessPoint)
        {
            return ServiceError_instance_not_available;
        }
    }
    /// 封装响应结构体
    CSafeStruct<Associate_ResponsePDU> respPtr;
    if (reqPtr->authenticationParameter != NULL)
    {
        /// 应用层安全校验
        if (!verifySoftSafe(reqPtr->authenticationParameter))
        {
            warnf("verify app safe failed\n");
            return ServiceError_failed_due_to_communications_constraint;
        }
        respPtr->authenticationParameter = CallocPtr(Associate_ResponsePDU::Associate_ResponsePDU__authenticationParameter);
        /// 回传检验信息
        if (!setSoftSafeAuth(respPtr->authenticationParameter))
        {
            warnf("set app safe param failed\n");
            return ServiceError_failed_due_to_communications_constraint;
        }
    }
    copyStrToOctStr(m_stationId, &respPtr->associationId);
    respPtr->serviceError = ServiceError_no_error;

    /// 对响应结构体进行编码
    if (!Encode(respPtr, response))
    {
        errorf("encode %s response failed\n", funcName.c_str());
        return ServiceError_failed_due_to_communications_constraint;
    }
    PrintAPER(respPtr);
    info->status = Associate;
    CServiceManager::instance()->setStatus(message.clientId, CServiceManager::SUCCESS);
    return ServiceError_no_error;
}

bool CAssociate::verifySoftSafe(void *ptr)
{
    if (NULL == m_pAppSec)
    {
        warnf("app sec may be not enable\n");
        return false;
    }
    auto *pAuth = (Associate_RequestPDU::Associate_RequestPDU__authenticationParameter*)ptr;
    std::string strCert = (char *)pAuth->signatureCertificate.buf;
    CertContent clientCert;
    if (x509_cert_from_pem_buf(clientCert.buf, &clientCert.len, sizeof(clientCert.buf), (uint8_t *)strCert.c_str(), strCert.length()) != 1)
    {
        return false;
    }
    if (x509_cert_verify_by_ca_cert(clientCert.buf, clientCert.len, m_pAppSec->rootCA.buf, m_pAppSec->rootCA.len, SM2_DEFAULT_ID, strlen(SM2_DEFAULT_ID)) < 0)
    {
        return false;
    }
    SM2_KEY key;
    if (x509_cert_get_subject_public_key(clientCert.buf, clientCert.len, &key) != 1)
    {
        errorf("get pubkey failed\n");
        return false;
    }
    SM2_SIGN_CTX verifyCtx;
    if (sm2_verify_init(&verifyCtx, &key, SM2_DEFAULT_ID, strlen(SM2_DEFAULT_ID)) != 1)
    {
        warnf("verify init failed\n");
        return false;
    }
    if (sm2_verify_update(&verifyCtx, pAuth->signedTime.buf, pAuth->signedTime.size) != 1)
    {
        warnf("update verify failed\n");
        return false;
    }
    if (sm2_verify_finish_no_der(&verifyCtx, pAuth->signedValue.buf, pAuth->signedValue.size) != 1)
    {
        errorf("sm2 verify failed\n");
        return false;
    }
    CMS_UTC_TIME stamp;
    getUtcTime(pAuth->signedTime, stamp);
    time_t timeNow = time(NULL);
    infof("stamp[%u], timeNow[%lu]\n", stamp.timeNow, timeNow);
    int timeDiff = m_cfgValue["timeDiff"].asInt();
    if (fabs(timeNow - stamp.timeNow) > timeDiff)
    {
        return false;
    }
    infof("verify cert success\n");
    return true;
}

bool CAssociate::setSoftSafeAuth(void *ptr)
{
    if (NULL == m_pAppSec)
    {
        warnf("app sec may be not enable\n");
        return false;
    }
    auto *pAuth = (Associate_RequestPDU::Associate_RequestPDU__authenticationParameter*)ptr;
    const Json::Value &appSec = m_cfgValue["safe"]["application"];
    const std::string &serverCertPath = appSec["serverCert"].asString();
    FILE *fpSvr = fopen(serverCertPath.c_str(), "r");
    if (NULL == fpSvr)
    {
        warnf("open svr cer failed\n");
        return false;
    }
    fseek(fpSvr, 0, SEEK_END);
    int length = ftell(fpSvr);
    void *svrMem = malloc((length + 1) * sizeof(char));
    rewind(fpSvr);
    fread(svrMem, length, 1, fpSvr);
    copyStrToOctStr((const char *)svrMem, length, &pAuth->signatureCertificate);
    free(svrMem);
    fclose(fpSvr);
    CMS_UTC_TIME stamp;
    stamp.timeNow = time(NULL);
    setUtcTime(stamp, pAuth->signedTime);

    SM2_KEY key;
    uint8_t *attr = (uint8_t*)calloc(512, sizeof(uint8_t));
    size_t attlen = 0;
    const std::string &serverKeyPath = appSec["serverKey"].asString();
    FILE *fpKey = fopen(serverKeyPath.c_str(), "r");
	if (sm2_private_key_info_from_pem(&key, &attr, &attlen, fpKey) != 1)
    {
        warnf("private key decryption failure\n");
        return false;
    }
#ifdef DEBUG
    for (int i = 0; i < 32; i++)
    {
        rawf("%x ", key.private_key[i]);
    }
    rawf("\n");
#endif
    free(attr);
    SM2_SIGN_CTX signCtx;
    if (sm2_sign_init(&signCtx, &key, SM2_DEFAULT_ID, strlen(SM2_DEFAULT_ID)) != 1)
    {
        warnf("sign init error\n");
        return false;
    }
    if (sm2_sign_update(&signCtx, pAuth->signedTime.buf, pAuth->signedTime.size) != 1)
    {
        warnf("sign update failed\n");
        return false;
    }
    uint8_t sig[SM2_MAX_SIGNATURE_SIZE];
    size_t siglen;
    if (sm2_sign_finish_no_der(&signCtx, sig, &siglen) != 1)
    {
        warnf("sign update failed\n");
        return false;
    }
    OCTET_STRING_t src;
    src.buf = sig;
    src.size = siglen;
    copyOctPtr(&pAuth->signedValue, &src, siglen);
    return true;
}

ServiceError CAssociate::release(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<Release_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("deocde %s failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    if ((const char *)reqPtr->associationId.buf != m_stationId)
    {
        errorf("release station[%s] failed, not find\n", reqPtr->associationId.buf);
        return ServiceError_parameter_value_inappropriate;
    }
    CSafeStruct<Release_ResponsePDU> respPtr;
    copyStrToOctStr(m_stationId, &respPtr->associationId);
    respPtr->serviceError = ServiceError_no_error;
    if (!Encode(respPtr, response))
    {
        errorf("encode Release response failed\n");
        return ServiceError_failed_due_to_communications_constraint;
    }
    infof("%s Release\n", m_stationId.c_str());
    m_mutex.lock();
    auto info = m_mapTcpInfo[message.clientId];
    m_mutex.unlock();
    info->status = Release;
    return ServiceError_no_error;
}

ServiceError CAssociate::abort(const std::string &funcName, const NetMessage &message, std::string &response)
{
    CSafeStruct<Abort_RequestPDU> reqPtr;
    if (!Decode(reqPtr, message.buf, message.len))
    {
        errorf("decode %s failed\n", funcName.c_str());
        return ServiceError_decode_error;
    }
    if ((const char *)reqPtr->associationId.buf != m_stationId)
    {
        errorf("abort station[%s] failed, not find\n", reqPtr->associationId.buf);
        return ServiceError_other;
    }
    infof("%s Abort, code[%ld]\n", m_stationId.c_str(), reqPtr->reason);
    CServiceManager::instance()->closeClient(message.clientId);
    m_mutex.lock();
    m_mapTcpInfo.erase(message.clientId);
    m_mutex.unlock();
    return ServiceError_no_error;
}

ServiceError CAssociate::test(const std::string &name, const NetMessage &message, std::string &response)
{
    m_mutex.lock();
    auto info = m_mapTcpInfo[message.clientId];
    m_mutex.unlock();
    info->aliveTime = base::CTime::getCurrentTime();
    return ServiceError_no_error;
}

static CAssociate s_ass;

}
