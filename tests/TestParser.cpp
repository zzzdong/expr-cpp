#include "ast.h"
#include "object.h"
#include "parser.h"

#include <iostream>
#include <memory>
#include <vector>

int test_parse_expression() {
  std::vector<std::string> inputs = {
      "a.b = a.a() + 1 + b * 2 / (3 - 4) * 5 - [0,1,2][1]",
  };

  for (auto &input : inputs) {
    auto parse = std::make_unique<Parser>(input);

    auto expr = parse->parse_expression();

    std::cout << ASTInspector::inspect(*expr) << std::endl;
  }

  return 0;
}

int test_parse_program() {
  std::vector<std::string> inputs = {
      "let a = 1; if (a % 2 == 1) { return a + 1; } else { return a; }",
      "let sum = 0; for (let i = 0; i < 10; i++) { sum = sum + 1; }",
  };

  for (auto input : inputs) {
    auto parse = std::make_unique<Parser>(input);

    auto program = parse->parse();

    std::cout << ASTInspector::inspect(*program) << std::endl;
  }

  return 0;
}

int main(int argc, const char *argv[]) {
  // test_parse_expression();
  test_parse_program();

  return 0;
}