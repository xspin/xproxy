#include "uv_base.hpp"
#include <memory>

namespace uvserver {

UvBase::UvBase(uv_loop_t *loop) : loop_(loop) {
}

UvBase::~UvBase() {
}

void UvBase::close() {
    doClose();
}

void UvBase::doClose() {
    if (closing_)
        return;
    closing_ = true;
}

void UvBase::onCloseHandle(uv_handle_t *handle) {
    auto *self = static_cast<UvBase *>(handle->data);
    if (self->close_cb_)
        self->close_cb_();
}

void UvBase::Ready() {
    if (ready_cb_) {
        ready_cb_();
    }
}

std::unique_ptr<struct sockaddr> ip4addr(const std::string &host, int port) {
    auto addr = std::make_unique<struct sockaddr_in>();

    if (uv_ip4_addr(host.c_str(), port, addr.get()) != 0) {
        return nullptr;
    }
    struct sockaddr_in *p = addr.release();
    return std::unique_ptr<struct sockaddr>(reinterpret_cast<struct sockaddr *>(p));
}

std::pair<std::string, int> addrstr(const struct sockaddr *addr) {
    char buf[INET6_ADDRSTRLEN];
    uv_ip_name(addr, buf, sizeof(buf));

    int port;
    auto family = addr->sa_family;
    if (family == AF_INET) {
        auto addr4 = reinterpret_cast<const struct sockaddr_in *>(addr);
        port = ntohs(addr4->sin_port);
    } else {
        auto addr6 = reinterpret_cast<const struct sockaddr_in6 *>(addr);
        port = ntohs(addr6->sin6_port);
    }
    return std::make_pair(buf, port);
}
} // namespace uvserver
