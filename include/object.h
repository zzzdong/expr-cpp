#include "ast.h"
#include <cstdint>
#include <ctime>
#include <format>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

enum class ValueKind {
    Undefined,
    Boolean,
    Integer,
    Float,
    String,
    Array,
    Object,
    UserFunction,
    NativeFunction,
};

enum class Comparison {
    Equal,
    Less,
    Greater,
};

std::string value_kind_str(ValueKind kind);

class InvalidOperate : public std::invalid_argument {
public:
    InvalidOperate(const std::string& msg)
        : invalid_argument(msg)
    {
    }

    InvalidOperate(Operator op, ValueKind obj)
        : invalid_argument(std::format("invalid {} unary operation for {}",
              operator_str(op), value_kind_str(obj)))
    {
    }

    InvalidOperate(Operator op, ValueKind lhs, ValueKind rhs)
        : invalid_argument(std::format("invalid {} operation for {} with {}",
              operator_str(op), value_kind_str(lhs),
              value_kind_str(rhs)))
    {
    }
};

class Value;

class Undefined;

class Boolean;

class Integer;

class Float;

class String;

class UserFunction;

class Object {
public:
    virtual ~Object() = default;

    virtual ValueKind kind() = 0;
    virtual std::string inspect() { return "<Unknown>"; }

    virtual Value add(const Value& other);

    virtual Value sub(const Value& other);

    virtual Value mul(const Value& other);

    virtual Value div(const Value& other);

    virtual Value mod(const Value& other);

    virtual Comparison compare(const Value& other);

    virtual Value index(const Value& index);

    virtual Value call(Value args...);

    virtual Value get_attr(std::string name);

    virtual void set_attr(std::string name, Value value);

    virtual Value method(std::string name, std::vector<const Value>& args);
};

template <typename T, typename... Arguments>
class NativeFunction : public Object {
public:
    NativeFunction(std::string name, std::function<T(Arguments...)> func)
        : m_name(name)
        , m_func(func)
    {
    }

    ValueKind kind() override { return ValueKind::NativeFunction; }

    std::string name() { return m_name; }

    std::string inspect() override
    {
        return std::format("<native fn {}>", this->name());
    }

    Value call(Value args...) override;

private:
    std::string m_name;
    std::function<T(Arguments...)> m_func;
};

// template <typename T, typename Arg0>
// class NativeFunction : public Object {
// public:
//     NativeFunction(std::string name, std::function<T(Arg0)> func)
//         : m_name(name)
//         , m_func(func)
//     {
//     }

//     ValueKind kind() override { return ValueKind::NativeFunction; }

//     std::string name() { return m_name; }

//     std::string inspect() override
//     {
//         return std::format("<native fn {}>", this->name());
//     }

//     Value call(Value args...) override;

// private:
//     std::string m_name;
//     std::function<T(Arg0)> m_func;
// };

class Value {
public:
    Value();

    Value(std::shared_ptr<Object> obj)
        : m_obj(obj)
    {
    }

    explicit Value(bool value);
    explicit Value(int value);
    explicit Value(int64_t value);
    explicit Value(double value);
    explicit Value(std::string value);

    Value operator=(const Value& other);
    operator bool() const;
    operator int64_t() const;
    operator double() const;
    operator std::string() const;

    ValueKind kind() const;
    std::string inspect();

    bool& as_boolean() const;
    int64_t& as_integer() const;
    double& as_float() const;
    std::string& as_string() const;
    UserFunction& as_user_function() const;

    template <typename T, typename... Arguments>
    NativeFunction<T, Arguments...>& as_native_function() const
    {
        return *std::dynamic_pointer_cast<NativeFunction<T, Arguments...>>(
            this->obj());
    }

    std::shared_ptr<Object> obj() const { return m_obj; }
    void set_obj(std::shared_ptr<Object> obj) { m_obj = obj; }
    void set(Value value);

private:
    std::shared_ptr<Object> m_obj;
};

class Undefined : public Object {
public:
    Undefined() = default;
    ValueKind kind() override { return ValueKind::Undefined; }

    Comparison compare(const Value& other) override;
};

class Boolean : public Object {
public:
    Boolean(bool value)
        : m_value(value)
    {
    }

    ValueKind kind() override { return ValueKind::Boolean; }

    bool& value() { return m_value; }

    Comparison compare(const Value& other) override;

private:
    bool m_value;
};

class Integer : public Object {
public:
    Integer(int64_t value)
        : m_value(value)
    {
    }
    int64_t& value() { return m_value; }

    Value add(const Value& other) override;
    Value sub(const Value& other) override;
    Value mul(const Value& other) override;
    Value div(const Value& other) override;
    Value mod(const Value& other) override;
    Comparison compare(const Value& other) override;

    ValueKind kind() override { return ValueKind::Integer; }
    std::string inspect() override { return std::to_string(this->value()); }

private:
    int64_t m_value;
};

class Float : public Object {
public:
    Float(double value)
        : m_value(value)
    {
    }
    double& value() { return m_value; }

    Value add(const Value& other) override;
    Value sub(const Value& other) override;
    Value mul(const Value& other) override;
    Value div(const Value& other) override;
    Comparison compare(const Value& other) override;

    ValueKind kind() override { return ValueKind::Float; }
    std::string inspect() override { return std::format("{}", this->value()); }

private:
    double m_value;
};

class String : public Object {
public:
    String(std::string value)
        : m_value(value)
    {
    }

    std::string& value() { return m_value; }

    Value add(const Value& other) override;
    Comparison compare(const Value& other) override;

    ValueKind kind() override { return ValueKind::String; }
    std::string inspect() override
    {
        return std::format("\"{}\"", this->value());
    }

private:
    std::string m_value;
};

class UserFunction : public Object {
public:
    UserFunction(std::string name)
        : m_name(name)
    {
    }

    ValueKind kind() override { return ValueKind::UserFunction; }
    std::string inspect() override
    {
        return std::format("<fn {}>", this->name());
    }

    std::string name() { return m_name; }

private:
    std::string m_name;
};

template <typename T>
concept to_value = requires(T t) {
    Value(t);
};

// template <typename... Arguments>
// class Callable : public Object {
// public:
//     Callable(std::string name, std::function<Value(Arguments...)> func)
//         : m_name(name)
//         , m_func(func)
//     {
//     }

//     ValueKind kind() override { return ValueKind::NativeFunction; }

//     std::string name() { return m_name; }

//     std::string inspect() override
//     {
//         return std::format("<native fn {}>", this->name());
//     }

//     Value call(std::vector<Value>& args) override
//     {
//         // return m_func(args);
//         return Value();
//     }

// private:
//     std::string m_name;
//     std::function<Value(Arguments...)> m_func;
// };
