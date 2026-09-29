#include "tcp_client.hpp"
#include "uv.h"
#include "xutils/logger.hpp"
#include <cstdio>

namespace uvserver {

TcpClient::TcpClient(uv_loop_t *loop) : StreamBase(loop) {
    connect_req_.data = this;
}

TcpClient::~TcpClient() {
    doClose();
}

void TcpClient::connect(const std::string &host, int port, ConnectCallback cb) {
    connect_cb_ = std::move(cb);

    struct sockaddr_in addr;
    if (uv_ip4_addr(host.c_str(), port, &addr) != 0) {
        if (connect_cb_)
            connect_cb_(false, "invalid address");
        return;
    }
    log_debug << "connect to " << host << " : " << port;

    int r = uv_tcp_connect(&connect_req_, &tcp_, (const struct sockaddr *)&addr, onConnect);
    if (r < 0) {
        log_debug << "TCP connect failed: " << uv_strerror(r);
        if (connect_cb_)
            connect_cb_(false, uv_strerror(r));
    }
}

void TcpClient::onConnect(uv_connect_t *req, int status) {
    auto *self = static_cast<TcpClient *>(req->data);

    if (status < 0) {
        log_debug << "TCP connect failed: " << uv_strerror(status);
        if (self->connect_cb_)
            self->connect_cb_(false, uv_strerror(status));
        self->doClose();
        return;
    }

    self->startRecv();

    if (self->connect_cb_)
        self->connect_cb_(true, "");
}

} // namespace uvserver
