#pragma once

#include <format>
#include <functional>
#include <string_view>
#include <unordered_map>

enum class TokenKind {
    Eof = 0, // end of file
    Identifier, // identifier
    Null, // null
    True, // true
    False, // false
    Integer, // integer
    Float, // float
    String, // string
    Let, // let
    Fn, // fn
    If, // if
    Else, // else
    For, // for
    Break, // break
    Continue, // continue
    Return, // return
    Comma, // ,
    Semicolon, // ;
    Colon, // :
    LParen, // (
    RParen, // )
    LBrace, // {
    RBrace, // }
    LBracket, // [
    RBracket, // ]
    Dot, // .
    Plus, // +
    Minus, // -
    Star, // *
    Slash, // /
    Percent, // %
    Bang, // !
    Equals, // ==
    NotEquals, // !=
    GreaterThan, // >
    GreaterThanOrEqual, // >=
    LessThan, // <
    LessThanOrEqual, // <=
    LogicAnd, // &&
    LogicOr, // ||
    Assign, // =
    Increase, // ++
    Decrease, // --
    Invalid,
};

const static std::unordered_map<std::string_view, TokenKind> KEYWORDS = {
    { "null", TokenKind::Null },
    { "true", TokenKind::True },
    { "false", TokenKind::False },
    { "let", TokenKind::Let },
    { "fn", TokenKind::Fn },
    { "if", TokenKind::If },
    { "else", TokenKind::Else },
    { "for", TokenKind::For },
    { "break", TokenKind::Break },
    { "continue", TokenKind::Continue },
    { "return", TokenKind::Return },

};

const static std::unordered_map<char32_t, TokenKind> PUNCTUATIONS = {
    { ',', TokenKind::Comma },
    { ';', TokenKind::Semicolon },
    { '.', TokenKind::Dot },
    { '{', TokenKind::LBrace },
    { '[', TokenKind::LBracket },
    { '(', TokenKind::LParen },
    { '}', TokenKind::RBrace },
    { ']', TokenKind::RBracket },
    { ')', TokenKind::RParen },
    { ':', TokenKind::Colon },
};

struct Token {
    TokenKind kind;
    std::string_view text;
};

class Tokenizer {
public:
    // Tokenizer();
    Tokenizer(std::string_view input);

    Token next();

private:
    std::string_view m_input;

    char32_t peek_char();

    char32_t next_char();

    Token make_token(TokenKind kind, std::string_view text);
    Token make_token(TokenKind kind, const char* start);

    Token eat_identifier();

    Token eat_number();

    Token eat_string();

    Token eat_punctuation();

    std::string_view eat_while(std::function<bool(char32_t)> predicate);
};

// 新增std::formatter特化
template <> struct std::formatter<TokenKind> : std::formatter<std::string_view> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin(); // 解析格式说明符（无需特殊处理）
    }

    template <typename FormatContext> auto format(TokenKind tok, FormatContext& ctx) const
    {
        switch (tok) {
        case TokenKind::Eof:
            return std::format_to(ctx.out(), "Eof");
        case TokenKind::Identifier:
            return std::format_to(ctx.out(), "Identifier");
        case TokenKind::Null:
            return std::format_to(ctx.out(), "Null");
        case TokenKind::True:
            return std::format_to(ctx.out(), "True");
        case TokenKind::False:
            return std::format_to(ctx.out(), "False");
        case TokenKind::Integer:
            return std::format_to(ctx.out(), "Integer");
        case TokenKind::Float:
            return std::format_to(ctx.out(), "Float");
        case TokenKind::String:
            return std::format_to(ctx.out(), "String");
        case TokenKind::Let:
            return std::format_to(ctx.out(), "Let");
        case TokenKind::Fn:
            return std::format_to(ctx.out(), "Fn");
        case TokenKind::If:
            return std::format_to(ctx.out(), "If");
        case TokenKind::Else:
            return std::format_to(ctx.out(), "Else");
        case TokenKind::For:
            return std::format_to(ctx.out(), "For");
        case TokenKind::Break:
            return std::format_to(ctx.out(), "Break");
        case TokenKind::Continue:
            return std::format_to(ctx.out(), "Continue");
        case TokenKind::Return:
            return std::format_to(ctx.out(), "Return");
        case TokenKind::Comma:
            return std::format_to(ctx.out(), "Comma");
        case TokenKind::Semicolon:
            return std::format_to(ctx.out(), "Semicolon");
        case TokenKind::Colon:
            return std::format_to(ctx.out(), "Colon");
        case TokenKind::LParen:
            return std::format_to(ctx.out(), "LParen");
        case TokenKind::RParen:
            return std::format_to(ctx.out(), "RParen");
        case TokenKind::LBrace:
            return std::format_to(ctx.out(), "LBrace");
        case TokenKind::RBrace:
            return std::format_to(ctx.out(), "RBrace");
        case TokenKind::LBracket:
            return std::format_to(ctx.out(), "LBracket");
        case TokenKind::RBracket:
            return std::format_to(ctx.out(), "RBracket");
        case TokenKind::Dot:
            return std::format_to(ctx.out(), "Dot");
        case TokenKind::Plus:
            return std::format_to(ctx.out(), "Plus");
        case TokenKind::Minus:
            return std::format_to(ctx.out(), "Minus");
        case TokenKind::Star:
            return std::format_to(ctx.out(), "Star");
        case TokenKind::Slash:
            return std::format_to(ctx.out(), "Slash");
        case TokenKind::Percent:
            return std::format_to(ctx.out(), "Percent");
        case TokenKind::Bang:
            return std::format_to(ctx.out(), "Bang");
        case TokenKind::Equals:
            return std::format_to(ctx.out(), "Equals");
        case TokenKind::NotEquals:
            return std::format_to(ctx.out(), "NotEquals");
        case TokenKind::GreaterThan:
            return std::format_to(ctx.out(), "GreaterThan");
        case TokenKind::GreaterThanOrEqual:
            return std::format_to(ctx.out(), "GreaterThanOrEqual");
        case TokenKind::LessThan:
            return std::format_to(ctx.out(), "LessThan");
        case TokenKind::LessThanOrEqual:
            return std::format_to(ctx.out(), "LessThanOrEqual");
        case TokenKind::LogicAnd:
            return std::format_to(ctx.out(), "LogicAnd");
        case TokenKind::LogicOr:
            return std::format_to(ctx.out(), "LogicOr");
        case TokenKind::Assign:
            return std::format_to(ctx.out(), "Assign");
        case TokenKind::Increase:
            return std::format_to(ctx.out(), "Increase");
        case TokenKind::Decrease:
            return std::format_to(ctx.out(), "Decrease");
        case TokenKind::Invalid:
            return std::format_to(ctx.out(), "Invalid");
        default:
            return std::format_to(ctx.out(), "Unknown"); // 未知枚举值
        }
    }
};
