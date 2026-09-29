#include "proxy_tcp.hpp"
#include "forward.hpp"
#include "proxy_base.hpp"
#include "uv_base.hpp"
#include "xutils/logger.hpp"

namespace xproxy {
using namespace uvserver;

TcpProxy::TcpProxy(uv_loop_t *loop, const std::string &in_ip, int in_port, const std::string &dst_ip, int dst_port)
    : ProxyBase(loop, in_ip, in_port, dst_ip, dst_port), server_(loop) {
}

TcpProxy::~TcpProxy() {
    log_debug << "TcpProxy exit";
}

bool TcpProxy::start() {
    server_.onConnect([self = this](StreamBase *stream) {
        auto client = new TcpClient(self->loop_);
        log_debug << stream << " new client " << client;

        stream->onClose([stream, client] {
            log_debug << stream << " closed and delete client " << client;
            delete client;
        });

        stream->onRecv([self, stream, client](const char *data, size_t len) {
            self->forward.request(data, len, [client](const char *d, size_t n, SendCallback cb) {
                if (client->connected())
                    client->send(d, n, cb);
                else {
                    log_warning << "client " << client << " is not connected";
                }
            });
        });

        client->connect(self->dst_ip_, self->dst_port_, [stream, client](bool ret, const std::string &err) {
            if (!ret) {
                log_warning << stream << " client " << client << " Connect failed: " << err;
                return;
            }
            stream->startRecv();
        });

        client->onRecv([self, stream, client](const char *data, size_t len) {
            self->forward.response(data, len,
                                   [stream](const char *d, size_t n, SendCallback cb) { stream->send(d, n, cb); });
        });
    });

    if (!server_.listen(in_ip_, in_port_)) {
        return false;
    }

    log_info << logger::format("Started TCP proxy on %s:%d to %s:%d", in_ip_.c_str(), in_port_, dst_ip_.c_str(),
                               dst_port_);

    return true;
}

} // namespace xproxy
