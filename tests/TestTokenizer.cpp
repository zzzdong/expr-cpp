#include "tokenizer.h"
#include <gtest/gtest.h> // 添加gtest头文件

TEST(TestTokenizer, SampleTest)
{
    auto input = "let a = 1 + b * 2;";

    Tokenizer tokenizer(input);

    // 预期的token序列
    std::vector<Token> expected_tokens = {
        { TokenKind::Let, "let" },
        { TokenKind::Identifier, "a" },
        { TokenKind::Assign, "=" },
        { TokenKind::Integer, "1" },
        { TokenKind::Plus, "+" },
        { TokenKind::Identifier, "b" },
        { TokenKind::Star, "*" },
        { TokenKind::Integer, "2" },
        { TokenKind::Semicolon, ";" },
    };

    size_t index = 0;
    while (true) {
        auto token = tokenizer.next();

        if (token.kind == TokenKind::Eof) {
            break;
        }

        // 断言检查
        EXPECT_EQ(token.kind, expected_tokens[index].kind) << "At index " << index;
        EXPECT_EQ(token.text, expected_tokens[index].text);

        index++;
    }

    // 确保所有预期的token都被处理
    EXPECT_EQ(index, expected_tokens.size());
}