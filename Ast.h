#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace Ast {

struct Number {
  int value{};
};
struct RefVariable {
  std::string name{};
};

struct Node;
using NodePtr = std::unique_ptr<Node>;
using NodeVec = std::vector<Node>;
struct Operator {
  enum { Assign, Add, Multiply } type{};
  NodePtr leftOperand{};
  NodePtr rightOperand{};
};
struct DefineVariable {
  std::string name{};
  NodePtr valueNode{};
};
using NodeVariant = std::variant<Number, RefVariable, Operator, DefineVariable>;
struct Node {
  NodeVariant v;
};

} // namespace Ast
