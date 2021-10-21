#pragma once
#include "UniqueCoroutineHandle.h"
#include "coroutine.h"

#include <type_traits>

template<class T = void>
struct Task;

struct FinalAwaiter {
  bool await_ready() const noexcept { return false; }
  void await_resume() const noexcept {}

  template<class Promise>
  auto await_suspend(std::coroutine_handle<Promise> awaitingCoroutine) const noexcept
      -> std::coroutine_handle<> {
    return awaitingCoroutine.promise().m_continuation;
  }
};

struct PromiseVoid {
  std::coroutine_handle<> m_continuation = std::noop_coroutine();

  auto initial_suspend() const noexcept -> std::suspend_always { return {}; }
  auto final_suspend() const noexcept -> FinalAwaiter { return {}; }

  void unhandled_exception() const noexcept {} // ignore

  auto get_return_object() noexcept -> Task<>;
  void return_void() noexcept {};
};
template<class T>
struct PromiseSome {
  std::coroutine_handle<> m_continuation = std::noop_coroutine();
  T m_result{};

  auto initial_suspend() const noexcept -> std::suspend_always { return {}; }
  auto final_suspend() const noexcept -> FinalAwaiter { return {}; }

  void unhandled_exception() const noexcept {} // ignore

  auto get_return_object() noexcept -> Task<T>;

  template<class V>
  void return_value(V &&value) noexcept requires(std::is_same_v<std::remove_cvref_t<V>, T>) {
    m_result = std::forward<V>(value);
  }
};

template<class T>
using PromiseFor = std::conditional_t<std::is_same_v<T, void>, PromiseVoid, PromiseSome<T>>;

template<class T>
struct Task {
  using promise_type = PromiseFor<T>;
  using Coroutine = std::coroutine_handle<promise_type>;
  using UniqueCoroutine = UniqueCoroutineHandle<promise_type>;

  Task() = default;
  explicit Task(Coroutine coroutine) noexcept
      : m_coroutine(coroutine) {}

  auto operator co_await() noexcept {
    struct Awaiter {
      Coroutine m_coroutine = {};

      bool await_ready() const noexcept { return false; }

      auto await_suspend(std::coroutine_handle<> awaitingCoroutine) const noexcept
          -> std::coroutine_handle<> {
        m_coroutine.promise().m_continuation = awaitingCoroutine;
        return m_coroutine;
      }
      auto await_resume() const noexcept -> T {
        if constexpr (std::is_same_v<T, void>) {
          return;
        }
        else {
          return std::move(m_coroutine.promise().m_result);
        }
      }
    };
    return Awaiter{m_coroutine};
  }

  auto extractHandle() -> Coroutine { return m_coroutine.extract(); }

private:
  UniqueCoroutine m_coroutine{};
};

auto PromiseVoid::get_return_object() noexcept -> Task<> {
  return Task<>{std::coroutine_handle<PromiseVoid>::from_promise(*this)};
}

template<class T>
auto PromiseSome<T>::get_return_object() noexcept -> Task<T> {
  return Task<T>{std::coroutine_handle<PromiseSome>::from_promise(*this)};
}
