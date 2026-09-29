#include "proxy_udp.hpp"
#include "udp_client.hpp"
#include "uv.h"
#include "uv_base.hpp"
#include "xutils/logger.hpp"

namespace xproxy {
using namespace uvserver;

UdpProxy::UdpProxy(uv_loop_t *loop, const std::string &in_ip, int in_port,
                   const std::string &dst_ip, int dst_port)
    : ProxyBase(loop, in_ip, in_port, dst_ip, dst_port), server_(loop) {
}

UdpProxy::~UdpProxy() {
    log_debug << "UdpProxy exit";
}

bool UdpProxy::start() {
    server_.onConnect([self = this](UdpClient *peer) {
        auto client = new UdpClient(self->loop_);
        client->bindPeer(self->dst_ip_, self->dst_port_);
        log_debug << peer << " new client " << client;

        peer->onClose([peer, client] {
            log_debug << peer << " delete client " << client;
            delete client;
        });

        peer->onRecv([self, client](const char *data, size_t len, const struct sockaddr *addr) {
            self->forward.request(data, len, [client](const char *d, size_t n, SendCallback cb) {
                client->send(d, n, cb);
            });
        });

        client->onRecv([self, peer](const char *data, size_t len, const struct sockaddr *addr) {
            self->forward.response(data, len, [peer](const char *d, size_t n, SendCallback cb) {
                peer->send(d, n, cb);
            });
        });
    });

    if (!server_.bind(in_ip_, in_port_)) {
        log_warning << "UDP bind failed\n";
        return false;
    }
    log_info << logger::format("Started UDP proxy on %s:%d to %s:%d", in_ip_.c_str(), in_port_,
                               dst_ip_.c_str(), dst_port_);
    return true;
}

} // namespace xproxy
