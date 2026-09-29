#include <gtest/gtest.h>

#include "proxy_tcp.hpp"
#include "proxy_udp.hpp"

using namespace xproxy;

TEST(xproxy, forward) {
    uv_loop_t *loop = uv_default_loop();

    TcpProxy tcpProxy(loop, "127.0.0.1", 7000, "127.0.0.1", 8000);

    tcpProxy.forward.onRequest([](const char *data, size_t len, Forward::ForwardCallback cb) {
        const char *d = data;
        printf("[TCP] -> req %lu bytes \n", len);
        cb(d, len, [](bool ret, const std::string &err) {
            if (!ret) {
                printf("failed: %s\n", err.c_str());
            }
        });
    });

    tcpProxy.forward.onResponse([](const char *data, size_t len, Forward::ForwardCallback cb) {
        const char *d = data;
        printf("[TCP] <- rsp %lu bytes \n", len);
        cb(d, len, [](bool ret, const std::string &err) {
            if (!ret) {
                printf("failed: %s\n", err.c_str());
            }
        });
    });

    tcpProxy.start();

    UdpProxy udpProxy(loop, "127.0.0.1", 7001, "127.0.0.1", 8001);

    udpProxy.forward.onRequest([loop](const char *data, size_t len, Forward::ForwardCallback cb) {
        const char *d = data;
        if (strncmp(d, "exit", 4) == 0) {
            uv_stop(loop);
            printf("stop uv loop\n");
            return;
        }
        printf("[UDP] -> req %lu bytes \n", len);
        cb(d, len, [](bool ret, const std::string &err) {
            if (!ret) {
                printf("failed: %s\n", err.c_str());
            }
        });
    });

    udpProxy.forward.onResponse([](const char *data, size_t len, Forward::ForwardCallback cb) {
        const char *d = data;
        printf("[UDP] <- rsp %lu bytes \n", len);
        cb(d, len, [](bool ret, const std::string &err) {
            if (!ret) {
                printf("failed: %s\n", err.c_str());
            }
        });
    });

    udpProxy.start();
    uv_run(loop, UV_RUN_DEFAULT);
}
