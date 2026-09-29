#pragma once
#include "datagram_base.hpp"
#include <cstddef>
#include <functional>
#include <sys/socket.h>

namespace uvserver {

class UdpClient : public DatagramBase {
  public:
    using SendFunc = std::function<bool(const char *data, size_t len, SendCallback cb)>;
    using TimeoutCallback = std::function<void(void)>;

    explicit UdpClient(uv_loop_t *loop);
    ~UdpClient();

    // using DatagramBase::sendTo;

    using DatagramBase::onRecv;

    bool bindPeer(const struct sockaddr *addr);
    bool bindPeer(const std::string &host, int port);

    bool send(const char *data, size_t len, SendCallback cb = nullptr);

    bool send(const std::string &s, SendCallback cb = nullptr) {
        return send(s.data(), s.size(), cb);
    }

    void setSender(SendFunc fn) {
        send_ = std::move(fn);
    }

    void doRecv(const char *data, size_t len);

    const struct sockaddr *getPeerAddr() {
        return peer_addr_.get();
    }

    void timeout(int seconds, TimeoutCallback cb);
    void keepalive();

  private:
    SendFunc send_;
    std::unique_ptr<struct sockaddr> peer_addr_;
    uv_timer_t timer_;
    TimeoutCallback timeout_cb_;
    int timeout_;

    static void onTimeout(uv_timer_t *handle);
};

} // namespace uvserver
