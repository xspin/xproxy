#pragma once

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace result {

// ---- 标签类型：把成功/失败编码进类型，避免 T == E 时的歧义 ----
template <typename T> struct _Ok { T value; };

template <typename E> struct _Err { E value; };

// 推导辅助：_Ok(x) -> _Ok<T>，_Err(x) -> _Err<E>
template <typename T> _Ok(T) -> _Ok<T>;
template <typename E> _Err(E) -> _Err<E>;

template <typename T, typename E> class Result {
  public:
    // ---- 构造 ----
    static Result ok(T &&value) { return Result(_Ok<T>{std::move(value)}); }
    static Result err(E &&error) { return Result(_Err<E>{std::move(error)}); }

    // ---- 状态查询 ----
    bool is_ok() const noexcept { return std::holds_alternative<_Ok<T>>(data_); }
    bool is_err() const noexcept { return std::holds_alternative<_Err<E>>(data_); }
    explicit operator bool() const noexcept { return is_ok(); }

    // ---- 取值 ----
    T &unwrap() {
        if (!is_ok())
            throw std::logic_error("Result::unwrap on error");
        return std::get<_Ok<T>>(data_).value;
    }
    const T &unwrap() const {
        if (!is_ok())
            throw std::logic_error("Result::unwrap on error");
        return std::get<_Ok<T>>(data_).value;
    }

    E &unwrap_err() {
        if (!is_err())
            throw std::logic_error("Result::unwrap_err on ok");
        return std::get<_Err<E>>(data_).value;
    }
    const E &unwrap_err() const {
        if (!is_err())
            throw std::logic_error("Result::unwrap_err on ok");
        return std::get<_Err<E>>(data_).value;
    }

    // 类似 Rust 的 unwrap_or
    T unwrap_or(T fallback) const { return is_ok() ? std::get<_Ok<T>>(data_).value : std::move(fallback); }

    // ---- 链式：map ----
    // 成功时用 f 变换 T -> U，失败时原样传递 E
    template <typename F> auto map(F &&f) const -> Result<std::invoke_result_t<F, const T &>, E> {
        using U = std::invoke_result_t<F, const T &>;
        if (is_ok())
            return Result<U, E>::ok(f(std::get<_Ok<T>>(data_).value));
        return Result<U, E>::err(std::get<_Err<E>>(data_).value);
    }

    // ---- 链式：and_then ----
    // 成功时用 f 继续一个返回 Result<U, E> 的操作，失败时原样传递 E
    template <typename F> auto and_then(F &&f) const -> std::invoke_result_t<F, const T &> {
        using R = std::invoke_result_t<F, const T &>;
        if (is_ok())
            return f(std::get<_Ok<T>>(data_).value);
        return R::err(std::get<_Err<E>>(data_).value);
    }

    // ---- 链式：map_err ----
    // 失败时用 f 变换 E -> F，成功时原样传递 T
    template <typename F> auto map_err(F &&f) const -> Result<T, std::invoke_result_t<F, const E &>> {
        using F2 = std::invoke_result_t<F, const E &>;
        if (is_ok())
            return Result<T, F2>::ok(std::get<_Ok<T>>(data_).value);
        return Result<T, F2>::err(f(std::get<_Err<E>>(data_).value));
    }

  private:
    explicit Result(_Ok<T> o) : data_(std::move(o)) {}
    explicit Result(_Err<E> e) : data_(std::move(e)) {}

    std::variant<_Ok<T>, _Err<E>> data_;
};

template <typename T> using TResult = result::Result<T, std::string>;

using Res = TResult<bool>;

template <typename T> static inline TResult<T> Ok(T &&value) { return TResult<T>::ok(std::forward<T>(value)); }

static inline TResult<bool> Ok() { return TResult<bool>::ok(true); }

template <typename T, typename... Args> static inline TResult<T> Errf(const char *fmt, Args &&...args) {
    char buf[1024];
    std::snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);
    return TResult<T>::err(buf);
}

template <typename T = bool, typename... Args> static inline TResult<bool> Errf(const char *fmt, Args &&...args) {
    char buf[1024];
    std::snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);
    return TResult<bool>::err(buf);
}

template <typename T = bool> static inline TResult<bool> Errf(const char *s) { return TResult<bool>::err(s); }

} // namespace result
