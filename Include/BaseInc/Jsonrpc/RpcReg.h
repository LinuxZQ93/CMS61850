#ifndef __RPCSERVER_INCLUDE_RPCREG_H__
#define __RPCSERVER_INCLUDE_RPCREG_H__

#include <map>
#include <mutex>

#include "SingTon/SingTon.h"
#include "IRpc.h"

namespace rpc {

class CRpcReg {

#undef RpcPtrFactory
#define RpcPtrFactory(name) \
    struct name##Factory { \
    IRPC *ptr = NULL; \
    name##Factory() { \
        ptr = new name(); \
        CRpcReg::instance()->attachRpcPtr(#name, ptr); \
        } \
    ~name##Factory() { \
        delete ptr; \
        ptr = NULL; \
    } \
};

SINGTON_DECLAR(CRpcReg)

public:
    bool attachRpcPtr(const std::string &name, IRPC *ptr);
    bool init();
    bool start();
    bool stop();
    bool destroy();

private:
    void attachSystemEvent(const std::string &key, const Json::Value &cfg);

private:
    std::map<std::string, IRPC*> m_mapRpcPtr;
    std::mutex m_mutexPtr;

};

}

#endif /* ifndef __RPCSERVER_INCLUDE_RPCREG_H__ */
