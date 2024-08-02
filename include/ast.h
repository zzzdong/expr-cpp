#pragma once

#include <memory>
#include <string>
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
        LiteralExpr,
        IndexExpr,
        CallExpr,
        AccessExpr,
        ArrayExpr,
    };

    virtual Kind kind() const = 0;
};

class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(Operator op, std::unique_ptr<Expression> left,
        std::unique_ptr<Expression> right)
        : m_operator(std::move(op))
        , m_left(std::move(left))
        , m_right(std::move(right))
    {
    }

    Kind kind() const override { return Kind::BinaryExpr; }

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

    Operator op() const { return m_operator; }
    Expression& expr() const { return *m_expr; }

private:
    Operator m_operator;
    std::unique_ptr<Expression> m_expr;
};

enum class LiteralKind {
    Undefined,
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

class UndefinedLiteral : public LiteralExpression {
public:
    //
    LiteralKind literal_kind() const override { return LiteralKind::Undefined; }

    void* value() { return nullptr; }
};

class BooleanLiteral : public LiteralExpression {
public:
    BooleanLiteral(bool value)
        : m_value(value)
    {
    }
    // void accept(std::shared_ptr<ASTVisitor> visitor) override;
    LiteralKind literal_kind() const override { return LiteralKind::Boolean; }

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

    //
    LiteralKind literal_kind() const override { return LiteralKind::Integer; }

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

    //
    LiteralKind literal_kind() const override { return LiteralKind::Float; }

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

    //
    LiteralKind literal_kind() const override { return LiteralKind::String; }
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

    std::vector<std::unique_ptr<Expression>>& elements() { return m_elements; }

private:
    std::vector<std::unique_ptr<Expression>> m_elements;
};

class IndexExpression : public Expression {
public:
    IndexExpression(std::unique_ptr<Expression> object,
        std::unique_ptr<Expression> index)
        : m_object(std::move(object))
        , m_index(std::move(index))
    {
    }

    Kind kind() const override { return Kind::IndexExpr; }

    Expression& object() { return *m_object; }
    Expression& index() { return *m_index; }

private:
    std::unique_ptr<Expression> m_object;
    std::unique_ptr<Expression> m_index;
};

class CallExpression : public Expression {
public:
    CallExpression(std::unique_ptr<Expression> callee,
        std::vector<std::unique_ptr<Expression>> args)
        : m_callee(std::move(callee))
        , m_args(std::move(args))
    {
    }

    Kind kind() const override { return Kind::CallExpr; }

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
};

class LetStatement : public Statement {
public:
    LetStatement(std::string name, std::unique_ptr<Expression> value)
        : m_name(std::move(name))
        , m_value(std::move(value))
    {
    }

    Kind kind() const override { return Kind::LetStmt; }
    std::string& name() { return m_name; }
    std::unique_ptr<Expression>& value() { return m_value; }

private:
    std::string m_name;
    std::unique_ptr<Expression> m_value;
};

class IfStatement : public Statement {
public:
    IfStatement(std::unique_ptr<Expression> condition,
        std::unique_ptr<Statement> then_branch,
        std::unique_ptr<Statement> else_branch)
        : m_condition(std::move(condition))
        , m_then_branch(std::move(then_branch))
        , m_else_branch(std::move(else_branch))
    {
    }

    Kind kind() const override { return Kind::IfStmt; }

    Expression& condition() { return *m_condition; }
    Statement& then_branch() { return *m_then_branch; }
    std::unique_ptr<Statement>& else_branch() { return m_else_branch; }

private:
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Statement> m_then_branch;
    std::unique_ptr<Statement> m_else_branch;
};

class ForStatement : public Statement {
public:
    ForStatement(std::unique_ptr<Statement> initializer,
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> increment,
        std::unique_ptr<Statement> body)
        : m_initializer(std::move(initializer))
        , m_condition(std::move(condition))
        , m_increment(std::move(increment))
        , m_body(std::move(body))
    {
    }

    Kind kind() const override { return Kind::ForStmt; }

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

class BlockStatement : public Statement {
public:
    explicit BlockStatement(std::vector<std::unique_ptr<Statement>> statements)
        : m_statements(std::move(statements))
    {
    }

    Kind kind() const override { return Kind::BlockStmt; }

    std::vector<std::unique_ptr<Statement>>& statements() { return m_statements; }

private:
    std::vector<std::unique_ptr<Statement>> m_statements;
};

class ExpressionStatement : public Statement {
public:
    explicit ExpressionStatement(std::unique_ptr<Expression> expr)
        : m_expr(std::move(expr))
    {
    }

    Kind kind() const override { return Kind::ExprStmt; }

    Expression& expr() { return *m_expr; }

private:
    std::unique_ptr<Expression> m_expr;
};

class ReturnStatement : public Statement {
public:
    explicit ReturnStatement(std::unique_ptr<Expression> value)
        : m_value(std::move(value))
    {
    }

    Kind kind() const override { return Kind::ReturnStmt; }

    std::unique_ptr<Expression>& value() { return m_value; }

private:
    std::unique_ptr<Expression> m_value;
};

class BreakStatement : public Statement {
public:
    BreakStatement() { }

    Kind kind() const override { return Kind::BreakStmt; }
};

class ContinueStatement : public Statement {
public:
    ContinueStatement() { }

    Kind kind() const override { return Kind::ContinueStmt; }
};

class FnStatement : public Statement {
public:
    FnStatement(std::string name, std::vector<std::string> params,
        std::unique_ptr<Statement> body)
        : m_name(std::move(name))
        , m_params(std::move(params))
        , m_body(std::move(body))
    {
    }

    Kind kind() const override { return Kind::FnStmt; }

    std::string& name() { return m_name; }
    std::vector<std::string>& params() { return m_params; }
    Statement& body() { return *m_body; }

private:
    std::string m_name;
    std::vector<std::string> m_params;
    std::unique_ptr<Statement> m_body;
};

class Program : public ASTNode {
public:
    Program(
        std::vector<std::unique_ptr<Statement>> statements,
        std::unordered_map<std::string, std::shared_ptr<FnStatement>> functions)
        : m_statements(std::move(statements))
        , m_functions(functions)
    {
    }

    Program(std::unique_ptr<Expression> expression)
    {
        std::unique_ptr<ReturnStatement> stmt = std::make_unique<ReturnStatement>(std::move(expression));

        m_statements.push_back(std::move(stmt));
    }

    Kind kind() const override { return Kind::Program; }

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
