#include "ast.h"
#include "parser.h"
#include <gtest/gtest.h>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

// 修复后的完整测试用例
TEST(TestParser, ParseLiteral)
{
    std::vector<std::tuple<std::string, std::unique_ptr<ASTNode>>> testCases;

    testCases.emplace_back("null", std::make_unique<NullLiteral>());
    testCases.emplace_back("true", std::make_unique<BooleanLiteral>(true));
    testCases.emplace_back("false", std::make_unique<BooleanLiteral>(false));
    testCases.emplace_back("123", std::make_unique<IntegerLiteral>(123));
    testCases.emplace_back("1.23", std::make_unique<FloatLiteral>(1.23));
    testCases.emplace_back("\"hello\"", std::make_unique<StringLiteral>("hello"));

    for (const auto& [input, expected] : testCases) {
        auto parser = std::make_unique<Parser>(input);
        auto actual = parser->parse_expression();

        // // 断言基础检查
        // EXPECT_THAT(actual, NotNull()) << "解析失败: " << input;

        // 类型匹配检查
        EXPECT_EQ(actual->kind(), expected->kind()) << "类型不匹配: " << input << ", 实际类型: " << int(actual->kind())
                                                    << ", 预期类型: " << int(expected->kind());

        EXPECT_EQ(*actual, *expected) << "常量表达式解析出错: " << input;
    }
}
TEST(TestParser, ParseExpression)
{
    std::vector<std::tuple<std::string, std::unique_ptr<ASTNode>>> testCases;

    testCases.emplace_back("1 + 1",
        std::make_unique<BinaryExpression>(Operator::Add,
            std::make_unique<IntegerLiteral>(1), // 左操作数
            std::make_unique<IntegerLiteral>(1) // 右操作数
            ));
    testCases.emplace_back("5 * 2",
        std::make_unique<BinaryExpression>(
            Operator::Multiply, std::make_unique<IntegerLiteral>(5), std::make_unique<IntegerLiteral>(2)));
    testCases.emplace_back("10 / 2",
        std::make_unique<BinaryExpression>(
            Operator::Divide, std::make_unique<IntegerLiteral>(10), std::make_unique<IntegerLiteral>(2)));
    testCases.emplace_back("4 - 1",
        std::make_unique<BinaryExpression>(
            Operator::Subtract, std::make_unique<IntegerLiteral>(4), std::make_unique<IntegerLiteral>(1)));
    testCases.emplace_back("3 + (4 * 2)",
        std::make_unique<BinaryExpression>(Operator::Add, std::make_unique<IntegerLiteral>(3),
            std::make_unique<BinaryExpression>(
                Operator::Multiply, std::make_unique<IntegerLiteral>(4), std::make_unique<IntegerLiteral>(2))));

    std::vector<std::unique_ptr<Expression>> args;
    args.push_back(std::make_unique<IntegerLiteral>(1));
    args.push_back(std::make_unique<IntegerLiteral>(2));
    testCases.emplace_back(
        "a(1, 2)", std::make_unique<CallExpression>(std::make_unique<VariableExpression>("a"), std::move(args)));

    for (const auto& [input, expected] : testCases) {
        auto parser = std::make_unique<Parser>(input);
        auto actual = parser->parse_expression();

        // 类型匹配检查
        EXPECT_EQ(actual->kind(), expected->kind()) << "类型不匹配: " << input << ", 实际类型: " << int(actual->kind())
                                                    << ", 预期类型: " << int(expected->kind());

        EXPECT_EQ(*actual, *expected) << "表达式解析出错: " << input;
    }
}

TEST(TestParser, ParseStatement)
{
    std::vector<std::tuple<std::string, std::unique_ptr<ASTNode>>> testCases;

    testCases.emplace_back(";", std::make_unique<EmptyStatement>());

    testCases.emplace_back("break;", std::make_unique<BreakStatement>());

    testCases.emplace_back("continue;", std::make_unique<ContinueStatement>());

    testCases.emplace_back("return;", std::make_unique<ReturnStatement>(nullptr));

    testCases.emplace_back("return 10;", std::make_unique<ReturnStatement>(std::make_unique<IntegerLiteral>(10)));

    testCases.emplace_back("let a;", std::make_unique<LetStatement>(std::string("a"), nullptr));

    testCases.emplace_back(
        "let a = 1;", std::make_unique<LetStatement>(std::string("a"), std::make_unique<IntegerLiteral>(1)));

    // if-then
    {
        std::vector<std::unique_ptr<Statement>> thenStmts;
        std::cout << thenStmts.size() << std::endl;
        testCases.emplace_back("if true {  }",
            std::make_unique<IfStatement>(std::make_unique<BooleanLiteral>(true),
                std::make_unique<BlockStatement>(std::move(thenStmts)), nullptr));
    }

    // if-then-else
    {
        std::vector<std::unique_ptr<Statement>> thenStmts;
        thenStmts.push_back(std::make_unique<ExpressionStatement>(std::make_unique<IntegerLiteral>(1)));
        std::vector<std::unique_ptr<Statement>> elseStmts;
        elseStmts.push_back(std::make_unique<ExpressionStatement>(std::make_unique<IntegerLiteral>(2)));

        testCases.emplace_back("if false { 1; } else { 2; }",
            std::make_unique<IfStatement>(std::make_unique<BooleanLiteral>(false),
                std::make_unique<BlockStatement>(std::move(thenStmts)),
                std::make_unique<BlockStatement>(std::move(elseStmts))));
    }
    // empty for statement
    {
        std::vector<std::unique_ptr<Statement>> blockStmts;
        testCases.emplace_back("for ; ; { }",
            std::make_unique<ForStatement>(
                nullptr, nullptr, nullptr, std::make_unique<BlockStatement>(std::move(blockStmts))));
    }

    // for statement
    {
        std::vector<std::unique_ptr<Statement>> blockStmts;
        blockStmts.push_back(std::make_unique<ExpressionStatement>(std::make_unique<IntegerLiteral>(1)));
        testCases.emplace_back("for let i = 0; i < 10; i = i + 1 { 1; }",
            std::make_unique<ForStatement>(
                std::make_unique<LetStatement>(std::string("i"), std::make_unique<IntegerLiteral>(0)),
                std::make_unique<BinaryExpression>(Operator::LessThan, std::make_unique<VariableExpression>("i"),
                    std::make_unique<IntegerLiteral>(10)),
                std::make_unique<BinaryExpression>(Operator::Assign, std::make_unique<VariableExpression>("i"),
                    std::make_unique<BinaryExpression>(
                        Operator::Add, std::make_unique<VariableExpression>("i"), std::make_unique<IntegerLiteral>(1))),
                std::make_unique<BlockStatement>(std::move(blockStmts))));
    }

    // expression statement
    testCases.emplace_back("3 * 4;",
        std::make_unique<ExpressionStatement>(std::make_unique<BinaryExpression>(
            Operator::Multiply, std::make_unique<IntegerLiteral>(3), std::make_unique<IntegerLiteral>(4))));

    // block statement
    {
        std::vector<std::unique_ptr<Statement>> blockStmts;
        blockStmts.push_back(std::make_unique<LetStatement>("x", std::make_unique<IntegerLiteral>(5)));
        blockStmts.push_back(std::make_unique<ReturnStatement>(std::make_unique<VariableExpression>("x")));
        testCases.emplace_back("{ let x =5; return x; }", std::make_unique<BlockStatement>(std::move(blockStmts)));
    }

    // fn statement
    {
        std::vector<std::string> params;
        params.push_back("x");
        params.push_back("y");

        std::vector<std::unique_ptr<Statement>> blockStmts;
        blockStmts.push_back(std::make_unique<ReturnStatement>(std::make_unique<BinaryExpression>(
            Operator::Add, std::make_unique<VariableExpression>("x"), std::make_unique<VariableExpression>("y"))));

        auto body = std::make_unique<BlockStatement>(std::move(blockStmts));

        testCases.emplace_back("fn add(x, y) { return x + y; }",
            std::make_unique<FnStatement>(std::move(std::string("add")), std::move(params), std::move(body)));
    }

    for (const auto& [input, expected] : testCases) {
        auto parser = std::make_unique<Parser>(input);
        auto actual = parser->parse_statement();

        // 类型匹配检查
        EXPECT_EQ(actual->kind(), expected->kind()) << "类型不匹配: " << input << ", 实际类型: " << int(actual->kind())
                                                    << ", 预期类型: " << int(expected->kind());

        EXPECT_EQ(*actual, *expected) << "语句解析出错: " << input << std::endl
                                      << ASTInspector::inspect(*actual) << " vs " << ASTInspector::inspect(*expected);
    }
}
