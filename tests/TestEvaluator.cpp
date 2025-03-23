#include "eval.h"
#include "parser.h"
#include <gtest/gtest.h>
TEST(EvaluatorTest, TestExpression)
{
    std::vector<std::tuple<std::string_view, Value>> tests = {
        { "(1 + 2) * 3 / 4.0", Value(2.25) },
        { "3 / 4.0", Value(0.75) },
    };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);

            auto expr = parse->parse_expression();

            auto context = Context {};

            auto ret = std::make_unique<Evaluator>(context)->eval(*expr);

            // 替换原有两个断言为单个比较
            EXPECT_EQ(ret, expected);
        } catch (std::exception& e) {
            FAIL() << "Exception: " << e.what();
        }
    }
}

TEST(EvaluatorTest, TestProgram)
{
    std::vector<std::tuple<std::string_view, Value>> tests = {
        { "return (1 + 2) * 3 / 4.0;", Value(2.25) },
        { "return 3 / 4.0;", Value(0.75) },
    };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);
            auto program = parse->parse();

            auto context = Context(std::move(program));

            auto ret = std::make_unique<Evaluator>(context)->eval();

            // 替换原有断言为单个比较
            EXPECT_EQ(ret, expected);
        } catch (std::exception& e) {
            FAIL() << "Exception: " << e.what();
        }
    }
}

TEST(EvaluatorTest, TestEnvironment)
{
    std::vector<std::tuple<std::string_view, Value>> tests = {
        { "return a + 1;", Value(2) },
        { "return a;", Value(1) },
    };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);
            auto program = parse->parse();

            auto context = Context(std::move(program));

            context.define("a", 1);

            auto ret = std::make_unique<Evaluator>(context)->eval();

            // 替换原有断言为单个比较
            EXPECT_EQ(ret, expected);
        } catch (std::exception& e) {
            FAIL() << "Exception: " << e.what();
        }
    }
}
