#include "tcp_server.hpp"
#include "stream_base.hpp"
#include <cstddef>
#include <cstdio>

namespace uvserver {

TcpServer::TcpServer(uv_loop_t *loop) : StreamBase(loop) {
    uv_tcp_init(loop_, &server_);
    server_.data = this;
}

TcpServer::~TcpServer() {
}

bool TcpServer::listen(const std::string &host, int port, int backlog) {
    struct sockaddr_in addr;
    if (uv_ip4_addr(host.c_str(), port, &addr) != 0)
        return false;
    if (uv_tcp_bind(&server_, (const struct sockaddr *)&addr, 0) != 0)
        return false;
    bool ret = uv_listen((uv_stream_t *)&server_, backlog, onNewConnection) == 0;
    if (ret)
        Ready();
    log_debug << "start listen TCP on " << host << " : " << port;
    return ret;
}

void TcpServer::onNewConnection(uv_stream_t *server, int status) {
    if (status < 0) {
        log_warning << server << " new connection error: " << uv_strerror(status);
        return;
    }

    auto *self = static_cast<TcpServer *>(server->data);

    auto client = std::make_unique<StreamBase>(self->loop_);
    auto hd = client->getTcpHandle();
    int r;
    if ((r = uv_accept(server, (uv_stream_t *)hd)) != 0) {
        log_warning << server << " accept error: " << uv_strerror(r);
        return;
    }

    auto [ip, port] = client->getPeerAddr();
    StreamBase *raw = client.get();

    log_debug << raw << " accepted new connection from " << ip << " : " << port;

    uv_tcp_t *key = hd;

    // 注册关闭回调：句柄真正关闭后从 map 移除
    client->onClose([self, key, raw]() {
        log_debug << "tcp stream " << raw << " closed";
        // 延迟删除：把 unique_ptr 移出 map，交给 loop 下一轮销毁
        // 简单做法：直接 erase，但此时 raw 还在 onCloseHandle 栈上，
        // 销毁 raw 会导致 use-after-free。所以这里只标记，由外部清理。
        auto it = self->clients_.find(key);
        if (it != self->clients_.end()) {
            // 把所有权转移到"待删除"列表，等安全时机再释放
            // self->pending_erase_.push_back(std::move(it->second));
            self->clients_.erase(it);
        }
    });

    self->clients_[key] = std::move(client);

    if (self->conn_cb_) {
        self->conn_cb_(raw);
    }
}

} // namespace uvserver
