#include <cstdio>
#include <sys/socket.h>
#include <utility>

#include "forward.hpp"
#include "proxy_tcp.hpp"
#include "proxy_udp.hpp"

using namespace xproxy;

int main() {
    uv_loop_t *loop = uv_default_loop();

    TcpProxy tcpProxy(loop, "127.0.0.1", 7000, "127.0.0.1", 8000);
    tcpProxy.forward.onRequest([](const char *data, size_t len, Forward::ForwardCallback cb) {
        const char *d = data;
        printf(" -> req %lu bytes \n", len);
        cb(d, len, [](bool ret, const std::string &err) {
            if (!ret) {
                printf("failed: %s\n", err.c_str());
            }
        });
    });

    tcpProxy.forward.onResponse([](const char *data, size_t len, Forward::ForwardCallback cb) {
        const char *d = data;
        printf(" <- rsp %lu bytes \n", len);
        cb(d, len, [](bool ret, const std::string &err) {
            if (!ret) {
                printf("failed: %s\n", err.c_str());
            }
        });
    });

    tcpProxy.start();

    return uv_run(loop, UV_RUN_DEFAULT);
}
