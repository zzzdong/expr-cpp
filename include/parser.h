#pragma once

#include "ast.h"
#include "tokenizer.h"

#include <format>
#include <memory>
#include <string_view>

class Parser {
public:
    Parser() = delete;
    Parser(std::string_view input);

    std::unique_ptr<Program> parse();
    std::unique_ptr<Statement> parse_statement();
    std::unique_ptr<Expression> parse_expression();

private:
    Tokenizer m_tokenizer;
    std::shared_ptr<Token> m_peek_token;

private:
    std::shared_ptr<Token> peek_token();
    std::shared_ptr<Token> next_token();
    std::shared_ptr<Token> consum_token(TokenKind kind);

    std::unique_ptr<LetStatement> parse_let_statement();
    std::unique_ptr<IfStatement> parse_if_statement();
    std::unique_ptr<ForStatement> parse_for_statement();
    std::unique_ptr<BlockStatement> parse_block_statement();
    std::unique_ptr<ReturnStatement> parse_return_statement();
    std::unique_ptr<BreakStatement> parse_break_statement();
    std::unique_ptr<ContinueStatement> parse_continue_statement();
    std::unique_ptr<FnStatement> parse_fn_statement();

    std::unique_ptr<Expression> parse_expression_precedence(Precedence precedence);
    std::unique_ptr<Expression> parse_prefix_expression();
    std::unique_ptr<Expression> parse_postfix_expression(
        Token& token, std::unique_ptr<Expression> expr);
    std::unique_ptr<Expression> parse_infix_expression(
        std::unique_ptr<Expression> expr, Precedence precedence);
    std::unique_ptr<Expression> parse_primary();
    std::string parse_identifier();

    template <typename T>
    std::vector<T> parse_list(TokenKind end, TokenKind separator, std::function<T()> parse)
    {
        std::vector<T> list;

        auto peek = peek_token();
        while (true) {
            peek = peek_token();
            if (peek->kind == end) {
                return list;
            }

            list.push_back(parse());

            peek = peek_token();
            if (peek->kind == end) {
                return list;
            }

            if (peek->kind != separator) {
                throw std::runtime_error(
                    std::format("Expected '{0}' or '{1}'", int(separator), int(end)));
            }

            next_token();
        }

        throw std::runtime_error("Unterminated list");
    }
};

Precedence get_precedence(TokenKind kind);

Operator binary_operator(TokenKind kind);