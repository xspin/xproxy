#pragma once
#include "uv.h"
#include "uv_base.hpp"
#include <functional>
#include <iterator>

namespace xproxy {

class Forward {
  public:
    using AfterCallback = uvserver::SendCallback;

    using ForwardCallback = std::function<void(const char *data, size_t len, AfterCallback cb)>;
    using HookCallback = std::function<void(const char *data, size_t len, ForwardCallback cb)>;

    explicit Forward(uv_loop_t *loop);
    ~Forward();

    void onRequest(HookCallback cb);
    void onResponse(HookCallback cb);

    void request(const char *data, size_t len, ForwardCallback);
    void response(const char *data, size_t len, ForwardCallback);

  private:
    uv_loop_t *loop_;

    HookCallback request_hook_;
    HookCallback response_hook_;
    AfterCallback after_request_cb_;
    AfterCallback after_response_cb_;
};

} // namespace xproxy
