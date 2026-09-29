#pragma once
#include <functional>
#include <string>
#include <sys/socket.h>
#include <uv.h>
#include <xutils/logger.hpp>

namespace uvserver {

using SendCallback = std::function<void(bool ret, const std::string &err)>;

class UvBase {
  public:
    using CloseCallback = std::function<void()>;
    using ReadyCallback = std::function<void()>;

    virtual ~UvBase();

    void onReady(ReadyCallback cb) {
        ready_cb_ = std::move(cb);
    }

    void close();
    void onClose(CloseCallback cb) {
        close_cb_ = std::move(cb);
    }

    bool closing() const {
        return closing_;
    }

  protected:
    explicit UvBase(uv_loop_t *loop);

    void Ready();
    void doClose();

    uv_loop_t *loop_;
    CloseCallback close_cb_;
    bool closing_ = false;
    ReadyCallback ready_cb_;

    static void onCloseHandle(uv_handle_t *handle);
};

std::unique_ptr<struct sockaddr> ip4addr(const std::string &host, int port);

std::pair<std::string, int> addrstr(const struct sockaddr *addr);

} // namespace uvserver
