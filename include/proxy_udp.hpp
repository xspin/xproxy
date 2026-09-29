#pragma once

#include "proxy_base.hpp"
#include "udp_client.hpp"
#include "udp_server.hpp"

namespace xproxy {

class UdpProxy : public ProxyBase {
  public:
    explicit UdpProxy(uv_loop_t *loop, const std::string &in_ip_, int in_port, const std::string &dst_ip_,
                      int dst_port);

    ~UdpProxy() override;

    bool start() override;

  private:
    uvserver::UdpServer server_;
};

} // namespace xproxy
