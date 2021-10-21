#pragma once
#include "UniqueCoroutineHandle.h"

template<class T>
class ScopedExchange {
  T &m_actual;
  T m_old = {};

public:
  constexpr ScopedExchange(T &target, const T &value)
      : m_actual(target)
      , m_old(std::move(m_actual)) {
    m_actual = value;
  }
  constexpr ~ScopedExchange() noexcept { m_actual = std::move(m_old); }
};
template<class T, class V>
ScopedExchange(T &, const V &) -> ScopedExchange<T>;

class CoFiber {
  struct Data {
    UniqueCoroutineHandle<> coroutine{};
  };
  using DataPtr = std::shared_ptr<Data>;

  DataPtr m{};

public:
  static thread_local CoFiber current;

  CoFiber() = default;
  explicit CoFiber(std::shared_ptr<Data> &&ptr)
      : m(ptr) {}

  auto run(auto lambda) noexcept -> decltype(auto) {
    auto scoped = ScopedExchange(CoFiber::current, *this);
    return lambda();
  }

  template<class Functor, class... Args>
  static auto launch(Functor functor, Args &&... args) {
    static_assert(std::is_empty_v<Functor>, "Captures don't work here!");
    auto fiber = CoFiber{std::make_shared<Data>()};
    fiber.run([=, &args...] {
      auto task = functor((Args &&) args...);
      auto handle = task.extractHandle();
      fiber.m->coroutine = UniqueCoroutineHandle<>{handle};
      handle.resume();
    });
    return fiber;
  }
};

inline thread_local CoFiber CoFiber::current;
