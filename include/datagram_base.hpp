#pragma once
#include "uv_base.hpp"

namespace uvserver {

class DatagramBase : public UvBase {
  public:
    // UDP 接收回调：多一个来源地址
    using RecvCallback = std::function<void(const char *data, size_t len, const struct sockaddr *addr)>;

    ~DatagramBase() override;

    // 发送到指定地址（UDP 必须指定目标）
    bool sendTo(const char *data, size_t len, const struct sockaddr *addr, SendCallback cb = nullptr);

    bool sendTo(const std::string &s, const struct sockaddr *addr, SendCallback cb = nullptr) {
        return sendTo(s.data(), s.size(), addr, cb);
    }

    void onRecv(RecvCallback cb) {
        recv_cb_ = std::move(cb);
    }

  protected:
    explicit DatagramBase(uv_loop_t *loop);
    void startRecv();
    void doClose();

    uv_udp_t udp_;
    RecvCallback recv_cb_;

  private:
    static void onAlloc(uv_handle_t *handle, size_t suggested, uv_buf_t *buf);
    static void onRecvImpl(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr,
                           unsigned flags);
    static void onSendImpl(uv_udp_send_t *req, int status);
};

} // namespace uvserver
