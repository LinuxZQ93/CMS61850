#include "IService.h"

#include "SCL/SCLParse.h"

namespace cms {

IService::IService()
{
    m_scl = CSCLParse::instance();
}

}
