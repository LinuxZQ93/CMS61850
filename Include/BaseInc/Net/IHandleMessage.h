#ifndef __FRAMEWORK_INCLUDE_NET_IHANDLEMESSAGE_H__
#define __FRAMEWORK_INCLUDE_NET_IHANDLEMESSAGE_H__

#include <string>

namespace base {

class IHandleMessage {
public:
    IHandleMessage(){}
    virtual ~IHandleMessage(){}

public:
    /**
    *@brief 处理接受来的消息，并做出回应
    *@param id 发送消息的id标识
    *@param buf 接受的内容缓冲区，由外界传输
    *@param respons 响应内容，如果有内容，则自动发给对方，若为空，不发送
    *@return void
    */
    virtual void handleMessage(int id, const char *buf, int len, std::string &respons) = 0;
    /**
    *@brief 发送第一个消息
    *@param respons 响应内容，如果有内容，则自动发给对方，若为空，不发送
    *@return void
    */
    virtual void setFirstMessage(std::string &respons){}
    /**
    *@brief 发送最后一个消息
    *@param respons 响应内容，如果有内容，则自动发给对方，若为空，不发送
    *@return void
    */
    virtual void setLastMessage(std::string &respons){}
    /**
    *@brief 流程结束，客户端或服务端已停止工作
    *@return void
    */
    virtual void processOver(){}
    /**
    *@brief 停止handle功能，做一些清理工作
    *@return void
    */
    virtual void stop(){}

};

class ITcpServerHandleMessage : public IHandleMessage {
public:
    ITcpServerHandleMessage(){}
    virtual ~ITcpServerHandleMessage(){}
public:
    /**
    *@brief 客户端上线通知
    *@param ip 客户端的ip
    *@param port 客户端的端口
    *@param clientId 客户端的id
    *@return void
    */
    virtual void clientConnected(const std::string &ip, int port, int clientId){}
    /**
    *@brief 客户端离线通知
    *@param ip 客户端的ip
    *@param port 客户端的端口
    *@param clientId 客户端的id
    *@return void
    */
    virtual void clientDisconnect(const std::string &ip, int port, int clientId){}
};

} // namespace base


#endif /* __FRAMEWORK_INCLUDE_NET_IHANDLEMESSAGE_H__ */
