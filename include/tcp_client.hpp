#pragma once
#include "stream_base.hpp"

namespace uvserver {

class TcpClient : public StreamBase {
  public:
    using ConnectCallback = std::function<void(bool success, const std::string &err)>;

    explicit TcpClient(uv_loop_t *loop);
    ~TcpClient();

    void connect(const std::string &host, int port, ConnectCallback cb = nullptr);

  private:
    static void onConnect(uv_connect_t *req, int status);

    uv_connect_t connect_req_;
    ConnectCallback connect_cb_;
};

} // namespace uvserver
