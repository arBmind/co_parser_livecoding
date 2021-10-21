#pragma once
#include "CoFiber.h"
#include "Task.h"

#include <map>
#include <string>
#include <variant>

struct ScopeVariable {
  int value = {};
};

struct ScopeWaiter {
  CoFiber fiber{};
  std::coroutine_handle<> coroutine{};

  auto resume() {
    fiber.run([this] { coroutine.resume(); });
  }
};

struct ScopeAwaiter {
  ScopeWaiter &m_slot;

  bool await_ready() const noexcept { return false; }
  void await_resume() const noexcept {}

  template<class Promise>
  auto await_suspend(std::coroutine_handle<Promise> awaitingCoroutine) const noexcept
      -> std::coroutine_handle<> {
    m_slot.coroutine = awaitingCoroutine;
    m_slot.fiber = CoFiber::current;
    return std::noop_coroutine();
  }
};

struct Scope {
  using IdVariables = std::map<std::string, ScopeVariable, std::less<>>;

  using Waiters = std::vector<ScopeWaiter>;
  using IdWaiters = std::map<std::string, Waiters, std::less<>>;

  IdVariables m_variables;
  IdWaiters m_waiters;

  using QueryResult = std::variant<std::monostate, const ScopeVariable *>;
  auto query(const std::string_view &id) const -> QueryResult {
    if (auto it = m_variables.find(id); it != m_variables.end()) {
      return &it->second;
    }
    return {};
  }

  auto waitFor(std::string id) -> ScopeAwaiter {
    return ScopeAwaiter{m_waiters[std::move(id)].emplace_back()};
  }

  void defineVariable(std::string id) {
    auto [it, inserted] = m_variables.emplace(std::move(id), ScopeVariable{});
    if (!inserted) {
      throw "fail!";
    }
    auto key = std::string_view{it->first};
    if (auto it = m_waiters.find(key); it != m_waiters.end()) {
      for (auto &waiter : it->second) {
        waiter.resume();
      }
      m_waiters.erase(it);
    }
  }
};
