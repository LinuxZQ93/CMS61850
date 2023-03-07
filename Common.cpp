#include "Common.h"

#include <map>
#include <set>
#include "Print/Print.h"

namespace cms {
SafeDef(ServiceError)
void encodeErr(ServiceError code, std::string &response)
{
    CSafeStruct<ServiceError> respPtr;
    *respPtr = code;
    Encode(respPtr, response);
}

void copyStrToOctStr(const std::string &src, OCTET_STRING_t *dst)
{
    if (NULL == dst)
    {
        return;
    }
    if (NULL == dst->buf)
    {
        dst->buf = (uint8_t *)malloc(src.length() * sizeof(uint8_t));
    }
    memcpy(dst->buf, src.c_str(), src.length());
    dst->size = src.length();
}

void copyStrToOctStr(const char *src, int len, OCTET_STRING_t *dst)
{
    if (NULL == dst)
    {
        return;
    }
    if (NULL == dst->buf)
    {
        dst->buf = (uint8_t *)malloc(len * sizeof(uint8_t));
    }
    memcpy(dst->buf, src, len);
    dst->size = len;
}

DataDefinition_PR getDataDefPR(const std::string &dataType)
{
    static std::map<std::string, DataDefinition_PR> s_mapDef{
        {"BOOLEAN", DataDefinition_PR_boolean},
        {"Struct", DataDefinition_PR_structure},
        {"INT8", DataDefinition_PR_int8},
        {"Enum", DataDefinition_PR_int8},
        {"INT16", DataDefinition_PR_int16},
        {"INT32", DataDefinition_PR_int32},
        {"INT64", DataDefinition_PR_int64},
        {"INT8U", DataDefinition_PR_int8u},
        {"INT16U", DataDefinition_PR_int16u},
        {"INT32U", DataDefinition_PR_int32u},
        {"INT64U", DataDefinition_PR_int64u},
        {"FLOAT32", DataDefinition_PR_float32},
        {"FLOAT64", DataDefinition_PR_float64},
        {"VisString255", DataDefinition_PR_visible_string},
        {"VisString64", DataDefinition_PR_visible_string},
        {"Octet64", DataDefinition_PR_octet_string},
        {"Unicode255", DataDefinition_PR_unicode_string},
        {"Timestamp", DataDefinition_PR_utc_time},
        {"Quality", DataDefinition_PR_quality},
        {"Dbpos", DataDefinition_PR_dbpos},
        {"Check", DataDefinition_PR_check}
    };
    if (s_mapDef.find(dataType) != s_mapDef.end())
    {
        return s_mapDef[dataType];
    }
    if (!dataType.empty())
    {
        warnf("dataType[%s] not support\n", dataType.c_str());
    }
    return DataDefinition_PR_NOTHING;
}

Data_PR getDataPR(const std::string &dataType)
{
    static std::map<std::string, Data_PR> s_mapDef{
        {"BOOLEAN", Data_PR_boolean},
        {"Struct", Data_PR_structure},
        {"INT8", Data_PR_int8},
        {"Enum", Data_PR_int8},
        {"INT16", Data_PR_int16},
        {"INT32", Data_PR_int32},
        {"INT64", Data_PR_int64},
        {"INT8U", Data_PR_int8u},
        {"INT16U", Data_PR_int16u},
        {"INT32U", Data_PR_int32u},
        {"INT64U", Data_PR_int64u},
        {"FLOAT32", Data_PR_float32},
        {"FLOAT64", Data_PR_float64},
        {"VisString255", Data_PR_visible_string},
        {"VisString64", Data_PR_visible_string},
        {"Octet64", Data_PR_octet_string},
        {"Unicode255", Data_PR_unicode_string},
        {"Timestamp", Data_PR_utc_time},
        {"Quality", Data_PR_quality},
        {"Dbpos", Data_PR_dbpos},
        {"Check", Data_PR_check}
    };
    if (s_mapDef.find(dataType) != s_mapDef.end())
    {
        return s_mapDef[dataType];
    }
    if (!dataType.empty())
    {
        warnf("dataType[%s] not support\n", dataType.c_str());
    }
    return Data_PR_NOTHING;

}

void copyMem(const std::string &name, void *dst, void *src, int len)
{
    static std::map<std::string, TFunction<void, void*, void*, int>> s_mapFunc{
        {"Timestamp", copyOctPtr},
        {"VisString64", copyOctPtr},
        {"VisString255", copyOctPtr},
        {"Unicode255", copyOctPtr},
        {"FLOAT32", copyOctPtr},
        {"FLOAT64", copyOctPtr},
        {"Octet64", copyOctPtr},
        {"Quality", copyBitPtr},
        {"Dbpos", copyBitPtr},
        {"Check", copyBitPtr}
    };
    if (s_mapFunc.find(name) != s_mapFunc.end())
    {
        s_mapFunc[name](dst, src, len);
    }
    else
    {
        copyRawPtr(dst, src, len);
    }
}

void copyOctPtr(void *dst, void *src, int len)
{
    if (NULL == dst)
    {
        return;
    }
    OCTET_STRING_t *pSrc = (OCTET_STRING_t*)src;
    OCTET_STRING_t *pDst = (OCTET_STRING_t*)dst;
    if (NULL == pDst->buf)
    {
        pDst->buf = (uint8_t *)calloc(1, pSrc->size * sizeof(uint8_t));
    }
    pDst->size = pSrc->size;
    debugf("dstbuf[%p], srcBuf[%p], size[%lu]\n", pDst->buf, pSrc->buf, pSrc->size);
    memcpy(pDst->buf, pSrc->buf, pSrc->size);
}

void copyBitPtr(void *dst, void *src, int len)
{
    if (NULL == dst)
    {
        return;
    }
    BIT_STRING_t *pSrc = (BIT_STRING_t *)src;
    BIT_STRING_t *pDst = (BIT_STRING_t *)dst;
    if (NULL == pDst->buf)
    {
        pDst->buf = (uint8_t *)calloc(1, pSrc->size * sizeof(uint8_t));
    }
    debugf("dstbuf[%p], srcBuf[%p], size[%lu], bitunuse[%d]\n", pDst->buf, pSrc->buf, pSrc->size, pSrc->bits_unused);
    memcpy(pDst->buf, pSrc->buf, pSrc->size);
    pDst->size = pSrc->size;
    pDst->bits_unused = pSrc->bits_unused;
}

bool splitStr(const std::string &symbol, const std::string &str, std::vector<std::string> &result)
{
    std::string::size_type pos = 0;
    std::string::size_type pos1 = str.find(symbol);
    while (pos1 != std::string::npos)
    {
        const std::string &tmp = str.substr(pos, pos1 - pos);
        if (!tmp.empty())
        {
            result.push_back(tmp);
        }
        pos = pos1 + 1;
        pos1 = str.find(symbol, pos1 + 1);
    }
    if (pos != std::string::npos)
    {
        const std::string &tmp = str.substr(pos);
        if (!tmp.empty())
        {
            result.push_back(tmp);
        }
    }
    return !result.empty();
}

#define BUFFER_SIZE 8192
struct Buffer {
    char buf[BUFFER_SIZE];
    int size;
};

static int printCb(const void *buffer, size_t size, void *app_key)
{
    Buffer *buf = (Buffer*)app_key;
    int resv = (buf->size + size) > BUFFER_SIZE ? BUFFER_SIZE - buf->size : size;
    memcpy(buf->buf + buf->size, buffer, resv);
    buf->size += resv;
    return 0;
}

void printAsn1(const asn_TYPE_descriptor_t *td, const void *structPtr)
{
    if(!td || !structPtr) {
        return;
	}
    Buffer buf;
    memset(buf.buf, 0, BUFFER_SIZE);
    buf.size = 0;

	/* Invoke type-specific printer */
    td->op->print_struct(td, structPtr, 1, printCb, &buf);
    debugf("%s\n", buf.buf);
}

void printHex(const char *buf, uint32_t len)
{
    debugf("len is %d\n", len);
    for (uint32_t i = 0; i < len; i++)
    {
        rawf("0x%.2x, ", buf[i]);
    }
    rawf("\n");
}

void getUtcTime(UtcTime_t &src, CMS_UTC_TIME &dst)
{
    dst.timeNow = (src.buf[0] << 24) + (src.buf[1] << 16) + (src.buf[2] << 8) + src.buf[3];
    dst.fraction = (src.buf[4] << 16) + (src.buf[5] << 8) + src.buf[6];
    dst.quality = src.buf[7];
}

void setUtcTime(CMS_UTC_TIME &src, UtcTime_t &dst)
{
    if (NULL == dst.buf)
    {
        dst.buf = (uint8_t *)malloc(8 * sizeof(uint8_t));
    }
    dst.buf[0] = src.timeNow >> 24;
    dst.buf[1] = src.timeNow >> 16 & 0xff;
    dst.buf[2] = src.timeNow >> 8 & 0xff;
    dst.buf[3] = src.timeNow & 0xff;
    dst.buf[4] = src.fraction >> 16;
    dst.buf[5] = src.fraction >> 8 & 0xff;
    dst.buf[6] = src.fraction & 0xff;
    dst.buf[7] = src.quality;
    dst.size = 8;
}

bool validFC(const std::string &fc)
{
    static std::set<std::string> s_fcSet = {
        "ST", ///< 状态
        "MX", ///< 测量
        "SP", ///< 设点
        "SV", ///< 取代
        "CF", ///< 配置
        "DC", ///< 描述
        "SG", ///< 定制组
        "SE", ///< 可编辑定制组
        "SR", ///< 服务响应
        "OR", ///< 接受操作
        "BL", ///< 闭锁
        "EX", ///< 扩充定义
        "XX"  ///< 所有属性
    };
    /// 类同于XX
    if (fc.empty())
    {
        return true;
    }
    if (fc == "00 00")
    {
        return false;
    }
    if (s_fcSet.find(fc) == s_fcSet.end())
    {
        return false;
    }
    return true;
}

}
