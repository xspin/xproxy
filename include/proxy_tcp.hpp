#pragma once

#include "proxy_base.hpp"
#include "tcp_client.hpp"
#include "tcp_server.hpp"

namespace xproxy {

class TcpProxy : public ProxyBase {
  public:
    explicit TcpProxy(uv_loop_t *loop, const std::string &in_ip, int in_port, const std::string &dst_ip, int dst_port);

    ~TcpProxy() override;

    bool start() override;

  private:
    uvserver::TcpServer server_;
};

} // namespace xproxy
