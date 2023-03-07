#include "SCLParse.h"

#include "Print/Print.h"
#include "Config/IConfigManager.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "asn1/Data.h"
#include "asn1/RCBOptFlds.h"
#include "asn1/TriggerConditions.h"
#include "Common.h"

namespace cms {

SINGTON_DEFINITION(CSCLParse)

CSCLParse::CSCLParse()
{}

CSCLParse::~CSCLParse()
{}

bool CSCLParse::init()
{
    infof("parse scl info start...\n");
    auto *pConfig = base::CComponentManager::instance()->getComponent<base::IConfigManager>("ConfigManager");
    Json::Value cfgValue;
    pConfig->getConfig("CMS61850", cfgValue);
    const std::string &icdPath = cfgValue["icdPath"].asString();
    if (icdPath.empty())
    {
        fatalf("please set icd path in CMS61850.json\n");
        assert(0);
    }
    TiXmlDocument doc(icdPath);
    doc.LoadFile();
    TiXmlElement *rootEle = doc.RootElement();
    if (NULL == rootEle)
    {
        errorf("parse IEC61850.icd failed\n");
        assert(0);
    }
    if (!parseIed(rootEle))
    {
        errorf("parse IED info failed\n");
        return false;
    }
    decltype(m_mapFunc) mapFunc {
        {"BOOLEAN", {1, base::function(&CSCLParse::allocDetail<BOOLEAN_t>, this)}},
        {"INT32", {4, base::function(&CSCLParse::allocDetail<INT32_t>, this)}},
        {"INT32U", {4, base::function(&CSCLParse::allocDetail<INT32U_t>, this)}},
        {"INT8", {1, base::function(&CSCLParse::allocDetail<INT8_t>, this)}},
        {"INT8U", {1, base::function(&CSCLParse::allocDetail<INT8U_t>, this)}},
        {"INT16", {2, base::function(&CSCLParse::allocDetail<INT16_t>, this)}},
        {"INT16U", {2, base::function(&CSCLParse::allocDetail<INT16U_t>, this)}},
        {"Enum", {1, base::function(&CSCLParse::allocDetail<INT8_t>, this)}},
        {"Timestamp", {8, base::function(&CSCLParse::allocDetail<UtcTime_t>, this)}},
        {"VisString64", {64, base::function(&CSCLParse::allocDetail<VisibleString64_t>, this)}},
        {"VisString255", {255, base::function(&CSCLParse::allocDetail<VisibleString255_t>, this)}},
        {"Unicode255", {510, base::function(&CSCLParse::allocDetail<UnicodeString255_t>, this)}},
        {"FLOAT32", {4, base::function(&CSCLParse::allocDetail<FLOAT32_t>, this)}},
        {"FLOAT64", {8, base::function(&CSCLParse::allocDetail<FLOAT64_t>, this)}},
        {"Quality", {13, base::function(&CSCLParse::allocDetail<Quality_t>, this)}},
        {"Dbpos", {2, base::function(&CSCLParse::allocDetail<Dbpos_t>, this)}},
        {"Check", {2, base::function(&CSCLParse::allocDetail<Check_t>, this)}},
        {"Octet64", {64, base::function(&CSCLParse::allocDetail<Octet64_t>, this)}}
    };
    m_mapFunc.swap(mapFunc);
    arrangeInfo();
    infof("parse scl info end\n");
    return true;
}

CSCLParse::LdInfo *CSCLParse::getLdInfo(const std::string &domName)
{
    if (domName.empty())
    {
        return NULL;
    }
    for (auto &iter : m_info.vecLdInfo)
    {
        if (iter.complexName == domName)
        {
            return &iter;
        }
    }
    return NULL;
}

CSCLParse::LnInfo *CSCLParse::getLnInfo(LdInfo *ldInfo, const std::string &lnName)
{
    if (NULL == ldInfo || lnName.empty())
    {
        return NULL;
    }
    for (auto &iter : ldInfo->vecLnInfo)
    {
        if (iter.complexName == lnName)
        {
            return &iter;
        }
    }
    return NULL;
}

CSCLParse::LnInfo *CSCLParse::getLnInfo(const std::string &ldName, const std::string &lnName)
{
    auto *ldInfo = getLdInfo(ldName);
    if (NULL == ldInfo)
    {
        return NULL;
    }
    return getLnInfo(ldInfo, lnName);
}

CSCLParse::Report *CSCLParse::getReportInfo(CSCLParse::LnInfo *lnInfo, const std::string &rpName)
{
    if (NULL == lnInfo || rpName.empty())
    {
        return NULL;
    }
    for (auto &iter : lnInfo->vecReport)
    {
        if (iter.rpName == rpName)
        {
            return &iter;
        }
    }
    return NULL;
}

CSCLParse::DataSet *CSCLParse::getDataSetInfo(CSCLParse::LnInfo *lnInfo, const std::string &dsName)
{
    if (NULL == lnInfo || dsName.empty())
    {
        return NULL;
    }
    for (auto &iter : lnInfo->vecDataSet)
    {
        if (iter.name == dsName)
        {
            return &iter;
        }
    }
    return NULL;
}

CSCLParse::DataInfo::DomInfo *CSCLParse::findDomInfo(const std::string &domName)
{
    if (domName.empty())
    {
        return NULL;
    }
    for (uint32_t i = 0; i < m_dInfo.vecLeafName.size(); i++)
    {
        if (m_dInfo.vecLeafName[i].domName == domName)
        {
            return &m_dInfo.vecLeafName[i];
        }
    }
    return NULL;
}

CSCLParse::DataInfo::LeafInfo *CSCLParse::findLeafInfo(DataInfo::DomInfo *domInfo, const std::string &leafName)
{
    if (leafName.empty() || NULL == domInfo)
    {
        return NULL;
    }
    auto f = [&](std::vector<DataInfo::LeafInfo> &vecInfo)
    {
        TFunction<DataInfo::LeafInfo*, std::vector<DataInfo::LeafInfo> &> s;
        s = [&](std::vector<DataInfo::LeafInfo> &vecInfo)
        {
            DataInfo::LeafInfo *ptr = NULL;
            /// 叶节点一般都在后面，倒序查找会更快点
            for (int i = vecInfo.size() - 1; i >= 0; i--)
            // for (int i = 0; i< vecInfo.size(); i++)
            {
                auto &tmp = vecInfo[i];
                if (tmp.name == leafName)
                {
                    debugf("find[%s], num[%d] ok\n", leafName.c_str(), i);
                    return &vecInfo[i];
                }
                else if (!tmp.vecInfo.empty())
                {
                    ptr = s(tmp.vecInfo);
                    if (NULL != ptr)
                    {
                        return ptr;
                    }
                }
            }
            return ptr;
        };
        return s(vecInfo);
    };
    return f(domInfo->leafInfo);
}

CSCLParse::DataInfo::LeafInfo *CSCLParse::findLeafInfo(const std::string &domName, const std::string &leafName)
{
    DataInfo::DomInfo *domInfo = findDomInfo(domName);
    if (NULL == domInfo)
    {
        return NULL;
    }
    return findLeafInfo(domInfo, leafName);
}

bool CSCLParse::parseIed(TiXmlElement *rootEle)
{
    TiXmlElement *iedEle = rootEle->FirstChildElement("IED");
    if (NULL == iedEle)
    {
        return false;
    }
    m_info.configVersion = iedEle->Attribute("configVersion");
    m_info.iedName = iedEle->Attribute("name");
    TiXmlElement *apEle = iedEle->FirstChildElement("AccessPoint");
    if (NULL == apEle)
    {
        return false;
    }
    TiXmlElement *dataTempEle = rootEle->FirstChildElement("DataTypeTemplates");
    if (NULL == dataTempEle)
    {
        return false;
    }
    if (!parseDateTemplate(dataTempEle))
    {
        errorf("parse dataTemplate failed\n");
        return false;
    }
    m_info.accessPoint = apEle->Attribute("name");
    if (!parseLd(apEle))
    {
        errorf("parse LD failed\n");
        return false;
    }
    return true;
}

bool CSCLParse::parseLd(TiXmlElement *ele)
{
    TiXmlElement *ldEle = ele->FirstChildElement()->FirstChildElement("LDevice");
    if (NULL == ldEle)
    {
        return false;
    }
    for (TiXmlElement *tmp = ldEle; tmp != NULL; tmp = tmp->NextSiblingElement())
    {
        const std::string &ldInst = tmp->Attribute("inst");
        LdInfo info;
        info.inst = ldInst;
        info.complexName = m_info.iedName + ldInst;
        if (!parseLn(tmp, info))
        {
            errorf("parse LN failed\n");
        }
        m_info.vecLdInfo.push_back(std::move(info));
    }
    return true;
}

bool CSCLParse::parseLn(TiXmlElement *ele, LdInfo &ldInfo)
{
    TiXmlElement *lnEle = ele->FirstChildElement();
    if (NULL == lnEle)
    {
        return false;
    }
    const std::string &ldName = m_info.iedName + ele->Attribute("inst");
    m_dInfo.vecLeafName.emplace_back();
    DataInfo::DomInfo &domInfo = *m_dInfo.vecLeafName.rbegin();
    domInfo.domName = ldName;
    for (TiXmlElement *tmp = lnEle; tmp != NULL; tmp = tmp->NextSiblingElement())
    {
        std::string desc;
        tmp->QueryValueAttribute("desc", &desc);
        std::string inst;
        tmp->QueryValueAttribute("inst", &inst);
        std::string lnClass;
        tmp->QueryValueAttribute("lnClass", &lnClass);
        std::string lnType;
        tmp->QueryValueAttribute("lnType", &lnType);
        std::string prefix;
        tmp->QueryValueAttribute("prefix", &prefix);
        const std::string &lnName = prefix + lnClass + inst;
        LnInfo info;
        info.complexName = lnName;
        info.desc = desc;
        info.inst = inst;
        info.lnClass = lnClass;
        info.lnType = lnType;
        info.prefix = prefix;
        TiXmlElement *doiEle = tmp->FirstChildElement();
        if (m_mapNodeEle.find(lnType) == m_mapNodeEle.end())
        {
            warnf("lnType not find[%s]\n", lnType.c_str());
            continue;
        }
        if (!parseNodeType(m_mapNodeEle[lnType], info.vecDataType, ldName, lnName, domInfo))
        {
            return false;
        }
        if (lnClass != "LLN0")
        {
            for (; doiEle != NULL; doiEle = doiEle->NextSiblingElement())
            {
                parseDOI(doiEle, info.vecDoInfo, ldName, lnName);
            }
        }
        else
        {
            for (; doiEle != NULL; doiEle = doiEle->NextSiblingElement())
            {
                const std::string &type = doiEle->ValueStr();
                if (type == "DataSet")
                {
                    parseDataSet(doiEle, info.vecDataSet);
                }
                else if (type == "ReportControl")
                {
                    parseReport(ldName, doiEle, info.vecReport);
                }
                else if (type == "SettingControl")
                {
                    info.setCtrl = new SettingCtrl();
                    if (!parseSetting(doiEle, info.setCtrl))
                    {
                        return false;
                    }
                }
            }
        }
        ldInfo.vecLnInfo.push_back(std::move(info));
    }
    return true;
}

bool CSCLParse::parseDateTemplate(TiXmlElement *ele)
{
    TiXmlElement *nodeEle = ele->FirstChildElement();
    if (NULL == nodeEle)
    {
        return false;
    }
    for (; nodeEle != NULL; nodeEle = nodeEle->NextSiblingElement())
    {
        std::string strId;
        nodeEle->QueryValueAttribute("id", &strId);
        m_mapNodeEle[strId] = nodeEle;
    }
    return true;
}

bool CSCLParse::parseNodeType(TiXmlElement *ele, std::vector<DataType> &vecDataType, const std::string &ldName, const std::string &lnName, DataInfo::DomInfo &domInfo)
{
    TiXmlElement *doEle = ele->FirstChildElement();
    if (NULL == doEle)
    {
        return false;
    }
    for (; doEle != NULL; doEle = doEle->NextSiblingElement())
    {
        std::string prefixName(lnName);
        std::string name;
        doEle->QueryValueAttribute("name", &name);
        std::string type;
        doEle->QueryValueAttribute("type", &type);
        if (m_mapNodeEle.find(type) == m_mapNodeEle.end())
        {
            warnf("node name[%s], type[%s] not find\n", name.c_str(), type.c_str());
            return false;
        }
        DataType dataType;
        dataType.doName = name;
        prefixName += "." + name;
        parseDoType(m_mapNodeEle[type], dataType, ldName, prefixName, domInfo);
        vecDataType.push_back(std::move(dataType));
    }
    return true;
}

bool CSCLParse::parseDoType(TiXmlElement *ele, DataType &dataType, const std::string &ldName, const std::string &prefixName, DataInfo::DomInfo &domInfo)
{
    ele->QueryValueAttribute("cdc", &dataType.cdc);
    TiXmlElement *daEle = ele->FirstChildElement();
    if (NULL == daEle)
    {
        return false;
    }

    DataInfo::LeafInfo leafInfo;
    leafInfo.name = prefixName;
    // auto &leafInfo = m_dInfo.mapLeafInfo[ldName][prefixName];
    leafInfo.cdcType = dataType.cdc;
    leafInfo.valueType = "Struct";
    for (; daEle != NULL; daEle = daEle->NextSiblingElement())
    {
        DataAttr attr;
        std::string pName(prefixName);
        daEle->QueryValueAttribute("bType", &attr.btype);
        daEle->QueryValueAttribute("name", &attr.name);
        daEle->QueryValueAttribute("fc", &attr.fc);
        /// 标准所述，CO不应该继续使用
        if (attr.fc == "CO")
        {
            warnf("name[%s], fc[CO] not support in cms61850\n", attr.name.c_str());
            continue;
        }
        pName += "." + attr.name;
        std::string type;
        daEle->QueryValueAttribute("type", &type);
        TiXmlElement *valEle = daEle->FirstChildElement("Val");
        if (NULL != valEle)
        {
            attr.value = valEle->GetText();
        }
        DataInfo::LeafInfo leafInfo1;
        if (!type.empty() && attr.btype != "Enum")
        {
            if (m_mapNodeEle.find(type) == m_mapNodeEle.end())
            {
                return false;
            }
            parseDaType(m_mapNodeEle[type], attr.bda, ldName, pName, leafInfo1);
        }
        leafInfo1.name = pName;
        leafInfo1.fc = attr.fc;
        if (!attr.value.empty())
        {
            leafInfo1.initValue = attr.value;
        }
        leafInfo1.valueType = attr.btype;
        leafInfo.vecInfo.push_back(std::move(leafInfo1));
        /// 内存地址绑定
        attr.data = &leafInfo1.data;
        dataType.attr.push_back(std::move(attr));
    }
    domInfo.leafInfo.push_back(std::move(leafInfo));
    return true;
}

bool CSCLParse::parseDaType(TiXmlElement *ele, std::vector<DataAttr> &bda, const std::string &ldName, const std::string &prefixName, DataInfo::LeafInfo &leafInfo)
{
    TiXmlElement *baEle = ele->FirstChildElement();
    if (NULL == baEle)
    {
        return false;
    }
    for (; baEle != NULL; baEle = baEle->NextSiblingElement())
    {
        DataAttr tmp;
        std::string pName(prefixName);
        baEle->QueryValueAttribute("name", &tmp.name);
        baEle->QueryValueAttribute("bType", &tmp.btype);
        pName += "." + tmp.name;
        std::string type;
        baEle->QueryValueAttribute("type", &type);
        DataInfo::LeafInfo leafInfo1;
        if (!type.empty() && tmp.btype != "Enum")
        {
            if (m_mapNodeEle.find(type) == m_mapNodeEle.end())
            {
                return false;
            }
            parseDaType(m_mapNodeEle[type], tmp.bda, ldName, pName, leafInfo1);
        }
        leafInfo1.name = pName;
        leafInfo1.valueType = tmp.btype;
        /// 内存地址绑定
        tmp.data = &leafInfo1.data;
        bda.push_back(std::move(tmp));
        leafInfo.vecInfo.push_back(std::move(leafInfo1));
    }
    return true;
}

bool CSCLParse::parseDataSet(TiXmlElement *ele, std::vector<DataSet> &vecSet)
{
    const std::string &name = ele->Attribute("name");
    DataSet setInfo;;
    setInfo.name = "LLN0." + name;
    TiXmlElement *fcdaEle = ele->FirstChildElement();
    for (; fcdaEle != NULL; fcdaEle = fcdaEle->NextSiblingElement())
    {
        DataSet::FCDA fcda;
        fcdaEle->QueryValueAttribute("ldInst", &fcda.ldInst);
        fcdaEle->QueryValueAttribute("lnInst", &fcda.lnInst);
        fcdaEle->QueryValueAttribute("prefix", &fcda.prefix);
        fcdaEle->QueryValueAttribute("lnClass", &fcda.lnClass);
        fcdaEle->QueryValueAttribute("doName", &fcda.doName);
        fcdaEle->QueryValueAttribute("fc", &fcda.fc);
        setInfo.fcda.push_back(std::move(fcda));
    }
    vecSet.push_back(std::move(setInfo));
    return true;
}

bool CSCLParse::parseReport(const std::string &ldName, TiXmlElement *ele, std::vector<Report> &vecReport)
{
    TiXmlElement *enableEle = ele->FirstChildElement("RptEnabled");
    if (NULL == enableEle)
    {
        return false;
    }
    int max = 0;
    if (enableEle->QueryIntAttribute("max", &max) != TIXML_SUCCESS)
    {
        errorf("get report max failed\n");
        return false;
    }
    if (max < 0 || max > 99)
    {
        errorf("rptEnable must 1-99\n");
        return false;
    }

    for (int i = 1; i <= max; i++)
    {
        std::stringstream stream;
        stream.fill('0');
        stream << std::setw(2) << i;
        std::string name;
        ele->QueryValueAttribute("name", &name);
        const std::string &rpName = name + stream.str();
        const std::string &rptId = ele->Attribute("rptID");
        Report reportInfo;
        reportInfo.rptEna = false;
        reportInfo.max = max;
        reportInfo.rptId = rpName;
        std::string buffed;
        ele->QueryValueAttribute("buffered", &buffed);
        if (buffed == "false" || buffed.empty())
        {
            reportInfo.buffered = false;
        }
        else
        {
            reportInfo.buffered = true;
        }
        if (ele->QueryIntAttribute("confRev", &reportInfo.confRev) != TIXML_SUCCESS)
        {
            return false;
        }
        std::string dsName;
        ele->QueryValueAttribute("datSet", &dsName);
        reportInfo.datSet = ldName + "/LLN0." + dsName;
        std::string intgPd;
        ele->QueryValueAttribute("intgPd", &intgPd);
        reportInfo.intgPd = atoi(intgPd.c_str());
        TiXmlElement *trgEle = ele->FirstChildElement("TrgOps");
        if (NULL == trgEle)
        {
            return false;
        }
        if (!parseTrgOps(trgEle, reportInfo))
        {
            return false;
        }
        TiXmlElement *optEle = ele->FirstChildElement("OptFields");
        if (NULL == optEle)
        {
            return false;
        }
        if (!parseOptFields(optEle, reportInfo))
        {
            return false;
        }
        reportInfo.rpName = rpName;
        vecReport.push_back(std::move(reportInfo));
    }
    return true;
}

bool CSCLParse::parseSetting(TiXmlElement *ele, SettingCtrl *setCtrl)
{
    if (ele->QueryIntAttribute("actSG", &setCtrl->actSG) != TIXML_SUCCESS)
    {
        return false;
    }
    setCtrl->editSG = setCtrl->actSG;
    if (ele->QueryIntAttribute("numOfSGs", &setCtrl->numOfSGs) != TIXML_SUCCESS)
    {
        return false;
    }
    return true;
}

bool CSCLParse::parseTrgOps(TiXmlElement *ele, Report &info)
{
    DataInfo::LeafInfo leafInfo;
    allocDetail<BIT_STRING_t>(6, leafInfo);
    info.trgOps = (BIT_STRING_t *)leafInfo.data.data;
    auto f = [&](const std::string &value, e_TriggerConditions trg)
    {
        bool result;
        (value == "false" || value.empty()) ? result = false : result = true;
        if (result)
        {
            BIT_SET_ON(info.trgOps->buf, trg);
        }
    };
    std::string value;
    ele->QueryValueAttribute("dchg", &value);
    f(value, TriggerConditions_data_change);
    value.clear();
    ele->QueryValueAttribute("qchg", &value);
    f(value, TriggerConditions_quality_change);
    value.clear();
    ele->QueryValueAttribute("period", &value);
    f(value, TriggerConditions_integrity);
    value.clear();
    ele->QueryValueAttribute("dupd", &value);
    f(value, TriggerConditions_data_update);
    value.clear();
    ele->QueryValueAttribute("gi", &value);
    f(value, TriggerConditions_general_interrogation);
    return true;
}

bool CSCLParse::parseOptFields(TiXmlElement *ele, Report &info)
{
    DataInfo::LeafInfo leafInfo;
    allocDetail<BIT_STRING_t>(10, leafInfo);
    info.optFields = (BIT_STRING_t *)leafInfo.data.data;
    auto f = [&](const std::string &value, e_RCBOptFlds opt)
    {
        bool result = false;
        (value == "false" || value.empty()) ? result = false : result = true;
        if (result)
        {
            BIT_SET_ON(info.optFields->buf, opt);
        }
    };
    std::string value;
    ele->QueryValueAttribute("configRef", &value);
    f(value, RCBOptFlds_conf_revision);
    value.clear();
    ele->QueryValueAttribute("dataRef", &value);
    f(value, RCBOptFlds_data_reference);
    value.clear();
    ele->QueryValueAttribute("dataSet", &value);
    f(value, RCBOptFlds_data_set_name);
    value.clear();
    ele->QueryValueAttribute("entryID", &value);
    f(value, RCBOptFlds_entryID);
    value.clear();
    ele->QueryValueAttribute("reasonCode", &value);
    f(value, RCBOptFlds_reason_for_inclusion);
    value.clear();
    ele->QueryValueAttribute("seqNum", &value);
    f(value, RCBOptFlds_sequence_number);
    value.clear();
    ele->QueryValueAttribute("timeStamp", &value);
    f(value, RCBOptFlds_report_time_stamp);
    value.clear();
    ele->QueryValueAttribute("bufOvfl", &value);
    f(value, RCBOptFlds_buffer_overflow);
    return true;
}

bool CSCLParse::parseDOI(TiXmlElement *ele, std::vector<DOIInfo> &vecDoi, const std::string &ldName, const std::string &lnName)
{
    const std::string &name = ele->Attribute("name");
    DOIInfo doiInfo;
    doiInfo.name = name;
    TiXmlElement *diEle = ele->FirstChildElement();
    for (; diEle != NULL; diEle = diEle->NextSiblingElement())
    {
        std::string prefixName(lnName + "." + name);
        const std::string &type = diEle->ValueStr();
        if (type == "SDI")
        {
            parseSDI(diEle, doiInfo.sdi, ldName, prefixName);
        }
        else if (type == "DAI")
        {
            parseDAI(diEle, doiInfo.dai, ldName, prefixName);
        }
    }
    vecDoi.push_back(std::move(doiInfo));
    return true;
}

bool CSCLParse::parseSDI(TiXmlElement *ele, std::vector<DOIInfo::SDI> &vecSdi, const std::string &ldName, std::string &prefixName)
{
    DOIInfo::SDI sdi;
    ele->QueryValueAttribute("name", &sdi.name);
    prefixName += "." + sdi.name;
    TiXmlElement *diEle = ele->FirstChildElement();
    for (; diEle != NULL; diEle = diEle->NextSiblingElement())
    {
        const std::string &type = diEle->ValueStr();
        if (type == "SDI")
        {
            parseSDI(diEle, sdi.sdi, ldName, prefixName);
        }
        else if (type == "DAI")
        {
            parseDAI(diEle, sdi.dai, ldName, prefixName);
        }
    }
    vecSdi.push_back(std::move(sdi));
    return true;
}

bool CSCLParse::parseDAI(TiXmlElement *ele, std::vector<DOIInfo::DAI> &vecDai, const std::string &ldName, std::string &prefixName)
{
    DOIInfo::DAI dai;
    ele->QueryValueAttribute("name", &dai.name);
    ele->QueryValueAttribute("sAddr", &dai.sAddr);
    prefixName += "." + dai.name;
    DataInfo::LeafInfo *leafInfo = findLeafInfo(ldName, prefixName);
    if (NULL == leafInfo)
    {
        errorf("find domName[%s], lnName[%s] dai failed\n", ldName.c_str(), prefixName.c_str());
        return false;
    }
    leafInfo->name = prefixName;
    TiXmlElement *valEle = ele->FirstChildElement("Val");
    if (valEle != NULL)
    {
        dai.val = valEle->GetText();
        leafInfo->initValue = dai.val;
    }
    vecDai.push_back(std::move(dai));
    return true;
}

void CSCLParse::arrangeInfo()
{
    std::fstream fp("datamap.cfg", std::ios::out | std::ios::trunc);
    auto f = [&](const std::string &domName, std::vector<DataInfo::LeafInfo> &info)
    {
        TFunction<void, const std::string &, std::vector<DataInfo::LeafInfo> &> s;
        s = [&](const std::string &domName, std::vector<DataInfo::LeafInfo> &info)
        {
            for (auto &iter : info)
            {
                if (m_mapFunc.find(iter.valueType) != m_mapFunc.end())
                {
                    const auto &func = m_mapFunc[iter.valueType];
                    func.second(func.first, iter);
                }
                fp << domName << "    \t" << iter.name << "    \t" << iter.valueType << "\t\n";
                if (!iter.vecInfo.empty())
                {
                    s(domName, iter.vecInfo);
                }
            }
        };
        s(domName, info);
    };
    for (auto &iter : m_dInfo.vecLeafName)
    {
        f(iter.domName, iter.leafInfo);
    }
    fp.close();
}

}
