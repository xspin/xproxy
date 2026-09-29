#include "forward.hpp"
#include "uv.h"
#include "uv_base.hpp"

namespace xproxy {

Forward::Forward(uv_loop_t *loop) : loop_(loop) {
}

Forward::~Forward(){};

void Forward::onRequest(HookCallback cb) {
    request_hook_ = std::move(cb);
}

void Forward::onResponse(HookCallback cb) {
    response_hook_ = std::move(cb);
}

void Forward::request(const char *data, size_t len, ForwardCallback cb) {
    request_hook_(data, len, [self = this, cb = std::move(cb)](const char *data, size_t n, AfterCallback after) {
        cb(data, n, after);
    });
}

void Forward::response(const char *data, size_t len, ForwardCallback cb) {
    response_hook_(data, len, [self = this, cb = std::move(cb)](const char *data, size_t n, AfterCallback after) {
        cb(data, n, after);
    });
}

} // namespace xproxy
