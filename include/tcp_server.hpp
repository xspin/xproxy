#pragma once
#include "stream_base.hpp"
#include <memory>
#include <unordered_map>

namespace uvserver {

class TcpServer : public StreamBase {
  public:
    using ConnectCallback = std::function<void(StreamBase *client)>;

    explicit TcpServer(uv_loop_t *loop);
    ~TcpServer();

    bool listen(const std::string &host, int port, int backlog = 128);
    void onConnect(ConnectCallback cb) { conn_cb_ = std::move(cb); }

  private:
    static void onNewConnection(uv_stream_t *server, int status);

    uv_tcp_t server_;
    ConnectCallback conn_cb_;
    std::unordered_map<uv_tcp_t *, std::unique_ptr<StreamBase>> clients_;
};

} // namespace uvserver
