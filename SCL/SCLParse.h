/**
 * @file SCLParse.h
 * @author (linuxzq93@163.com)
 * @brief SCL解析类
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_SCL_SCLPARSE_H__
#define __CMS61850_SCL_SCLPARSE_H__

#include "SingTon/SingTon.h"
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "tinyxml/tinyxml.h"
#include "asn1/VisibleString64.h"
#include "asn1/VisibleString129.h"
#include "asn1/VisibleString255.h"
#include "asn1/UnicodeString255.h"
#include "asn1/Octet64.h"
#include <asn1/BIT_STRING.h>
#include "Function/Bind.h"
#include "Traits/Traits.h"
#include "Print/Print.h"

namespace cms {

class CSCLParse {
SINGTON_DECLAR(CSCLParse)
public:
/// 叶节点内存
struct DataMem {
    void *data;
    int len;
    DataMem() : data(NULL), len(0) {}
};

struct DataAttr {
    std::string btype;
    std::string fc;
    std::string name;
    std::string value;
    DataMem *data;
    std::vector<DataAttr> bda;
};
struct DataType {
    std::string doName;
    std::string cdc;
    std::vector<DataAttr> attr;
};

struct DataSet {
    struct FCDA {
        std::string ldInst;
        std::string prefix;
        std::string lnInst;
        std::string lnClass;
        std::string doName;
        std::string fc;
    };
    std::string name;
    std::vector<FCDA> fcda;
};

struct Report {
    bool rptEna;
    std::string rptId;
    bool buffered;
    int confRev;
    std::string datSet;
    int intgPd;
    BIT_STRING_t *trgOps;
    BIT_STRING_t *optFields;
    int max;

    std::string rpName;
};

struct SettingCtrl {
    int actSG;
    int editSG;
    int numOfSGs;
};

struct DOIInfo {
    struct DAI {
        std::string name;
        std::string val;
        std::string sAddr;
    };
    struct SDI {
        std::string name;
        std::vector<DAI> dai;
        std::vector<SDI> sdi;
    };
    std::vector<DAI> dai;
    std::vector<SDI> sdi;
    std::string name;
};

struct LnInfo {
    std::string desc;
    std::string inst;
    std::string lnClass;
    std::string lnType;
    std::string prefix;
    /// 组装的名字，prefix+lnClass+inst
    std::string complexName;
    SettingCtrl *setCtrl;
    std::vector<DataSet> vecDataSet;
    std::vector<Report> vecReport;
    std::vector<DOIInfo> vecDoInfo;
    std::vector<DataType> vecDataType;
    LnInfo() : setCtrl(NULL) {}
};

struct LdInfo {
    std::string inst;
    std::string complexName;
    std::vector<LnInfo> vecLnInfo;
    // std::map<std::string, LnInfo> mapLnInfo;
};

struct SclInfo {
    std::string configVersion;
    std::string iedName;
    std::string accessPoint;
    std::vector<LdInfo> vecLdInfo;
    // std::map<std::string, LdInfo> mapLdInfo;
};

struct DataInfo {
    struct LeafInfo {
        std::string name;
        std::string valueType;
        std::string fc;
        DataMem data;
        std::string initValue;
        std::string cdcType;
        /// 子节点
        std::vector<LeafInfo> vecInfo;
    };
    struct DomInfo {
        std::string domName;
        std::vector<LeafInfo> leafInfo;
    };
    /* key1 domName
       key2 lnName
    */
    // std::map<std::string, std::map<std::string, LeafInfo>> mapLeafInfo;
    std::vector<DomInfo> vecLeafName;
};

public:
    bool init();
    SclInfo &getSCLInfo() {return m_info;}
    DataInfo &getDataInfo() {return m_dInfo;}

public:
    /// 查找scl里的信息
    LdInfo *getLdInfo(const std::string &domName);
    LnInfo *getLnInfo(LdInfo *ldInfo, const std::string &lnName);
    LnInfo *getLnInfo(const std::string &ldName, const std::string &lnName);
    Report *getReportInfo(LnInfo *lnInfo, const std::string &rpName);
    DataSet *getDataSetInfo(LnInfo *lnInfo, const std::string &dsName);

public:
    /// 查找响应节点信息
    /// domName iedName + ldName
    /// leafName ln.do.name
    DataInfo::DomInfo *findDomInfo(const std::string &domName);
    DataInfo::LeafInfo *findLeafInfo(DataInfo::DomInfo *domInfo, const std::string &leafName);
    DataInfo::LeafInfo *findLeafInfo(const std::string &domName, const std::string &leafName);

private:
    bool parseIed(TiXmlElement *rootEle);
    bool parseLd(TiXmlElement *ele);
    bool parseLn(TiXmlElement *ele, LdInfo &ldInfo);
    bool parseDateTemplate(TiXmlElement *ele);
    bool parseNodeType(TiXmlElement *ele, std::vector<DataType> &vecDataType, const std::string &ldName, const std::string &lnName, DataInfo::DomInfo &domInfo);
    bool parseDoType(TiXmlElement *ele, DataType &dataType, const std::string &ldName, const std::string &prefixName, DataInfo::DomInfo &domInfo);
    bool parseDaType(TiXmlElement *ele, std::vector<DataAttr> &bda, const std::string &ldName, const std::string &prefixName, DataInfo::LeafInfo &leafInfo);

    bool parseDataSet(TiXmlElement *ele, std::vector<DataSet> &vecSet);

    bool parseReport(const std::string &ldName, TiXmlElement *ele, std::vector<Report> &vecReport);
    bool parseSetting(TiXmlElement *ele, SettingCtrl *setCtrl);
    bool parseTrgOps(TiXmlElement *ele, Report &info);
    bool parseOptFields(TiXmlElement *ele, Report &info);

    bool parseDOI(TiXmlElement *ele, std::vector<DOIInfo> &vecDoi, const std::string &ldName, const std::string &prefixName);
    bool parseSDI(TiXmlElement *ele, std::vector<DOIInfo::SDI> &sdi, const std::string &ldName, std::string &prefixName);
    bool parseDAI(TiXmlElement *ele, std::vector<DOIInfo::DAI> &dai, const std::string &ldName, std::string &prefixName);

    void arrangeInfo();

    /// return SFINAE
    /// 为统一调用函数接口
    template<typename type>
    typename std::enable_if<base::negation<std::is_class<type>>::value>::type
    allocDetail(int maxLen, DataInfo::LeafInfo &info)
    {
        info.data.data = calloc(1, sizeof(type));
        info.data.len = sizeof(type);
        if (!info.initValue.empty())
        {
            type val = atoi(info.initValue.c_str());
            memcpy(info.data.data, &val, sizeof(type));
        }
        debugf2("type[%s], data[%p], len[%d]\n", info.valueType.c_str(), info.data.data, info.data.len);
        // *(type*)info.data.data = 1;
    }
    template<typename type>
    typename std::enable_if<std::is_convertible<type, OCTET_STRING_t>::value>::type
    allocDetail(int maxLen, DataInfo::LeafInfo &info)
    {
        type *pData = (type *)calloc(1, sizeof(type));
        pData->buf = (uint8_t*)calloc(1, maxLen);
        pData->size = maxLen;
        /// 字符串的长度初始为0，其余类型都有默认值及长度
        if (info.valueType.find("VisString") != std::string::npos || info.valueType.find("Unicode") != std::string::npos)
        {
            pData->size = 0;
        }
        info.data.data = pData;
        info.data.len = sizeof(type);
        if (!info.initValue.empty())
        {
            memcpy(pData->buf, info.initValue.c_str(), info.initValue.length());
            pData->size = info.initValue.length();
        }
        debugf2("type[%s], data[%p], len[%d], size[%lu]\n", info.valueType.c_str(), info.data.data, info.data.len, pData->size);
    }
    template <typename type>
    typename std::enable_if<std::is_convertible<type, BIT_STRING_t>::value>::type
    allocDetail(int maxLen, DataInfo::LeafInfo &info)
    {
        type *pData = (type *)calloc(1, sizeof(type));
        int len = (maxLen + 7) / 8;
        pData->buf = (uint8_t*)calloc(1, len);
        pData->size = len;
        pData->bits_unused = (8 - maxLen % 8) % 8;
        info.data.data = pData;
        info.data.len = sizeof(type);
        if (!info.initValue.empty())
        {
            memcpy(pData->buf, info.initValue.c_str(), info.initValue.length());
            pData->size = (info.initValue.length() + 7) / 8;
            pData->bits_unused = (8 - pData->size % 8) % 8;
        }
        debugf2("type[%s], data[%p], len[%d], size[%lu]\n", info.valueType.c_str(), info.data.data, info.data.len, pData->size);
    }

private:
    SclInfo m_info;
    std::map<std::string, TiXmlElement*> m_mapNodeEle;
    DataInfo m_dInfo;
    std::map<std::string, std::pair<int, TFunction<void, int, DataInfo::LeafInfo&>>> m_mapFunc;
};

}

#endif /* __CMS61850_SCL_SCLPARSE_H__ */
