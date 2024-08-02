#include "ast.h"
#include "eval.h"
#include "parser.h"

#include <format>
#include <iostream>
#include <memory>
#include <string_view>
#include <tuple>
#include <vector>

int test_eval_expression()
{
    std::vector<std::tuple<std::string_view, Value>> tests = {
        { "(1 + 2) * 3 / 4.0", Value(2.25) },
        { "3 / 4.0", Value(0.75) },

    };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);

            auto expr = parse->parse_expression();

            std::cout << ASTInspector::inspect(*expr) << std::endl;

            auto context = Context {};

            auto ret = std::make_unique<Evaluator>(context)->eval(*expr);

            if (ret.kind() != expected.kind()) {
                throw std::runtime_error(std::format("expected: {}, got: {}",
                    value_kind_str(expected.kind()),
                    value_kind_str(ret.kind())));
            }

        } catch (std::exception& e) {
            std::cout << std::format("FAILED: {}", e.what()) << std::endl;
            return -1;
        }
    }

    return 0;
}

int test_eval_program()
{
    std::vector<std::tuple<std::string_view, Value>> tests = {
        { "return (1 + 2) * 3 / 4.0;", Value(2.25) },
        { "let a = 1; if (a % 2 == 1) { return a + 1; } else { return a; }",
            Value(2) },
        { "let sum = 0; for (let i = 0; i < 10; i++) { sum = sum + i; } return sum;",
            Value(45) },
        { "let sum = 0; for (let i = 0; i < 10; i++) { if (i % 2 == 1) { sum = sum + i; } } return sum;",
            Value(25) },
        { "fn add(a, b) { return a + b; } return add(1, 2);",
            Value(3) },
        { "fn fib(n) { if (n <= 0) { return 0; } if (n <= 2) { return 1; } return fib(n - 1) + fib(n - 2); } return fib(10);",
            Value(55) }
    };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);

            auto program = parse->parse();

            std::cout << ASTInspector::inspect(*program) << std::endl;

            auto context = Context(std::move(program));

            auto ret = std::make_unique<Evaluator>(context)->eval();

            if (ret.kind() != expected.kind()) {
                throw std::runtime_error(std::format("expected: {}, got: {}",
                    value_kind_str(expected.kind()),
                    value_kind_str(ret.kind())));
            }

            if (ret.obj()->compare(expected) != Comparison::Equal) {
                throw std::runtime_error(std::format(
                    "expected: {}, got: {}", expected.inspect(), ret.inspect()));
            }
            std::cout << std::format("PASSED: `{}` = {}", input, ret.inspect())
                      << std::endl;
        } catch (std::exception& e) {
            std::cout << std::format("FAILED: {}", e.what()) << std::endl;
            return -1;
        }
    }

    return 0;
}

int fib(int n)
{
    if (n <= 0) {
        return 0;
    } else if (n <= 2) {
        return 1;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

int test_eval_environment()
{
    int a = 1;

    std::vector<std::tuple<std::string_view, Value>> tests = {
        { "return a + 1;", Value(2) },
        // { "return fib(10);", Value(55) },
    };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);

            auto program = parse->parse();

            std::cout << ASTInspector::inspect(*program) << std::endl;

            auto context = Context(std::move(program));

            context.define("a", 1);
            // context.define("fib", std::make_shared<NativeFunction<int, int>>("fib", fib));

            // std::shared_ptr<Object> f = std::make_shared<Callable<int,int>>("fib", fib);

            auto ret = std::make_unique<Evaluator>(context)->eval();

            if (ret.kind() != expected.kind()) {
                throw std::runtime_error(std::format("expected: {}, got: {}",
                    value_kind_str(expected.kind()),
                    value_kind_str(ret.kind())));
            }

            if (ret.obj()->compare(expected) != Comparison::Equal) {
                throw std::runtime_error(std::format(
                    "expected: {}, got: {}", expected.inspect(), ret.inspect()));
            }
            std::cout << std::format("PASSED: `{}` = {}", input, ret.inspect())
                      << std::endl;
        } catch (std::exception& e) {
            std::cout << std::format("FAILED: {}", e.what()) << std::endl;
            return -1;
        }
    }

    return 0;
}

int main(int argc, const char* argv[])
{

    std::cout << "Testing evaluator..." << std::endl;

    test_eval_expression();

    test_eval_program();

    test_eval_environment();

    return 0;
}