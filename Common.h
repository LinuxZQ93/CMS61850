/**
 * @file Common.h
 * @author (linuxzq93@163.com)
 * @brief 一些对业务有帮助的函数封装
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __61850CMS_COMMON_H__
#define __61850CMS_COMMON_H__

#include "APER.h"
#include "asn1/ErrorPDU.h"
#include <string>
#include <vector>
#include <time.h>
#include "asn1/OCTET_STRING.h"
#include "asn1/DataDefinition.h"
#include "asn1/Data.h"
#include "asn1/UtcTime.h"

#include "Print/Print.h"

namespace cms {

/// 打开位开关，从高往低数位，0开始
#define BIT_SET_ON(ptr,bitnum) \
	( ((uint8_t *)(ptr))[(bitnum)/8] |= (0x80>>((bitnum)&7)) )
#define BIT_SET_OFF(ptr,bitnum) \
	( ((uint8_t *)(ptr))[(bitnum)/8] &= ~(0x80>>((bitnum)&7)) )

#define BIT_GET(ptr,bitnum) \
	(( ((uint8_t *)(ptr))[(bitnum)/8] &  (0x80>>((bitnum)&7)) ) ? 1:0)

/// 以下宏及模板，为了减少对底层asn1理解设计，并且更容易控制内存泄露等问题
#define CallocPtr(type) (type*)calloc(1, sizeof(type));
#define FreeStruct(type, ptr) ASN_STRUCT_FREE(asn_DEF_##type, ptr);

#define Decode(ptr, buf, len) decode(ptr.getTypePtr(), (void **)(ptr.getRef()), buf, len)
#define Encode(ptr, response) encode(ptr.getTypePtr(), ptr.get(), response)
#define Check(ptr) checkValid(ptr.getTypePtr(), ptr.get())

#define StructPtr(type) &asn_DEF_##type
#define DEFType(type) type##_t

#define PrintAPER(ptr) printAsn1(ptr.getTypePtr(), ptr.get());

template<typename T>
class CSafeStruct;

#define SafeDef(type) \
template<>\
class CSafeStruct<type> {\
using realType = DEFType(type);\
public:\
    explicit CSafeStruct() : m_bRelese(true) {m_ptr = CallocPtr(realType); m_stuPtr = StructPtr(type);}\
    CSafeStruct(realType *ptr) : m_bRelese(false) { m_ptr = ptr; m_stuPtr = StructPtr(type);}\
    ~CSafeStruct() {if (m_bRelese) FreeStruct(type, m_ptr);}\
public:\
    realType *get() {return m_ptr;}\
    realType **getRef(){return &m_ptr;}\
    realType *operator->() {return m_ptr;}\
    realType& operator*() {return *m_ptr;}\
    asn_TYPE_descriptor_t *getTypePtr() {return m_stuPtr;}\
private:\
    realType *m_ptr;\
    asn_TYPE_descriptor_t *m_stuPtr;\
    bool m_bRelese;\
};

void encodeErr(ServiceError code, std::string &response);

/// 普通字符串拷贝至oct字符串
void copyStrToOctStr(const std::string &src, OCTET_STRING_t *dst);
void copyStrToOctStr(const char *src, int len, OCTET_STRING_t *dst);

DataDefinition_PR getDataDefPR(const std::string &dataType);
Data_PR getDataPR(const std::string &dataType);

/// 内存拷贝函数
void copyMem(const std::string &name, void *dst, void *src, int len);
inline void copyRawPtr(void *dst, void *src, int len) {
    debugf("dstbuf[%p], srcBuf[%p], size[%d]\n", dst, src, len);
     memcpy(dst, src, len);}
void copyOctPtr(void *dst, void *src, int len);
void copyBitPtr(void *dst, void *src, int len);

/// 字符串分割函数
bool splitStr(const std::string &symbol, const std::string &str, std::vector<std::string> &result);

/// 可视化打印asn1编码数据
void printAsn1(const asn_TYPE_descriptor_t *td, const void *structPtr);

/// 循环打印十六进制
void printHex(const char *buf, uint32_t len);

/// 时间结构体
struct CMS_UTC_TIME {
    uint32_t timeNow;
    uint32_t fraction;
    uint8_t quality;
    CMS_UTC_TIME() : timeNow(0), fraction(0), quality(0){}
};

void getUtcTime(UtcTime_t &src, CMS_UTC_TIME &dst);
void setUtcTime(CMS_UTC_TIME &src, UtcTime_t &dst);

bool validFC(const std::string &fc);

}

#endif /* __61850CMS_COMMON_H__ */
