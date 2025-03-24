#pragma once

#include "ast.h"
#include "object.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct StackFrame {
    std::unordered_map<std::string, Value> locals;
};

class Stack {
public:
    Stack() { m_frames.push_back(StackFrame()); }

    void enter_scope() { m_frames.push_back(StackFrame()); }
    void level_scope() { m_frames.pop_back(); }

    void insert(std::string name, Value value) { m_frames.back().locals.insert({ name, value }); }

    void set(std::string name, Value value)
    {
        for (auto it = m_frames.rbegin(); it != m_frames.rend(); ++it) {
            auto frame = it;
            auto it2 = frame->locals.find(name);
            if (it2 != frame->locals.end()) {
                frame->locals.insert_or_assign(name, value);
                return;
            }
        }

        throw std::runtime_error("Variable not found: " + name);
    }

    std::optional<Value> lookup(std::string name)
    {
        for (auto it = m_frames.rbegin(); it != m_frames.rend(); ++it) {
            auto frame = it;
            auto it2 = frame->locals.find(name);
            if (it2 != frame->locals.end()) {
                return it2->second;
            }
        }

        return std::nullopt;
    }

    std::string inspect();

private:
    std::vector<StackFrame> m_frames;
};

class ControlFlow {
public:
    enum class Kind {
        None,
        Break,
        Continue,
        Return,
    };

    explicit ControlFlow(Kind kind)
        : m_kind(kind)
    {
    }
    explicit ControlFlow(Kind kind, Value value)
        : m_kind(kind)
        , m_value(value)
    {
    }

    Kind kind() const { return m_kind; }
    Value& value() { return m_value; }

private:
    Kind m_kind;
    Value m_value;
};

class Context {
public:
    Context() { }
    Context(std::shared_ptr<Program> program)
        : m_program(program)
        , m_stack(Stack())
    {
        for (auto& fn : program->functions()) {
            insert_variable(fn.first, Value(std::make_shared<UserFunction>(fn.first)));
        }
    }

    void enter_scope() { m_stack.enter_scope(); }

    void level_scope() { m_stack.level_scope(); }

    void insert_variable(std::string name, Value value) { m_stack.insert(name, value); }

    Value get_variable(std::string name)
    {
        auto var = m_stack.lookup(name);
        if (var.has_value()) {
            return var.value();
        }

        auto found = m_environment.find(name);
        if (found != m_environment.end()) {
            return found->second;
        }

        throw std::runtime_error("Variable not found: " + name);
    }
    void set_variable(std::string name, Value value) { return m_stack.set(name, value); }

    std::vector<std::unique_ptr<Statement>>& statements() { return m_program->statements(); }

    std::shared_ptr<FnStatement> get_function(std::string name)
    {
        auto fn = m_program->functions()[name];
        if (!fn) {
            throw std::runtime_error("Function not found");
        }

        return fn;
    }

    Stack& stack() { return m_stack; }

    template <typename T>
        requires to_value<T>
    void define(std::string name, T value)
    {
        m_environment.insert_or_assign(name, Value(value));
    }

private:
    Stack m_stack;
    std::unordered_map<std::string, Value> m_environment;
    std::shared_ptr<Program> m_program;
};

class Evaluator {
public:
    Evaluator(Context& context)
        : m_context(context)
    {
    }

    Value eval();
    Value eval(Expression& expression);

private:
    ControlFlow eval(Statement& statement);
    ControlFlow eval(ReturnStatement& statement);
    ControlFlow eval(LetStatement& statement);
    ControlFlow eval(IfStatement& statement);
    ControlFlow eval(ForStatement& statement);
    ControlFlow eval(BlockStatement& statement);
    ControlFlow eval(ExpressionStatement& statement);

    Value eval(LiteralExpression& literal);
    Value eval(VariableExpression& expression);
    Value eval(BinaryExpression& expression);
    Value eval(PrefixExpression& expression);
    Value eval(PostfixExpression& expression);
    Value eval(CallExpression& expression);

    Value eval_call(FnStatement& fn, std::vector<Value>& args);

    template <typename Arguments> Value eval_call(Object& obj, Arguments args) { return obj.call(args); }

    Context& m_context;
};