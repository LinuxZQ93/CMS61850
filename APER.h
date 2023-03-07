/**
 * @file APER.h
 * @author (linuxzq93@163.com)
 * @brief 对asn1c的底层封装，编解码及有效性校验
 * @version 1.0
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CMS61850_SERVICE_APER_H__
#define __CMS61850_SERVICE_APER_H__

#include <string>
#include "asn1/aper_encoder.h"
#include "asn1/aper_decoder.h"

namespace cms {

bool encode(const asn_TYPE_descriptor_t *td, const void *sptr, std::string &result);

bool decode(const asn_TYPE_descriptor_s *type_descriptor, void **sptr, const void *buffer, size_t size);

bool checkValid(const asn_TYPE_descriptor_t *td, const void *sptr);

}

#endif /* __CMS61850_SERVICE_APER_H__ */
