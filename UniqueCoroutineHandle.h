#pragma once
#include "coroutine.h"

template<class Promise = void>
class UniqueCoroutineHandle final {
  using Handle = std::coroutine_handle<Promise>;
  Handle m{};

public:
  UniqueCoroutineHandle() = default;
  explicit UniqueCoroutineHandle(Handle h) noexcept
      : m(h) {}
  UniqueCoroutineHandle(const UniqueCoroutineHandle &) = delete;
  UniqueCoroutineHandle &operator=(const UniqueCoroutineHandle &) = delete;
  UniqueCoroutineHandle(UniqueCoroutineHandle &&o) noexcept
      : m(std::exchange(o.m, nullptr)) {}
  UniqueCoroutineHandle &operator=(UniqueCoroutineHandle &&o) {
    if (m) m.destroy();
    m = std::exchange(o.m, nullptr);
    return *this;
  }
  ~UniqueCoroutineHandle() {
    if (m) m.destroy();
  }

  explicit operator bool() const noexcept { return (bool)m; }
  operator Handle() noexcept { return m; }

  auto extract() -> Handle { return std::exchange(m, nullptr); }

  void resume() const { m.resume(); }
};
