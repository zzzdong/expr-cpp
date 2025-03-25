#include "eval.h"
#include "ast.h"

#include <format>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>

std::string Stack::inspect()
{

    std::stringstream ss;

    ss << "Stack: [" << std::endl;
    for (auto it = m_frames.rbegin(); it != m_frames.rend(); ++it) {
        ss << "  (";

        auto frame = it;
        for (auto& [name, value] : frame->locals) {
            ss << name << ": " << value.inspect() << ",";
        }

        ss << ")," << std::endl;
    }

    ss << "]" << std::endl;

    return ss.str();
}

Value Evaluator::eval()
{
    for (auto& stmt : m_context.statements()) {
        auto control_flow = eval(*stmt);
        if (control_flow.kind() == ControlFlow::Kind::Return) {
            return control_flow.value();
        }
    }

    return Value(std::make_shared<Null>());
}

ControlFlow Evaluator::eval(Statement& statement)
{
    switch (statement.kind()) {
    case ASTNode::Kind::LetStmt:
        return eval(dynamic_cast<LetStatement&>(statement));
    case ASTNode::Kind::IfStmt:
        return eval(dynamic_cast<IfStatement&>(statement));
    case ASTNode::Kind::ForStmt:
        return eval(dynamic_cast<ForStatement&>(statement));
    case ASTNode::Kind::BlockStmt:
        return eval(dynamic_cast<BlockStatement&>(statement));
    case ASTNode::Kind::BreakStmt:
        return ControlFlow(ControlFlow::Kind::Break);
    case ASTNode::Kind::ContinueStmt:
        return ControlFlow(ControlFlow::Kind::Continue);
    case ASTNode::Kind::EmptyStmt:
        return ControlFlow(ControlFlow::Kind::None);
    case ASTNode::Kind::ReturnStmt:
        return eval(dynamic_cast<ReturnStatement&>(statement));
    case ASTNode::Kind::ExprStmt:
        return eval(dynamic_cast<ExpressionStatement&>(statement));
    default:
        throw std::runtime_error(
            std::format("unimplemented eval for {}", ASTInspector::inspect(statement)));
    }
}

ControlFlow Evaluator::eval(ReturnStatement& statement)
{
    auto value = statement.value() ? eval(*statement.value()) : Value(std::make_shared<Null>());
    return ControlFlow(ControlFlow::Kind::Return, value);
}

ControlFlow Evaluator::eval(LetStatement& statement)
{
    if (statement.value() != nullptr) {
        this->m_context.insert_variable(statement.name(), eval(*statement.value()));
    } else {
        this->m_context.insert_variable(statement.name(), Value(std::make_shared<Null>()));
    }

    return ControlFlow(ControlFlow::Kind::None);
}

ControlFlow Evaluator::eval(IfStatement& statement)
{
    auto condition = eval(statement.condition());
    if (condition.kind() != ValueKind::Boolean) {
        throw InvalidOperate(Operator::Equals, ValueKind::Boolean, condition.kind());
    }

    if (condition.as_boolean()) {
        return eval(statement.then_branch());
    } else if (statement.else_branch() != nullptr) {
        return eval(*statement.else_branch());
    }

    return ControlFlow(ControlFlow::Kind::None);
}

ControlFlow Evaluator::eval(ForStatement& statement)
{
    if (statement.initializer() != nullptr) {
        eval(*statement.initializer());
    }
    while (true) {
        if (statement.condition() != nullptr) {
            auto condition = eval(*statement.condition());
            if (Boolean(true).compare(condition) != Comparison::Equal) {
                return ControlFlow(ControlFlow::Kind::None);
            }
        }

        auto ctrl = eval(statement.body());

        switch (ctrl.kind()) {
        case ControlFlow::Kind::Break:
            return ControlFlow(ControlFlow::Kind::None);
        case ControlFlow::Kind::Return:
            return ctrl;
        default:
            break;
        }

        if (statement.increment() != nullptr) {
            eval(*statement.increment());
        }
    }

    return ControlFlow(ControlFlow::Kind::None);
}

ControlFlow Evaluator::eval(BlockStatement& statement)
{
    m_context.enter_scope();
    for (auto& stmt : statement.statements()) {
        auto control_flow = eval(*stmt);
        if (control_flow.kind() != ControlFlow::Kind::None) {
            m_context.level_scope();
            return control_flow;
        }
    }
    m_context.level_scope();

    return ControlFlow(ControlFlow::Kind::None);
}

ControlFlow Evaluator::eval(ExpressionStatement& statement)
{
    eval(statement.expr());
    return ControlFlow(ControlFlow::Kind::None);
}

Value Evaluator::eval(Expression& expression)
{
    switch (expression.kind()) {
    case ASTNode::Kind::LiteralExpr:
        return eval(dynamic_cast<LiteralExpression&>(expression));
    case ASTNode::Kind::VariableExpr:
        return eval(dynamic_cast<VariableExpression&>(expression));
    case ASTNode::Kind::EnvVariableExpr:
        return eval(dynamic_cast<EnvVariableExpression&>(expression));
    case ASTNode::Kind::BinaryExpr:
        return eval(dynamic_cast<BinaryExpression&>(expression));
    case ASTNode::Kind::PrefixExpr:
        return eval(dynamic_cast<PrefixExpression&>(expression));
    case ASTNode::Kind::PostfixExpr:
        return eval(dynamic_cast<PostfixExpression&>(expression));
    case ASTNode::Kind::CallExpr:
        return eval(dynamic_cast<CallExpression&>(expression));
    default:
        throw std::runtime_error(
            std::format("unimplemented for eval: {}", ASTInspector::inspect(expression)));
    }
}

Value Evaluator::eval(LiteralExpression& literal)
{
    switch (literal.literal_kind()) {
    case LiteralKind::Null: {
        return Value();
    }
    case LiteralKind::Boolean: {
        BooleanLiteral& bool_lit = dynamic_cast<BooleanLiteral&>(literal);
        return Value(bool_lit.value());
    }
    case LiteralKind::Integer: {
        IntegerLiteral& int_lit = dynamic_cast<IntegerLiteral&>(literal);
        return Value(int_lit.value());
    }
    case LiteralKind::Float: {
        FloatLiteral& float_lit = dynamic_cast<FloatLiteral&>(literal);
        return Value(float_lit.value());
    }
    case LiteralKind::String: {
        StringLiteral& string_lit = dynamic_cast<StringLiteral&>(literal);
        auto v = Value(string_lit.value());
        return v;
    }
    default:
        throw std::runtime_error("Invalid literal kind");
    }
}

Value Evaluator::eval(VariableExpression& expression)
{
    return m_context.get_variable(expression.name());
}

Value Evaluator::eval(EnvVariableExpression& expression)
{
    return m_context.get_env_variable(expression.name());
}

Value Evaluator::eval(BinaryExpression& expression)
{
    auto lhs = eval(expression.left());
    auto rhs = eval(expression.right());

    switch (expression.op()) {
    case Operator::Add:
        return lhs.obj()->add(rhs);
    case Operator::Subtract:
        return lhs.obj()->sub(rhs);
    case Operator::Multiply:
        return lhs.obj()->mul(rhs);
    case Operator::Divide:
        return lhs.obj()->div(rhs);
    case Operator::Modulo:
        return lhs.obj()->mod(rhs);
    case Operator::Equals: {
        auto result = lhs.obj()->compare(rhs);
        return Value(result == Comparison::Equal);
    }
    case Operator::NotEquals: {
        auto result = lhs.obj()->compare(rhs);
        return Value(result != Comparison::Equal);
    }
    case Operator::GreaterThan: {
        auto result = lhs.obj()->compare(rhs);
        return Value(result == Comparison::Greater);
    }
    case Operator::GreaterThanOrEqual: {
        auto result = lhs.obj()->compare(rhs);
        return Value(result == Comparison::Equal || result == Comparison::Greater);
    }
    case Operator::LessThan: {
        auto result = lhs.obj()->compare(rhs);
        return Value(result == Comparison::Less);
    }
    case Operator::LessThanOrEqual: {
        auto result = lhs.obj()->compare(rhs);
        return Value(result == Comparison::Equal || result == Comparison::Less);
    }
    case Operator::LogicAnd: {
        auto result = lhs.obj()->logic_and(rhs);
        return Value(result);
    }
    case Operator::LogicOr: {
        auto result = lhs.obj()->logic_or(rhs);
        return Value(result);
    }
    case Operator::Assign: {
        switch (expression.left().kind()) {
        case ASTNode::Kind::VariableExpr: {
            auto variable = dynamic_cast<VariableExpression&>(expression.left());
            this->m_context.set_variable(variable.name(), rhs);
            return rhs;
        }
        default:
            throw InvalidOperate(std::format(
                "Invalid assignment target, {}", ASTInspector::inspect(expression.left())));
        }
    }
    default:
        throw InvalidOperate(expression.op(), lhs.kind(), rhs.kind());
    }
}

Value Evaluator::eval(PrefixExpression& expression)
{
    auto value = eval(expression.expr());

    switch (expression.op()) {
    case Operator::Subtract: {
        switch (value.kind()) {
        case ValueKind::Integer: {
            return Value(-(value.as_integer()));
        }
        case ValueKind::Float: {
            return Value(-(value.as_float()));
        }
        default:
            throw InvalidOperate(expression.op(), value.kind());
        }
    }
    case Operator::Not: {
        switch (value.kind()) {
        case ValueKind::Boolean: {
            return Value(!(value.as_boolean()));
        }
        default:
            throw InvalidOperate(expression.op(), value.kind());
        }
    }
    default:
        throw InvalidOperate(expression.op(), value.kind());
    }
}

Value Evaluator::eval(PostfixExpression& expression)
{
    auto value = eval(expression.expr());

    switch (expression.op()) {
    case Operator::Increase: {
        switch (value.kind()) {
        case ValueKind::Integer: {
            auto variable = dynamic_cast<VariableExpression&>(expression.expr());
            value.as_integer() = value.as_integer() + 1;
            return value;
        }
        default:
            throw InvalidOperate(expression.op(), value.kind());
        }
    }
    case Operator::Decrease: {
        switch (value.kind()) {
        case ValueKind::Integer: {
            auto variable = dynamic_cast<VariableExpression&>(expression.expr());
            value.as_integer() = value.as_integer() - 1;
            return value;
        }
        default:
            throw InvalidOperate(expression.op(), value.kind());
        }
    }
    default:
        throw InvalidOperate(expression.op(), value.kind());
    }
}

Value Evaluator::eval(CallExpression& expression)
{
    auto callee = eval(expression.callee());
    switch (callee.kind()) {
    case ValueKind::UserFunction: {
        auto fn = callee.as_user_function();
        std::vector<Value> args;
        for (auto& arg : expression.args()) {
            args.push_back(eval(*arg));
        }

        auto fn_stmt = m_context.get_function(fn.name());

        return eval_call(*fn_stmt, args);
    }
    case ValueKind::NativeFunction: {

        std::vector<Value> args;
        for (auto& arg : expression.args()) {
            args.push_back(eval(*arg));
        }

        return eval_call(*callee.obj(), args[0]);
    }

    default:
        throw InvalidOperate(std::format("Invalid call for {}", ASTInspector::inspect(expression)));
    }
}

Value Evaluator::eval_call(FnStatement& fn, std::vector<Value>& args)
{
    if (fn.params().size() != args.size()) {
        throw InvalidOperate(std::format("Invalid call for {}", ASTInspector::inspect(fn)));
    }

    m_context.enter_scope();

    // std::cout << m_context.stack().inspect() << std::endl;

    for (size_t i = 0; i < fn.params().size(); ++i) {
        m_context.insert_variable(fn.params()[i], args[i]);
    }

    auto ret = eval(fn.body());

    m_context.level_scope();

    if (ret.kind() == ControlFlow::Kind::Return) {
        return ret.value();
    }

    return Value();
}