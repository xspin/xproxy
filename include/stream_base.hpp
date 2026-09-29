#pragma once
#include "uv.h"
#include "uv_base.hpp"
#include <cstddef>
#include <functional>

namespace uvserver {

class StreamBase : public UvBase {
  public:
    using RecvCallback = std::function<void(const char *data, size_t len)>;

    explicit StreamBase(uv_loop_t *loop);
    ~StreamBase() override;

    uv_tcp_t *getTcpHandle();
    bool send(const char *data, size_t len, SendCallback cb = nullptr);
    bool send(const std::string &s, SendCallback cb = nullptr) {
        return send(s.data(), s.size(), cb);
    }

    void onRecv(RecvCallback cb) {
        recv_cb_ = std::move(cb);
    }

    bool connected() const {
        return connected_;
    }

    void startRecv();

    using UvBase::onReady;

    std::pair<std::string, int> getPeerAddr();

  protected:
    void doClose();

    uv_tcp_t tcp_;
    uv_stream_t *stream_ = nullptr;
    RecvCallback recv_cb_;
    bool connected_ = false;

  private:
    static void onAlloc(uv_handle_t *handle, size_t suggested, uv_buf_t *buf);
    static void onRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
    static void onWrite(uv_write_t *req, int status);
};

} // namespace uvserver
