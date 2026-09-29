#include <gtest/gtest.h>

#include "tcp_server.hpp"
#include "udp_server.hpp"

using namespace uvserver;

TEST(TCP, server) {

    uv_loop_t *loop = uv_default_loop();

    TcpServer tcpServer(loop);
    tcpServer.onConnect([loop](StreamBase *stream) {
        auto [ip, port] = stream->getPeerAddr();
        log_info << logger::format("new stream: %p from %s:%d\n", stream, ip.c_str(), port);

        stream->onRecv([stream, loop](const char *data, ssize_t n) {
            printf("%p tcpServer recv: %.*s\n", stream, (int)n, data);
            std::string reply = "tcp reply: ";
            reply += std::string(data, n);
            if (!stream->send(reply, [](bool ret, const std::string &err) {
                    if (!ret)
                        log_error << " server send failed: " << err;
                })) {
                log_error << stream << " server send failed";
            }
            if (strncmp(data, "exit", 4) == 0) {
                log_info << " exit loop";
                uv_stop(loop);
            }
        });
    });

    if (!tcpServer.listen("127.0.0.1", 7000)) {
        log_error << "tcp listen failed";
        ASSERT_FALSE(true);
    }

    UdpServer udp(loop);
    udp.onRecv([loop, &udp](const char *data, ssize_t n, const struct sockaddr *addr) {
        auto s = addrstr(addr);
        std::printf("UdpServer recv: %.*s from %s %d\n", (int)n, data, s.first.c_str(), s.second);
        std::string reply = "udp reply: ";
        reply += std::string(data, n);
        udp.sendTo(reply, addr, [&](bool ret, const std::string &err) {});
        if (strncmp(data, "exit", 4) == 0) {
            log_info << " exit loop";
            uv_stop(loop);
        }
    });
    if (!udp.bind("127.0.0.1", 7000)) {
        std::fprintf(stderr, "UDP bind failed\n");
    }

    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
}
