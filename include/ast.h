#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

enum class Operator {
    Invalid,
    Add, // +
    Subtract, // -
    Multiply, // *
    Divide, // /
    Modulo, // %
    Power, // ^
    Equals, // ==
    NotEquals, // !=
    LessThan, // <
    LessThanOrEqual, // <=
    GreaterThan, // >
    GreaterThanOrEqual, // >=
    LogicAnd, // &&
    LogicOr, // ||
    Not, // !
    Assign, // =
    Access, // .
    Increase, // ++
    Decrease, // --
    Call, // ()
};

std::string operator_str(Operator op);

enum class Precedence {
    Lowest,
    Assign, // =
    LogicOr, // ||
    LogicAnd, // &&
    Equality, // ==
    Comparison, // > or <
    Term, // +
    Factor, // *
    Prefix, // -X or !X
    Postfix, // ?, ++, --
    Call, // myFunction(X)
    Index, // array[index]
    Access, // object.property
    Primary,
};

class ASTNode {
public:
    enum class Kind {
        Program,
        FnStmt,
        EmptyStmt,
        BlockStmt,
        LetStmt,
        IfStmt,
        ForStmt,
        ReturnStmt,
        BreakStmt,
        ContinueStmt,
        ExprStmt,
        BinaryExpr,
        PrefixExpr,
        PostfixExpr,
        VariableExpr,
        EnvVariableExpr,
        LiteralExpr,
        IndexExpr,
        CallExpr,
        AccessExpr,
        ArrayExpr,
    };

    virtual ~ASTNode() = default;

    virtual Kind kind() const = 0;

    virtual bool equals(const ASTNode& other) const = 0;

    bool operator==(const ASTNode& other) const
    {
        if (other.kind() != kind())
            return false;
        return equals(other);
    }
};

class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(
        Operator op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : m_operator(std::move(op))
        , m_left(std::move(left))
        , m_right(std::move(right))
    {
    }

    Kind kind() const override { return Kind::BinaryExpr; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::BinaryExpr)
            return false;
        auto& other_binary = static_cast<const BinaryExpression&>(other);
        return m_operator == other_binary.m_operator && *m_left == *other_binary.m_left
            && *m_right == *other_binary.m_right;
    }

    Operator op() const { return m_operator; }
    Expression& left() const { return *m_left; }
    Expression& right() const { return *m_right; }

private:
    Operator m_operator;
    std::unique_ptr<Expression> m_left;
    std::unique_ptr<Expression> m_right;
};

class PrefixExpression : public Expression {
public:
    PrefixExpression(Operator op, std::unique_ptr<Expression> expr)
        : m_operator(op)
        , m_expr(std::move(expr))
    {
    }

    Kind kind() const override { return Kind::PrefixExpr; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::PrefixExpr)
            return false;
        auto& other_prefix = static_cast<const PrefixExpression&>(other);
        return m_operator == other_prefix.m_operator && *m_expr == *other_prefix.m_expr;
    }

    Operator op() const { return m_operator; }
    Expression& expr() const { return *m_expr; }

private:
    Operator m_operator;
    std::unique_ptr<Expression> m_expr;
};

class PostfixExpression : public Expression {
public:
    PostfixExpression(Operator op, std::unique_ptr<Expression> expr)
        : m_operator(op)
        , m_expr(std::move(expr))
    {
    }

    Kind kind() const override { return Kind::PostfixExpr; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::PostfixExpr)
            return false;
        auto& other_postfix = static_cast<const PostfixExpression&>(other);
        return m_operator == other_postfix.m_operator && *m_expr == *other_postfix.m_expr;
    }

    Operator op() const { return m_operator; }
    Expression& expr() const { return *m_expr; }

private:
    Operator m_operator;
    std::unique_ptr<Expression> m_expr;
};

enum class LiteralKind {
    Null,
    Boolean,
    Integer,
    Float,
    String,
};

class LiteralExpression : public Expression {
public:
    virtual ~LiteralExpression() = default;
    virtual LiteralKind literal_kind() const = 0;

    Kind kind() const override { return Kind::LiteralExpr; }
};

class NullLiteral : public LiteralExpression {
public:
    NullLiteral() { }
    //
    LiteralKind literal_kind() const override { return LiteralKind::Null; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::LiteralExpr)
            return false;
        auto& other_literal = static_cast<const LiteralExpression&>(other);
        return other_literal.literal_kind() == LiteralKind::Null;
    }

    void* value() { return nullptr; }
};

class BooleanLiteral : public LiteralExpression {
public:
    BooleanLiteral(bool value)
        : m_value(value)
    {
    }
    LiteralKind literal_kind() const override { return LiteralKind::Boolean; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::LiteralExpr)
            return false;
        auto& other_literal = static_cast<const BooleanLiteral&>(other);
        return other_literal.literal_kind() == LiteralKind::Boolean
            && other_literal.m_value == this->m_value;
    }

    bool value() { return m_value; }

private:
    bool m_value;
};

class IntegerLiteral : public LiteralExpression {
public:
    explicit IntegerLiteral(int64_t value)
        : m_value(value)
    {
    }

    LiteralKind literal_kind() const override { return LiteralKind::Integer; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::LiteralExpr)
            return false;
        auto& other_literal = static_cast<const IntegerLiteral&>(other);
        return other_literal.literal_kind() == LiteralKind::Integer
            && other_literal.m_value == this->m_value;
    }

    int64_t value() { return m_value; }

private:
    int64_t m_value;
};

class FloatLiteral : public LiteralExpression {
public:
    explicit FloatLiteral(double value)
        : m_value(value)
    {
    }

    LiteralKind literal_kind() const override { return LiteralKind::Float; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::LiteralExpr)
            return false;
        auto& other_literal = static_cast<const FloatLiteral&>(other);
        return other_literal.literal_kind() == LiteralKind::Float
            && other_literal.m_value == this->m_value;
    }

    double value() { return m_value; }

private:
    double m_value;
};

class StringLiteral : public LiteralExpression {
public:
    StringLiteral(std::string value)
        : m_value(std::move(value))
    {
    }

    LiteralKind literal_kind() const override { return LiteralKind::String; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::LiteralExpr)
            return false;
        auto& other_literal = static_cast<const StringLiteral&>(other);
        return other_literal.literal_kind() == LiteralKind::String
            && other_literal.m_value == this->m_value;
    }

    std::string& value() { return m_value; }

private:
    std::string m_value;
};

class VariableExpression : public Expression {
public:
    VariableExpression(std::string name)
        : m_name(std::move(name))
    {
    }

    Kind kind() const override { return Kind::VariableExpr; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::VariableExpr)
            return false;
        auto& other_variable = static_cast<const VariableExpression&>(other);
        return m_name == other_variable.m_name;
    }

    std::string& name() { return m_name; }

private:
    std::string m_name;
};

class EnvVariableExpression : public Expression {
public:
    EnvVariableExpression(std::string name)
        : m_name(std::move(name))
    {
    }

    EnvVariableExpression(std::string_view name)
        : m_name(std::move(name))
    {
    }

    Kind kind() const override { return Kind::EnvVariableExpr; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::EnvVariableExpr)
            return false;
        auto& other_variable = static_cast<const EnvVariableExpression&>(other);
        return m_name == other_variable.m_name;
    }

    std::string& name() { return m_name; }

private:
    std::string m_name;
};

class ArrayExpression : public Expression {
public:
    ArrayExpression(std::vector<std::unique_ptr<Expression>> elements)
        : m_elements(std::move(elements))
    {
    }

    Kind kind() const override { return Kind::ArrayExpr; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::ArrayExpr)
            return false;
        auto& other_array = static_cast<const ArrayExpression&>(other);
        if (m_elements.size() != other_array.m_elements.size())
            return false;
        for (size_t i = 0; i < m_elements.size(); i++) {
            if (!(*m_elements[i] == *other_array.m_elements[i])) {
                return false;
            }
        }

        return true;
    }

    std::vector<std::unique_ptr<Expression>>& elements() { return m_elements; }

private:
    std::vector<std::unique_ptr<Expression>> m_elements;
};

class IndexExpression : public Expression {
public:
    IndexExpression(std::unique_ptr<Expression> object, std::unique_ptr<Expression> index)
        : m_object(std::move(object))
        , m_index(std::move(index))
    {
    }

    Kind kind() const override { return Kind::IndexExpr; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::IndexExpr)
            return false;
        auto& other_index = static_cast<const IndexExpression&>(other);
        return *m_object == *other_index.m_object && *m_index == *other_index.m_index;
    }

    Expression& object() { return *m_object; }
    Expression& index() { return *m_index; }

private:
    std::unique_ptr<Expression> m_object;
    std::unique_ptr<Expression> m_index;
};

class CallExpression : public Expression {
public:
    CallExpression(
        std::unique_ptr<Expression> callee, std::vector<std::unique_ptr<Expression>> args)
        : m_callee(std::move(callee))
        , m_args(std::move(args))
    {
    }

    Kind kind() const override { return Kind::CallExpr; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::CallExpr) {
            return false;
        }

        auto& other_call = static_cast<const CallExpression&>(other);
        if (!(*m_callee == *other_call.m_callee)) {
            return false;
        }

        if (m_args.size() != other_call.m_args.size()) {
            return false;
        }

        for (size_t i = 0; i < m_args.size(); i++) {
            if (!(*m_args[i] == *other_call.m_args[i])) {
                return false;
            }
        }

        return true;
    }

    Expression& callee() { return *m_callee; }
    std::vector<std::unique_ptr<Expression>>& args() { return m_args; }

private:
    std::unique_ptr<Expression> m_callee;
    std::vector<std::unique_ptr<Expression>> m_args;
};

class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

class EmptyStatement : public Statement {
public:
    Kind kind() const override { return Kind::EmptyStmt; }

    bool equals(const ASTNode& other) const override { return other.kind() == Kind::EmptyStmt; }
};

class LetStatement : public Statement {
public:
    LetStatement(std::string name, std::unique_ptr<Expression> value)
        : m_name(std::move(name))
        , m_value(std::move(value))
    {
    }

    Kind kind() const override { return Kind::LetStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::LetStmt)
            return false;
        auto& other_let = static_cast<const LetStatement&>(other);
        return (m_value == nullptr && other_let.m_value == nullptr)
            || (m_value != nullptr && other_let.m_value != nullptr
                && *m_value == *other_let.m_value);
    }

    std::string& name() { return m_name; }
    std::unique_ptr<Expression>& value() { return m_value; }

private:
    std::string m_name;
    std::unique_ptr<Expression> m_value;
};

class ExpressionStatement : public Statement {
public:
    explicit ExpressionStatement(std::unique_ptr<Expression> expr)
        : m_expr(std::move(expr))
    {
    }

    Kind kind() const override { return Kind::ExprStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::ExprStmt)
            return false;
        auto& other_expr = static_cast<const ExpressionStatement&>(other);
        return *m_expr == *other_expr.m_expr;
    }

    Expression& expr() { return *m_expr; }

private:
    std::unique_ptr<Expression> m_expr;
};
class BlockStatement : public Statement {
public:
    explicit BlockStatement(std::vector<std::unique_ptr<Statement>> statements)
        : m_statements(std::move(statements))
    {
    }

    Kind kind() const override { return Kind::BlockStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::BlockStmt)
            return false;
        auto& other_block = static_cast<const BlockStatement&>(other);
        if (m_statements.size() != other_block.m_statements.size())
            return false;
        for (size_t i = 0; i < m_statements.size(); i++) {
            if (!(*m_statements[i] == *other_block.m_statements[i]))
                return false;
        }
        return true;
    }

    std::vector<std::unique_ptr<Statement>>& statements() { return m_statements; }

private:
    std::vector<std::unique_ptr<Statement>> m_statements;
};

class IfStatement : public Statement {
public:
    IfStatement(std::unique_ptr<Expression> condition, std::unique_ptr<BlockStatement> then_branch,
        std::unique_ptr<BlockStatement> else_branch)
        : m_condition(std::move(condition))
        , m_then_branch(std::move(then_branch))
        , m_else_branch(std::move(else_branch))
    {
    }

    Kind kind() const override { return Kind::IfStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::IfStmt)
            return false;
        auto& other_if = static_cast<const IfStatement&>(other);
        return *m_condition == *other_if.m_condition && *m_then_branch == *other_if.m_then_branch
            && (m_else_branch == nullptr && other_if.m_else_branch == nullptr
                || (m_else_branch != nullptr && other_if.m_else_branch != nullptr
                    && *m_else_branch == *other_if.m_else_branch));
    }

    Expression& condition() { return *m_condition; }
    BlockStatement& then_branch() { return *m_then_branch; }
    std::unique_ptr<BlockStatement>& else_branch() { return m_else_branch; }

private:
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<BlockStatement> m_then_branch;
    std::unique_ptr<BlockStatement> m_else_branch;
};

class ForStatement : public Statement {
public:
    ForStatement(std::unique_ptr<Statement> initializer, std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> increment, std::unique_ptr<Statement> body)
        : m_initializer(std::move(initializer))
        , m_condition(std::move(condition))
        , m_increment(std::move(increment))
        , m_body(std::move(body))
    {
    }

    Kind kind() const override { return Kind::ForStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::ForStmt)
            return false;
        auto& other_for = static_cast<const ForStatement&>(other);
        return ((m_initializer == nullptr && other_for.m_initializer == nullptr)
                   || (m_initializer != nullptr && other_for.m_initializer != nullptr
                       && *m_initializer == *other_for.m_initializer))
            && ((m_condition == nullptr && other_for.m_condition == nullptr)
                || (m_condition != nullptr && other_for.m_condition != nullptr
                    && *m_condition == *other_for.m_condition))
            && ((m_increment == nullptr && other_for.m_increment == nullptr)
                || (m_increment != nullptr && other_for.m_increment != nullptr
                    && *m_increment == *other_for.m_increment))
            && *m_body == *other_for.m_body;
    }

    std::unique_ptr<Statement>& initializer() { return m_initializer; }
    std::unique_ptr<Expression>& condition() { return m_condition; }
    std::unique_ptr<Expression>& increment() { return m_increment; }
    Statement& body() { return *m_body; };

private:
    std::unique_ptr<Statement> m_initializer;
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Expression> m_increment;
    std::unique_ptr<Statement> m_body;
};

class ReturnStatement : public Statement {
public:
    explicit ReturnStatement(std::unique_ptr<Expression> value)
        : m_value(std::move(value))
    {
    }

    Kind kind() const override { return Kind::ReturnStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::ReturnStmt) {
            return false;
        }

        auto& other_return = static_cast<const ReturnStatement&>(other);
        return (m_value == nullptr && other_return.m_value == nullptr)
            || (m_value != nullptr && other_return.m_value != nullptr
                && *m_value == *other_return.m_value);
    }

    std::unique_ptr<Expression>& value() { return m_value; }

private:
    std::unique_ptr<Expression> m_value;
};

class BreakStatement : public Statement {
public:
    BreakStatement() { }

    Kind kind() const override { return Kind::BreakStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::BreakStmt)
            return false;
        return true;
    }
};

class ContinueStatement : public Statement {
public:
    ContinueStatement() { }

    Kind kind() const override { return Kind::ContinueStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::ContinueStmt)
            return false;
        return true;
    }
};

class FnStatement : public Statement {
public:
    FnStatement(
        std::string name, std::vector<std::string> params, std::unique_ptr<BlockStatement> body)
        : m_name(std::move(name))
        , m_params(std::move(params))
        , m_body(std::move(body))
    {
    }

    Kind kind() const override { return Kind::FnStmt; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::FnStmt)
            return false;
        auto& other_fn = static_cast<const FnStatement&>(other);
        return m_name == other_fn.m_name && m_params == other_fn.m_params
            && *m_body == *other_fn.m_body;
    }

    std::string& name() { return m_name; }
    std::vector<std::string>& params() { return m_params; }
    BlockStatement& body() { return *m_body; }

private:
    std::string m_name;
    std::vector<std::string> m_params;
    std::unique_ptr<BlockStatement> m_body;
};

class Program : public ASTNode {
public:
    Program(std::vector<std::unique_ptr<Statement>> statements,
        std::unordered_map<std::string, std::shared_ptr<FnStatement>> functions)
        : m_statements(std::move(statements))
        , m_functions(functions)
    {
    }

    Program(std::unique_ptr<Expression> expression)
    {
        std::unique_ptr<ReturnStatement> stmt
            = std::make_unique<ReturnStatement>(std::move(expression));

        m_statements.push_back(std::move(stmt));
    }

    Kind kind() const override { return Kind::Program; }

    bool equals(const ASTNode& other) const override
    {
        if (other.kind() != Kind::Program)
            return false;
        auto& other_program = static_cast<const Program&>(other);
        if (m_statements.size() != other_program.m_statements.size())
            return false;
        for (size_t i = 0; i < m_statements.size(); i++) {
            if (!(*m_statements[i] == *other_program.m_statements[i]))
                return false;
        }
        return true;
    }

    std::vector<std::unique_ptr<Statement>>& statements() { return m_statements; }
    std::unordered_map<std::string, std::shared_ptr<FnStatement>>& functions()
    {
        return m_functions;
    }

private:
    std::vector<std::unique_ptr<Statement>> m_statements;
    std::unordered_map<std::string, std::shared_ptr<FnStatement>> m_functions;
};

class ASTInspector {
public:
    ASTInspector() { }

    static std::string inspect(ASTNode& node);
};
