#pragma once
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace Token {

struct Num {
  int value{};
};
struct Id {
  std::string name{};
};
enum Simple {
  Var,
  Assign,
  Add,
  Multiply,
};
using Token = std::variant<Num, Id, Simple>;
using Span = std::span<const Token>;
using Vec = std::vector<Token>;

} // namespace Token
