#include "stream_base.hpp"
#include "uv.h"
#include "xutils/logger.hpp"
#include <cstdio>
#include <cstring>
#include <utility>

namespace uvserver {

StreamBase::StreamBase(uv_loop_t *loop) : UvBase(loop) {
    uv_tcp_init(loop_, &tcp_);
    tcp_.data = this;
    stream_ = (uv_stream_t *)&tcp_;
}

StreamBase::~StreamBase() {
}

uv_tcp_t *StreamBase::getTcpHandle() {
    return &tcp_;
}

void StreamBase::startRecv() {
    if (connected_) {
        log_warning << "already ready!";
        return;
    }
    assert(stream_);
    connected_ = true;
    uv_read_start(stream_, onAlloc, onRead);
    Ready();
}

void StreamBase::onAlloc(uv_handle_t *, size_t suggested, uv_buf_t *buf) {
    buf->base = new char[suggested];
    buf->len = suggested;
}

void StreamBase::onRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    auto *self = static_cast<StreamBase *>(stream->data);

    if (nread < 0) {
        if (nread != UV_EOF)
            log_warning << "read error: " << uv_err_name(nread);
        delete[] buf->base;
        self->doClose();
        return;
    }

    if (nread > 0 && self->recv_cb_)
        self->recv_cb_(buf->base, nread);
    delete[] buf->base;
}

using SendReqData = std::pair<char *, SendCallback>;

bool StreamBase::send(const char *data, size_t len, SendCallback cb) {
    if (!connected_ || closing_) {
        log_debug << logger::format("%p unconnected. connected: %d, closing: %d", stream_, connected_, closing_);
        return false;
    }

    auto *req = new uv_write_t();
    char *copy = new char[len];
    std::memcpy(copy, data, len);
    uv_buf_t buf = uv_buf_init(copy, (unsigned)len);

    req->data = new SendReqData(copy, cb);
    int r = uv_write(req, stream_, &buf, 1, onWrite);
    if (r < 0) {
        log_warning << stream_ << " tcp write failed: " << uv_strerror(r);
        if (cb)
            cb(false, uv_strerror(r));
        delete[] copy;
        delete static_cast<SendReqData *>(req->data);
        delete req;
        return false;
    }
    return true;
}

void StreamBase::onWrite(uv_write_t *req, int status) {
    if (status < 0)
        log_warning << req << " tcp req write failed: " << uv_strerror(status);
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

void StreamBase::doClose() {
    if (closing_)
        return;
    connected_ = false;
    if (!uv_is_closing((uv_handle_t *)&tcp_))
        uv_close((uv_handle_t *)&tcp_, onCloseHandle);
    UvBase::close();
}

std::pair<std::string, int> StreamBase::getPeerAddr() {
    struct sockaddr_storage addr;
    int len = sizeof(addr);

    int r;
    if ((r = uv_tcp_getpeername((const uv_tcp_t *)stream_, (struct sockaddr *)&addr, &len)) == 0) {
        return addrstr((struct sockaddr *)&addr);
    } else {
        log_error << "getPeerAddr failed: " << uv_strerror(r);
    }
    return std::make_pair("", 0);
}
} // namespace uvserver
