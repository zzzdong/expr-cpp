#include "ast.h"
#include <format>
#include <sstream>
#include <string>

std::string operator_str(Operator op)
{
    switch (op) {
    case Operator::Add:
        return "+";
    case Operator::Subtract:
        return "-";
    case Operator::Multiply:
        return "*";
    case Operator::Divide:
        return "/";
    case Operator::Modulo:
        return "%";
    case Operator::Power:
        return "^";
    case Operator::Equals:
        return "==";
    case Operator::NotEquals:
        return "!=";
    case Operator::GreaterThan:
        return ">";
    case Operator::GreaterThanOrEqual:
        return ">=";
    case Operator::LessThan:
        return "<";
    case Operator::LessThanOrEqual:
        return "<=";
    case Operator::LogicAnd:
        return "&&";
    case Operator::LogicOr:
        return "||";
    case Operator::Not:
        return "!";
    case Operator::Assign:
        return "=";
    case Operator::Access:
        return ".";
    case Operator::Call:
        return "()";
    default:
        return "";
    }
}

std::string ASTInspector::inspect(ASTNode& node)
{
    switch (node.kind()) {
    case ASTNode::Kind::Program: {
        std::stringstream ss;

        auto& program = dynamic_cast<Program&>(node);

        for (auto& [name, fn] : program.functions()) {
            ss << std::format("{0}", ASTInspector::inspect(*fn));
        }

        if (program.statements().size() > 0) {
            auto begin = program.statements().begin();
            for (; begin != (program.statements().end() - 1); begin++) {
                ss << std::format("{0}", ASTInspector::inspect(**begin)) << std::endl;
            }
            ss << std::format("{0}", ASTInspector::inspect(**begin));
        }

        return ss.str();
    }
    case ASTNode::Kind::ReturnStmt: {
        auto& return_stmt = dynamic_cast<ReturnStatement&>(node);
        return std::format(
            "return {0};", return_stmt.value() != nullptr ? ASTInspector::inspect(*return_stmt.value()) : "nullptr");
    }
    case ASTNode::Kind::LetStmt: {
        LetStatement& let_stmt = dynamic_cast<LetStatement&>(node);

        return std::format("LetStmt(name: {0}, value: {1})", let_stmt.name(),
            let_stmt.value() != nullptr ? ASTInspector::inspect(*let_stmt.value()) : "nullptr");
    }
    case ASTNode::Kind::ForStmt: {
        ForStatement& for_stmt = dynamic_cast<ForStatement&>(node);

        return std::format("ForStmt(initializer: {0}, condition: {1}, increment: {2}, body: {3})",
            for_stmt.initializer() ? ASTInspector::inspect(*for_stmt.initializer()) : "nullptr",
            for_stmt.condition() ? ASTInspector::inspect(*for_stmt.condition()) : "nullptr",
            for_stmt.increment() ? ASTInspector::inspect(*for_stmt.increment()) : "nullptr",
            ASTInspector::inspect(for_stmt.body()));
    }
    case ASTNode::Kind::BreakStmt:
        return "BreakStmt()";
    case ASTNode::Kind::ContinueStmt:
        return "ContinueStmt()";
    case ASTNode::Kind::EmptyStmt:
        return "EmptyStmt()";
    case ASTNode::Kind::BlockStmt: {
        BlockStatement& block_stmt = dynamic_cast<BlockStatement&>(node);
        std::stringstream ss;
        for (auto& stmt : block_stmt.statements()) {
            ss << std::format("{0}, ", ASTInspector::inspect(*stmt));
        }
        return std::format("BlockStmt(statements: [{0}])", ss.str());
    }
    case ASTNode::Kind::IfStmt: {
        IfStatement& if_stmt = dynamic_cast<IfStatement&>(node);
        return std::format("IfStmt(condition: {0}, then_branch: {1}, else_branch: "
                           "{2})",
            ASTInspector::inspect(if_stmt.condition()), ASTInspector::inspect(if_stmt.then_branch()),
            if_stmt.else_branch() ? ASTInspector::inspect(*if_stmt.else_branch()) : "nullptr");
    }
    case ASTNode::Kind::ExprStmt: {
        ExpressionStatement& expr_stmt = dynamic_cast<ExpressionStatement&>(node);
        return std::format("ExpressionStmt(expr: {0})", ASTInspector::inspect(expr_stmt.expr()));
    }
    case ASTNode::Kind::FnStmt: {
        FnStatement& fn_stmt = dynamic_cast<FnStatement&>(node);
        std::stringstream ss;

        for (auto& param : fn_stmt.params()) {
            ss << std::format("{0}, ", param);
        }

        return std::format("FnStmt(name: {0}, params: [{1}], body: {2})", fn_stmt.name(), ss.str(),
            ASTInspector::inspect(fn_stmt.body()));
    }
    case ASTNode::Kind::BinaryExpr: {
        BinaryExpression& bin = dynamic_cast<BinaryExpression&>(node);
        return std::format("BinaryExpression(op: {0}, left: {1}, right: {2})", operator_str(bin.op()),
            inspect(bin.left()), inspect(bin.right()));
    }
    case ASTNode::Kind::PrefixExpr: {
        PrefixExpression& prefix = dynamic_cast<PrefixExpression&>(node);
        return std::format("PrefixExpression(op: {0}, expr: {1})", operator_str(prefix.op()), inspect(prefix.expr()));
    }
    case ASTNode::Kind::PostfixExpr: {
        PostfixExpression& postfix = dynamic_cast<PostfixExpression&>(node);
        return std::format(
            "PostfixExpression(op: {0}, expr: {1})", operator_str(postfix.op()), inspect(postfix.expr()));
    }
    case ASTNode::Kind::LiteralExpr: {
        LiteralExpression& lit = dynamic_cast<LiteralExpression&>(node);
        LiteralKind kind = lit.literal_kind();
        switch (kind) {
        case LiteralKind::Boolean: {
            BooleanLiteral& bool_lit = dynamic_cast<BooleanLiteral&>(lit);
            return std::format("BooleanLiteral(value: {0})", bool_lit.value());
        }
        case LiteralKind::Float: {
            FloatLiteral& float_lit = dynamic_cast<FloatLiteral&>(lit);
            return std::format("FloatLiteral(value: {0})", float_lit.value());
        }
        case LiteralKind::Integer: {
            IntegerLiteral& int_lit = dynamic_cast<IntegerLiteral&>(lit);
            return std::format("IntegerLiteral(value: {0})", int_lit.value());
        }
        case LiteralKind::Null: {
            NullLiteral& null_lit = dynamic_cast<NullLiteral&>(lit);
            return std::format("NullLiteral()");
        }
        case LiteralKind::String: {
            StringLiteral& str_lit = dynamic_cast<StringLiteral&>(lit);
            return std::format("StringLiteral(value: {0})", str_lit.value());
        }
        }
    }
    case ASTNode::Kind::VariableExpr: {
        VariableExpression& var_expr = dynamic_cast<VariableExpression&>(node);
        return std::format("VariableExpr(name: {0})", var_expr.name());
    }
    case ASTNode::Kind::ArrayExpr: {
        ArrayExpression& arr_expr = dynamic_cast<ArrayExpression&>(node);

        std::stringstream ss;
        for (auto& element : arr_expr.elements()) {
            ss << std::format("{0}, ", inspect(*element));
        }

        return std::format("ArrayExpr(elements: [{0}])", ss.str());
    }
    case ASTNode::Kind::IndexExpr: {
        IndexExpression& index_expr = dynamic_cast<IndexExpression&>(node);

        return std::format(
            "IndexExpr(object: {0}, index: {1})", inspect(index_expr.object()), inspect(index_expr.index()));
    }
    case ASTNode::Kind::CallExpr: {
        CallExpression& call_expr = dynamic_cast<CallExpression&>(node);
        std::stringstream ss;
        for (auto& arg : call_expr.args()) {
            ss << inspect(*arg) << ", ";
        }

        return std::format("CallExpr(callee: {0}, args: [{1}])", inspect(call_expr.callee()), ss.str());
    }

    default:
        throw std::runtime_error(std::format("Unknown ASTNode kind: {}", static_cast<int>(node.kind())));
    }
}