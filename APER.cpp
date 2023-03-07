#include "APER.h"

namespace cms {

bool encode(const asn_TYPE_descriptor_t *td, const void *sptr, std::string &result)
{
    char *buffer = NULL;
    ssize_t ret = aper_encode_to_new_buffer(td, NULL, sptr, (void **)&buffer);
    if (ret <= 0)
    {
        return false;
    }
    std::string tmp(buffer, ret);
    result.swap(tmp);
    free(buffer);
    return true;
}

bool decode(const asn_TYPE_descriptor_s *td, void **sptr, const void *buffer, size_t size)
{
    asn_dec_rval_t ret = aper_decode_complete(td, sptr, buffer, size);
    return ret.code == RC_OK ? true : false;
}

bool checkValid(const asn_TYPE_descriptor_t *td, const void *sptr)
{
    char *buf = NULL;
    size_t len = 0;
    /// 不需要得到具体错误信息，只需知道合不合法
    int ret = asn_check_constraints(td, sptr, buf, &len);
    return ret == 0 ? true : false;
}

}
