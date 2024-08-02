#pragma once

#include <functional>
#include <string_view>
#include <unordered_map>

enum class TokenKind {
    Eof = 0, // end of file
    Identifier, // identifier
    Undefined, // undefined
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
