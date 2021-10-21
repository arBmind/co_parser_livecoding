#include "Ast.ostream.h"
#include "CoFiber.h"
#include "Scope.h"
#include "Task.h"
#include "Token.h"
#include "Token.ostream.h"

#include <array>
#include <iostream>

template<Token::Simple simple>
auto ignoreTokenIf(Token::Span &tokens) -> bool {
  if (tokens.empty()) {
    return false;
  }
  auto simplePtr = std::get_if<Token::Simple>(&tokens.front());
  if (simplePtr && *simplePtr == simple) {
    tokens = tokens.subspan(1);
    return true;
  }
  return false;
}

template<class T>
auto consumeToken(Token::Span &tokens) -> T {
  if (tokens.empty()) {
    throw 42;
  }
  auto tok = std::get_if<T>(&tokens.front());
  if (!tok) {
    throw 32;
  }
  tokens = tokens.subspan(1);
  return *tok;
}

auto parseValue(Scope &scope, Token::Span &tokens) -> Task<Ast::Node> {
  if (tokens.empty()) {
    throw 42;
  }
  auto token = tokens.front();
  if (auto *numTok = std::get_if<Token::Num>(&token); numTok) {
    std::cout << "found number " << *numTok << std::endl;
    tokens = tokens.subspan(1);
    co_return Ast::Node{Ast::Number{numTok->value}};
  }
  if (auto *idTok = std::get_if<Token::Id>(&token); idTok) {
    auto idv = std::string_view{idTok->name};
    auto r = scope.query(idv);
    if (std::holds_alternative<std::monostate>(r)) {
      std::cout << "await scope for " << idv << std::endl;
      co_await scope.waitFor(idTok->name);
      std::cout << "resumed for " << idv << std::endl;
      r = scope.query(idv);
    }
    if (auto *variable = std::get_if<const ScopeVariable *>(&r); variable) {
      std::cout << "found variable for " << idv << std::endl;
      tokens = tokens.subspan(1);
      co_return Ast::Node{Ast::RefVariable{idTok->name}};
    }
  }
  throw 32;
}

auto parseOperation(Scope &scope, Token::Span &tokens) -> Task<Ast::Node>;

auto parseMultiplyAdd(Scope &scope, Token::Span &tokens) -> Task<Ast::Node> {
  auto left = co_await parseValue(scope, tokens);
  if (ignoreTokenIf<Token::Multiply>(tokens)) {
    auto right = co_await parseOperation(scope, tokens);
    co_return Ast::Node{Ast::Operator{
        .type = Ast::Operator::Multiply,
        .leftOperand = std::make_unique<Ast::Node>(std::move(left)),
        .rightOperand = std::make_unique<Ast::Node>(std::move(right)),
    }};
  }
  if (ignoreTokenIf<Token::Add>(tokens)) {
    auto right = co_await parseOperation(scope, tokens);
    co_return Ast::Node{Ast::Operator{
        .type = Ast::Operator::Add,
        .leftOperand = std::make_unique<Ast::Node>(std::move(left)),
        .rightOperand = std::make_unique<Ast::Node>(std::move(right)),
    }};
  }
  co_return left;
}

auto parseOperation(Scope &scope, Token::Span &tokens) -> Task<Ast::Node> {
  return parseMultiplyAdd(scope, tokens);
}

auto parseDefineVariable(Scope &scope, Token::Span &tokens) -> Task<Ast::Node> {
  std::cout << "parseDefineVariable " << tokens << std::endl;
  auto id = consumeToken<Token::Id>(tokens);
  scope.defineVariable(id.name);
  auto value = Ast::NodePtr{};
  if (ignoreTokenIf<Token::Assign>(tokens)) {
    value = std::make_unique<Ast::Node>(co_await parseOperation(scope, tokens));
  }
  co_return Ast::Node{Ast::DefineVariable{
      .name = id.name,
      .valueNode = std::move(value),
  }};
}

auto parseLine(Scope &scope, Token::Span tokens) -> Task<Ast::Node> {
  std::cout << "parseLine " << tokens << std::endl;
  if (tokens.empty()) {
    co_return Ast::Node{};
  }
  if (ignoreTokenIf<Token::Var>(tokens)) {
    co_return co_await parseDefineVariable(scope, tokens);
  }
  else {
    co_return co_await parseOperation(scope, tokens);
  }
}

auto parse(std::span<Token::Vec> tokenLines) -> Ast::NodeVec {
  auto r = Ast::NodeVec{};
  auto scope = Scope{};
  for (auto &line : tokenLines) {
    CoFiber::launch(
        [](Ast::NodeVec &r, Scope &scope, Token::Span line) -> Task<> {
          auto lineAst = co_await parseLine(scope, line);
          std::cout << "finished " << line << ", got " << lineAst << '\n';
          r.emplace_back(std::move(lineAst));
        },
        r, scope, line);
  }
  // scope.cleanup();
  return r;
}

int main() {
  auto tokens = []() {
    using namespace Token;
    return std::array{
        Vec{Var, Id{"x"}, Assign, Id{"a"}, Add, Id{"b"}},
        Vec{Var, Id{"a"}, Assign, Num{29}},
        Vec{Var, Id{"b"}, Assign, Num{32}},
    };
  }();
  std::cout << "Tokens:\n" << tokens << '\n';

  auto ast = parse(tokens);

  std::cout << "Ast: " << ast << '\n';
  std::cout << "Done!" << std::endl;
}
