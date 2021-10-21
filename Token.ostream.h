#pragma once
#include "Token.h"

#include <ostream>

namespace Token {

inline auto operator<<(std::ostream &out, const Num &num) -> std::ostream & {
  return out << "Num{" << num.value << '}';
}
inline auto operator<<(std::ostream &out, const Id &id) -> std::ostream & {
  return out << "Id{\"" << id.name << "\"}";
}
inline auto operator<<(std::ostream &out, const Simple &simple) -> std::ostream & {
  switch (simple) {
  case Var: return out << "Var";
  case Assign: return out << "Assign";
  case Add: return out << "Add";
  case Multiply: return out << "Multiply";
  }
  return out << "Tok::<unknown>";
}
inline auto operator<<(std::ostream &out, const Token &t) -> std::ostream & {
  return std::visit([&](auto &&e) -> std::ostream & { return out << e; }, t);
}
inline auto operator<<(std::ostream &out, Span ts) -> std::ostream & {
  if (ts.empty()) {
    return out;
  }
  out << ts.front();
  ts = ts.subspan(1);
  for (auto &t : ts) {
    out << ", " << t;
  }
  return out;
}
template<size_t N>
inline auto operator<<(std::ostream &out, const std::array<Vec, N> &lines) -> std::ostream & {
  for (auto &l : lines) {
    out << l << '\n';
  }
  return out;
}

} // namespace Token
