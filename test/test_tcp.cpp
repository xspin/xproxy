#include <gtest/gtest.h>

#include "tcp_client.hpp"
#include "tcp_server.hpp"

using namespace uvserver;

TEST(TCP, client_server) {
    logger::set_flag(logger::Flag::ALL);
    logger::set_level(logger::Level::DEBUG);

    std::string server_recv, client_recv;
    uv_loop_t *loop = uv_default_loop();

    TcpServer tcpServer(loop);
    tcpServer.onConnect([&](StreamBase *stream) {
        auto [ip, port] = stream->getPeerAddr();
        log_info << logger::format("new stream: %p from %s:%d\n", stream, ip.c_str(), port);
        stream->onRecv([&server_recv, stream](const char *data, ssize_t n) {
            log_info << stream << logger::format("  tcpServer recv: %.*s\n", (int)n, data);
            server_recv = data;
            if (!stream->send("reply", [](bool ret, const std::string &err) {
                    if (!ret)
                        log_error << " server send failed: " << err;
                })) {
                log_error << stream << " server send failed";
            }
        });
        stream->onReady([stream] { log_info << stream << " stream ready"; });
        stream->startRecv();
    });
    tcpServer.onReady([] { log_info << "Server ready"; });

    if (!tcpServer.listen("127.0.0.1", 7000)) {
        std::fprintf(stderr, "TCP listen failed\n");
        ASSERT_FALSE(true);
        return;
    }

    TcpClient tcpClient(loop);
    tcpClient.connect("127.0.0.1", 7000, [&tcpClient](bool ret, const std::string &err) {
        log_info << logger::format("connect %s %s", ret ? "successfully" : "failed", err.c_str());

        if (!tcpClient.send("hello", [](bool ret, const std::string &err) {
                if (!ret)
                    log_error << " client send failed: " << err;
            })) {
            log_error << "client send failed";
        }
    });
    tcpClient.onReady([] { log_info << "Client ready"; });
    tcpClient.onRecv([&client_recv, loop](const char *data, ssize_t n) {
        client_recv = data;
        std::printf("tcpClient recv: %.*s\n", (int)n, data);
        uv_stop(loop);
    });

    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);

    EXPECT_EQ(server_recv, "hello");
    EXPECT_EQ(client_recv, "reply");
}
