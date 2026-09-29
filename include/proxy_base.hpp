#pragma once
#include "forward.hpp"

namespace xproxy {

class ProxyBase {
  public:
    virtual ~ProxyBase();

    virtual bool start() = 0;
    // virtual void stop();

    Forward forward;

  protected:
    explicit ProxyBase(uv_loop_t *loop, const std::string &in_ip, int in_port, const std::string &dst_ip, int dst_port)
        : loop_(loop), in_ip_(in_ip), in_port_(in_port), dst_ip_(dst_ip), dst_port_(dst_port), forward(loop) {
    }

    uv_loop_t *loop_;
    std::string in_ip_;
    int in_port_;
    std::string dst_ip_;
    int dst_port_;
};

} // namespace xproxy
