#include "../yuan/http/http_connection.h"
#include "../yuan/log.h"
#include "../yuan/iomanager.h"

#include <iostream>

static yuan::Logger::ptr g_logger = YUAN_GET_ROOT_LOGGER();

// 测试http请求
void run() {
    yuan::Address::ptr addr = yuan::Address::LookupAnyIPAdress("www.sylar.top:80");
    if (!addr) {
        YUAN_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    yuan::Socket::ptr sock = yuan::Socket::CreateTCP(addr);
    bool ret = sock->connect(addr);
    if (!ret) {
        YUAN_LOG_INFO(g_logger) << "connect" << *addr << " failed";
        return;
    }

    yuan::http::HttpConnection::ptr conn(new yuan::http::HttpConnection(sock));
    yuan::http::HttpRequest::ptr req(new yuan::http::HttpRequest());
    // 如果服务器使用了Nginx：如果req不设置Host头部。会响应400，由Nginx处理请求，如果没找到Host头部则返回400。如果加了host，则Nginx会返回301重定向
    req->setHeader("Host", "www.sylar.top");
    // 这个uri可以测试chunked类型的响应
    req->setPath("/blog/");
    YUAN_LOG_INFO(g_logger) << "req:" << std::endl << *req;

    conn->sendRequest(req);
    auto resp = conn->recvResponse();

    if (!resp) {
        YUAN_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    YUAN_LOG_INFO(g_logger) << "resp:" << std::endl << *resp;
}

int main(int argc, char **argv) {
    yuan::IOManager iom(2);
    iom.schedule(run);
    return 0;
}