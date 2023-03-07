#ifndef __RPCSERVER_INCLUDE_IRPC_H__
#define __RPCSERVER_INCLUDE_IRPC_H__

#include "json/json.h"

namespace rpc {

class IRPC {
public:
    IRPC(){}
    virtual ~IRPC(){}

public:
    virtual bool init(){return true;}
    virtual bool start(){return true;}
    virtual bool stop(){return true;}
    virtual bool destroy(){return true;}

};

}

#endif /* ifndef __RPCSERVER_INCLUDE_IRPC_H__ */
