#include "parser.h"
#include "ast.h"
#include "tokenizer.h"
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

Parser::Parser(std::string_view input)
    : m_tokenizer(std::move(Tokenizer(input)))
{

    auto peek_token = m_tokenizer.next();
    if (peek_token.kind == TokenKind::Eof) {
        m_peek_token = nullptr;
    } else {
        m_peek_token = std::make_shared<Token>(peek_token);
    }
}

std::unique_ptr<Program> Parser::parse()
{
    std::vector<std::unique_ptr<Statement>> statements;
    std::unordered_map<std::string, std::shared_ptr<FnStatement>> functions;

    auto peek = peek_token();
    while (peek != nullptr) {
        auto stmt = parse_statement();
        if (stmt->kind() == ASTNode::Kind::FnStmt) {
            std::shared_ptr<Statement> fn_stmt = std::move(stmt);
            std::shared_ptr<FnStatement> fn = std::dynamic_pointer_cast<FnStatement>(fn_stmt);
            functions.insert(std::make_pair(fn->name(), fn));
        } else {
            statements.push_back(std::move(stmt));
        }

        peek = peek_token();
    }

    return std::make_unique<Program>(std::move(statements), std::move(functions));
}

std::unique_ptr<Statement> Parser::parse_statement()
{
    auto peek = peek_token();

    switch (m_peek_token->kind) {
    case TokenKind::Fn:
        return parse_fn_statement();
    case TokenKind::Let:
        return parse_let_statement();
    case TokenKind::If:
        return parse_if_statement();
    case TokenKind::For:
        return parse_for_statement();
    case TokenKind::Return:
        return parse_return_statement();
    case TokenKind::Break:
        return parse_break_statement();
    case TokenKind::Continue:
        return parse_continue_statement();
    case TokenKind::LBrace:
        return parse_block_statement();
    case TokenKind::Semicolon:
        return std::make_unique<EmptyStatement>();
    default: {
        auto expr = parse_expression();
        consum_token(TokenKind::Semicolon);
        return std::make_unique<ExpressionStatement>(std::move(expr));
    }
    }
}

std::unique_ptr<Statement> Parser::parse_let_statement()
{
    consum_token(TokenKind::Let);
    auto next = next_token();
    if (next == nullptr || next->kind != TokenKind::Identifier) {
        throw std::runtime_error(std::format(
            "Expected identifier for let statement but got token(kind({})`{}`)",
            int(next->kind), next->text));
    }
    auto name = std::string(next->text);

    std::unique_ptr<Expression> expr = nullptr;
    // let ident [= expr] ;
    auto peek = peek_token();
    if (peek == nullptr) {
        throw std::runtime_error(
            std::format("Incomplete let statement, expect `;` or `=`"));
    }
    switch (peek->kind) {
    case TokenKind::Assign: {
        consum_token(TokenKind::Assign);
        expr = parse_expression();
        break;
    }
    case TokenKind::Semicolon: {
        expr = nullptr;
        break;
    }
    default:
        throw std::runtime_error(std::format(
            "Expected expression for let statement but got token(kind({})`{}`)",
            int(next->kind), next->text));
    }

    consum_token(TokenKind::Semicolon);

    return std::make_unique<LetStatement>(name, std::move(expr));
}

std::unique_ptr<Statement> Parser::parse_if_statement()
{
    consum_token(TokenKind::If);
    consum_token(TokenKind::LParen);
    auto condition = parse_expression();
    consum_token(TokenKind::RParen);
    auto then_branch = parse_block_statement();
    std::unique_ptr<Statement> else_branch = nullptr;
    auto peek = peek_token();
    if (peek != nullptr && peek->kind == TokenKind::Else) {
        consum_token(TokenKind::Else);
        else_branch = parse_statement();
    }

    return std::make_unique<IfStatement>(
        std::move(condition), std::move(then_branch), std::move(else_branch));
}

std::unique_ptr<Statement> Parser::parse_for_statement()
{
    std::unique_ptr<Statement> initializer = nullptr;
    std::unique_ptr<Expression> condition = nullptr;
    std::unique_ptr<Expression> increment = nullptr;
    std::unique_ptr<Statement> body = nullptr;

    consum_token(TokenKind::For);

    consum_token(TokenKind::LParen);

    auto peek = peek_token();
    if (peek != nullptr && peek->kind == TokenKind::Semicolon) {
        consum_token(TokenKind::Semicolon);
    } else {
        initializer = parse_statement();
    }

    peek = peek_token();
    if (peek != nullptr && peek->kind == TokenKind::Semicolon) {
    } else {
        condition = parse_expression();
    }
    consum_token(TokenKind::Semicolon);

    peek = peek_token();
    if (peek != nullptr && peek->kind == TokenKind::RParen) {
    } else {
        increment = parse_expression();
    }
    consum_token(TokenKind::RParen);

    body = parse_statement();

    return std::make_unique<ForStatement>(std::move(initializer),
        std::move(condition),
        std::move(increment), std::move(body));
}

std::unique_ptr<Statement> Parser::parse_block_statement()
{
    consum_token(TokenKind::LBrace);

    std::vector<std::unique_ptr<Statement>> statements;
    auto peek = peek_token();
    while (peek != nullptr && peek->kind != TokenKind::RBrace) {
        statements.push_back(parse_statement());
        peek = peek_token();
    }

    consum_token(TokenKind::RBrace);

    return std::make_unique<BlockStatement>(std::move(statements));
}

std::unique_ptr<Statement> Parser::parse_return_statement()
{
    consum_token(TokenKind::Return);
    auto peek = peek_token();
    if (peek == nullptr) {
        throw std::runtime_error("Incomplete resturn statement");
    }
    if (peek->kind == TokenKind::Semicolon) {
        consum_token(TokenKind::Semicolon);
        return std::make_unique<ReturnStatement>(nullptr);
    }

    auto expr = parse_expression();
    consum_token(TokenKind::Semicolon);

    return std::make_unique<ReturnStatement>(std::move(expr));
}

std::unique_ptr<Statement> Parser::parse_break_statement()
{
    consum_token(TokenKind::Break);
    consum_token(TokenKind::Semicolon);

    return std::make_unique<BreakStatement>();
}

std::unique_ptr<Statement> Parser::parse_continue_statement()
{
    consum_token(TokenKind::Continue);
    consum_token(TokenKind::Semicolon);

    return std::make_unique<ContinueStatement>();
}

std::unique_ptr<Statement> Parser::parse_fn_statement()
{
    consum_token(TokenKind::Fn);

    auto name = parse_identifier();

    consum_token(TokenKind::LParen);
    auto params = parse_list<std::string>(TokenKind::RParen, TokenKind::Comma, [this]() { return parse_identifier(); });
    consum_token(TokenKind::RParen);

    auto body = parse_block_statement();

    return std::make_unique<FnStatement>(name, params, std::move(body));
}

std::unique_ptr<Expression> Parser::parse_expression()
{
    return parse_expression_precedence(Precedence::Lowest);
}

std::shared_ptr<Token> Parser::peek_token() { return m_peek_token; }

std::shared_ptr<Token> Parser::next_token()
{
    auto curr = m_peek_token;

    auto peek = m_tokenizer.next();
    if (peek.kind == TokenKind::Eof) {
        m_peek_token = nullptr;
    } else {
        m_peek_token = std::make_shared<Token>(peek);
    }

    return curr;
}

std::shared_ptr<Token> Parser::consum_token(TokenKind kind)
{
    auto next = next_token();
    if (next == nullptr || next->kind != kind) {
        throw std::runtime_error(
            std::format("Expected token of kind({}), but got token(kind({})`{}`)",
                int(kind), int(next->kind), next->text));
    }

    return next;
}

std::unique_ptr<Expression>
Parser::parse_expression_precedence(Precedence precedence)
{
    auto expr = parse_prefix_expression();

    auto peek = peek_token();
    while (peek != nullptr) {
        auto next_precedence = get_precedence(peek->kind);
        if (next_precedence <= precedence) {
            return expr;
        }

        expr = parse_infix_expression(std::move(expr), next_precedence);
        peek = peek_token();
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parse_prefix_expression()
{
    auto peek = peek_token();
    if (peek == nullptr) {
        throw std::runtime_error(
            "Expected token for prefix expression but got EOF");
    }

    switch (peek->kind) {
    case TokenKind::Bang: {
        next_token();
        auto expr = parse_expression();
        if (expr == nullptr) {
            throw std::runtime_error(
                "Expected expression for prefix expression but got null");
        }
        return std::make_unique<PrefixExpression>(Operator::Not, std::move(expr));
    }
    case TokenKind::Minus: {
        next_token();
        auto expr = parse_expression();
        if (expr == nullptr) {
            throw std::runtime_error(
                "Expected expression for prefix expression but got null");
        }
        return std::make_unique<PrefixExpression>(Operator::Subtract,
            std::move(expr));
    }
    default:
        return parse_primary();
    }
}

std::unique_ptr<Expression>
Parser::parse_postfix_expression(Token& peek,
    std::unique_ptr<Expression> expr)
{
    switch (peek.kind) {
    case TokenKind::LBracket: {
        consum_token(TokenKind::LBracket);
        auto index = parse_expression();
        if (index == nullptr) {
            throw std::runtime_error("Expected expression for index but got null");
        }
        consum_token(TokenKind::RBracket);
        return std::make_unique<IndexExpression>(std::move(expr), std::move(index));
    }
    case TokenKind::LParen: {

        consum_token(TokenKind::LParen);
        auto args = parse_list<std::unique_ptr<Expression>>(
            TokenKind::RParen, TokenKind::Comma,
            [this]() { return this->parse_expression(); });
        consum_token(TokenKind::RParen);

        return std::make_unique<CallExpression>(std::move(expr), std::move(args));
    }
    case TokenKind::Increase: {
        consum_token(TokenKind::Increase);
        return std::make_unique<PostfixExpression>(Operator::Increase,
            std::move(expr));
    }
    case TokenKind::Decrease: {
        consum_token(TokenKind::Decrease);
        return std::make_unique<PostfixExpression>(Operator::Decrease,
            std::move(expr));
    }
    default:
        return std::move(expr);
    }
}

std::unique_ptr<Expression>
Parser::parse_infix_expression(std::unique_ptr<Expression> expr,
    Precedence precedence)
{
    auto peek = peek_token();
    if (peek == nullptr) {
        return std::move(expr);
    }

    switch (peek->kind) {
    case TokenKind::LParen:
    case TokenKind::LBracket:
    case TokenKind::Increase:
    case TokenKind::Decrease:
        return parse_postfix_expression(*peek, std::move(expr));
    default: {
        auto token = next_token();
        auto op = binary_operator(token->kind);
        if (op == Operator::Invalid) {
            throw std::runtime_error(
                std::format("Invalid binary operator of `{0}`", token->text));
        }
        auto rhs = parse_expression_precedence(precedence);
        if (rhs == nullptr) {
            throw std::runtime_error("Invalid binary rhs");
        }
        return std::make_unique<BinaryExpression>(op, std::move(expr),
            std::move(rhs));
    }
    }
}

std::unique_ptr<Expression> Parser::parse_primary()
{
    auto peek = peek_token();
    if (peek == nullptr) {
        throw std::runtime_error("Unexpected EOF");
    }

    switch (peek->kind) {
    case TokenKind::True: {
        auto token = next_token();
        return std::make_unique<BooleanLiteral>(true);
    }
    case TokenKind::False: {
        auto token = next_token();
        return std::make_unique<BooleanLiteral>(false);
    }
    case TokenKind::Undefined: {
        auto token = next_token();
        return std::make_unique<UndefinedLiteral>();
    }
    case TokenKind::Integer: {
        auto token = next_token();
        auto i = std::stoll(std::string(token->text));
        return std::make_unique<IntegerLiteral>(i);
    }
    case TokenKind::Float: {
        auto token = next_token();
        auto f = std::stod(std::string(token->text));
        return std::make_unique<FloatLiteral>(f);
    }
    case TokenKind::String: {
        auto token = next_token();
        std::string result;
        auto view = token->text;
        view.remove_prefix(1);
        view.remove_suffix(1);

        for (auto c = view.begin(); c != view.end(); ++c) {
            if (*c == '\\') {
                c++;
                switch (*c) {
                case 'n':
                    result.push_back('\n');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case '\\':
                    result.push_back('\\');
                    break;
                default:
                    result.push_back('\\');
                    result.push_back(*c);
                }
            }
            result.push_back(*c);
        }
        return std::make_unique<StringLiteral>(result);
    }
    case TokenKind::Identifier: {
        auto token = next_token();
        return std::make_unique<VariableExpression>(std::string(token->text));
    }

    case TokenKind::LParen: {
        // grouped expression
        consum_token(TokenKind::LParen);
        auto expr = parse_expression();
        consum_token(TokenKind::RParen);
        return expr;
    }

    case TokenKind::LBracket: {
        // array expression
        consum_token(TokenKind::LBracket);
        std::vector<std::unique_ptr<Expression>> elements = parse_list<std::unique_ptr<Expression>>(
            TokenKind::RBracket, TokenKind::Comma,
            [this]() { return this->parse_expression(); });
        consum_token(TokenKind::RBracket);

        return std::make_unique<ArrayExpression>(std::move(elements));
    }

    default:
        throw std::runtime_error(std::format(
            "unexpected token(`{}`) for primary expression", peek->text));
    }
}

std::string Parser::parse_identifier()
{
    auto ident = consum_token(TokenKind::Identifier);
    return std::string(ident->text);
}

Precedence get_precedence(TokenKind kind)
{
    switch (kind) {
    case TokenKind::Assign:
        return Precedence::Assign;
    case TokenKind::LogicOr:
        return Precedence::LogicOr;
    case TokenKind::LogicAnd:
        return Precedence::LogicAnd;
    case TokenKind::Equals:
    case TokenKind::NotEquals:
        return Precedence::Equality;
    case TokenKind::GreaterThan:
    case TokenKind::GreaterThanOrEqual:
    case TokenKind::LessThan:
    case TokenKind::LessThanOrEqual:
        return Precedence::Comparison;
    case TokenKind::Plus:
    case TokenKind::Minus:
        return Precedence::Term;
    case TokenKind::Star:
    case TokenKind::Slash:
    case TokenKind::Percent:
        return Precedence::Factor;
    case TokenKind::Increase:
    case TokenKind::Decrease:
        return Precedence::Postfix;
    case TokenKind::LParen:
        return Precedence::Call;
    case TokenKind::LBracket:
        return Precedence::Index;
    case TokenKind::Dot:
        return Precedence::Access;
    case TokenKind::Undefined:
    case TokenKind::True:
    case TokenKind::False:
    case TokenKind::Integer:
    case TokenKind::Float:
    case TokenKind::String:
    case TokenKind::Identifier:
        return Precedence::Primary;
    default:
        return Precedence::Lowest;
    }
}

const static std::unordered_map<TokenKind, Operator> BINARY_OPERATORS = {
    { TokenKind::Plus, Operator::Add },
    { TokenKind::Minus, Operator::Subtract },
    { TokenKind::Star, Operator::Multiply },
    { TokenKind::Slash, Operator::Divide },
    { TokenKind::Percent, Operator::Modulo },
    { TokenKind::Equals, Operator::Equals },
    { TokenKind::NotEquals, Operator::NotEquals },
    { TokenKind::GreaterThan, Operator::GreaterThan },
    { TokenKind::GreaterThanOrEqual, Operator::GreaterThanOrEqual },
    { TokenKind::LessThan, Operator::LessThan },
    { TokenKind::LessThanOrEqual, Operator::LessThanOrEqual },
    { TokenKind::LogicAnd, Operator::LogicAnd },
    { TokenKind::LogicOr, Operator::LogicOr },
    { TokenKind::Assign, Operator::Assign },
    { TokenKind::Dot, Operator::Access },
};

Operator binary_operator(TokenKind kind)
{
    auto found = BINARY_OPERATORS.find(kind);
    if (found != BINARY_OPERATORS.end()) {
        return found->second;
    }

    return Operator::Invalid;
}