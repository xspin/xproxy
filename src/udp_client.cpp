#include "udp_client.hpp"
#include "xutils/logger.hpp"
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>

namespace uvserver {

UdpClient::UdpClient(uv_loop_t *loop) : DatagramBase(loop) {
    uv_timer_init(loop, &timer_);
    timer_.data = this;
}

UdpClient::~UdpClient() {
    doClose();
}

bool UdpClient::bindPeer(const struct sockaddr *addr) {
    auto p = new struct sockaddr_in();
    memcpy(p, addr, sizeof(struct sockaddr_in));
    peer_addr_.reset(reinterpret_cast<struct sockaddr *>(p));
    startRecv();
    return true;
}

bool UdpClient::bindPeer(const std::string &host, int port) {
    auto addr = new struct sockaddr_in();

    if (uv_ip4_addr(host.c_str(), port, addr) != 0) {
        log_warning << "get addr failed";
        delete addr;
        return false;
    }

    peer_addr_.reset(reinterpret_cast<struct sockaddr *>(addr));

    startRecv();

    return true;
}

bool UdpClient::send(const char *data, size_t len, SendCallback cb) {
    keepalive();
    if (send_) {
        return send_(data, len, cb);
    }
    return sendTo(data, len, peer_addr_.get(), cb);
}

void UdpClient::doRecv(const char *data, size_t len) {
    keepalive();
    recv_cb_(data, len, peer_addr_.get());
}

void UdpClient::timeout(int seconds, TimeoutCallback cb) {
    timeout_ = seconds;
    timeout_cb_ = std::move(cb);
    log_debug << "set timeout " << timeout_ << " seconds";
    uv_timer_start(&timer_, onTimeout, timeout_ * 1000, 0);
}

void UdpClient::keepalive() {
    if (timeout_cb_) {
        uv_timer_start(&timer_, onTimeout, timeout_ * 1000, 0);
    }
}

void UdpClient::onTimeout(uv_timer_t *handle) {
    auto *self = static_cast<UdpClient *>(handle->data);
    log_debug << self << " timeout";
    self->timeout_cb_();
}

} // namespace uvserver
