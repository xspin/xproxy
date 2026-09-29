#pragma once
#include <functional>
#include <map>

#include "datagram_base.hpp"
#include "udp_client.hpp"

namespace uvserver {

class UdpServer : public DatagramBase {
  public:
    using ConnectCallback = std::function<void(UdpClient *peer)>;

    explicit UdpServer(uv_loop_t *loop);

    bool bind(const std::string &host, int port);

    bool bind();

    using DatagramBase::sendTo;

    using DatagramBase::onRecv;

    void onConnect(ConnectCallback cb);

  private:
    struct sockaddr peer_addr_;

    std::map<std::pair<std::string, int>, std::unique_ptr<UdpClient>> peers_;
};

} // namespace uvserver
