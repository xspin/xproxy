#include <gtest/gtest.h>
#include <string>
#include <thread>

#include "udp_client.hpp"
#include "udp_server.hpp"

using namespace uvserver;

std::mutex mtx;

TEST(UDP, client_server) {
    mtx.lock();

    std::string server_recv, client_recv;

    std::thread server([&] {
        uv_loop_t loop1;
        printf("udp server start\n");
        uv_loop_init(&loop1);
        UdpServer udp(&loop1);

        udp.onRecv([&server_recv, &loop1, &udp](const char *data, ssize_t n, const struct sockaddr *addr) {
            auto [ip, port] = addrstr(addr);
            std::printf("UdpServer recv: %.*s from %s %d\n", (int)n, data, ip.c_str(), port);
            server_recv = data;
            udp.sendTo("reply", addr, [&](bool ret, const std::string &err) {
                printf("udp server replied\n");
                uv_stop(&loop1);
            });
        });
        udp.onReady([] {
            printf("udp server ready\n");
            mtx.unlock();
        });
        if (!udp.bind("127.0.0.1", 7001)) {
            std::fprintf(stderr, "UDP bind failed\n");
        }
        uv_run(&loop1, UV_RUN_DEFAULT);
        uv_loop_close(&loop1);
        printf("server closing\n");
    });

    std::thread client([&] {
        uv_loop_t loop2;
        printf("udp client start\n");
        uv_loop_init(&loop2);

        UdpClient udp(&loop2);
        udp.bindPeer("127.0.0.1", 7001);

        udp.onRecv([&client_recv, &loop2](const char *data, ssize_t n, const struct sockaddr *addr) {
            auto [ip, port] = addrstr(addr);
            std::printf("UdpClient recv: %.*s from %s %d\n", (int)n, data, ip.c_str(), port);
            client_recv = data;
            uv_stop(&loop2);
        });
        udp.onReady([] { printf("udp client ready\n"); });
        mtx.lock();
        udp.send("hello", [](bool, const std::string &) { printf("udp client sended hello\n"); });

        uv_run(&loop2, UV_RUN_DEFAULT);
        uv_loop_close(&loop2);
        printf("client closing\n");
    });

    server.join();
    client.join();

    EXPECT_EQ(server_recv, "hello");
    EXPECT_EQ(client_recv, "reply");
}
