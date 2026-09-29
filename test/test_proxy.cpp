
#include <gtest/gtest.h>
#include <sys/socket.h>

#include "tcp_client.hpp"
#include "tcp_server.hpp"
#include "udp_client.hpp"
#include "udp_server.hpp"
#include "uv_base.hpp"

using namespace uvserver;

TEST(TCP, proxy) {
    uv_loop_t *loop = uv_default_loop();

    // tcp - tcp proxy
    TcpServer tcpServer(loop);

    tcpServer.onConnect([loop](StreamBase *stream) {
        auto *tcpClient = new TcpClient(loop);
        printf("new stream: %p %p\n", stream, tcpClient);

        stream->onClose([tcpClient] {
            printf("delete client %p\n", tcpClient);
            delete tcpClient;
        });

        stream->onRecv([stream, tcpClient](const char *data, size_t n) {
            std::printf("%p %p -> TCP recv: %ld bytes\n", stream, tcpClient, n);
            if (tcpClient->connected())
                tcpClient->send(data, n);
        });

        tcpClient->connect("127.0.0.1", 8000, [stream, tcpClient](bool ret, const std::string &err) {
            printf("%p %p client connected to :8000\n", stream, tcpClient);
            if (!ret) {
                log_warning << "Connect failed: " << err;
                return;
            }
            stream->startRecv();
        });
        tcpClient->onRecv([stream, tcpClient](const char *data, size_t n) {
            std::printf("%p %p <- TCP recv: %ld bytes\n", stream, tcpClient, n);
            stream->send(data, n);
        });
    });

    if (!tcpServer.listen("127.0.0.1", 7000)) {
        std::fprintf(stderr, "TCP listen failed\n");
    }

    // udp - udp proxy
    UdpServer udpServer(loop);
    udpServer.onConnect([loop, &udpServer](UdpClient *peer) {
        auto udpClient = new UdpClient(loop);
        udpClient->bindPeer("127.0.0.1", 8001);
        peer->onClose([udpClient] {
            printf("delete client %p\n", udpClient);
            delete udpClient;
        });

        udpClient->onRecv([peer, &udpServer](const char *data, size_t n, const struct sockaddr *addr) {
            printf("<- UDP recv: %ld bytes\n", n);
            peer->send(data, n);
        });

        peer->onRecv([loop, udpClient](const char *data, size_t n, const struct sockaddr *addr) {
            printf("-> UDP recv: %ld bytes\n", n);
            udpClient->send(data, n);
        });
    });

    if (udpServer.bind("127.0.0.1", 7001)) {
        fprintf(stderr, "UDP bind failed\n");
    }

    printf("Runing ...\n");
    uv_run(loop, UV_RUN_DEFAULT);
}
