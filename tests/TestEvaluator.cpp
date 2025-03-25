#include "ast.h"
#include "eval.h"
#include "object.h"
#include "parser.h"

#include <memory>

#include <gtest/gtest.h>

TEST(EvaluatorTest, TestLiteral)
{
    std::vector<std::tuple<std::string_view, Value>> tests = {
        { "null", Value() },
        { "true", Value(true) },
        { "false", Value(false) },
        { "1", Value(1) },
        { "1.0", Value(1.0) },
        { "\"hello\"", Value("hello") },
    };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);

            auto expr = parse->parse_expression();

            // std::cout << "Input: " << input << ", Parsed: " <<
            // ASTInspector::inspect(*expr) << std::endl;

            auto context = Context {};

            auto ret = std::make_unique<Evaluator>(context)->eval(*expr);

            EXPECT_EQ(ret, expected)
                << "Input: " << input << ", Parsed: " << ASTInspector::inspect(*expr) << std::endl
                << "Expected: " << expected.inspect() << ", Got: " << ret.inspect();
        } catch (std::exception& e) {
            FAIL() << "Exception: " << e.what();
        }
    }
}

TEST(EvaluatorTest, TestExpression)
{
    std::vector<std::tuple<std::string_view, Value>> tests = {
        { "-1", Value(-1) }, // 补充负号运算测试
        { "!true", Value(false) }, // 补充逻辑非运算测试
        { "!false", Value(true) }, // 逻辑非真值测试

        // 添加二元运算测试用例
        { "1 + 2", Value(3) }, // 加法测试
        { "5 - 3", Value(2) }, // 减法测试
        { "4 * 3", Value(12) }, // 乘法测试
        { "8 / 2", Value(4) }, // 整数除法
        { "5.0 / 2", Value(2.5) }, // 浮点除法
        { "3 > 2", Value(true) }, // 比较运算
        { "3 < 5", Value(true) }, // 反向比较
        { "true && false", Value(false) }, // 逻辑与
        { "false || true", Value(true) }, // 逻辑或
        { "1 + 2.5", Value(3.5) }, // 类型自动提升
        { "2 + 3 * 5", Value(17) }, // 运算符优先级验证

        // 新增括号优先级测试
        { "-(3 + 2)", Value(-5) }, // 带括号的复合表达式测试
        { "(2 + 3) * 5", Value(25) }, // 括号改变优先级测试
        { "4 * (6 - (2 + 1))", Value(12) }, // 多层括号嵌套
        { "(5 > 3) && (2 < 4)", Value(true) }, // 括号组合逻辑运算
    };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);

            auto expr = parse->parse_expression();

            // std::cout << "Input: " << input << ", Parsed: " <<
            // ASTInspector::inspect(*expr) << std::endl;

            auto context = Context {};

            auto ret = std::make_unique<Evaluator>(context)->eval(*expr);

            EXPECT_EQ(ret, expected)
                << "Input: " << input << ", Parsed: " << ASTInspector::inspect(*expr) << std::endl
                << "Expected: " << expected.inspect() << ", Got: " << ret.inspect();
        } catch (std::exception& e) {
            FAIL() << "Exception: " << e.what();
        }
    }
}

TEST(EvaluatorTest, TestProgram)
{
    std::vector<std::tuple<std::string_view, Value>> tests = { { "return;", Value() },
        { "return 1;", Value(1) }, { "if true { return 1; } else { return 2; }", Value(1) },
        { "if false { return 1; } else { return 2; }", Value(2) },
        { "if true { return 3; }", Value(3) }, { "if false { return 4; } return 5;", Value(5) },
        { "let i; for i = 0; i < 10; i++ { } return i;", Value(10) },
        { "let sum = 0; for let i = 1; i <= 5; i++ { sum = sum + i; } return sum;", Value(15) },
        { "let i; for i = 0; i < 5; i++ { if i == 3 { break; } } return i;", Value(3) },
        { "let sum = 0; for let i = 1; i <= 4; i++ { if i % 2 == 0 { continue; } sum = sum + "
          "i; } return sum;",
            Value(4) },
        { "let j = 0; for j = 0; j < 3; j = j + 1 { continue; } return j;", Value(3) },
        { "fn f() {return 1;} return f();", Value(1) },
        { "fn add(a, b) {return a + b;} return add(1, 2);", Value(3) },
        { "fn fib(n) { if n <= 1 { return n; } else { return fib(n - 1) + fib(n - 2); } } return "
          "fib(10);",
            Value(55) } };

    for (auto& [input, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);
            std::shared_ptr<Program> program = std::move(parse->parse());

            // std::cout << "Input: " << input << ", Parsed: " << ASTInspector::inspect(*program)
            //           << std::endl;

            auto context = Context(program);

            auto ret = std::make_unique<Evaluator>(context)->eval();

            EXPECT_EQ(ret, expected)
                << "Input: " << input << ", Parsed: " << ASTInspector::inspect(*program)
                << std::endl
                << "Expected: " << expected.inspect() << ", Got: " << ret.inspect();
        } catch (std::exception& e) {
            FAIL() << "Exception: " << e.what();
        }
    }
}

TEST(EvaluatorTest, TestEnvironment)
{
    std::vector<std::tuple<std::string_view, Value, Value>> tests = {
        { "return $a;", Value(1), Value(1) },
        { "return $a + 1;", Value(1), Value(2) },
        { "return $a + \", world\";", Value("hello"), Value("hello, world") },
    };

    for (auto& [input, env, expected] : tests) {
        try {
            auto parse = std::make_unique<Parser>(input);
            std::shared_ptr<Program> program = std::move(parse->parse());

            auto context = Context(program);

            context.define("a", env);

            auto ret = std::make_unique<Evaluator>(context)->eval();

            EXPECT_EQ(ret, expected)
                << "Input: " << input << ", Parsed: " << ASTInspector::inspect(*program)
                << std::endl
                << "Expected: " << expected.inspect() << ", Got: " << ret.inspect();
        } catch (std::exception& e) {
            FAIL() << "Exception: " << e.what();
        }
    }
}
