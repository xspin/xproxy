#include "datagram_base.hpp"
#include "uv.h"
#include "xutils/logger.hpp"
#include <cstdio>
#include <cstring>

namespace uvserver {

DatagramBase::DatagramBase(uv_loop_t *loop) : UvBase(loop) {
    uv_udp_init(loop_, &udp_);
    udp_.data = this;
}

DatagramBase::~DatagramBase() {
}

void DatagramBase::startRecv() {
    uv_udp_recv_start(&udp_, onAlloc, onRecvImpl);
    Ready();
}

void DatagramBase::onAlloc(uv_handle_t *, size_t suggested, uv_buf_t *buf) {
    buf->base = new char[suggested];
    buf->len = suggested;
}

void DatagramBase::onRecvImpl(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
                              const struct sockaddr *addr, unsigned) {
    auto *self = static_cast<DatagramBase *>(handle->data);

    if (nread < 0) {
        log_warning << handle << " udp recv error: " << uv_err_name(nread);
        delete[] buf->base;
        return;
    }
    if (addr == nullptr) {
        // log_warning << handle << " null addr";
        delete[] buf->base;
        return;
    }
    if (self->recv_cb_)
        self->recv_cb_(buf->base, nread, addr);
    delete[] buf->base;
}

using SendReqData = std::pair<char *, SendCallback>;

bool DatagramBase::sendTo(const char *data, size_t len, const struct sockaddr *addr,
                          SendCallback cb) {
    if (closing_) {
        log_warning << "socket is closing";
        return false;
    }

    auto *req = new uv_udp_send_t();
    char *copy = new char[len];
    std::memcpy(copy, data, len);
    uv_buf_t buf = uv_buf_init(copy, (unsigned)len);

    req->data = new SendReqData(copy, cb);
    int r = uv_udp_send(req, &udp_, &buf, 1, addr, onSendImpl);
    if (r < 0) {
        log_warning << req << " udp req send failed: " << uv_err_name(r);
        cb(false, uv_strerror(r));
        delete[] copy;
        delete static_cast<SendReqData *>(req->data);
        delete req;
        return false;
    }
    return true;
}

void DatagramBase::onSendImpl(uv_udp_send_t *req, int status) {
    if (status < 0) {
        log_warning << req << " udp req send error: " << uv_strerror(status);
    }
    auto p = static_cast<SendReqData *>(req->data);
    if (p->second) {
        if (status) {
            p->second(false, uv_strerror(status));
        } else {
            p->second(true, "");
        }
    }
    delete[] p->first;
    delete p;
    delete req;
}

void DatagramBase::doClose() {
    if (closing_)
        return;
    if (!uv_is_closing((uv_handle_t *)&udp_))
        uv_close((uv_handle_t *)&udp_, onCloseHandle);
}

} // namespace uvserver
