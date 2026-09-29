#include "udp_server.hpp"
#include "xutils/logger.hpp"
#include <sys/socket.h>

namespace uvserver {

UdpServer::UdpServer(uv_loop_t *loop) : DatagramBase(loop) {
}

bool UdpServer::bind() {
    startRecv();
    return true;
}

bool UdpServer::bind(const std::string &host, int port) {
    struct sockaddr_in addr;
    int r;

    log_debug << "start listen UDP on " << host << " : " << port;
    if (uv_ip4_addr(host.c_str(), port, &addr) != 0) {
        log_warning << "get addr failed";
        return false;
    }

    if ((r = uv_udp_bind(&udp_, (const struct sockaddr *)&addr, 0)) != 0) {
        log_warning << "bind udp failed: " << uv_strerror(r);
        return false;
    }

    startRecv();

    return true;
}

void UdpServer::onConnect(ConnectCallback cb) {
    onRecv([conn_cb = std::move(cb), self = this](const char *data, size_t n, const struct sockaddr *addr) {
        auto key = addrstr(addr);
        UdpClient *peer;
        auto it = self->peers_.find(key);
        if (it == self->peers_.end()) {
            self->peers_[key] = std::make_unique<UdpClient>(self->loop_);
            peer = self->peers_[key].get();
            peer->bindPeer(addr);
            peer->setSender([self, peer](const char *d, size_t len, SendCallback cb) {
                return self->sendTo(d, len, peer->getPeerAddr(), cb);
            });
            log_debug << "connect new peer " << key.first << " : " << key.second << " " << peer;
            peer->timeout(60, [self, key, peer] {
                log_debug << "erase peer " << key.first << " : " << key.second << " " << peer;
                self->peers_.erase(key);
            });
            conn_cb(peer);
        } else {
            peer = it->second.get();
        }
        peer->doRecv(data, n);
    });
}

} // namespace uvserver
