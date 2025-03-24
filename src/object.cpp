#include "object.h"
#include "ast.h"

#include <cstdint>
#include <memory>

std::string value_kind_str(ValueKind kind)
{
    switch (kind) {
    case ValueKind::Null:
        return "null";
    case ValueKind::Boolean:
        return "Boolean";
    case ValueKind::Integer:
        return "Integer";
    case ValueKind::Float:
        return "Float";
    case ValueKind::String:
        return "String";
    case ValueKind::Array:
        return "Array";
    case ValueKind::Object:
        return "Object";
    case ValueKind::UserFunction:
        return "UserFunction";
    case ValueKind::NativeFunction:
        return "NativeFunction";
    default:
        throw std::runtime_error("Invalid ValueKind");
    }
}

Value Object::add(const Value& other) { throw InvalidOperate(Operator::Add, this->kind(), other.kind()); }

Value Object::sub(const Value& other) { throw InvalidOperate(Operator::Subtract, this->kind(), other.kind()); }

Value Object::mul(const Value& other) { throw InvalidOperate(Operator::Multiply, this->kind(), other.kind()); }

Value Object::div(const Value& other) { throw InvalidOperate(Operator::Divide, this->kind(), other.kind()); }

Value Object::mod(const Value& other) { throw InvalidOperate(Operator::Modulo, this->kind(), other.kind()); }

Comparison Object::compare(const Value& other) { throw InvalidOperate(Operator::Equals, this->kind(), other.kind()); }

Value Object::index(const Value& index) { throw std::runtime_error("Not implemented"); }

Value Object::call(Value args...) { throw std::runtime_error("Not implemented"); }

Value Object::get_attr(std::string name) { throw std::runtime_error("Not implemented"); }

void Object::set_attr(std::string name, Value value) { throw std::runtime_error("Not implemented"); }

Value Object::method(std::string name, std::vector<const Value>& args) { throw std::runtime_error("Not implemented"); }

template <typename T, typename... Arguments> Value NativeFunction<T, Arguments...>::call(Value args...)
{
    return Value(m_func(args));
}

Value::Value()
    : m_obj(std::make_shared<Null>())
{
}

Value::Value(bool value)
    : m_obj(std::make_shared<Boolean>(value))
{
}

Value::Value(int value)
    : m_obj(std::make_shared<Integer>(value))
{
}

Value::Value(int64_t value)
    : m_obj(std::make_shared<Integer>(value))
{
}

Value::Value(double value)
    : m_obj(std::make_shared<Float>(value))
{
}

Value::Value(std::string value)
    : m_obj(std::make_shared<String>(value))
{
}

Value Value::operator=(const Value& other)
{
    m_obj = other.m_obj;
    return *this;
}

Value::operator bool() const { return std::dynamic_pointer_cast<Boolean>(this->m_obj)->value(); }

Value::operator int64_t() const { return std::dynamic_pointer_cast<Integer>(this->m_obj)->value(); }

Value::operator double() const { return std::dynamic_pointer_cast<Float>(this->m_obj)->value(); }

Value::operator std::string() const { return std::dynamic_pointer_cast<String>(this->m_obj)->value(); }

void Value::set(Value value) { m_obj = value.m_obj; }

ValueKind Value::kind() const { return m_obj->kind(); }

std::string Value::inspect() { return m_obj->inspect(); }

bool& Value::as_boolean() const { return std::dynamic_pointer_cast<Boolean>(this->m_obj)->value(); }

int64_t& Value::as_integer() const { return std::dynamic_pointer_cast<Integer>(this->m_obj)->value(); }

double& Value::as_float() const { return std::dynamic_pointer_cast<Float>(this->m_obj)->value(); }

std::string& Value::as_string() const { return std::dynamic_pointer_cast<String>(this->m_obj)->value(); }

UserFunction& Value::as_user_function() const { return *std::dynamic_pointer_cast<UserFunction>(this->m_obj); }

Comparison Null::compare(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Null:
        return Comparison::Equal;
    default:
        throw InvalidOperate(Operator::Equals, this->kind(), other.kind());
    }
}

Comparison Boolean::compare(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Boolean: {
        auto other_bool = other.as_boolean();

        if (this->value() == other_bool) {
            return Comparison::Equal;
        } else if (this->value() > other_bool) {
            return Comparison::Greater;
        } else {
            return Comparison::Less;
        }
    }
    default:
        throw InvalidOperate(Operator::Equals, this->kind(), other.kind());
    }
}

Value Integer::add(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() + other_int);
    }
    case ValueKind::Float: {
        auto other_float = other.as_float();
        return Value(double(this->value()) + other_float);
    }
    default:
        throw InvalidOperate(Operator::Add, this->kind(), other.kind());
    }
}

Value Integer::sub(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() - other_int);
    }
    case ValueKind::Float: {
        auto other_float = other.as_float();
        return Value(double(this->value()) - other_float);
    }
    default:
        throw InvalidOperate(Operator::Subtract, this->kind(), other.kind());
    }
}

Value Integer::mul(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() * other_int);
    }
    case ValueKind::Float: {
        auto other_int = other.as_float();
        return Value(double(this->value()) * other_int);
    }
    default:
        throw InvalidOperate(Operator::Multiply, this->kind(), other.kind());
    }
}

Value Integer::div(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() / other_int);
    }
    case ValueKind::Float: {
        auto other_int = other.as_float();
        return Value(double(this->value()) / other_int);
    }
    default:
        throw InvalidOperate(Operator::Divide, ValueKind::Integer, other.kind());
    }
}

Value Integer::mod(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() % other_int);
    }
    default:
        throw InvalidOperate(Operator::Divide, ValueKind::Integer, other.kind());
    }
}

Comparison Integer::compare(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        if (this->value() == other_int) {
            return Comparison::Equal;
        } else if (this->value() > other_int) {
            return Comparison::Greater;
        } else {
            return Comparison::Less;
        }
    }
    case ValueKind::Float: {
        auto other_float = other.as_float();
        if (this->value() == other_float) {
            return Comparison::Equal;
        } else if (this->value() > other_float) {
            return Comparison::Greater;
        } else {
            return Comparison::Less;
        }
    }
    default:
        throw InvalidOperate(Operator::Equals, this->kind(), other.kind());
    }
}

Value Float::add(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Float: {
        auto other_float = other.as_float();
        return Value(this->value() + other_float);
    }
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() + double(other_int));
    }
    default:
        throw InvalidOperate(Operator::Add, this->kind(), other.kind());
    }
}

Value Float::sub(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Float: {
        auto other_float = other.as_float();
        return Value(this->value() - other_float);
    }
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() - double(other_int));
    }
    default:
        throw InvalidOperate(Operator::Subtract, this->kind(), other.kind());
    }
}

Value Float::mul(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Float: {
        auto other_float = other.as_float();
        return Value(this->value() * other_float);
    }
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() * double(other_int));
    }

    default:
        throw InvalidOperate(Operator::Multiply, this->kind(), other.kind());
    }
}

Value Float::div(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Float: {
        auto other_float = other.as_float();
        return Value(this->value() / other_float);
    }
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        return Value(this->value() / double(other_int));
    }
    default:
        throw InvalidOperate(Operator::Divide, this->kind(), other.kind());
    }
}

Comparison Float::compare(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::Float: {
        auto other_float = other.as_float();
        if (this->value() == other_float) {
            return Comparison::Equal;
        } else if (this->value() > other_float) {
            return Comparison::Greater;
        } else {
            return Comparison::Less;
        }
    }
    case ValueKind::Integer: {
        auto other_int = other.as_integer();
        if (this->value() == other_int) {
            return Comparison::Equal;
        } else if (this->value() > other_int) {
            return Comparison::Greater;
        } else {
            return Comparison::Less;
        }
    }
    default:
        throw InvalidOperate(Operator::Equals, this->kind(), other.kind());
    }
}

Value String::add(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::String: {
        auto other_string = other.as_string();
        return Value(this->value() + other_string);
    }
    default:
        throw InvalidOperate(Operator::Add, this->kind(), other.kind());
    }
}

Comparison String::compare(const Value& other)
{
    switch (other.kind()) {
    case ValueKind::String: {
        auto other_string = other.as_string();
        return this->value() == other_string ? Comparison::Equal
            : this->value() > other_string   ? Comparison::Greater
                                             : Comparison::Less;
    }
    default:
        throw InvalidOperate(Operator::Equals, this->kind(), other.kind());
    }
}