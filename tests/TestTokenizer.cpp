#include "tokenizer.h"

#include <string>
#include <tuple>
#include <vector>

#include <gtest/gtest.h> // 添加gtest头文件

TEST(TestTokenizer, SingleTest)
{
    std::vector<std::tuple<std::string, Token>> testCases = {
        { "let", Token { TokenKind::Let, "let" } },
        { "fn", Token { TokenKind::Fn, "fn" } },
        { "if", Token { TokenKind::If, "if" } },
        { "else", Token { TokenKind::Else, "else" } },
        { "for", Token { TokenKind::For, "for" } },
        { "return", Token { TokenKind::Return, "return" } },
        { "true", Token { TokenKind::True, "true" } },
        { "false", Token { TokenKind::False, "false" } },
        { "null", Token { TokenKind::Null, "null" } },
        { "+", Token { TokenKind::Plus, "+" } },
        { "-", Token { TokenKind::Minus, "-" } },
        { "*", Token { TokenKind::Star, "*" } },
        { "/", Token { TokenKind::Slash, "/" } },
        { "%", Token { TokenKind::Percent, "%" } },
        { "!", Token { TokenKind::Bang, "!" } },
        { "==", Token { TokenKind::Equals, "==" } },
        { "!=", Token { TokenKind::NotEquals, "!=" } },
        { "<", Token { TokenKind::LessThan, "<" } },
        { ">", Token { TokenKind::GreaterThan, ">" } },
        { "<=", Token { TokenKind::LessThanOrEqual, "<=" } },
        { ">=", Token { TokenKind::GreaterThanOrEqual, ">=" } },
        { "&&", Token { TokenKind::LogicAnd, "&&" } },
        { "||", Token { TokenKind::LogicOr, "||" } },
        { ",", Token { TokenKind::Comma, "," } },
        { ";", Token { TokenKind::Semicolon, ";" } },
        { "(", Token { TokenKind::LParen, "(" } },
        { ")", Token { TokenKind::RParen, ")" } },
        { "{", Token { TokenKind::LBrace, "{" } },
        { "}", Token { TokenKind::RBrace, "}" } },
        { "[", Token { TokenKind::LBracket, "[" } },
        { "]", Token { TokenKind::RBracket, "]" } },
        { ":", Token { TokenKind::Colon, ":" } },
        { ".", Token { TokenKind::Dot, "." } },
        { "++", Token { TokenKind::Increase, "++" } },
        { "--", Token { TokenKind::Decrease, "--" } },
        { "=", Token { TokenKind::Assign, "=" } },
        { "var", Token { TokenKind::Identifier, "var" } },
        { "$env", Token { TokenKind::EnvVariable, "$env" } },
        { "\"hello\"", Token { TokenKind::String, "\"hello\"" } },
        { "\", world\"", Token { TokenKind::String, "\", world\"" } },

    };

    for (const auto& [input, expected] : testCases) {
        Tokenizer tokenizer(input);
        Token token = tokenizer.next();

        EXPECT_EQ(token.kind, expected.kind);
        EXPECT_EQ(token.text, expected.text);
    }
}

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