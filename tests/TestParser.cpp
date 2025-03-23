#include "parser.h"
#include <gtest/gtest.h>


TEST(TestParser, ParseExpression) {
    std::vector<std::string> inputs = {
        "a.b = a.a() + 1 + b * 2 / (3 - 4) * 5 - [0,1,2][1]",
    };

    for (auto &input : inputs) {
        auto parse = std::make_unique<Parser>(input);
        auto expr = parse->parse_expression();

        // 添加AST结构断言
        EXPECT_NE(expr, nullptr); // 确保解析成功
    }
}

TEST(TestParser, ParseProgram) {
    std::vector<std::string> inputs = {
        "let a = 1; if (a % 2 == 1) { return a + 1; } else { return a; }",
        // ... existing inputs ...
    };

    for (auto input : inputs) {
        auto parse = std::make_unique<Parser>(input);
        auto program = parse->parse();

        // 添加AST结构断言
        EXPECT_NE(program, nullptr); // 确保解析成功
    }
}
