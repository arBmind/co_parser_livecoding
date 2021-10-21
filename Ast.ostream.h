#pragma once
#include "Ast.h"

#include <ostream>
#include <span>

namespace Ast {

inline auto operator<<(std::ostream &out, const Number &num) -> std::ostream & {
  return out << "Number{" << num.value << '}';
}
inline auto operator<<(std::ostream &out, const RefVariable &var) -> std::ostream & {
  return out << "Variable{" << var.name << '}';
}
inline auto operator<<(std::ostream &out, const Node &n) -> std::ostream &;
inline auto operator<<(std::ostream &out, const NodePtr &np) -> std::ostream & { return out << *np; }
inline auto operator<<(std::ostream &out, const Operator &op) -> std::ostream & {
  auto type2string = [](auto type) -> std::string {
    switch (type) {
    case Operator::Assign: return "=";
    case Operator::Add: return "+";
    case Operator::Multiply: return "*";
    }
    return "<>";
  };
  return out << *op.leftOperand << ' ' << type2string(op.type) << ' ' << *op.rightOperand;
}
inline auto operator<<(std::ostream &out, const DefineVariable &dv) -> std::ostream & {
  return out << "var " << dv.name << " = " << *dv.valueNode;
}
inline auto operator<<(std::ostream &out, const Node &n) -> std::ostream & {
  return std::visit([&](auto &&e) -> std::ostream & { return out << e; }, n.v);
}
inline auto operator<<(std::ostream &out, std::span<Node> span) -> std::ostream & {
  for (auto &n : span) {
    out << n << '\n';
  }
  return out;
}

} // namespace Ast
