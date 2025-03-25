#include "tokenizer.h"

#include <cctype>
#include <cstddef>
#include <cuchar>
#include <functional>
#include <stdexcept>
#include <string_view>

Tokenizer::Tokenizer(std::string_view input)
    : m_input(input)
{
}

Token Tokenizer::next()
{
    if (m_input.empty()) {
        return Token { TokenKind::Eof, "" };
    }

    while (true) {
        char32_t peek = peek_char();
        if (peek == 0) {
            return Token { TokenKind::Eof, "" };
        }

        if (peek == ' ' || peek == '\t' || peek == '\n' || peek == '\r') {
            next_char();
            continue;
        }

        if (peek == '$') {
            return eat_env_variable();
        }

        if (std::isdigit(peek)) {
            return eat_number();
        }

        if (std::isalpha(peek) || peek == '_') {
            return eat_identifier();
        }

        if (peek == '"') {
            return eat_string();
        }

        return eat_punctuation();
    }
}

char32_t Tokenizer::peek_char()
{
    if (m_input.empty())
        return 0;

    char32_t ch;
    size_t rc = std::mbrtoc32(&ch, m_input.data(), m_input.size(), nullptr);
    if (rc == 0) {
        return 0;
    }

    if (rc < 0) {
        throw std::runtime_error("Invalid UTF-8 sequence");
    }

    return ch;
}

char32_t Tokenizer::next_char()
{
    char32_t ch = peek_char();
    if (ch == 0) {
        return 0;
    }

    m_input.remove_prefix(std::c32rtomb(nullptr, ch, nullptr));
    return ch;
}

Token Tokenizer::make_token(TokenKind kind, std::string_view text) { return Token { kind, text }; }

Token Tokenizer::make_token(TokenKind kind, const char* start)
{
    return Token { kind, std::string_view(start, m_input.begin() - start) };
}

Token Tokenizer::eat_env_variable()
{
    auto start = m_input.begin();

    next_char(); // eat $

    auto peek = peek_char();
    if (peek != '_' && !std::isalpha(peek)) { // must start with _ or a letter
        return make_token(TokenKind::Invalid, start);
    }

    auto s = eat_while([](char32_t ch) { return std::isalnum(ch) || ch == '_'; });
    auto found = KEYWORDS.find(s);
    if (found != KEYWORDS.end()) { // must not be a keyword
        return make_token(TokenKind::Invalid, start);
    }

    return make_token(TokenKind::EnvVariable, start);
}

Token Tokenizer::eat_identifier()
{
    auto s = eat_while([](char32_t ch) { return std::isalnum(ch) || ch == '_'; });
    auto found = KEYWORDS.find(s);
    if (found != KEYWORDS.end()) {
        return make_token(found->second, s);
    }

    return make_token(TokenKind::Identifier, s);
}

Token Tokenizer::eat_number()
{
    auto start = m_input.begin();
    eat_while([](char32_t ch) { return std::isdigit(ch); });

    auto peek = peek_char();
    if (peek == '.') {
        next_char();
        eat_while([](char32_t ch) { return std::isdigit(ch); });

        return make_token(TokenKind::Float, start);
    }

    return make_token(TokenKind::Integer, start);
}

Token Tokenizer::eat_string()
{
    auto start = m_input.begin();

    next_char(); // eat "

    bool escaped = false;
    char32_t ch = next_char();

    while (ch != 0) {
        if (ch == '"') {
            if (!escaped) {
                return make_token(TokenKind::String, start);
            } else {
                escaped = false; // 处理转义的双引号（\"的情况）
            }
        } else if (ch == '\\') {
            escaped = !escaped; // 简化转义标志切换
        } else {
            escaped = false;
        }

        ch = next_char();
    }

    return make_token(TokenKind::Invalid, "Unclosed string literal");
}

Token Tokenizer::eat_punctuation()
{
    auto start = m_input.begin();

    auto peek = peek_char();
    auto found = PUNCTUATIONS.find(peek);
    if (found != PUNCTUATIONS.end()) {
        next_char();
        return make_token(found->second, start);
    }

    switch (peek_char()) {
    case '!':
        next_char();
        if (peek_char() == '=') {
            next_char();
            return make_token(TokenKind::NotEquals, start);
        }
        return make_token(TokenKind::Bang, start);
    case '=':
        next_char();
        if (peek_char() == '=') {
            next_char();
            return make_token(TokenKind::Equals, start);
        }
        return make_token(TokenKind::Assign, start);
    case '>':
        next_char();
        if (peek_char() == '=') {
            next_char();
            return make_token(TokenKind::GreaterThanOrEqual, start);
        }
        return make_token(TokenKind::GreaterThan, start);
    case '<':
        next_char();
        if (peek_char() == '=') {
            next_char();
            return make_token(TokenKind::LessThanOrEqual, start);
        }
        return make_token(TokenKind::LessThan, start);
    case '+':
        next_char();
        if (peek_char() == '+') {
            next_char();
            return make_token(TokenKind::Increase, start);
        }
        return make_token(TokenKind::Plus, start);
    case '-':
        next_char();
        if (peek_char() == '-') {
            next_char();
            return make_token(TokenKind::Decrease, start);
        }
        return make_token(TokenKind::Minus, start);
    case '*':
        next_char();
        return make_token(TokenKind::Star, start);
    case '/':
        next_char();
        return make_token(TokenKind::Slash, start);
    case '%':
        next_char();
        return make_token(TokenKind::Percent, start);
    case '&':
        next_char();
        if (peek_char() == '&') {
            next_char();
            return make_token(TokenKind::LogicAnd, start);
        }
        return make_token(TokenKind::Invalid, start);
    case '|':
        next_char();
        if (peek_char() == '|') {
            next_char();
            return make_token(TokenKind::LogicOr, start);
        }
        return make_token(TokenKind::Invalid, start);
    default:
        return make_token(TokenKind::Invalid, start);
    }
}

std::string_view Tokenizer::eat_while(std::function<bool(char32_t)> predicate)
{
    auto start = m_input.begin();

    while (true) {
        char32_t peek = peek_char();

        if (!predicate(peek)) {
            return std::string_view(start, m_input.begin() - start);
        }

        next_char();
    }
}