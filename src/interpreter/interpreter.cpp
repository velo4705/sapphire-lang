#include "interpreter.h"
#include "../error/exception.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <functional>

// Milestone 1 Libraries
#include "../stdlib/collections/array.h"
#include "../stdlib/math/math.h"
#include "../stdlib/io/csv.h"

// Milestone 2 Libraries
#include "../stdlib/json/json.h"
#include "../stdlib/random/random.h"

// Milestone 3 Libraries
#include "../stdlib/plotting/plotting.h"
#include "../stdlib/plot3d/plot3d.h"

// Milestone 4 Libraries
#include "../stdlib/http/http.h"
#include "../stdlib/websocket/websocket.h"
#include "../stdlib/template/template.h"
#include "../stdlib/database/database.h"
#include "../stdlib/database/database_capi.h"

// Milestone 5 Libraries
#include "../stdlib/orm/orm.h"
#include "../stdlib/auth/auth.h"
#include "../stdlib/rest/rest.h"
#include "../stdlib/graphql/graphql.h"

// Milestone 6 Libraries
#include "../stdlib/blockchain/blockchain.h"
// Milestone 12 Libraries
#include "../stdlib/network/network.h"
#include "../stdlib/crypto/crypto.h"
#include "../stdlib/smartcontract/smartcontract.h"
#include "../stdlib/dapp/dapp.h"
#include "../stdlib/security/security.h"
#include "../stdlib/mobile/mobile.h"
#include "../stdlib/game/game.h"
#include "../stdlib/system/sysprog.h"
#include "../stdlib/gui/gui.h"
#include "../stdlib/gl3d/gl3d.h"

// Milestone 15 Libraries
#include "../stdlib/advancedcrypto/advancedcrypto.h"
// Milestone 16 Libraries
#include "../stdlib/mathx/mathx.h"
#include "../stdlib/polish/polish.h"
#include "../stdlib/ecosystem/ecosystem.h"

// Stubs for legacy database dispatch (old M4-era placeholders, never called at runtime)
inline void* sapphire_postgresql_create() { return nullptr; }
inline int   sapphire_postgresql_connect(void*, const char*) { return 0; }
inline char* sapphire_postgresql_query(void*, const char*) { static char e[]=""; return e; }
inline void* sapphire_mysql_create() { return nullptr; }
inline int   sapphire_mysql_connect(void*, const char*) { return 0; }
inline char* sapphire_mysql_query(void*, const char*) { static char e[]=""; return e; }
inline void  sapphire_database_free_result(char*) {}
inline void* orm_model_create(const char*) { return nullptr; }
inline void  orm_model_add_field(void*, const char*, const char*) {}
inline void  orm_model_add_string_field(void*, const char*, int) {}
inline void  orm_model_add_int_field(void*, const char*) {}
inline void  orm_model_add_primary_key(void*, const char*) {}
inline const char* orm_model_get_sql(void*) { return ""; }
inline const char* orm_model_get_create_sql(void*) { return ""; }

// Milestone 13 Libraries
#include "../stdlib/texture/texture.h"
#include "../stdlib/shader/shader.h"
#include "../stdlib/model/model.h"
#include "../stdlib/physics/physics.h"
#include "../stdlib/simulation/simulation.h"
#include "../stdlib/os/os.h"
#include "../stdlib/fs/fs.h"

namespace sapphire {

// Environment implementation

void Environment::define(const std::string& name, const Value& value, Ownership o) {
    values[name] = value;
    ownerships[name] = o;
}

Value Environment::get(const std::string& name) {
    if (values.find(name) != values.end()) {
        return values[name];
    }
    
    if (enclosing != nullptr) {
        return enclosing->get(name);
    }
    
    throw RuntimeError("Undefined variable '" + name + "'");
}

void Environment::assign(const std::string& name, const Value& value) {
    if (values.find(name) != values.end()) {
        Ownership o = ownerships[name];
        if (o == Ownership::BORROW) {
            throw RuntimeError("Cannot assign to borrowed variable '" + name + "'");
        }
        values[name] = value;
        return;
    }
    
    if (enclosing != nullptr) {
        enclosing->assign(name, value);
        return;
    }
    
    throw RuntimeError("Undefined variable '" + name + "'");
}

Ownership Environment::getOwnership(const std::string& name) {
    if (ownerships.find(name) != ownerships.end()) {
        return ownerships[name];
    }
    if (enclosing != nullptr) {
        return enclosing->getOwnership(name);
    }
    return Ownership::NONE;
}

// Class implementation

std::shared_ptr<Function> Class::findMethod(const std::string& method_name) const {
    auto it = methods.find(method_name);
    if (it != methods.end()) {
        return it->second;
    }
    if (superclass) {
        return superclass->findMethod(method_name);
    }
    return nullptr;
}

// Interpreter implementation

Interpreter::Interpreter() {
    environment = std::make_shared<Environment>();
    
    // Built-in functions
    environment->define("print", std::string("__builtin_print__"));
    environment->define("range", std::string("__builtin_range__"));
    environment->define("len", std::string("__builtin_len__"));
    environment->define("input", std::string("__builtin_input__"));
    environment->define("read_file", std::string("__builtin_read_file__"));
    environment->define("write_file", std::string("__builtin_write_file__"));
    environment->define("time", std::string("__builtin_time__"));
    environment->define("sleep", std::string("__builtin_sleep__"));
    environment->define("sort", std::string("__builtin_sort__"));
    environment->define("make_array", std::string("__builtin_make_array__"));
    environment->define("WaitGroup", std::string("__builtin_waitgroup__"));
    
    // Milestones 1-5 are opt-in via import statements (see visitImportStmt)

    // Option type constructors
    environment->define("Some", std::string("__builtin_some__"));
    environment->define("None", std::string("__builtin_none__"));
    
    // Result type constructors
    environment->define("Ok", std::string("__builtin_ok__"));
    environment->define("Err", std::string("__builtin_err__"));
    
    // Smart pointer constructors
    environment->define("Rc", std::string("__builtin_rc__"));
    environment->define("Arc", std::string("__builtin_arc__"));
    environment->define("Weak", std::string("__builtin_weak__"));
    
    // Built-in decorators
    environment->define("cache", std::string("__builtin_cache__"));
    environment->define("timing", std::string("__builtin_timing__"));
    environment->define("deprecated", std::string("__builtin_deprecated__"));
    environment->define("dataclass", std::string("__builtin_dataclass__"));
    environment->define("singleton", std::string("__builtin_singleton__"));
    environment->define("constexpr", std::string("__builtin_constexpr__"));
    environment->define("staticmethod", std::string("__builtin_staticmethod__"));
    environment->define("classmethod", std::string("__builtin_classmethod__"));
    environment->define("property", std::string("__builtin_property__"));
    
    // Collections module
    environment->define("Set", std::string("__builtin_set__"));
    environment->define("Queue", std::string("__builtin_queue__"));
    environment->define("Stack", std::string("__builtin_stack__"));
    environment->define("Deque", std::string("__builtin_deque__"));
    
    // Itertools module
    environment->define("map", std::string("__builtin_map__"));
    environment->define("filter", std::string("__builtin_filter__"));
    environment->define("reduce", std::string("__builtin_reduce__"));
    environment->define("take", std::string("__builtin_take__"));
    environment->define("drop", std::string("__builtin_drop__"));
    environment->define("zip", std::string("__builtin_zip__"));
    environment->define("enumerate", std::string("__builtin_enumerate__"));
    environment->define("chunk", std::string("__builtin_chunk__"));
    environment->define("flatten", std::string("__builtin_flatten__"));
    environment->define("all", std::string("__builtin_all__"));
    environment->define("any", std::string("__builtin_any__"));
    environment->define("find", std::string("__builtin_find__"));
    environment->define("unique", std::string("__builtin_unique__"));
    environment->define("reverse", std::string("__builtin_reverse__"));
    
    // Testing module
    environment->define("assert_true", std::string("__builtin_assert_true__"));
    environment->define("assert_false", std::string("__builtin_assert_false__"));
    environment->define("assert_equal", std::string("__builtin_assert_equal__"));
    
    // Reflection module
    environment->define("typeof", std::string("__builtin_typeof__"));
    environment->define("type_name", std::string("__builtin_type_name__"));
    environment->define("fields", std::string("__builtin_fields__"));
    environment->define("methods", std::string("__builtin_methods__"));
    environment->define("is_type", std::string("__builtin_is_type__"));
    environment->define("has_field", std::string("__builtin_has_field__"));
    environment->define("has_method", std::string("__builtin_has_method__"));
}

void Interpreter::interpret(std::vector<std::unique_ptr<Stmt>>& statements) {
    try {
        for (auto& stmt : statements) {
            stmt->accept(*this);
        }
        
        // Execute any pending goroutines
        executePendingTasks();
    } catch (const ReturnException& e) {
        // Top-level return is ignored
        (void)e;
    } catch (SapphireException& e) {
        // Attach stack trace
        for (const auto& frame : call_stack) {
            e.addStackFrame(frame);
        }
        throw;
    }
}

void Interpreter::throwException(const std::string& type, const std::string& message) {
    std::string full_message = message;
    if (!type.empty() && !message.empty()) {
        full_message = type + ": " + message;
    } else if (!type.empty()) {
        full_message = type;
    }
    
    if (type == "DivisionByZeroError") {
        throw DivisionByZeroError(full_message);
    } else if (type == "FileNotFoundError") {
        throw FileNotFoundError(full_message);
    } else if (type == "IndexError") {
        throw IndexError(full_message);
    } else if (type == "TypeError") {
        throw TypeError(full_message);
    } else if (type == "ValueError") {
        throw ValueError(full_message);
    } else if (type == "PermissionError") {
        throw PermissionError(full_message);
    } else if (type == "RuntimeError" || type.empty()) {
        throw RuntimeError(full_message);
    } else {
        throw SapphireException(full_message, type);
    }
}

std::string Interpreter::valueToString(const Value& value) {
    if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<std::nullptr_t>(value)) {
        return "none";
    } else if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
        auto func = std::get<std::shared_ptr<Function>>(value);
        return "<function " + func->name + ">";
    } else if (std::holds_alternative<std::shared_ptr<Class>>(value)) {
        auto cls = std::get<std::shared_ptr<Class>>(value);
        return "<class " + cls->name + ">";
    } else if (std::holds_alternative<std::shared_ptr<Instance>>(value)) {
        auto inst = std::get<std::shared_ptr<Instance>>(value);
        
        // If the class is a dataclass, generate a nice string representation
        if (inst->klass->is_dataclass) {
            std::string result = inst->klass->name + "(";
            bool first = true;
            for (const auto& [field_name, field_value] : inst->fields) {
                if (!first) result += ", ";
                result += field_name + "=" + valueToString(field_value);
                first = false;
            }
            result += ")";
            return result;
        }
        
        return "<" + inst->klass->name + " instance>";
    } else if (std::holds_alternative<std::shared_ptr<BoundMethod>>(value)) {
        auto bm = std::get<std::shared_ptr<BoundMethod>>(value);
        return "<bound method " + bm->method->name + " of " + bm->instance->klass->name + ">";
    } else if (std::holds_alternative<std::shared_ptr<StaticMethod>>(value)) {
        auto sm = std::get<std::shared_ptr<StaticMethod>>(value);
        if (sm->is_classmethod) {
            return "<class method " + sm->method->name + " of " + sm->klass->name + ">";
        } else {
            return "<static method " + sm->method->name + " of " + sm->klass->name + ">";
        }
    } else if (std::holds_alternative<std::shared_ptr<ArrayValue>>(value)) {
        auto array = std::get<std::shared_ptr<ArrayValue>>(value);
        std::string result = "[";
        for (size_t i = 0; i < array->elements.size(); i++) {
            result += valueToString(array->elements[i]);
            if (i < array->elements.size() - 1) {
                result += ", ";
            }
        }
        result += "]";
        return result;
    } else if (std::holds_alternative<std::shared_ptr<ChannelValue>>(value)) {
        auto channel = std::get<std::shared_ptr<ChannelValue>>(value);
        return "<channel capacity=" + std::to_string(channel->capacity) + 
               " size=" + std::to_string(channel->buffer.size()) + 
               (channel->is_closed() ? " closed" : "") + ">";
    } else if (std::holds_alternative<std::shared_ptr<ChannelMethod>>(value)) {
        auto cm = std::get<std::shared_ptr<ChannelMethod>>(value);
        return "<channel method " + cm->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<WaitGroupValue>>(value)) {
        auto wg = std::get<std::shared_ptr<WaitGroupValue>>(value);
        return "<WaitGroup counter=" + std::to_string(wg->counter) + ">";
    } else if (std::holds_alternative<std::shared_ptr<WaitGroupMethod>>(value)) {
        auto wgm = std::get<std::shared_ptr<WaitGroupMethod>>(value);
        return "<WaitGroup method " + wgm->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<OptionValue>>(value)) {
        auto option = std::get<std::shared_ptr<OptionValue>>(value);
        if (option->isNone()) {
            return "None";
        } else {
            return "Some(" + valueToString(option->value) + ")";
        }
    } else if (std::holds_alternative<std::shared_ptr<OptionMethod>>(value)) {
        auto om = std::get<std::shared_ptr<OptionMethod>>(value);
        return "<Option method " + om->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<ResultValue>>(value)) {
        auto result = std::get<std::shared_ptr<ResultValue>>(value);
        if (result->isOk()) {
            return "Ok(" + valueToString(result->value) + ")";
        } else {
            return "Err(" + valueToString(result->value) + ")";
        }
    } else if (std::holds_alternative<std::shared_ptr<ResultMethod>>(value)) {
        auto rm = std::get<std::shared_ptr<ResultMethod>>(value);
        return "<Result method " + rm->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<ArrayMethod>>(value)) {
        auto am = std::get<std::shared_ptr<ArrayMethod>>(value);
        return "<Array method " + am->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<StringMethod>>(value)) {
        auto sm = std::get<std::shared_ptr<StringMethod>>(value);
        return "<String method " + sm->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<HashMapValue>>(value)) {
        auto hashmap = std::get<std::shared_ptr<HashMapValue>>(value);
        std::string result = "{";
        bool first = true;
        for (const auto& pair : hashmap->pairs) {
            if (!first) result += ", ";
            result += "\"" + pair.first + "\": " + valueToString(pair.second);
            first = false;
        }
        result += "}";
        return result;
    } else if (std::holds_alternative<std::shared_ptr<HashMapMethod>>(value)) {
        auto hm = std::get<std::shared_ptr<HashMapMethod>>(value);
        return "<HashMap method " + hm->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<RcValue>>(value)) {
        auto rc = std::get<std::shared_ptr<RcValue>>(value);
        return "Rc(" + valueToString(*(rc->inner)) + ", count=" + std::to_string(rc->inner.use_count()) + ")";
    } else if (std::holds_alternative<std::shared_ptr<RcMethod>>(value)) {
        auto rm = std::get<std::shared_ptr<RcMethod>>(value);
        return "<Rc method " + rm->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<ArcValue>>(value)) {
        auto arc = std::get<std::shared_ptr<ArcValue>>(value);
        return "Arc(" + valueToString(*(arc->inner)) + ", count=" + std::to_string(arc->inner.use_count()) + ")";
    } else if (std::holds_alternative<std::shared_ptr<ArcMethod>>(value)) {
        auto am = std::get<std::shared_ptr<ArcMethod>>(value);
        return "<Arc method " + am->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<WeakValue>>(value)) {
        auto weak = std::get<std::shared_ptr<WeakValue>>(value);
        return std::string("<Weak ") + (weak->expired() ? "alive" : "expired") + ">";
    } else if (std::holds_alternative<std::shared_ptr<WeakMethod>>(value)) {
        auto wm = std::get<std::shared_ptr<WeakMethod>>(value);
        return "<Weak method " + wm->method_name + ">";
    } else if (std::holds_alternative<std::shared_ptr<ExtensionMethod>>(value)) {
        auto em = std::get<std::shared_ptr<ExtensionMethod>>(value);
        return "<extension method " + em->method->name + ">";
    }
    return "unknown";
}

bool Interpreter::isTruthy(const Value& value) {
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    } else if (std::holds_alternative<std::nullptr_t>(value)) {
        return false;
    } else if (std::holds_alternative<int>(value)) {
        return std::get<int>(value) != 0;
    } else if (std::holds_alternative<double>(value)) {
        return std::get<double>(value) != 0.0;
    } else if (std::holds_alternative<std::string>(value)) {
        return !std::get<std::string>(value).empty();
    } else if (std::holds_alternative<std::shared_ptr<Instance>>(value)) {
        return true;
    } else if (std::holds_alternative<std::shared_ptr<Class>>(value)) {
        return true;
    } else if (std::holds_alternative<std::shared_ptr<BoundMethod>>(value)) {
        return true;
    }
    return true;
}

Value Interpreter::evaluateExpr(Expr& expr) {
    expr.accept(*this);
    return last_value;
}

// Expression visitors

void Interpreter::visitLiteralExpr(LiteralExpr& expr) {
    switch (expr.type) {
        case LiteralExpr::Type::INTEGER:
            last_value = std::stoi(expr.value);
            break;
        case LiteralExpr::Type::FLOAT:
            last_value = std::stod(expr.value);
            break;
        case LiteralExpr::Type::STRING:
            last_value = expr.value;
            break;
        case LiteralExpr::Type::BOOLEAN:
            last_value = (expr.value == "true");
            break;
        case LiteralExpr::Type::NONE:
            last_value = nullptr;
            break;
    }
}

void Interpreter::visitVariableExpr(VariableExpr& expr) {
    last_value = environment->get(expr.name);
}

void Interpreter::visitAssignExpr(AssignExpr& expr) {
    Value value = evaluateExpr(*expr.value);
    // Check ownership before assigning
    Ownership o = environment->getOwnership(expr.name);
    if (o == Ownership::BORROW) {
        throw RuntimeError("Cannot assign to borrowed variable '" + expr.name + "'");
    }
    environment->define(expr.name, value);
    last_value = value;
}

void Interpreter::visitBinaryExpr(BinaryExpr& expr) {
    Value left = evaluateExpr(*expr.left);
    Value right = evaluateExpr(*expr.right);
    
    // Arithmetic operations
    if (expr.op == "+") {
        // Array concatenation
        if (std::holds_alternative<std::shared_ptr<ArrayValue>>(left) &&
            std::holds_alternative<std::shared_ptr<ArrayValue>>(right)) {
            auto result = std::make_shared<ArrayValue>();
            auto& la = std::get<std::shared_ptr<ArrayValue>>(left)->elements;
            auto& ra = std::get<std::shared_ptr<ArrayValue>>(right)->elements;
            result->elements.insert(result->elements.end(), la.begin(), la.end());
            result->elements.insert(result->elements.end(), ra.begin(), ra.end());
            last_value = result;
        } else if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            last_value = std::get<int>(left) + std::get<int>(right);
        } else if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right)) {
            // String concatenation: coerce either side to string
            auto to_str = [](const Value& v) -> std::string {
                if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
                if (std::holds_alternative<int>(v)) return std::to_string(std::get<int>(v));
                if (std::holds_alternative<double>(v)) {
                    std::ostringstream oss; oss << std::get<double>(v); return oss.str();
                }
                if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? "true" : "false";
                return "";
            };
            last_value = to_str(left) + to_str(right);
        } else {
            double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
            double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
            last_value = l + r;
        }
    } else if (expr.op == "-") {
        if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            last_value = std::get<int>(left) - std::get<int>(right);
        } else {
            double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
            double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
            last_value = l - r;
        }
    } else if (expr.op == "*") {
        if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            last_value = std::get<int>(left) * std::get<int>(right);
        } else {
            double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
            double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
            last_value = l * r;
        }
    } else if (expr.op == "/") {
        double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
        double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
        if (r == 0.0) {
            throw DivisionByZeroError("Division by zero");
        }
        last_value = l / r;
    } else if (expr.op == "%") {
        int r = std::get<int>(right);
        if (r == 0) {
            throw DivisionByZeroError("Modulo by zero");
        }
        last_value = std::get<int>(left) % r;
    } else if (expr.op == "**") {
        double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
        double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
        last_value = std::pow(l, r);
    }
    // Comparison operations
    else if (expr.op == "==") {
        last_value = (left == right);
    } else if (expr.op == "!=") {
        last_value = (left != right);
    } else if (expr.op == "<") {
        if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            last_value = std::get<int>(left) < std::get<int>(right);
        } else {
            double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
            double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
            last_value = l < r;
        }
    } else if (expr.op == "<=") {
        if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            last_value = std::get<int>(left) <= std::get<int>(right);
        } else {
            double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
            double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
            last_value = l <= r;
        }
    } else if (expr.op == ">") {
        if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            last_value = std::get<int>(left) > std::get<int>(right);
        } else {
            double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
            double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
            last_value = l > r;
        }
    } else if (expr.op == ">=") {
        if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
            last_value = std::get<int>(left) >= std::get<int>(right);
        } else {
            double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
            double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
            last_value = l >= r;
        }
    }
    // Logical operations
    else if (expr.op == "and") {
        last_value = isTruthy(left) && isTruthy(right);
    } else if (expr.op == "or") {
        last_value = isTruthy(left) || isTruthy(right);
    }
}

void Interpreter::visitUnaryExpr(UnaryExpr& expr) {
    Value operand = evaluateExpr(*expr.operand);
    
    if (expr.op == "-") {
        if (std::holds_alternative<int>(operand)) {
            last_value = -std::get<int>(operand);
        } else {
            last_value = -std::get<double>(operand);
        }
    } else if (expr.op == "not") {
        last_value = !isTruthy(operand);
    }
}

void Interpreter::visitCallExpr(CallExpr& expr) {
    Value callee = evaluateExpr(*expr.callee);
    
    // Evaluate arguments
    std::vector<Value> arguments;
    for (auto& arg : expr.arguments) {
        arguments.push_back(evaluateExpr(*arg));
    }
    
    // Handle channel methods
    if (std::holds_alternative<std::shared_ptr<ChannelMethod>>(callee)) {
        auto cm = std::get<std::shared_ptr<ChannelMethod>>(callee);
        
        if (cm->method_name == "close") {
            if (!arguments.empty()) {
                throw TypeError("Channel.close() takes no arguments");
            }
            cm->channel->close();
            last_value = nullptr;
            return;
        } else if (cm->method_name == "is_closed") {
            if (!arguments.empty()) {
                throw TypeError("Channel.is_closed() takes no arguments");
            }
            last_value = cm->channel->is_closed();
            return;
        } else if (cm->method_name == "empty") {
            if (!arguments.empty()) {
                throw TypeError("Channel.empty() takes no arguments");
            }
            last_value = cm->channel->empty();
            return;
        } else if (cm->method_name == "size") {
            if (!arguments.empty()) {
                throw TypeError("Channel.size() takes no arguments");
            }
            last_value = static_cast<int>(cm->channel->buffer.size());
            return;
        }
        
        throw RuntimeError("Unknown channel method: " + cm->method_name);
    }
    
    // Handle array methods
    if (std::holds_alternative<std::shared_ptr<ArrayMethod>>(callee)) {
        auto am = std::get<std::shared_ptr<ArrayMethod>>(callee);
        
        if (am->method_name == "push") {
            if (arguments.size() != 1) {
                throw TypeError("Array.push() takes exactly 1 argument");
            }
            am->array->elements.push_back(arguments[0]);
            last_value = static_cast<int>(am->array->elements.size());
            return;
        } else if (am->method_name == "pop") {
            if (!arguments.empty()) {
                throw TypeError("Array.pop() takes no arguments");
            }
            if (am->array->elements.empty()) {
                throw RuntimeError("Cannot pop from empty array");
            }
            Value popped = am->array->elements.back();
            am->array->elements.pop_back();
            last_value = popped;
            return;
        }
        
        throw RuntimeError("Unknown array method: " + am->method_name);
    }
    
    // Handle string methods
    if (std::holds_alternative<std::shared_ptr<StringMethod>>(callee)) {
        auto sm = std::get<std::shared_ptr<StringMethod>>(callee);
        
        if (sm->method_name == "upper") {
            if (!arguments.empty()) {
                throw TypeError("String.upper() takes no arguments");
            }
            std::string result = sm->string_value;
            std::transform(result.begin(), result.end(), result.begin(), ::toupper);
            last_value = result;
            return;
        } else if (sm->method_name == "lower") {
            if (!arguments.empty()) {
                throw TypeError("String.lower() takes no arguments");
            }
            std::string result = sm->string_value;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            last_value = result;
            return;
        } else if (sm->method_name == "split") {
            if (arguments.size() != 1) {
                throw TypeError("String.split() takes exactly 1 argument");
            }
            if (!std::holds_alternative<std::string>(arguments[0])) {
                throw TypeError("String.split() argument must be a string");
            }
            
            std::string delimiter = std::get<std::string>(arguments[0]);
            std::string text = sm->string_value;
            auto result_array = std::make_shared<ArrayValue>();
            
            if (delimiter.empty()) {
                // Split into individual characters
                for (char c : text) {
                    result_array->elements.push_back(std::string(1, c));
                }
            } else {
                // Split by delimiter
                size_t start = 0;
                size_t end = text.find(delimiter);
                
                while (end != std::string::npos) {
                    result_array->elements.push_back(text.substr(start, end - start));
                    start = end + delimiter.length();
                    end = text.find(delimiter, start);
                }
                result_array->elements.push_back(text.substr(start));
            }
            
            last_value = result_array;
            return;
        } else if (sm->method_name == "trim") {
            if (!arguments.empty()) {
                throw TypeError("String.trim() takes no arguments");
            }
            std::string result = sm->string_value;
            // Trim leading whitespace
            size_t start = result.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) {
                last_value = std::string("");
                return;
            }
            result = result.substr(start);
            // Trim trailing whitespace
            size_t end = result.find_last_not_of(" \t\n\r");
            result = result.substr(0, end + 1);
            last_value = result;
            return;
        } else if (sm->method_name == "contains") {
            if (arguments.size() != 1) {
                throw TypeError("String.contains() takes exactly 1 argument");
            }
            if (!std::holds_alternative<std::string>(arguments[0])) {
                throw TypeError("String.contains() argument must be a string");
            }
            std::string substr = std::get<std::string>(arguments[0]);
            last_value = sm->string_value.find(substr) != std::string::npos;
            return;
        }
        
        throw RuntimeError("Unknown string method: " + sm->method_name);
    }
    
    // Handle hash map methods
    if (std::holds_alternative<std::shared_ptr<HashMapMethod>>(callee)) {
        auto hm = std::get<std::shared_ptr<HashMapMethod>>(callee);
        
        if (hm->method_name == "get") {
            if (arguments.size() != 1) {
                throw TypeError("HashMap.get() takes exactly 1 argument");
            }
            std::string key = valueToString(arguments[0]);
            last_value = hm->hashmap->get(key);
            return;
        } else if (hm->method_name == "set") {
            if (arguments.size() != 2) {
                throw TypeError("HashMap.set() takes exactly 2 arguments");
            }
            std::string key = valueToString(arguments[0]);
            hm->hashmap->pairs[key] = arguments[1];
            last_value = arguments[1];
            return;
        } else if (hm->method_name == "has") {
            if (arguments.size() != 1) {
                throw TypeError("HashMap.has() takes exactly 1 argument");
            }
            std::string key = valueToString(arguments[0]);
            last_value = hm->hashmap->has(key);
            return;
        } else if (hm->method_name == "remove") {
            if (arguments.size() != 1) {
                throw TypeError("HashMap.remove() takes exactly 1 argument");
            }
            std::string key = valueToString(arguments[0]);
            auto it = hm->hashmap->pairs.find(key);
            if (it != hm->hashmap->pairs.end()) {
                Value removed = it->second;
                hm->hashmap->pairs.erase(it);
                last_value = removed;
            } else {
                last_value = Value(nullptr);
            }
            return;
        } else if (hm->method_name == "keys") {
            if (!arguments.empty()) {
                throw TypeError("HashMap.keys() takes no arguments");
            }
            auto result_array = std::make_shared<ArrayValue>();
            for (const auto& pair : hm->hashmap->pairs) {
                result_array->elements.push_back(pair.first);
            }
            last_value = result_array;
            return;
        } else if (hm->method_name == "values") {
            if (!arguments.empty()) {
                throw TypeError("HashMap.values() takes no arguments");
            }
            auto result_array = std::make_shared<ArrayValue>();
            for (const auto& pair : hm->hashmap->pairs) {
                result_array->elements.push_back(pair.second);
            }
            last_value = result_array;
            return;
        }
        
        throw RuntimeError("Unknown hash map method: " + hm->method_name);
    }
    
    // Handle user-defined functions
    if (std::holds_alternative<std::shared_ptr<Function>>(callee)) {
        auto func = std::get<std::shared_ptr<Function>>(callee);
        
        // Extract original function name and check for built-in decorator markers
        std::string func_name = func->name;
        std::string original_name = func->name;
        bool is_cached = false;
        bool is_timed = false;
        bool is_deprecated = false;
        std::string deprecation_message;
        
        // Strip decorator prefixes in order (they're applied bottom-to-top, so we strip top-to-bottom)
        // Check for @timing decorator
        if (func_name.find("__timed__") == 0) {
            is_timed = true;
            func_name = func_name.substr(9);  // Remove "__timed__" prefix
        }
        
        // Check for @cache decorator
        if (func_name.find("__cached__") == 0) {
            is_cached = true;
            func_name = func_name.substr(10);  // Remove "__cached__" prefix
        }
        
        // Check for @deprecated decorator
        if (func_name.find("__deprecated__") == 0) {
            is_deprecated = true;
            size_t msg_start = func_name.find("__", 14);
            if (msg_start != std::string::npos) {
                deprecation_message = func_name.substr(msg_start + 2);
                func_name = func_name.substr(14, msg_start - 14);
            } else {
                func_name = func_name.substr(14);
            }
        }

        // Check for @singleton decorator — return cached instance if already created
        bool is_singleton = false;
        if (func_name.find("__singleton__") == 0) {
            is_singleton = true;
            func_name = func_name.substr(13);
            std::string cache_key = "__singleton_instance__" + func_name;
            auto cached_it = function_cache.find({cache_key, ""});
            if (cached_it != function_cache.end()) {
                last_value = cached_it->second;
                return;
            }
        }
        
        original_name = func_name;
        
        // Print deprecation warning if needed
        if (is_deprecated) {
            std::cout << "Warning: " << deprecation_message << std::endl;
        }
        
        // Check cache if @cache decorator is applied
        if (is_cached) {
            // Create cache key from function name and stringified arguments
            std::string args_str;
            for (size_t i = 0; i < arguments.size(); i++) {
                args_str += valueToString(arguments[i]);
                if (i < arguments.size() - 1) {
                    args_str += ",";
                }
            }
            auto cache_key = std::make_pair(original_name, args_str);
            auto it = function_cache.find(cache_key);
            if (it != function_cache.end()) {
                last_value = it->second;
                return;
            }
        }
        
        // Start timing if @timing decorator is applied
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Check argument count
        if (arguments.size() != func->parameters.size()) {
            throw TypeError("Function '" + original_name + "' expects " + 
                            std::to_string(func->parameters.size()) + 
                            " arguments but got " + 
                            std::to_string(arguments.size()));
        }
        
        // Create new environment for function execution
        auto func_env = std::make_shared<Environment>(func->closure);
        
        // Bind parameters to arguments
        for (size_t i = 0; i < func->parameters.size(); i++) {
            func_env->define(func->parameters[i].name, arguments[i], func->parameters[i].ownership);
        }
        
        // Push stack frame
        call_stack.emplace_back(original_name, "", -1, -1);
        
        // Save current environment
        auto previous_env = environment;
        environment = func_env;
        
        // Execute function body
        try {
            for (auto& stmt : *func->getBody()) {
                stmt->accept(*this);
            }
            // If no return statement, return null
            last_value = nullptr;
        } catch (const ReturnException& ret) {
            // Function returned a value
            last_value = ret.value;
        }
        
        // Pop stack frame
        if (!call_stack.empty()) {
            call_stack.pop_back();
        }
        
        // Restore previous environment
        environment = previous_env;
        
        // Store result in cache if @cache decorator is applied
        if (is_cached) {
            // Create cache key from function name and stringified arguments
            std::string args_str;
            for (size_t i = 0; i < arguments.size(); i++) {
                args_str += valueToString(arguments[i]);
                if (i < arguments.size() - 1) {
                    args_str += ",";
                }
            }
            auto cache_key = std::make_pair(original_name, args_str);
            function_cache[cache_key] = last_value;
        }

        // Store singleton instance if @singleton decorator is applied
        if (is_singleton) {
            std::string cache_key = "__singleton_instance__" + func_name;
            function_cache[{cache_key, ""}] = last_value;
        }
        
        // Print timing if @timing decorator is applied
        if (is_timed) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            std::cout << "Function '" << original_name << "' took " 
                      << duration.count() << " microseconds" << std::endl;
        }
        
        return;
    }
    
    // Handle bound methods
    if (std::holds_alternative<std::shared_ptr<BoundMethod>>(callee)) {
        auto bm = std::get<std::shared_ptr<BoundMethod>>(callee);
        auto func = bm->method;
        
        if (func->parameters.empty()) {
            throw TypeError("Method '" + func->name + "' must have at least a 'self' parameter");
        }
        
        // Expected arguments: all parameters except 'self'
        if (arguments.size() != func->parameters.size() - 1) {
            throw TypeError("Method '" + func->name + "' expects " + 
                            std::to_string(func->parameters.size() - 1) + 
                            " arguments but got " + 
                            std::to_string(arguments.size()));
        }
        
        auto func_env = std::make_shared<Environment>(func->closure);
        
        // Bind self
        func_env->define(func->parameters[0].name, bm->instance, func->parameters[0].ownership);
        // Bind remaining parameters
        for (size_t i = 1; i < func->parameters.size(); i++) {
            func_env->define(func->parameters[i].name, arguments[i - 1], func->parameters[i].ownership);
        }
        
        auto previous_env = environment;
        environment = func_env;
        
        try {
            for (auto& stmt : *func->getBody()) {
                stmt->accept(*this);
            }
            last_value = nullptr;
        } catch (const ReturnException& ret) {
            last_value = ret.value;
        }
        
        environment = previous_env;
        return;
    }
    
    // Handle extension methods (extend keyword)
    if (std::holds_alternative<std::shared_ptr<ExtensionMethod>>(callee)) {
        auto em = std::get<std::shared_ptr<ExtensionMethod>>(callee);
        auto func = em->method;
        
        if (func->parameters.empty()) {
            throw TypeError("Extension method '" + func->name + "' must have at least a 'self' parameter");
        }
        
        if (arguments.size() != func->parameters.size() - 1) {
            throw TypeError("Extension method '" + func->name + "' expects " + 
                            std::to_string(func->parameters.size() - 1) + 
                            " arguments but got " + 
                            std::to_string(arguments.size()));
        }
        
        auto func_env = std::make_shared<Environment>(func->closure);
        
        // Bind self to the object
        func_env->define(func->parameters[0].name, em->object, func->parameters[0].ownership);
        // Bind remaining parameters
        for (size_t i = 1; i < func->parameters.size(); i++) {
            func_env->define(func->parameters[i].name, arguments[i - 1], func->parameters[i].ownership);
        }
        
        auto previous_env = environment;
        environment = func_env;
        
        try {
            for (auto& stmt : *func->getBody()) {
                stmt->accept(*this);
            }
            last_value = nullptr;
        } catch (const ReturnException& ret) {
            last_value = ret.value;
        }
        
        environment = previous_env;
        return;
    }
    
    // Handle static methods
    if (std::holds_alternative<std::shared_ptr<StaticMethod>>(callee)) {
        auto sm = std::get<std::shared_ptr<StaticMethod>>(callee);
        auto func = sm->method;
        
        if (sm->is_classmethod) {
            // @classmethod: first parameter is the class itself
            if (func->parameters.empty()) {
                throw TypeError("Class method '" + func->name + "' must have at least a 'cls' parameter");
            }
            
            // Expected arguments: all parameters except 'cls'
            if (arguments.size() != func->parameters.size() - 1) {
                throw TypeError("Class method '" + func->name + "' expects " + 
                                std::to_string(func->parameters.size() - 1) + 
                                " arguments but got " + 
                                std::to_string(arguments.size()));
            }
            
            auto func_env = std::make_shared<Environment>(func->closure);
            
            // Bind cls (first parameter) to the class
            func_env->define(func->parameters[0].name, sm->klass, func->parameters[0].ownership);
            // Bind remaining parameters
            for (size_t i = 1; i < func->parameters.size(); i++) {
            func_env->define(func->parameters[i].name, arguments[i - 1], func->parameters[i].ownership);
            }
            
            auto previous_env = environment;
            environment = func_env;
            
            try {
                for (auto& stmt : *func->getBody()) {
                    stmt->accept(*this);
                }
                last_value = nullptr;
            } catch (const ReturnException& ret) {
                last_value = ret.value;
            }
            
            environment = previous_env;
            return;
        } else {
            // @staticmethod: no implicit first parameter
            if (arguments.size() != func->parameters.size()) {
                throw TypeError("Static method '" + func->name + "' expects " + 
                                std::to_string(func->parameters.size()) + 
                                " arguments but got " + 
                                std::to_string(arguments.size()));
            }
            
            auto func_env = std::make_shared<Environment>(func->closure);
            
            // Bind all parameters
            for (size_t i = 0; i < func->parameters.size(); i++) {
                func_env->define(func->parameters[i].name, arguments[i], func->parameters[i].ownership);
            }
            
            auto previous_env = environment;
            environment = func_env;
            
            try {
                for (auto& stmt : *func->getBody()) {
                    stmt->accept(*this);
                }
                last_value = nullptr;
            } catch (const ReturnException& ret) {
                last_value = ret.value;
            }
            
            environment = previous_env;
            return;
        }
    }
    
    // Handle class constructors
    if (std::holds_alternative<std::shared_ptr<Class>>(callee)) {
        auto cls = std::get<std::shared_ptr<Class>>(callee);
        auto instance = std::make_shared<Instance>(cls);
        
        // Look for __init__ method
        auto init = cls->findMethod("__init__");
        if (init) {
            if (init->parameters.empty()) {
                throw TypeError("Method '__init__' must have at least a 'self' parameter");
            }
            
            if (arguments.size() != init->parameters.size() - 1) {
                throw TypeError("Constructor for class '" + cls->name + "' expects " +
                                std::to_string(init->parameters.size() - 1) +
                                " arguments but got " +
                                std::to_string(arguments.size()));
            }
            
            auto func_env = std::make_shared<Environment>(init->closure);
            // Bind self
            func_env->define(init->parameters[0].name, instance, init->parameters[0].ownership);
            // Bind remaining parameters
            for (size_t i = 1; i < init->parameters.size(); i++) {
                func_env->define(init->parameters[i].name, arguments[i - 1], init->parameters[i].ownership);
            }
            
            auto previous_env = environment;
            environment = func_env;
            
            try {
                for (auto& stmt : *init->getBody()) {
                    stmt->accept(*this);
                }
            } catch (const ReturnException&) {
                // Ignore return value from __init__
            }
            
            environment = previous_env;
        }
        
        last_value = instance;
        return;
    }
    
    // Handle built-in functions
    if (std::holds_alternative<std::string>(callee)) {
        std::string func_name = std::get<std::string>(callee);
        
        if (func_name == "__builtin_print__") {
            for (size_t i = 0; i < arguments.size(); i++) {
                std::cout << valueToString(arguments[i]);
                if (i < arguments.size() - 1) {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
            last_value = nullptr;
        } else if (func_name == "__builtin_range__") {
            // Simple range implementation
            int end = std::get<int>(arguments[0]);
            last_value = end;
        } else if (func_name == "__builtin_len__") {
            // Get length of array or string
            if (arguments.empty()) {
                throw TypeError("len() requires 1 argument");
            }
            
            Value arg = arguments[0];
            if (std::holds_alternative<std::shared_ptr<ArrayValue>>(arg)) {
                auto array = std::get<std::shared_ptr<ArrayValue>>(arg);
                last_value = static_cast<int>(array->elements.size());
            } else if (std::holds_alternative<std::string>(arg)) {
                std::string str = std::get<std::string>(arg);
                last_value = static_cast<int>(str.length());
            } else {
                throw TypeError("len() requires an array or string");
            }
        } else if (func_name == "__builtin_input__") {
            // Get user input
            // Optional prompt argument
            if (!arguments.empty()) {
                std::cout << valueToString(arguments[0]);
            }
            
            std::string input_line;
            std::getline(std::cin, input_line);
            last_value = input_line;
        } else if (func_name == "__builtin_read_file__") {
            // Read file contents
            if (arguments.empty()) {
                throw TypeError("read_file() requires 1 argument (filename)");
            }
            
            std::string filename = std::get<std::string>(arguments[0]);
            std::ifstream file(filename);
            
            if (!file.is_open()) {
                throw FileNotFoundError("Failed to open file: " + filename);
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            
            last_value = buffer.str();
        } else if (func_name == "__builtin_write_file__") {
            // Write content to file
            if (arguments.size() < 2) {
                throw TypeError("write_file() requires 2 arguments (filename, content)");
            }
            
            std::string filename = std::get<std::string>(arguments[0]);
            std::string content = valueToString(arguments[1]);
            
            std::ofstream file(filename);
            
            if (!file.is_open()) {
                throw PermissionError("Failed to open file for writing: " + filename);
            }
            
            file << content;
            file.close();
            
            last_value = true;  // Return true on success
        } else if (func_name == "__builtin_time__") {
            // Return current time in seconds since epoch as double
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration);
            last_value = seconds.count();
        } else if (func_name == "__builtin_sleep__") {
            // Sleep for specified seconds (accepts int or double)
            if (arguments.empty()) {
                throw TypeError("sleep() requires 1 argument (seconds)");
            }
            
            double seconds = 0.0;
            if (std::holds_alternative<int>(arguments[0])) {
                seconds = static_cast<double>(std::get<int>(arguments[0]));
            } else if (std::holds_alternative<double>(arguments[0])) {
                seconds = std::get<double>(arguments[0]);
            } else {
                throw TypeError("sleep() requires numeric argument");
            }
            
            auto duration = std::chrono::duration<double>(seconds);
            std::this_thread::sleep_for(duration);
            last_value = nullptr;
        } else if (func_name == "__builtin_sort__") {
            // Fast native sort - O(n log n)
            if (arguments.empty()) {
                throw TypeError("sort() requires 1 argument (array)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("sort() requires an array");
            }
            
            auto array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            
            // Sort using C++ std::sort (highly optimized)
            std::sort(array->elements.begin(), array->elements.end(), 
                [](const Value& a, const Value& b) {
                    if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
                        return std::get<int>(a) < std::get<int>(b);
                    } else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
                        return std::get<double>(a) < std::get<double>(b);
                    } else if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
                        return std::get<std::string>(a) < std::get<std::string>(b);
                    }
                    return false;
                });
            
            last_value = array;
        } else if (func_name == "__builtin_make_array__") {
            // Fast array generation for benchmarking
            // make_array(size, mode) where mode: "random", "sorted", "reverse"
            if (arguments.empty()) {
                throw TypeError("make_array() requires at least 1 argument (size)");
            }
            
            int size = std::get<int>(arguments[0]);
            std::string mode = "random";
            if (arguments.size() > 1) {
                mode = std::get<std::string>(arguments[1]);
            }
            
            auto array = std::make_shared<ArrayValue>();
            array->elements.reserve(size);
            
            if (mode == "reverse") {
                for (int i = size; i > 0; i--) {
                    array->elements.push_back(i);
                }
            } else if (mode == "sorted") {
                for (int i = 1; i <= size; i++) {
                    array->elements.push_back(i);
                }
            } else {
                // Random
                srand(42);
                for (int i = 0; i < size; i++) {
                    array->elements.push_back(rand() % 100000);
                }
            }
            
            last_value = array;
        } else if (func_name == "__builtin_waitgroup__") {
            // Create a new WaitGroup
            auto wg = std::make_shared<WaitGroupValue>();
            last_value = wg;
            return;
        } else if (func_name == "__builtin_some__") {
            // Create Some(value) - Option with a value
            if (arguments.size() != 1) {
                throw TypeError("Some() requires exactly 1 argument");
            }
            auto option = std::make_shared<OptionValue>(arguments[0]);
            last_value = option;
            return;
        } else if (func_name == "__builtin_none__") {
            // Create None - empty Option
            if (!arguments.empty()) {
                throw TypeError("None() takes no arguments");
            }
            auto option = std::make_shared<OptionValue>();
            last_value = option;
            return;
        } else if (func_name == "__builtin_ok__") {
            // Create Ok(value) - successful Result
            if (arguments.size() != 1) {
                throw TypeError("Ok() requires exactly 1 argument");
            }
            auto result = std::make_shared<ResultValue>(true, arguments[0]);
            last_value = result;
            return;
        } else if (func_name == "__builtin_err__") {
            // Create Err(error) - error Result
            if (arguments.size() != 1) {
                throw TypeError("Err() requires exactly 1 argument");
            }
            auto result = std::make_shared<ResultValue>(false, arguments[0]);
            last_value = result;
            return;
        } else if (func_name == "__builtin_rc__") {
            // Create Rc(value) - reference counted pointer
            if (arguments.size() != 1) {
                throw TypeError("Rc() requires exactly 1 argument");
            }
            // If argument is already an Rc, share the same level of indirection
            if (std::holds_alternative<std::shared_ptr<RcValue>>(arguments[0])) {
                auto existing = std::get<std::shared_ptr<RcValue>>(arguments[0]);
                auto rc = std::make_shared<RcValue>(existing->inner);
                last_value = rc;
                return;
            }
            auto rc = std::make_shared<RcValue>(arguments[0]);
            last_value = rc;
            return;
        } else if (func_name == "__builtin_arc__") {
            // Create Arc(value) - atomic reference counted pointer
            if (arguments.size() != 1) {
                throw TypeError("Arc() requires exactly 1 argument");
            }
            // If argument is already an Arc, share the same level of indirection
            if (std::holds_alternative<std::shared_ptr<ArcValue>>(arguments[0])) {
                auto existing = std::get<std::shared_ptr<ArcValue>>(arguments[0]);
                auto arc = std::make_shared<ArcValue>(existing->inner);
                last_value = arc;
                return;
            }
            auto arc = std::make_shared<ArcValue>(arguments[0]);
            last_value = arc;
            return;
        } else if (func_name == "__builtin_weak__") {
            // Create Weak(rc_or_arc) - weak reference
            if (arguments.size() != 1) {
                throw TypeError("Weak() requires exactly 1 argument");
            }
            if (std::holds_alternative<std::shared_ptr<RcValue>>(arguments[0])) {
                auto rc = std::get<std::shared_ptr<RcValue>>(arguments[0]);
                auto weak = std::make_shared<WeakValue>(rc);
                last_value = weak;
                return;
            } else if (std::holds_alternative<std::shared_ptr<ArcValue>>(arguments[0])) {
                auto arc = std::get<std::shared_ptr<ArcValue>>(arguments[0]);
                auto weak = std::make_shared<WeakValue>(arc);
                last_value = weak;
                return;
            } else {
                throw TypeError("Weak() requires an Rc or Arc argument");
            }
        } else if (func_name == "__builtin_map__") {
            // map(array, function) - Transform each element
            if (arguments.size() != 2) {
                throw TypeError("map() requires 2 arguments (array, function)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("map() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto func = arguments[1];
            
            auto result_array = std::make_shared<ArrayValue>();
            for (const auto& elem : input_array->elements) {
                // Call the function with the element
                std::vector<Value> args = {elem};
                
                // Save current last_value
                Value saved = last_value;
                
                // Create a CallExpr to call the function
                if (std::holds_alternative<std::shared_ptr<Function>>(func)) {
                    auto fn = std::get<std::shared_ptr<Function>>(func);
                    
                    // Create new environment for function call
                    auto fn_env = std::make_shared<Environment>(fn->closure);
                    
                    // Bind parameters
                    if (fn->parameters.size() != 1) {
                        throw TypeError("map() function must take exactly 1 argument");
                    }
                    fn_env->define(fn->parameters[0].name, elem, fn->parameters[0].ownership);
                    
                    // Execute function body
                    auto previous_env = environment;
                    environment = fn_env;
                    
                    try {
                        for (auto& stmt : *fn->getBody()) {
                            stmt->accept(*this);
                        }
                    } catch (const ReturnException& e) {
                        result_array->elements.push_back(e.value);
                        environment = previous_env;
                        continue;
                    }
                    
                    environment = previous_env;
                    result_array->elements.push_back(last_value);
                } else {
                    throw TypeError("map() second argument must be a function");
                }
                
                last_value = saved;
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_filter__") {
            // filter(array, predicate) - Keep elements that match
            if (arguments.size() != 2) {
                throw TypeError("filter() requires 2 arguments (array, predicate)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("filter() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto func = arguments[1];
            
            auto result_array = std::make_shared<ArrayValue>();
            
            if (std::holds_alternative<std::shared_ptr<Function>>(func)) {
                auto fn = std::get<std::shared_ptr<Function>>(func);
                
                for (const auto& elem : input_array->elements) {
                    // Create new environment for function call
                    auto fn_env = std::make_shared<Environment>(fn->closure);
                    
                    // Bind parameters
                    if (fn->parameters.size() != 1) {
                        throw TypeError("filter() predicate must take exactly 1 argument");
                    }
                    fn_env->define(fn->parameters[0].name, elem, fn->parameters[0].ownership);
                    
                    // Execute function body
                    auto previous_env = environment;
                    environment = fn_env;
                    
                    Value result_val;
                    try {
                        for (auto& stmt : *fn->getBody()) {
                            stmt->accept(*this);
                        }
                        result_val = last_value;
                    } catch (const ReturnException& e) {
                        result_val = e.value;
                    }
                    
                    environment = previous_env;
                    
                    // Check if result is truthy
                    if (isTruthy(result_val)) {
                        result_array->elements.push_back(elem);
                    }
                }
            } else {
                throw TypeError("filter() second argument must be a function");
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_reduce__") {
            // reduce(array, initial, function) - Combine elements
            if (arguments.size() != 3) {
                throw TypeError("reduce() requires 3 arguments (array, initial, function)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("reduce() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            Value accumulator = arguments[1];
            auto func = arguments[2];
            
            if (std::holds_alternative<std::shared_ptr<Function>>(func)) {
                auto fn = std::get<std::shared_ptr<Function>>(func);
                
                if (fn->parameters.size() != 2) {
                    throw TypeError("reduce() function must take exactly 2 arguments");
                }
                
                for (const auto& elem : input_array->elements) {
                    // Create new environment for function call
                    auto fn_env = std::make_shared<Environment>(fn->closure);
                    
                    // Bind parameters (accumulator, element)
                    fn_env->define(fn->parameters[0].name, accumulator, fn->parameters[0].ownership);
                    fn_env->define(fn->parameters[1].name, elem, fn->parameters[1].ownership);
                    
                    // Execute function body
                    auto previous_env = environment;
                    environment = fn_env;
                    
                    try {
                        for (auto& stmt : *fn->getBody()) {
                            stmt->accept(*this);
                        }
                        accumulator = last_value;
                    } catch (const ReturnException& e) {
                        accumulator = e.value;
                    }
                    
                    environment = previous_env;
                }
            } else {
                throw TypeError("reduce() third argument must be a function");
            }
            
            last_value = accumulator;
            return;
        } else if (func_name == "__builtin_take__") {
            // take(array, n) - Take first n elements
            if (arguments.size() != 2) {
                throw TypeError("take() requires 2 arguments (array, n)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("take() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            int n = std::get<int>(arguments[1]);
            
            auto result_array = std::make_shared<ArrayValue>();
            size_t count = std::min(static_cast<size_t>(n), input_array->elements.size());
            
            for (size_t i = 0; i < count; i++) {
                result_array->elements.push_back(input_array->elements[i]);
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_drop__") {
            // drop(array, n) - Skip first n elements
            if (arguments.size() != 2) {
                throw TypeError("drop() requires 2 arguments (array, n)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("drop() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            int n = std::get<int>(arguments[1]);
            
            auto result_array = std::make_shared<ArrayValue>();
            size_t start = std::min(static_cast<size_t>(n), input_array->elements.size());
            
            for (size_t i = start; i < input_array->elements.size(); i++) {
                result_array->elements.push_back(input_array->elements[i]);
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_reverse__") {
            // reverse(array) - Reverse array order
            if (arguments.size() != 1) {
                throw TypeError("reverse() requires 1 argument (array)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("reverse() argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto result_array = std::make_shared<ArrayValue>();
            
            for (auto it = input_array->elements.rbegin(); it != input_array->elements.rend(); ++it) {
                result_array->elements.push_back(*it);
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_unique__") {
            // unique(array) - Remove duplicates
            if (arguments.size() != 1) {
                throw TypeError("unique() requires 1 argument (array)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("unique() argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto result_array = std::make_shared<ArrayValue>();
            std::set<std::string> seen;  // Use string representation for comparison
            
            for (const auto& elem : input_array->elements) {
                std::string key = valueToString(elem);
                if (seen.find(key) == seen.end()) {
                    result_array->elements.push_back(elem);
                    seen.insert(key);
                }
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_enumerate__") {
            // enumerate(array) - Add indices to elements, returns array of [index, value]
            if (arguments.size() != 1) {
                throw TypeError("enumerate() requires 1 argument (array)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("enumerate() argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto result_array = std::make_shared<ArrayValue>();
            
            for (size_t i = 0; i < input_array->elements.size(); i++) {
                auto pair = std::make_shared<ArrayValue>();
                pair->elements.push_back(static_cast<int>(i));
                pair->elements.push_back(input_array->elements[i]);
                result_array->elements.push_back(pair);
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_zip__") {
            // zip(array1, array2) - Combine two arrays into pairs
            if (arguments.size() != 2) {
                throw TypeError("zip() requires 2 arguments (array1, array2)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0]) ||
                !std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[1])) {
                throw TypeError("zip() arguments must be arrays");
            }
            
            auto array1 = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto array2 = std::get<std::shared_ptr<ArrayValue>>(arguments[1]);
            auto result_array = std::make_shared<ArrayValue>();
            
            size_t min_size = std::min(array1->elements.size(), array2->elements.size());
            for (size_t i = 0; i < min_size; i++) {
                auto pair = std::make_shared<ArrayValue>();
                pair->elements.push_back(array1->elements[i]);
                pair->elements.push_back(array2->elements[i]);
                result_array->elements.push_back(pair);
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_find__") {
            // find(array, predicate) - Find first element matching predicate, returns Option
            if (arguments.size() != 2) {
                throw TypeError("find() requires 2 arguments (array, predicate)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("find() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto func = arguments[1];
            
            if (std::holds_alternative<std::shared_ptr<Function>>(func)) {
                auto fn = std::get<std::shared_ptr<Function>>(func);
                
                for (const auto& elem : input_array->elements) {
                    // Create new environment for function call
                    auto fn_env = std::make_shared<Environment>(fn->closure);
                    
                    if (fn->parameters.size() != 1) {
                        throw TypeError("find() predicate must take exactly 1 argument");
                    }
                    fn_env->define(fn->parameters[0].name, elem, fn->parameters[0].ownership);
                    
                    // Execute function body
                    auto previous_env = environment;
                    environment = fn_env;
                    
                    Value result_val;
                    try {
                        for (auto& stmt : *fn->getBody()) {
                            stmt->accept(*this);
                        }
                        result_val = last_value;
                    } catch (const ReturnException& e) {
                        result_val = e.value;
                    }
                    
                    environment = previous_env;
                    
                    // If predicate returns true, return Some(elem)
                    if (isTruthy(result_val)) {
                        auto option = std::make_shared<OptionValue>(elem);
                        last_value = option;
                        return;
                    }
                }
            } else {
                throw TypeError("find() second argument must be a function");
            }
            
            // Not found - return None
            auto option = std::make_shared<OptionValue>();
            last_value = option;
            return;
        } else if (func_name == "__builtin_all__") {
            // all(array, predicate) - Check if all elements match
            if (arguments.size() != 2) {
                throw TypeError("all() requires 2 arguments (array, predicate)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("all() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto func = arguments[1];
            
            if (std::holds_alternative<std::shared_ptr<Function>>(func)) {
                auto fn = std::get<std::shared_ptr<Function>>(func);
                
                for (const auto& elem : input_array->elements) {
                    auto fn_env = std::make_shared<Environment>(fn->closure);
                    
                    if (fn->parameters.size() != 1) {
                        throw TypeError("all() predicate must take exactly 1 argument");
                    }
                    fn_env->define(fn->parameters[0].name, elem, fn->parameters[0].ownership);
                    
                    auto previous_env = environment;
                    environment = fn_env;
                    
                    Value result_val;
                    try {
                        for (auto& stmt : *fn->getBody()) {
                            stmt->accept(*this);
                        }
                        result_val = last_value;
                    } catch (const ReturnException& e) {
                        result_val = e.value;
                    }
                    
                    environment = previous_env;
                    
                    // If any element doesn't match, return false
                    if (!isTruthy(result_val)) {
                        last_value = false;
                        return;
                    }
                }
            } else {
                throw TypeError("all() second argument must be a function");
            }
            
            last_value = true;
            return;
        } else if (func_name == "__builtin_any__") {
            // any(array, predicate) - Check if any element matches
            if (arguments.size() != 2) {
                throw TypeError("any() requires 2 arguments (array, predicate)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("any() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto func = arguments[1];
            
            if (std::holds_alternative<std::shared_ptr<Function>>(func)) {
                auto fn = std::get<std::shared_ptr<Function>>(func);
                
                for (const auto& elem : input_array->elements) {
                    auto fn_env = std::make_shared<Environment>(fn->closure);
                    
                    if (fn->parameters.size() != 1) {
                        throw TypeError("any() predicate must take exactly 1 argument");
                    }
                    fn_env->define(fn->parameters[0].name, elem, fn->parameters[0].ownership);
                    
                    auto previous_env = environment;
                    environment = fn_env;
                    
                    Value result_val;
                    try {
                        for (auto& stmt : *fn->getBody()) {
                            stmt->accept(*this);
                        }
                        result_val = last_value;
                    } catch (const ReturnException& e) {
                        result_val = e.value;
                    }
                    
                    environment = previous_env;
                    
                    // If any element matches, return true
                    if (isTruthy(result_val)) {
                        last_value = true;
                        return;
                    }
                }
            } else {
                throw TypeError("any() second argument must be a function");
            }
            
            last_value = false;
            return;
        } else if (func_name == "__builtin_chunk__") {
            // chunk(array, n) - Split into chunks of size n
            if (arguments.size() != 2) {
                throw TypeError("chunk() requires 2 arguments (array, n)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("chunk() first argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            int n = std::get<int>(arguments[1]);
            
            if (n <= 0) {
                throw ValueError("chunk() size must be positive");
            }
            
            auto result_array = std::make_shared<ArrayValue>();
            
            for (size_t i = 0; i < input_array->elements.size(); i += n) {
                auto chunk = std::make_shared<ArrayValue>();
                size_t end = std::min(i + n, input_array->elements.size());
                
                for (size_t j = i; j < end; j++) {
                    chunk->elements.push_back(input_array->elements[j]);
                }
                
                result_array->elements.push_back(chunk);
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_flatten__") {
            // flatten(array) - Flatten nested arrays
            if (arguments.size() != 1) {
                throw TypeError("flatten() requires 1 argument (array)");
            }
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                throw TypeError("flatten() argument must be an array");
            }
            
            auto input_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto result_array = std::make_shared<ArrayValue>();
            
            for (const auto& elem : input_array->elements) {
                if (std::holds_alternative<std::shared_ptr<ArrayValue>>(elem)) {
                    auto inner = std::get<std::shared_ptr<ArrayValue>>(elem);
                    for (const auto& inner_elem : inner->elements) {
                        result_array->elements.push_back(inner_elem);
                    }
                } else {
                    result_array->elements.push_back(elem);
                }
            }
            
            last_value = result_array;
            return;
        } else if (func_name == "__builtin_assert_true__") {
            // assert_true(condition, message?)
            if (arguments.empty()) {
                throw TypeError("assert_true() requires at least 1 argument");
            }
            
            bool condition = isTruthy(arguments[0]);
            std::string message = arguments.size() > 1 ? valueToString(arguments[1]) : "Assertion failed";
            
            if (!condition) {
                throw RuntimeError("AssertionError: " + message);
            }
            
            last_value = nullptr;
            return;
        } else if (func_name == "__builtin_assert_false__") {
            // assert_false(condition, message?)
            if (arguments.empty()) {
                throw TypeError("assert_false() requires at least 1 argument");
            }
            
            bool condition = isTruthy(arguments[0]);
            std::string message = arguments.size() > 1 ? valueToString(arguments[1]) : "Assertion failed";
            
            if (condition) {
                throw RuntimeError("AssertionError: " + message);
            }
            
            last_value = nullptr;
            return;
        } else if (func_name == "__builtin_assert_equal__") {
            // assert_equal(actual, expected, message?)
            if (arguments.size() < 2) {
                throw TypeError("assert_equal() requires at least 2 arguments");
            }
            
            bool equal = valuesEqual(arguments[0], arguments[1]);
            std::string message = arguments.size() > 2 ? valueToString(arguments[2]) : 
                "Expected " + valueToString(arguments[1]) + " but got " + valueToString(arguments[0]);
            
            if (!equal) {
                throw RuntimeError("AssertionError: " + message);
            }
            
            last_value = nullptr;
            return;
        } else if (func_name == "__builtin_typeof__") {
            // typeof(value) - Get type as string
            if (arguments.size() != 1) {
                throw TypeError("typeof() requires exactly 1 argument");
            }
            
            std::string type_str;
            const auto& val = arguments[0];
            
            if (std::holds_alternative<int>(val)) {
                type_str = "int";
            } else if (std::holds_alternative<double>(val)) {
                type_str = "float";
            } else if (std::holds_alternative<std::string>(val)) {
                type_str = "str";
            } else if (std::holds_alternative<bool>(val)) {
                type_str = "bool";
            } else if (std::holds_alternative<std::nullptr_t>(val)) {
                type_str = "none";
            } else if (std::holds_alternative<std::shared_ptr<ArrayValue>>(val)) {
                type_str = "array";
            } else if (std::holds_alternative<std::shared_ptr<Function>>(val)) {
                type_str = "function";
            } else if (std::holds_alternative<std::shared_ptr<Class>>(val)) {
                type_str = "class";
            } else if (std::holds_alternative<std::shared_ptr<Instance>>(val)) {
                auto inst = std::get<std::shared_ptr<Instance>>(val);
                type_str = inst->klass->name;
            } else if (std::holds_alternative<std::shared_ptr<ChannelValue>>(val)) {
                type_str = "channel";
            } else if (std::holds_alternative<std::shared_ptr<WaitGroupValue>>(val)) {
                type_str = "waitgroup";
            } else if (std::holds_alternative<std::shared_ptr<OptionValue>>(val)) {
                type_str = "option";
            } else if (std::holds_alternative<std::shared_ptr<ResultValue>>(val)) {
                type_str = "result";
            } else {
                type_str = "unknown";
            }
            
            last_value = type_str;
            return;
        } else if (func_name == "__builtin_type_name__") {
            // type_name(value) - Alias for typeof
            if (arguments.size() != 1) {
                throw TypeError("type_name() requires exactly 1 argument");
            }
            
            // Reuse typeof implementation
            std::vector<Value> args = {arguments[0]};
            arguments = args;
            func_name = "__builtin_typeof__";
            // Fall through to typeof implementation above
            
            std::string type_str;
            const auto& val = arguments[0];
            
            if (std::holds_alternative<int>(val)) {
                type_str = "int";
            } else if (std::holds_alternative<double>(val)) {
                type_str = "float";
            } else if (std::holds_alternative<std::string>(val)) {
                type_str = "str";
            } else if (std::holds_alternative<bool>(val)) {
                type_str = "bool";
            } else if (std::holds_alternative<std::nullptr_t>(val)) {
                type_str = "none";
            } else if (std::holds_alternative<std::shared_ptr<ArrayValue>>(val)) {
                type_str = "array";
            } else if (std::holds_alternative<std::shared_ptr<Function>>(val)) {
                type_str = "function";
            } else if (std::holds_alternative<std::shared_ptr<Class>>(val)) {
                type_str = "class";
            } else if (std::holds_alternative<std::shared_ptr<Instance>>(val)) {
                auto inst = std::get<std::shared_ptr<Instance>>(val);
                type_str = inst->klass->name;
            } else if (std::holds_alternative<std::shared_ptr<ChannelValue>>(val)) {
                type_str = "channel";
            } else if (std::holds_alternative<std::shared_ptr<WaitGroupValue>>(val)) {
                type_str = "waitgroup";
            } else if (std::holds_alternative<std::shared_ptr<OptionValue>>(val)) {
                type_str = "option";
            } else if (std::holds_alternative<std::shared_ptr<ResultValue>>(val)) {
                type_str = "result";
            } else {
                type_str = "unknown";
            }
            
            last_value = type_str;
            return;
        } else if (func_name == "__builtin_fields__") {
            // fields(object) - Get object fields as array
            if (arguments.size() != 1) {
                throw TypeError("fields() requires exactly 1 argument");
            }
            
            if (!std::holds_alternative<std::shared_ptr<Instance>>(arguments[0])) {
                throw TypeError("fields() requires an object instance");
            }
            
            auto inst = std::get<std::shared_ptr<Instance>>(arguments[0]);
            auto result = std::make_shared<ArrayValue>();
            
            for (const auto& [field_name, field_value] : inst->fields) {
                result->elements.push_back(field_name);
            }
            
            last_value = result;
            return;
        } else if (func_name == "__builtin_methods__") {
            // methods(object) - Get object methods as array
            if (arguments.size() != 1) {
                throw TypeError("methods() requires exactly 1 argument");
            }
            
            if (!std::holds_alternative<std::shared_ptr<Instance>>(arguments[0])) {
                throw TypeError("methods() requires an object instance");
            }
            
            auto inst = std::get<std::shared_ptr<Instance>>(arguments[0]);
            auto result = std::make_shared<ArrayValue>();
            
            for (const auto& [method_name, method] : inst->klass->methods) {
                result->elements.push_back(method_name);
            }
            
            last_value = result;
            return;
        } else if (func_name == "__builtin_is_type__") {
            // is_type(value, type_name) - Check if value is of given type
            if (arguments.size() != 2) {
                throw TypeError("is_type() requires exactly 2 arguments");
            }
            
            if (!std::holds_alternative<std::string>(arguments[1])) {
                throw TypeError("is_type() second argument must be a string");
            }
            
            std::string expected_type = std::get<std::string>(arguments[1]);
            const auto& val = arguments[0];
            bool matches = false;
            
            if (expected_type == "int" && std::holds_alternative<int>(val)) {
                matches = true;
            } else if (expected_type == "float" && std::holds_alternative<double>(val)) {
                matches = true;
            } else if (expected_type == "str" && std::holds_alternative<std::string>(val)) {
                matches = true;
            } else if (expected_type == "bool" && std::holds_alternative<bool>(val)) {
                matches = true;
            } else if (expected_type == "none" && std::holds_alternative<std::nullptr_t>(val)) {
                matches = true;
            } else if (expected_type == "array" && std::holds_alternative<std::shared_ptr<ArrayValue>>(val)) {
                matches = true;
            } else if (expected_type == "function" && std::holds_alternative<std::shared_ptr<Function>>(val)) {
                matches = true;
            } else if (expected_type == "class" && std::holds_alternative<std::shared_ptr<Class>>(val)) {
                matches = true;
            } else if (std::holds_alternative<std::shared_ptr<Instance>>(val)) {
                auto inst = std::get<std::shared_ptr<Instance>>(val);
                matches = (inst->klass->name == expected_type);
            }
            
            last_value = matches;
            return;
        } else if (func_name == "__builtin_has_field__") {
            // has_field(object, field_name) - Check if object has field
            if (arguments.size() != 2) {
                throw TypeError("has_field() requires exactly 2 arguments");
            }
            
            if (!std::holds_alternative<std::shared_ptr<Instance>>(arguments[0])) {
                last_value = false;
                return;
            }
            
            if (!std::holds_alternative<std::string>(arguments[1])) {
                throw TypeError("has_field() second argument must be a string");
            }
            
            auto inst = std::get<std::shared_ptr<Instance>>(arguments[0]);
            std::string field_name = std::get<std::string>(arguments[1]);
            
            last_value = (inst->fields.find(field_name) != inst->fields.end());
            return;
        } else if (func_name == "__builtin_has_method__") {
            // has_method(object, method_name) - Check if object has method
            if (arguments.size() != 2) {
                throw TypeError("has_method() requires exactly 2 arguments");
            }
            
            if (!std::holds_alternative<std::shared_ptr<Instance>>(arguments[0])) {
                last_value = false;
                return;
            }
            
            if (!std::holds_alternative<std::string>(arguments[1])) {
                throw TypeError("has_method() second argument must be a string");
            }
            
            auto inst = std::get<std::shared_ptr<Instance>>(arguments[0]);
            std::string method_name = std::get<std::string>(arguments[1]);
            
            last_value = (inst->klass->methods.find(method_name) != inst->klass->methods.end());
            return;
        
        // ===== MILESTONE 1: DYNAMIC ARRAYS =====
        } else if (func_name == "__builtin_array_create__") {
            // Create a new dynamic array
            void* array = sapphire_array_create();
            // Store as a raw pointer wrapped as an int (this is a temporary solution)
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(array));
            return;
            
        } else if (func_name == "__builtin_array_push__") {
            // Push element to array
            if (arguments.size() != 2) {
                throw TypeError("array_push() requires 2 arguments (array, value)");
            }
            
            void* array = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            
            if (std::holds_alternative<int>(arguments[1])) {
                sapphire_array_push_int(array, std::get<int>(arguments[1]));
            } else if (std::holds_alternative<double>(arguments[1])) {
                sapphire_array_push_double(array, std::get<double>(arguments[1]));
            } else if (std::holds_alternative<std::string>(arguments[1])) {
                sapphire_array_push_string(array, std::get<std::string>(arguments[1]).c_str());
            }
            
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_array_get__") {
            // Get element from array
            if (arguments.size() != 2) {
                throw TypeError("array_get() requires 2 arguments (array, index)");
            }
            
            void* array = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int index = std::get<int>(arguments[1]);
            
            // For now, assume integer arrays - we'll need type information later
            last_value = static_cast<int>(sapphire_array_get_int(array, index));
            return;
            
        } else if (func_name == "__builtin_array_size__") {
            // Get array size
            if (arguments.size() != 1) {
                throw TypeError("array_size() requires 1 argument (array)");
            }
            
            void* array = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = static_cast<int>(sapphire_array_size(array));
            return;
            
        // ===== MILESTONE 1: MATH LIBRARY =====
        } else if (func_name == "__builtin_math_sqrt__") {
            if (arguments.size() != 1) {
                throw TypeError("math_sqrt() requires 1 argument");
            }
            double x = std::holds_alternative<int>(arguments[0]) ? 
                      std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            last_value = sapphire_math_sqrt(x);
            return;
            
        } else if (func_name == "__builtin_math_pow__") {
            if (arguments.size() != 2) {
                throw TypeError("math_pow() requires 2 arguments");
            }
            double x = std::holds_alternative<int>(arguments[0]) ? 
                      std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            double y = std::holds_alternative<int>(arguments[1]) ? 
                      std::get<int>(arguments[1]) : std::get<double>(arguments[1]);
            last_value = sapphire_math_pow(x, y);
            return;
            
        } else if (func_name == "__builtin_math_sin__") {
            if (arguments.size() != 1) {
                throw TypeError("math_sin() requires 1 argument");
            }
            double x = std::holds_alternative<int>(arguments[0]) ? 
                      std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            last_value = sapphire_math_sin(x);
            return;
            
        } else if (func_name == "__builtin_math_cos__") {
            if (arguments.size() != 1) {
                throw TypeError("math_cos() requires 1 argument");
            }
            double x = std::holds_alternative<int>(arguments[0]) ? 
                      std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            last_value = sapphire_math_cos(x);
            return;
            
        } else if (func_name == "__builtin_math_abs__") {
            if (arguments.size() != 1) {
                throw TypeError("math_abs() requires 1 argument");
            }
            double x = std::holds_alternative<int>(arguments[0]) ? 
                      std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            last_value = sapphire_math_abs(x);
            return;
            
        } else if (func_name == "__builtin_math_floor__") {
            if (arguments.size() != 1) {
                throw TypeError("math_floor() requires 1 argument");
            }
            double x = std::holds_alternative<int>(arguments[0]) ? 
                      std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            last_value = static_cast<int>(sapphire_math_floor(x));
            return;
            
        } else if (func_name == "__builtin_math_ceil__") {
            if (arguments.size() != 1) {
                throw TypeError("math_ceil() requires 1 argument");
            }
            double x = std::holds_alternative<int>(arguments[0]) ? 
                      std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            last_value = sapphire_math_ceil(x);
            return;
            
        } else if (func_name == "__builtin_math_pi__") {
            if (arguments.size() != 0) {
                throw TypeError("math_pi() requires 0 arguments");
            }
            last_value = sapphire_math_pi();
            return;
            
        } else if (func_name == "__builtin_math_e__") {
            if (arguments.size() != 0) {
                throw TypeError("math_e() requires 0 arguments");
            }
            last_value = sapphire_math_e();
            return;

        } else if (func_name == "__builtin_math_atan2__") {
            double y = std::holds_alternative<int>(arguments[0]) ? std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            double x = std::holds_alternative<int>(arguments[1]) ? std::get<int>(arguments[1]) : std::get<double>(arguments[1]);
            last_value = std::atan2(y, x);
            return;

        } else if (func_name == "__builtin_math_clamp__") {
            double v   = std::holds_alternative<int>(arguments[0]) ? std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            double lo  = std::holds_alternative<int>(arguments[1]) ? std::get<int>(arguments[1]) : std::get<double>(arguments[1]);
            double hi  = std::holds_alternative<int>(arguments[2]) ? std::get<int>(arguments[2]) : std::get<double>(arguments[2]);
            last_value = v < lo ? lo : (v > hi ? hi : v);
            return;

        } else if (func_name == "__builtin_math_min__") {
            double a = std::holds_alternative<int>(arguments[0]) ? std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            double b = std::holds_alternative<int>(arguments[1]) ? std::get<int>(arguments[1]) : std::get<double>(arguments[1]);
            last_value = a < b ? a : b;
            return;

        } else if (func_name == "__builtin_math_max__") {
            double a = std::holds_alternative<int>(arguments[0]) ? std::get<int>(arguments[0]) : std::get<double>(arguments[0]);
            double b = std::holds_alternative<int>(arguments[1]) ? std::get<int>(arguments[1]) : std::get<double>(arguments[1]);
            last_value = a > b ? a : b;
            return;
            
        // ===== MILESTONE 1: CSV I/O =====
        } else if (func_name == "__builtin_csv_create__") {
            // Create a new CSV data structure
            void* csv = sapphire_csv_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(csv));
            return;
            
        } else if (func_name == "__builtin_csv_read_file__") {
            if (arguments.size() != 1) {
                throw TypeError("csv_read_file() requires 1 argument (filename)");
            }
            
            std::string filename = std::get<std::string>(arguments[0]);
            void* csv = sapphire_csv_read_file(filename.c_str(), true);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(csv));
            return;
            
        } else if (func_name == "__builtin_csv_row_count__") {
            if (arguments.size() != 1) {
                throw TypeError("csv_row_count() requires 1 argument (csv)");
            }
            
            void* csv = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = static_cast<int>(sapphire_csv_row_count(csv));
            return;
            
        } else if (func_name == "__builtin_csv_get__") {
            if (arguments.size() != 3) {
                throw TypeError("csv_get() requires 3 arguments (csv, row, col)");
            }
            
            void* csv = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int row = std::get<int>(arguments[1]);
            int col = std::get<int>(arguments[2]);
            
            const char* value = sapphire_csv_get(csv, row, col);
            last_value = std::string(value ? value : "");
            return;
            
        // ===== MILESTONE 2: JSON SUPPORT =====
        } else if (func_name == "__builtin_json_parse__") {
            if (arguments.size() != 1) {
                throw TypeError("json_parse() requires 1 argument (json_string)");
            }
            
            std::string json_text = std::get<std::string>(arguments[0]);
            void* json_value = sapphire_json_parse(json_text.c_str());
            if (json_value == nullptr) {
                throw RuntimeError("Failed to parse JSON string");
            }
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(json_value));
            return;
            
        } else if (func_name == "__builtin_json_stringify__") {
            if (arguments.size() < 1 || arguments.size() > 2) {
                throw TypeError("json_stringify() requires 1-2 arguments (json_value, pretty=false)");
            }
            
            void* json_value = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            bool pretty = arguments.size() > 1 ? std::get<bool>(arguments[1]) : false;
            
            const char* json_string = sapphire_json_stringify(json_value, pretty);
            last_value = std::string(json_string ? json_string : "null");
            return;
            
        } else if (func_name == "__builtin_json_create_object__") {
            if (arguments.size() != 0) {
                throw TypeError("json_create_object() requires 0 arguments");
            }
            
            void* json_object = sapphire_json_create_object();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(json_object));
            return;
            
        } else if (func_name == "__builtin_json_create_array__") {
            if (arguments.size() != 0) {
                throw TypeError("json_create_array() requires 0 arguments");
            }
            
            void* json_array = sapphire_json_create_array();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(json_array));
            return;
            
        } else if (func_name == "__builtin_json_get__") {
            if (arguments.size() != 2) {
                throw TypeError("json_get() requires 2 arguments (json_object, key)");
            }
            
            void* json_object = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string key = std::get<std::string>(arguments[1]);
            
            void* result = sapphire_json_object_get(json_object, key.c_str());
            if (result == nullptr) {
                last_value = nullptr;
            } else {
                last_value = static_cast<int>(reinterpret_cast<intptr_t>(result));
            }
            return;
            
        } else if (func_name == "__builtin_json_set__") {
            if (arguments.size() != 3) {
                throw TypeError("json_set() requires 3 arguments (json_object, key, value)");
            }
            
            void* json_object = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string key = std::get<std::string>(arguments[1]);
            
            // Convert Sapphire value to JSON value
            void* json_value = nullptr;
            if (std::holds_alternative<std::nullptr_t>(arguments[2])) {
                json_value = sapphire_json_create_null();
            } else if (std::holds_alternative<bool>(arguments[2])) {
                json_value = sapphire_json_create_bool(std::get<bool>(arguments[2]));
            } else if (std::holds_alternative<int>(arguments[2])) {
                json_value = sapphire_json_create_number(static_cast<double>(std::get<int>(arguments[2])));
            } else if (std::holds_alternative<double>(arguments[2])) {
                json_value = sapphire_json_create_number(std::get<double>(arguments[2]));
            } else if (std::holds_alternative<std::string>(arguments[2])) {
                json_value = sapphire_json_create_string(std::get<std::string>(arguments[2]).c_str());
            } else {
                throw TypeError("Unsupported value type for JSON");
            }
            
            sapphire_json_object_set(json_object, key.c_str(), json_value);
            last_value = arguments[2];
            return;
            
        // ===== JSON PUSH =====
        } else if (func_name == "__builtin_json_push__") {
            if (arguments.size() != 2) {
                throw TypeError("json_push() requires 2 arguments (array, value)");
            }
            void* json_array = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            void* json_value = nullptr;
            if (std::holds_alternative<std::nullptr_t>(arguments[1])) {
                json_value = sapphire_json_create_null();
            } else if (std::holds_alternative<bool>(arguments[1])) {
                json_value = sapphire_json_create_bool(std::get<bool>(arguments[1]));
            } else if (std::holds_alternative<int>(arguments[1])) {
                json_value = sapphire_json_create_number(static_cast<double>(std::get<int>(arguments[1])));
            } else if (std::holds_alternative<double>(arguments[1])) {
                json_value = sapphire_json_create_number(std::get<double>(arguments[1]));
            } else if (std::holds_alternative<std::string>(arguments[1])) {
                json_value = sapphire_json_create_string(std::get<std::string>(arguments[1]).c_str());
            } else {
                throw TypeError("Unsupported value type for json_push");
            }
            sapphire_json_array_push(json_array, json_value);
            last_value = nullptr;
            return;

        // ===== JSON SIZE =====
        } else if (func_name == "__builtin_json_size__") {
            if (arguments.size() != 1) {
                throw TypeError("json_size() requires 1 argument (json_value)");
            }
            void* json_value = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int64_t size = sapphire_json_array_size(json_value);
            last_value = static_cast<int>(size);
            return;

        // ===== JSON LOADS (native) =====
        } else if (func_name == "__builtin_json_loads__") {
            if (arguments.size() != 1) {
                throw TypeError("json_loads() requires 1 argument (json_string)");
            }
            std::string json_text = std::get<std::string>(arguments[0]);

            {
                auto parsed = sapphire::stdlib::JSON::parse_json(json_text);
                if (!parsed) {
                    throw RuntimeError("Failed to parse JSON string");
                }

                std::function<Value(const std::shared_ptr<sapphire::stdlib::JSON::JSONValue>&)> toNative;
                toNative = [&](const std::shared_ptr<sapphire::stdlib::JSON::JSONValue>& jv) -> Value {
                    using namespace sapphire::stdlib::JSON;
                    if (!jv || jv->is_null()) return Value{nullptr};
                    if (jv->is_bool()) return jv->as_bool();
                    if (jv->is_number()) {
                        double num = jv->as_number();
                        if (num == static_cast<int>(num)) return static_cast<int>(num);
                        return num;
                    }
                    if (jv->is_string()) return jv->as_string();
                    if (jv->is_array()) {
                        auto arr = std::make_shared<ArrayValue>();
                        for (const auto& elem : jv->as_array()) {
                            arr->elements.push_back(toNative(elem));
                        }
                        return arr;
                    }
                    if (jv->is_object()) {
                        auto map = std::make_shared<HashMapValue>();
                        for (const auto& [k, v] : jv->as_object()) {
                            map->pairs[k] = toNative(v);
                        }
                        return map;
                    }
                    return Value{nullptr};
                };

                last_value = toNative(parsed);
            }
            return;

        // ===== JSON DUMPS (native) =====
        } else if (func_name == "__builtin_json_dumps__") {
            if (arguments.size() < 1 || arguments.size() > 2) {
                throw TypeError("json_dumps() requires 1-2 arguments (value, pretty=false)");
            }
            {
                bool pretty = arguments.size() > 1 ? std::get<bool>(arguments[1]) : false;

                std::function<std::shared_ptr<sapphire::stdlib::JSON::JSONValue>(const Value&)> toJson;
                toJson = [&](const Value& val) -> std::shared_ptr<sapphire::stdlib::JSON::JSONValue> {
                    using namespace sapphire::stdlib::JSON;
                    if (std::holds_alternative<std::nullptr_t>(val)) return create_json_null();
                    if (std::holds_alternative<bool>(val)) return create_json_bool(std::get<bool>(val));
                    if (std::holds_alternative<int>(val)) return create_json_number(static_cast<double>(std::get<int>(val)));
                    if (std::holds_alternative<double>(val)) return create_json_number(std::get<double>(val));
                    if (std::holds_alternative<std::string>(val)) return create_json_string(std::get<std::string>(val));
                    if (std::holds_alternative<std::shared_ptr<ArrayValue>>(val)) {
                        auto arr_val = std::get<std::shared_ptr<ArrayValue>>(val);
                        JSONArray json_arr;
                        for (const auto& elem : arr_val->elements) {
                            json_arr.push_back(toJson(elem));
                        }
                        return std::make_shared<JSONValue>(json_arr);
                    }
                    if (std::holds_alternative<std::shared_ptr<HashMapValue>>(val)) {
                        auto map_val = std::get<std::shared_ptr<HashMapValue>>(val);
                        JSONObject json_obj;
                        for (const auto& [k, v] : map_val->pairs) {
                            json_obj[k] = toJson(v);
                        }
                        return std::make_shared<JSONValue>(json_obj);
                    }
                    return create_json_null();
                };

                auto json_val = toJson(arguments[0]);
                if (!json_val) {
                    throw RuntimeError("Failed to convert value to JSON");
                }
                std::string json_str = sapphire::stdlib::JSON::stringify_json(json_val, pretty);
                last_value = json_str;
            }
            return;

        // ===== MILESTONE 2: RANDOM NUMBERS =====
        } else if (func_name == "__builtin_random_seed__") {
            if (arguments.size() != 1) {
                throw TypeError("random_seed() requires 1 argument (seed)");
            }
            
            uint64_t seed = static_cast<uint64_t>(std::get<int>(arguments[0]));
            sapphire_random_seed(seed);
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_random__") {
            if (arguments.size() > 2) {
                throw TypeError("random() requires 0-2 arguments (min=0.0, max=1.0)");
            }
            
            if (arguments.size() == 0) {
                last_value = sapphire_random_uniform();
            } else if (arguments.size() == 2) {
                double min = std::get<double>(arguments[0]);
                double max = std::get<double>(arguments[1]);
                last_value = sapphire_random_uniform_range(min, max);
            } else {
                throw TypeError("random() requires 0 or 2 arguments");
            }
            return;
            
        } else if (func_name == "__builtin_random_int__") {
            if (arguments.size() != 2) {
                throw TypeError("random_int() requires 2 arguments (min, max)");
            }
            
            int64_t min = static_cast<int64_t>(std::get<int>(arguments[0]));
            int64_t max = static_cast<int64_t>(std::get<int>(arguments[1]));
            last_value = static_cast<int>(sapphire_random_int(min, max));
            return;
            
        } else if (func_name == "__builtin_random_normal__") {
            if (arguments.size() > 2) {
                throw TypeError("random_normal() requires 0-2 arguments (mean=0.0, stddev=1.0)");
            }
            
            if (arguments.size() == 0) {
                last_value = sapphire_random_normal();
            } else if (arguments.size() == 2) {
                double mean = std::get<double>(arguments[0]);
                double stddev = std::get<double>(arguments[1]);
                last_value = sapphire_random_normal_params(mean, stddev);
            } else {
                throw TypeError("random_normal() requires 0 or 2 arguments");
            }
            return;
            
        } else if (func_name == "__builtin_random_boolean__") {
            if (arguments.size() > 1) {
                throw TypeError("random_boolean() requires 0-1 arguments (probability=0.5)");
            }
            
            if (arguments.size() == 0) {
                last_value = sapphire_random_boolean();
            } else {
                double probability = std::get<double>(arguments[0]);
                last_value = sapphire_random_boolean_prob(probability);
            }
            return;
            
        } else if (func_name == "__builtin_crypto_random__") {
            if (arguments.size() != 0) {
                throw TypeError("crypto_random() requires 0 arguments");
            }
            
            last_value = sapphire_crypto_random_uniform();
            return;
            
        } else if (func_name == "__builtin_crypto_random_int__") {
            if (arguments.size() != 2) {
                throw TypeError("crypto_random_int() requires 2 arguments (min, max)");
            }
            
            int64_t min = static_cast<int64_t>(std::get<int>(arguments[0]));
            int64_t max = static_cast<int64_t>(std::get<int>(arguments[1]));
            last_value = static_cast<int>(sapphire_crypto_random_int(min, max));
            return;
            
        // ===== MILESTONE 3: DATA VISUALIZATION =====
        } else if (func_name == "__builtin_plot_create__") {
            if (arguments.size() < 1 || arguments.size() > 3) {
                throw TypeError("plot_create() requires 1-3 arguments (chart_type, width=800, height=600)");
            }
            
            int chart_type = std::get<int>(arguments[0]);
            int width = arguments.size() > 1 ? std::get<int>(arguments[1]) : 800;
            int height = arguments.size() > 2 ? std::get<int>(arguments[2]) : 600;
            
            void* plot = sapphire_plot_create(width, height, chart_type);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(plot));
            return;
            
        } else if (func_name == "__builtin_plot_add_line__") {
            if (arguments.size() < 3 || arguments.size() > 7) {
                throw TypeError("plot_add_line() requires 3-7 arguments (plot, x_array, y_array, label=\"\", r=0.0, g=0.0, b=1.0)");
            }
            
            void* plot = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            
            // Get arrays
            auto x_array = std::get<std::shared_ptr<ArrayValue>>(arguments[1]);
            auto y_array = std::get<std::shared_ptr<ArrayValue>>(arguments[2]);
            
            // Convert to double arrays
            std::vector<double> x_data, y_data;
            for (const auto& val : x_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    x_data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    x_data.push_back(std::get<double>(val));
                }
            }
            for (const auto& val : y_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    y_data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    y_data.push_back(std::get<double>(val));
                }
            }
            
            std::string label = arguments.size() > 3 ? std::get<std::string>(arguments[3]) : "";
            double r = arguments.size() > 4 ? std::get<double>(arguments[4]) : 0.0;
            double g = arguments.size() > 5 ? std::get<double>(arguments[5]) : 0.0;
            double b = arguments.size() > 6 ? std::get<double>(arguments[6]) : 1.0;
            
            sapphire_plot_add_line(plot, x_data.data(), y_data.data(), 
                                  static_cast<int>(std::min(x_data.size(), y_data.size())), 
                                  label.c_str(), r, g, b);
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_plot_add_scatter__") {
            if (arguments.size() < 3 || arguments.size() > 8) {
                throw TypeError("plot_add_scatter() requires 3-8 arguments (plot, x_array, y_array, label=\"\", r=1.0, g=0.0, b=0.0, marker=1)");
            }
            
            void* plot = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            
            // Get arrays
            auto x_array = std::get<std::shared_ptr<ArrayValue>>(arguments[1]);
            auto y_array = std::get<std::shared_ptr<ArrayValue>>(arguments[2]);
            
            // Convert to double arrays
            std::vector<double> x_data, y_data;
            for (const auto& val : x_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    x_data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    y_data.push_back(std::get<double>(val));
                }
            }
            for (const auto& val : y_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    y_data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    y_data.push_back(std::get<double>(val));
                }
            }
            
            std::string label = arguments.size() > 3 ? std::get<std::string>(arguments[3]) : "";
            double r = arguments.size() > 4 ? std::get<double>(arguments[4]) : 1.0;
            double g = arguments.size() > 5 ? std::get<double>(arguments[5]) : 0.0;
            double b = arguments.size() > 6 ? std::get<double>(arguments[6]) : 0.0;
            int marker = arguments.size() > 7 ? std::get<int>(arguments[7]) : 1;
            
            sapphire_plot_add_scatter(plot, x_data.data(), y_data.data(), 
                                     static_cast<int>(std::min(x_data.size(), y_data.size())), 
                                     label.c_str(), r, g, b, marker);
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_plot_add_histogram__") {
            if (arguments.size() < 2 || arguments.size() > 7) {
                throw TypeError("plot_add_histogram() requires 2-7 arguments (plot, data_array, bins=20, label=\"\", r=1.0, g=0.5, b=0.0)");
            }
            
            void* plot = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            
            // Get data array
            auto data_array = std::get<std::shared_ptr<ArrayValue>>(arguments[1]);
            
            // Convert to double array
            std::vector<double> data;
            for (const auto& val : data_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    data.push_back(std::get<double>(val));
                }
            }
            
            int bins = arguments.size() > 2 ? std::get<int>(arguments[2]) : 20;
            std::string label = arguments.size() > 3 ? std::get<std::string>(arguments[3]) : "";
            double r = arguments.size() > 4 ? std::get<double>(arguments[4]) : 1.0;
            double g = arguments.size() > 5 ? std::get<double>(arguments[5]) : 0.5;
            double b = arguments.size() > 6 ? std::get<double>(arguments[6]) : 0.0;
            
            sapphire_plot_add_histogram(plot, data.data(), static_cast<int>(data.size()), 
                                       bins, label.c_str(), r, g, b);
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_plot_set_title__") {
            if (arguments.size() != 2) {
                throw TypeError("plot_set_title() requires 2 arguments (plot, title)");
            }
            
            void* plot = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string title = std::get<std::string>(arguments[1]);
            
            sapphire_plot_set_title(plot, title.c_str());
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_plot_set_xlabel__") {
            if (arguments.size() != 2) {
                throw TypeError("plot_set_xlabel() requires 2 arguments (plot, label)");
            }
            
            void* plot = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string label = std::get<std::string>(arguments[1]);
            
            sapphire_plot_set_xlabel(plot, label.c_str());
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_plot_set_ylabel__") {
            if (arguments.size() != 2) {
                throw TypeError("plot_set_ylabel() requires 2 arguments (plot, label)");
            }
            
            void* plot = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string label = std::get<std::string>(arguments[1]);
            
            sapphire_plot_set_ylabel(plot, label.c_str());
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_plot_show__") {
            if (arguments.size() != 1) {
                throw TypeError("plot_show() requires 1 argument (plot)");
            }
            
            void* plot = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sapphire_plot_show(plot);
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_plot_save_svg__") {
            if (arguments.size() != 2) {
                throw TypeError("plot_save_svg() requires 2 arguments (plot, filename)");
            }
            
            void* plot = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string filename = std::get<std::string>(arguments[1]);
            
            sapphire_plot_save_svg(plot, filename.c_str());
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_quick_plot__") {
            if (arguments.size() < 1 || arguments.size() > 2) {
                throw TypeError("quick_plot() requires 1-2 arguments (y_array, title=\"\")");
            }
            
            // Get data array
            auto y_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            
            // Convert to double array
            std::vector<double> y_data;
            for (const auto& val : y_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    y_data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    y_data.push_back(std::get<double>(val));
                }
            }
            
            std::string title = arguments.size() > 1 ? std::get<std::string>(arguments[1]) : "";
            
            void* plot = sapphire_quick_plot(y_data.data(), static_cast<int>(y_data.size()), title.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(plot));
            return;
            
        } else if (func_name == "__builtin_quick_scatter__") {
            if (arguments.size() < 2 || arguments.size() > 3) {
                throw TypeError("quick_scatter() requires 2-3 arguments (x_array, y_array, title=\"\")");
            }
            
            // Get arrays
            auto x_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            auto y_array = std::get<std::shared_ptr<ArrayValue>>(arguments[1]);
            
            // Convert to double arrays
            std::vector<double> x_data, y_data;
            for (const auto& val : x_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    x_data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    x_data.push_back(std::get<double>(val));
                }
            }
            for (const auto& val : y_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    y_data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    y_data.push_back(std::get<double>(val));
                }
            }
            
            std::string title = arguments.size() > 2 ? std::get<std::string>(arguments[2]) : "";
            
            void* plot = sapphire_quick_scatter(x_data.data(), y_data.data(), 
                                              static_cast<int>(std::min(x_data.size(), y_data.size())), 
                                              title.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(plot));
            return;
            
        } else if (func_name == "__builtin_quick_histogram__") {
            if (arguments.size() < 1 || arguments.size() > 3) {
                throw TypeError("quick_histogram() requires 1-3 arguments (data_array, bins=20, title=\"\")");
            }
            
            // Get data array
            auto data_array = std::get<std::shared_ptr<ArrayValue>>(arguments[0]);
            
            // Convert to double array
            std::vector<double> data;
            for (const auto& val : data_array->elements) {
                if (std::holds_alternative<int>(val)) {
                    data.push_back(static_cast<double>(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    data.push_back(std::get<double>(val));
                }
            }
            
            int bins = arguments.size() > 1 ? std::get<int>(arguments[1]) : 20;
            std::string title = arguments.size() > 2 ? std::get<std::string>(arguments[2]) : "";
            
            void* plot = sapphire_quick_histogram(data.data(), static_cast<int>(data.size()), 
                                                bins, title.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(plot));
            return;

        // ===== PLOT3D: 3D browser plots via Plotly.js =====
        } else if (func_name == "__builtin_plot3d_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(plot3d_create()));
            return;
        } else if (func_name == "__builtin_plot3d_set_title__") {
            plot3d_set_title(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_plot3d_set_xlabel__") {
            plot3d_set_xlabel(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_plot3d_set_ylabel__") {
            plot3d_set_ylabel(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_plot3d_set_zlabel__") {
            plot3d_set_zlabel(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_plot3d_save_html__") {
            plot3d_save_html(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_plot3d_add_surface__") {
            // args: plot, z_array, rows, cols, label, colorscale
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            auto arr = std::get<std::shared_ptr<ArrayValue>>(arguments[1]);
            int rows = std::get<int>(arguments[2]);
            int cols = std::get<int>(arguments[3]);
            std::string label = arguments.size() > 4 ? std::get<std::string>(arguments[4]) : "";
            std::string cs    = arguments.size() > 5 ? std::get<std::string>(arguments[5]) : "Viridis";
            std::vector<double> z;
            for (const auto& v : arr->elements) {
                if (std::holds_alternative<int>(v))    z.push_back((double)std::get<int>(v));
                else if (std::holds_alternative<double>(v)) z.push_back(std::get<double>(v));
                else z.push_back(0.0);
            }
            plot3d_add_surface(p, z.data(), rows, cols, label.c_str(), cs.c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_plot3d_add_scatter__") {
            // args: plot, x_array, y_array, z_array, label
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            auto to_vec = [](const std::shared_ptr<ArrayValue>& a) {
                std::vector<double> v;
                for (const auto& e : a->elements) {
                    if (std::holds_alternative<int>(e))    v.push_back((double)std::get<int>(e));
                    else if (std::holds_alternative<double>(e)) v.push_back(std::get<double>(e));
                }
                return v;
            };
            auto xv = to_vec(std::get<std::shared_ptr<ArrayValue>>(arguments[1]));
            auto yv = to_vec(std::get<std::shared_ptr<ArrayValue>>(arguments[2]));
            auto zv = to_vec(std::get<std::shared_ptr<ArrayValue>>(arguments[3]));
            std::string label = arguments.size() > 4 ? std::get<std::string>(arguments[4]) : "";
            int n = (int)std::min({xv.size(), yv.size(), zv.size()});
            plot3d_add_scatter(p, xv.data(), yv.data(), zv.data(), n, label.c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_plot3d_add_line__") {
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            auto to_vec = [](const std::shared_ptr<ArrayValue>& a) {
                std::vector<double> v;
                for (const auto& e : a->elements) {
                    if (std::holds_alternative<int>(e))    v.push_back((double)std::get<int>(e));
                    else if (std::holds_alternative<double>(e)) v.push_back(std::get<double>(e));
                }
                return v;
            };
            auto xv = to_vec(std::get<std::shared_ptr<ArrayValue>>(arguments[1]));
            auto yv = to_vec(std::get<std::shared_ptr<ArrayValue>>(arguments[2]));
            auto zv = to_vec(std::get<std::shared_ptr<ArrayValue>>(arguments[3]));
            std::string label = arguments.size() > 4 ? std::get<std::string>(arguments[4]) : "";
            int n = (int)std::min({xv.size(), yv.size(), zv.size()});
            plot3d_add_line(p, xv.data(), yv.data(), zv.data(), n, label.c_str());
            last_value = nullptr; return;

        // ===== MILESTONE 4: HTTP SERVER/CLIENT =====
        } else if (func_name == "__builtin_http_server_create__") {
            // Create HTTP server
            if (arguments.size() != 1) {
                throw TypeError("http_server_create() requires 1 argument (port)");
            }
            
            int port = std::get<int>(arguments[0]);
            void* server = http_server_create(port);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(server));
            return;
            
        } else if (func_name == "__builtin_http_server_start__") {
            // Start HTTP server
            if (arguments.size() != 1) {
                throw TypeError("http_server_start() requires 1 argument (server)");
            }
            
            void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int result = http_server_start(server);
            last_value = (result == 1);
            return;
            
        } else if (func_name == "__builtin_http_server_stop__") {
            // Stop HTTP server
            if (arguments.size() != 1) {
                throw TypeError("http_server_stop() requires 1 argument (server)");
            }
            
            void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            http_server_stop(server);
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_http_client_create__") {
            // Create HTTP client
            if (arguments.size() != 0) {
                throw TypeError("http_client_create() requires 0 arguments");
            }
            
            void* client = http_client_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(client));
            return;
            
        } else if (func_name == "__builtin_http_client_get__") {
            // HTTP GET request
            if (arguments.size() != 2) {
                throw TypeError("http_client_get() requires 2 arguments (client, url)");
            }
            
            void* client = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string url = std::get<std::string>(arguments[1]);
            
            void* response = http_client_get(client, url.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(response));
            return;
            
        } else if (func_name == "__builtin_http_client_post__") {
            // HTTP POST request
            if (arguments.size() != 3) {
                throw TypeError("http_client_post() requires 3 arguments (client, url, body)");
            }
            
            void* client = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string url = std::get<std::string>(arguments[1]);
            std::string body = std::get<std::string>(arguments[2]);
            
            void* response = http_client_post(client, url.c_str(), body.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(response));
            return;
            
        } else if (func_name == "__builtin_http_response_status__") {
            // Get response status code
            if (arguments.size() != 1) {
                throw TypeError("http_response_status() requires 1 argument (response)");
            }
            
            void* response = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int status = http_response_get_status(response);
            last_value = status;
            return;
            
        } else if (func_name == "__builtin_http_response_body__") {
            // Get response body
            if (arguments.size() != 1) {
                throw TypeError("http_response_body() requires 1 argument (response)");
            }
            
            void* response = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            const char* body = http_response_get_body(response);
            last_value = std::string(body ? body : "");
            return;
            
        } else if (func_name == "__builtin_http_response_header__") {
            // Get response header
            if (arguments.size() != 2) {
                throw TypeError("http_response_header() requires 2 arguments (response, header_name)");
            }
            
            void* response = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string header_name = std::get<std::string>(arguments[1]);
            
            const char* header_value = http_response_get_header(response, header_name.c_str());
            last_value = std::string(header_value ? header_value : "");
            return;
            
        } else if (func_name == "__builtin_http_url_encode__") {
            // URL encode string
            if (arguments.size() != 1) {
                throw TypeError("http_url_encode() requires 1 argument (string)");
            }
            
            std::string input = std::get<std::string>(arguments[0]);
            char* encoded = http_url_encode(input.c_str());
            last_value = std::string(encoded);
            http_free_string(encoded);
            return;
            
        } else if (func_name == "__builtin_http_url_decode__") {
            // URL decode string
            if (arguments.size() != 1) {
                throw TypeError("http_url_decode() requires 1 argument (string)");
            }
            
            std::string input = std::get<std::string>(arguments[0]);
            char* decoded = http_url_decode(input.c_str());
            last_value = std::string(decoded);
            http_free_string(decoded);
            return;
            
        } else if (func_name == "__builtin_http_html_escape__") {
            // HTML escape string
            if (arguments.size() != 1) {
                throw TypeError("http_html_escape() requires 1 argument (string)");
            }
            
            std::string input = std::get<std::string>(arguments[0]);
            char* escaped = http_html_escape(input.c_str());
            last_value = std::string(escaped);
            http_free_string(escaped);
            return;

        // ===== HTTP ROUTE REGISTRATION =====
        } else if (func_name == "__builtin_http_server_get__") {
            if (arguments.size() != 3) {
                throw TypeError("http_server_get() requires 3 arguments (server, path, handler_name)");
            }
            {
                void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
                std::string path = std::get<std::string>(arguments[1]);
                std::string handler = std::get<std::string>(arguments[2]);
                http_server_get(server, path.c_str(), nullptr);
                last_value = nullptr;
            }
            return;

        } else if (func_name == "__builtin_http_server_post__") {
            if (arguments.size() != 3) {
                throw TypeError("http_server_post() requires 3 arguments (server, path, handler_name)");
            }
            {
                void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
                std::string path = std::get<std::string>(arguments[1]);
                std::string handler = std::get<std::string>(arguments[2]);
                http_server_post(server, path.c_str(), nullptr);
                last_value = nullptr;
            }
            return;

        } else if (func_name == "__builtin_http_server_put__") {
            if (arguments.size() != 3) {
                throw TypeError("http_server_put() requires 3 arguments (server, path, handler_name)");
            }
            {
                void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
                std::string path = std::get<std::string>(arguments[1]);
                std::string handler = std::get<std::string>(arguments[2]);
                http_server_put(server, path.c_str(), nullptr);
                last_value = nullptr;
            }
            return;

        } else if (func_name == "__builtin_http_server_delete__") {
            if (arguments.size() != 3) {
                throw TypeError("http_server_delete() requires 3 arguments (server, path, handler_name)");
            }
            {
                void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
                std::string path = std::get<std::string>(arguments[1]);
                std::string handler = std::get<std::string>(arguments[2]);
                http_server_delete(server, path.c_str(), nullptr);
                last_value = nullptr;
            }
            return;

        // ===== FS MODULE =====
        } else if (func_name == "__builtin_fs_exists__") {
            if (arguments.size() != 1) {
                throw TypeError("fs_exists() requires 1 argument (path)");
            }
            std::string path = std::get<std::string>(arguments[0]);
            last_value = static_cast<bool>(fs_exists(path.c_str()));
            return;

        } else if (func_name == "__builtin_fs_is_file__") {
            if (arguments.size() != 1) {
                throw TypeError("fs_is_file() requires 1 argument (path)");
            }
            std::string path = std::get<std::string>(arguments[0]);
            last_value = static_cast<bool>(fs_is_file(path.c_str()));
            return;

        } else if (func_name == "__builtin_fs_is_dir__") {
            if (arguments.size() != 1) {
                throw TypeError("fs_is_dir() requires 1 argument (path)");
            }
            std::string path = std::get<std::string>(arguments[0]);
            last_value = static_cast<bool>(fs_is_dir(path.c_str()));
            return;

        } else if (func_name == "__builtin_fs_create_dir__") {
            if (arguments.size() != 1) {
                throw TypeError("fs_create_dir() requires 1 argument (path)");
            }
            std::string path = std::get<std::string>(arguments[0]);
            last_value = static_cast<bool>(fs_create_dir(path.c_str()));
            return;

        } else if (func_name == "__builtin_fs_read_file__") {
            if (arguments.size() != 1) {
                throw TypeError("fs_read_file() requires 1 argument (path)");
            }
            std::string path = std::get<std::string>(arguments[0]);
            char* content = fs_read_file(path.c_str());
            if (content) {
                last_value = std::string(content);
                fs_free_string(content);
            } else {
                last_value = Value{};
            }
            return;

        } else if (func_name == "__builtin_fs_write_file__") {
            if (arguments.size() != 2) {
                throw TypeError("fs_write_file() requires 2 arguments (path, content)");
            }
            std::string path = std::get<std::string>(arguments[0]);
            std::string content = std::get<std::string>(arguments[1]);
            last_value = static_cast<bool>(fs_write_file(path.c_str(), content.c_str()));
            return;

        } else if (func_name == "__builtin_fs_delete__") {
            if (arguments.size() != 1) {
                throw TypeError("fs_delete() requires 1 argument (path)");
            }
            std::string path = std::get<std::string>(arguments[0]);
            last_value = static_cast<bool>(fs_delete(path.c_str()));
            return;

        } else if (func_name == "__builtin_fs_abs_path__") {
            if (arguments.size() != 1) {
                throw TypeError("fs_abs_path() requires 1 argument (path)");
            }
            std::string path = std::get<std::string>(arguments[0]);
            char* result = fs_abs_path(path.c_str());
            if (result) {
                last_value = std::string(result);
                fs_free_string(result);
            } else {
                last_value = Value{};
            }
            return;

        // ===== WEBSOCKET SUPPORT =====
        } else if (func_name == "__builtin_websocket_server_create__") {
            // Create WebSocket server
            if (arguments.size() != 1) {
                throw TypeError("websocket_server_create() requires 1 argument (port)");
            }
            
            int port = std::get<int>(arguments[0]);
            void* server = sapphire_websocket_server_create(port);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(server));
            return;
            
        } else if (func_name == "__builtin_websocket_server_start__") {
            // Start WebSocket server
            if (arguments.size() != 1) {
                throw TypeError("websocket_server_start() requires 1 argument (server)");
            }
            
            void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int result = sapphire_websocket_server_start(server);
            last_value = (result == 1);
            return;
            
        } else if (func_name == "__builtin_websocket_server_stop__") {
            // Stop WebSocket server
            if (arguments.size() != 1) {
                throw TypeError("websocket_server_stop() requires 1 argument (server)");
            }
            
            void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sapphire_websocket_server_stop(server);
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_websocket_server_broadcast_text__") {
            // Broadcast text to all connections
            if (arguments.size() != 2) {
                throw TypeError("websocket_server_broadcast_text() requires 2 arguments (server, text)");
            }
            
            void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string text = std::get<std::string>(arguments[1]);
            sapphire_websocket_server_broadcast_text(server, text.c_str());
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_websocket_server_broadcast_binary__") {
            // Broadcast binary data to all connections
            if (arguments.size() != 2) {
                throw TypeError("websocket_server_broadcast_binary() requires 2 arguments (server, data)");
            }
            
            void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string data = std::get<std::string>(arguments[1]);
            sapphire_websocket_server_broadcast_binary(server, 
                reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_websocket_server_connection_count__") {
            // Get connection count
            if (arguments.size() != 1) {
                throw TypeError("websocket_server_connection_count() requires 1 argument (server)");
            }
            
            void* server = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int count = sapphire_websocket_server_connection_count(server);
            last_value = count;
            return;
            
        } else if (func_name == "__builtin_websocket_client_create__") {
            // Create WebSocket client
            if (arguments.size() != 0) {
                throw TypeError("websocket_client_create() requires 0 arguments");
            }
            
            void* client = sapphire_websocket_client_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(client));
            return;
            
        } else if (func_name == "__builtin_websocket_client_connect__") {
            // Connect WebSocket client
            if (arguments.size() != 2) {
                throw TypeError("websocket_client_connect() requires 2 arguments (client, url)");
            }
            
            void* client = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string url = std::get<std::string>(arguments[1]);
            int result = sapphire_websocket_client_connect(client, url.c_str());
            last_value = (result == 1);
            return;
            
        } else if (func_name == "__builtin_websocket_client_disconnect__") {
            // Disconnect WebSocket client
            if (arguments.size() != 1) {
                throw TypeError("websocket_client_disconnect() requires 1 argument (client)");
            }
            
            void* client = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sapphire_websocket_client_disconnect(client);
            last_value = nullptr;
            return;
            
        } else if (func_name == "__builtin_websocket_client_send_text__") {
            // Send text message
            if (arguments.size() != 2) {
                throw TypeError("websocket_client_send_text() requires 2 arguments (client, text)");
            }
            
            void* client = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string text = std::get<std::string>(arguments[1]);
            int result = sapphire_websocket_client_send_text(client, text.c_str());
            last_value = (result == 1);
            return;
            
        } else if (func_name == "__builtin_websocket_client_send_binary__") {
            // Send binary data
            if (arguments.size() != 2) {
                throw TypeError("websocket_client_send_binary() requires 2 arguments (client, data)");
            }
            
            void* client = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string data = std::get<std::string>(arguments[1]);
            int result = sapphire_websocket_client_send_binary(client, 
                reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
            last_value = (result == 1);
            return;
            
        } else if (func_name == "__builtin_websocket_client_is_connected__") {
            // Check if client is connected
            if (arguments.size() != 1) {
                throw TypeError("websocket_client_is_connected() requires 1 argument (client)");
            }
            
            void* client = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int result = sapphire_websocket_client_is_connected(client);
            last_value = (result == 1);
            return;
            
        } else if (func_name == "__builtin_websocket_base64_encode__") {
            // Base64 encode data
            if (arguments.size() != 1) {
                throw TypeError("websocket_base64_encode() requires 1 argument (data)");
            }
            
            std::string data = std::get<std::string>(arguments[0]);
            char* encoded = sapphire_websocket_base64_encode(
                reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
            last_value = std::string(encoded);
            sapphire_websocket_free_string(encoded);
            return;
            
        } else if (func_name == "__builtin_websocket_base64_decode__") {
            // Base64 decode data
            if (arguments.size() != 1) {
                throw TypeError("websocket_base64_decode() requires 1 argument (encoded)");
            }
            
            std::string encoded = std::get<std::string>(arguments[0]);
            int size;
            uint8_t* decoded = sapphire_websocket_base64_decode(encoded.c_str(), &size);
            last_value = std::string(reinterpret_cast<char*>(decoded), size);
            sapphire_websocket_free_data(decoded);
            return;
            
        // ===== TEMPLATE ENGINE =====
        } else if (func_name == "__builtin_html_template_engine_create__") {
            // Create template engine
            if (arguments.size() != 0) {
                throw TypeError("html_template_engine_create() requires 0 arguments");
            }
            
            void* engine = sapphire_html_template_engine_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(engine));
            return;
            
        } else if (func_name == "__builtin_html_template_render__") {
            // Render template string
            if (arguments.size() != 3) {
                throw TypeError("html_template_render() requires 3 arguments (engine, template, context)");
            }
            
            void* engine = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string template_str = std::get<std::string>(arguments[1]);
            std::string context_str = std::get<std::string>(arguments[2]);
            
            // Simple context parsing (key=value pairs separated by ;)
            std::vector<const char*> keys, values;
            std::vector<std::string> key_storage, value_storage;
            
            size_t pos = 0;
            while (pos < context_str.length()) {
                size_t eq_pos = context_str.find('=', pos);
                if (eq_pos == std::string::npos) break;
                
                size_t semi_pos = context_str.find(';', eq_pos);
                if (semi_pos == std::string::npos) semi_pos = context_str.length();
                
                key_storage.push_back(context_str.substr(pos, eq_pos - pos));
                value_storage.push_back(context_str.substr(eq_pos + 1, semi_pos - eq_pos - 1));
                
                keys.push_back(key_storage.back().c_str());
                values.push_back(value_storage.back().c_str());
                
                pos = semi_pos + 1;
            }
            
            char* result = sapphire_html_template_render(engine, template_str.c_str(), 
                keys.data(), values.data(), keys.size());
            last_value = std::string(result);
            sapphire_html_template_free_string(result);
            return;
            
        // ===== DATABASE DRIVERS =====
        } else if (func_name == "__builtin_postgresql_create__") {
            // Create PostgreSQL connection
            if (arguments.size() != 0) {
                throw TypeError("postgresql_create() requires 0 arguments");
            }
            
            void* conn = sapphire_postgresql_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(conn));
            return;
            
        } else if (func_name == "__builtin_postgresql_connect__") {
            // Connect to PostgreSQL
            if (arguments.size() != 2) {
                throw TypeError("postgresql_connect() requires 2 arguments (connection, connection_string)");
            }
            
            void* conn = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string conn_str = std::get<std::string>(arguments[1]);
            int result = sapphire_postgresql_connect(conn, conn_str.c_str());
            last_value = (result == 1);
            return;
            
        } else if (func_name == "__builtin_postgresql_query__") {
            // Execute PostgreSQL query
            if (arguments.size() != 2) {
                throw TypeError("postgresql_query() requires 2 arguments (connection, query)");
            }
            
            void* conn = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string query = std::get<std::string>(arguments[1]);
            char* result = sapphire_postgresql_query(conn, query.c_str());
            last_value = std::string(result);
            sapphire_database_free_result(result);
            return;
            
        } else if (func_name == "__builtin_mysql_create__") {
            // Create MySQL connection
            if (arguments.size() != 0) {
                throw TypeError("mysql_create() requires 0 arguments");
            }
            
            void* conn = sapphire_mysql_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(conn));
            return;
            
        } else if (func_name == "__builtin_mysql_connect__") {
            // Connect to MySQL
            if (arguments.size() != 2) {
                throw TypeError("mysql_connect() requires 2 arguments (connection, connection_string)");
            }
            
            void* conn = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string conn_str = std::get<std::string>(arguments[1]);
            int result = sapphire_mysql_connect(conn, conn_str.c_str());
            last_value = (result == 1);
            return;
            
        } else if (func_name == "__builtin_mysql_query__") {
            // Execute MySQL query
            if (arguments.size() != 2) {
                throw TypeError("mysql_query() requires 2 arguments (connection, query)");
            }
            
            void* conn = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string query = std::get<std::string>(arguments[1]);
            char* result = sapphire_mysql_query(conn, query.c_str());
            last_value = std::string(result);
            sapphire_database_free_result(result);
            return;

        // ===== ORM FRAMEWORK =====
        } else if (func_name == "__builtin_orm_model_create__") {
            if (arguments.size() != 1) throw TypeError("orm_model_create() requires 1 argument (table_name)");
            std::string table = std::get<std::string>(arguments[0]);
            void* model = orm_model_create(table.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(model));
            return;

        } else if (func_name == "__builtin_orm_model_add_field__") {
            // orm_model_add_field(model, name, type) where type is "string","int","float","bool","text","datetime","json"
            if (arguments.size() != 3) throw TypeError("orm_model_add_field() requires 3 arguments (model, name, type)");
            void* model = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string name = std::get<std::string>(arguments[1]);
            std::string type = std::get<std::string>(arguments[2]);
            if (type == "string") orm_model_add_string_field(model, name.c_str(), 255);
            else if (type == "int") orm_model_add_int_field(model, name.c_str());
            else if (type == "text") {
                auto* m = static_cast<Sapphire::ORM::Model*>(model);
                m->add_text_field(name);
            } else if (type == "float" || type == "double") {
                auto* m = static_cast<Sapphire::ORM::Model*>(model);
                m->add_double_field(name);
            } else if (type == "bool") {
                auto* m = static_cast<Sapphire::ORM::Model*>(model);
                m->add_bool_field(name);
            } else if (type == "datetime") {
                auto* m = static_cast<Sapphire::ORM::Model*>(model);
                m->add_datetime_field(name);
            } else if (type == "json") {
                auto* m = static_cast<Sapphire::ORM::Model*>(model);
                m->add_json_field(name);
            } else {
                orm_model_add_string_field(model, name.c_str(), 255);
            }
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_orm_model_add_primary_key__") {
            if (arguments.size() != 2) throw TypeError("orm_model_add_primary_key() requires 2 arguments (model, name)");
            void* model = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string name = std::get<std::string>(arguments[1]);
            orm_model_add_primary_key(model, name.c_str());
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_orm_model_get_sql__") {
            if (arguments.size() != 1) throw TypeError("orm_model_get_sql() requires 1 argument (model)");
            void* model = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(orm_model_get_create_sql(model));
            return;

        } else if (func_name == "__builtin_orm_record_create__") {
            void* record = orm_record_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(record));
            return;

        } else if (func_name == "__builtin_orm_record_set__") {
            if (arguments.size() != 3) throw TypeError("orm_record_set() requires 3 arguments (record, field, value)");
            void* record = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string field = std::get<std::string>(arguments[1]);
            // Accept string or int value
            if (std::holds_alternative<std::string>(arguments[2])) {
                orm_record_set_string(record, field.c_str(), std::get<std::string>(arguments[2]).c_str());
            } else if (std::holds_alternative<int>(arguments[2])) {
                orm_record_set_int(record, field.c_str(), std::get<int>(arguments[2]));
            } else if (std::holds_alternative<double>(arguments[2])) {
                auto* r = static_cast<Sapphire::ORM::Record*>(record);
                r->set(field, std::get<double>(arguments[2]));
            } else if (std::holds_alternative<bool>(arguments[2])) {
                auto* r = static_cast<Sapphire::ORM::Record*>(record);
                r->set(field, std::get<bool>(arguments[2]));
            }
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_orm_record_get__") {
            if (arguments.size() != 2) throw TypeError("orm_record_get() requires 2 arguments (record, field)");
            void* record = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string field = std::get<std::string>(arguments[1]);
            last_value = std::string(orm_record_get_string(record, field.c_str()));
            return;

        } else if (func_name == "__builtin_orm_record_to_json__") {
            if (arguments.size() != 1) throw TypeError("orm_record_to_json() requires 1 argument (record)");
            void* record = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(orm_record_to_json(record));
            return;

        } else if (func_name == "__builtin_orm_query_create__") {
            if (arguments.size() != 1) throw TypeError("orm_query_create() requires 1 argument (table_name)");
            std::string table = std::get<std::string>(arguments[0]);
            void* query = orm_query_create(table.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(query));
            return;

        } else if (func_name == "__builtin_orm_query_where__") {
            if (arguments.size() != 3) throw TypeError("orm_query_where() requires 3 arguments (query, field, value)");
            void* query = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string field = std::get<std::string>(arguments[1]);
            std::string value = std::get<std::string>(arguments[2]);
            orm_query_where_eq(query, field.c_str(), value.c_str());
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_orm_query_limit__") {
            if (arguments.size() != 2) throw TypeError("orm_query_limit() requires 2 arguments (query, count)");
            void* query = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int count = std::get<int>(arguments[1]);
            orm_query_limit(query, count);
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_orm_query_offset__") {
            if (arguments.size() != 2) throw TypeError("orm_query_offset() requires 2 arguments (query, offset)");
            void* query = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int off = std::get<int>(arguments[1]);
            static_cast<Sapphire::ORM::QueryBuilder*>(query)->offset(off);
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_orm_query_order_by__") {
            if (arguments.size() < 2) throw TypeError("orm_query_order_by() requires 2-3 arguments (query, field, [asc])");
            void* query = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string field = std::get<std::string>(arguments[1]);
            bool asc = true;
            if (arguments.size() >= 3 && std::holds_alternative<std::string>(arguments[2])) {
                std::string dir = std::get<std::string>(arguments[2]);
                asc = (dir != "desc" && dir != "DESC");
            }
            static_cast<Sapphire::ORM::QueryBuilder*>(query)->order_by(field, asc);
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_orm_query_to_sql__") {
            if (arguments.size() != 1) throw TypeError("orm_query_to_sql() requires 1 argument (query)");
            void* query = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(orm_query_to_sql(query));
            return;

        } else if (func_name == "__builtin_orm_db_create__") {
            void* db = orm_db_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(db));
            return;

        } else if (func_name == "__builtin_orm_db_connect__") {
            if (arguments.size() != 2) throw TypeError("orm_db_connect() requires 2 arguments (db, connection_string)");
            void* db = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string conn_str = std::get<std::string>(arguments[1]);
            int result = orm_db_connect_sqlite(db, conn_str.c_str());
            last_value = (result == 1);
            return;

        } else if (func_name == "__builtin_orm_db_execute__") {
            if (arguments.size() != 2) throw TypeError("orm_db_execute() requires 2 arguments (db, sql)");
            void* db = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string sql = std::get<std::string>(arguments[1]);
            int result = orm_db_execute_command(db, sql.c_str());
            last_value = (result == 1);
            return;

        // ===== AUTH FRAMEWORK =====
        } else if (func_name == "__builtin_jwt_create__") {
            if (arguments.size() < 1) throw TypeError("jwt_create() requires 1-2 arguments (secret, [algorithm])");
            std::string secret = std::get<std::string>(arguments[0]);
            std::string algo = arguments.size() >= 2 ? std::get<std::string>(arguments[1]) : "HS256";
            void* jwt = jwt_create(secret.c_str(), algo.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(jwt));
            return;

        } else if (func_name == "__builtin_jwt_generate__") {
            if (arguments.size() < 2) throw TypeError("jwt_generate() requires 2-3 arguments (jwt, subject, [expires_in])");
            void* jwt = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string subject = std::get<std::string>(arguments[1]);
            int expires = arguments.size() >= 3 ? std::get<int>(arguments[2]) : 3600;
            last_value = std::string(jwt_generate(jwt, subject.c_str(), expires));
            return;

        } else if (func_name == "__builtin_jwt_validate__") {
            if (arguments.size() != 2) throw TypeError("jwt_validate() requires 2 arguments (jwt, token)");
            void* jwt = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string token = std::get<std::string>(arguments[1]);
            last_value = (jwt_validate(jwt, token.c_str()) == 1);
            return;

        } else if (func_name == "__builtin_jwt_get_subject__") {
            if (arguments.size() != 2) throw TypeError("jwt_get_subject() requires 2 arguments (jwt, token)");
            void* jwt = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string token = std::get<std::string>(arguments[1]);
            last_value = std::string(jwt_get_subject(jwt, token.c_str()));
            return;

        } else if (func_name == "__builtin_jwt_get_claim__") {
            if (arguments.size() != 3) throw TypeError("jwt_get_claim() requires 3 arguments (jwt, token, claim)");
            void* jwt = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string token = std::get<std::string>(arguments[1]);
            std::string claim = std::get<std::string>(arguments[2]);
            last_value = std::string(jwt_get_claim(jwt, token.c_str(), claim.c_str()));
            return;

        } else if (func_name == "__builtin_jwt_refresh__") {
            if (arguments.size() < 2) throw TypeError("jwt_refresh() requires 2-3 arguments (jwt, token, [extend_seconds])");
            void* jwt = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string token = std::get<std::string>(arguments[1]);
            int extend = arguments.size() >= 3 ? std::get<int>(arguments[2]) : 3600;
            last_value = std::string(jwt_refresh(jwt, token.c_str(), extend));
            return;

        } else if (func_name == "__builtin_jwt_blacklist__") {
            if (arguments.size() != 2) throw TypeError("jwt_blacklist() requires 2 arguments (jwt, token)");
            void* jwt = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string token = std::get<std::string>(arguments[1]);
            jwt_blacklist(jwt, token.c_str());
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_jwt_is_blacklisted__") {
            if (arguments.size() != 2) throw TypeError("jwt_is_blacklisted() requires 2 arguments (jwt, token)");
            void* jwt = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string token = std::get<std::string>(arguments[1]);
            last_value = (jwt_is_blacklisted(jwt, token.c_str()) == 1);
            return;

        // Sessions
        } else if (func_name == "__builtin_session_manager_create__") {
            int ttl = arguments.size() >= 1 ? std::get<int>(arguments[0]) : 3600;
            void* sm = session_manager_create(ttl);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sm));
            return;

        } else if (func_name == "__builtin_session_create__") {
            if (arguments.size() != 2) throw TypeError("session_create() requires 2 arguments (sm, user_id)");
            void* sm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string uid = std::get<std::string>(arguments[1]);
            last_value = std::string(session_create(sm, uid.c_str()));
            return;

        } else if (func_name == "__builtin_session_is_valid__") {
            if (arguments.size() != 2) throw TypeError("session_is_valid() requires 2 arguments (sm, session_id)");
            void* sm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string sid = std::get<std::string>(arguments[1]);
            last_value = (session_is_valid(sm, sid.c_str()) == 1);
            return;

        } else if (func_name == "__builtin_session_set_data__") {
            if (arguments.size() != 4) throw TypeError("session_set_data() requires 4 arguments (sm, session_id, key, value)");
            void* sm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string sid = std::get<std::string>(arguments[1]);
            std::string key = std::get<std::string>(arguments[2]);
            std::string val = std::get<std::string>(arguments[3]);
            session_set_data(sm, sid.c_str(), key.c_str(), val.c_str());
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_session_get_data__") {
            if (arguments.size() != 3) throw TypeError("session_get_data() requires 3 arguments (sm, session_id, key)");
            void* sm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string sid = std::get<std::string>(arguments[1]);
            std::string key = std::get<std::string>(arguments[2]);
            last_value = std::string(session_get_data(sm, sid.c_str(), key.c_str()));
            return;

        } else if (func_name == "__builtin_session_destroy__") {
            if (arguments.size() != 2) throw TypeError("session_destroy() requires 2 arguments (sm, session_id)");
            void* sm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string sid = std::get<std::string>(arguments[1]);
            last_value = (session_destroy(sm, sid.c_str()) == 1);
            return;

        } else if (func_name == "__builtin_session_regenerate__") {
            if (arguments.size() != 2) throw TypeError("session_regenerate() requires 2 arguments (sm, session_id)");
            void* sm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string sid = std::get<std::string>(arguments[1]);
            last_value = std::string(session_regenerate(sm, sid.c_str()));
            return;

        } else if (func_name == "__builtin_session_cleanup__") {
            if (arguments.size() != 1) throw TypeError("session_cleanup() requires 1 argument (sm)");
            void* sm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = session_cleanup(sm);
            return;

        } else if (func_name == "__builtin_session_get_user_id__") {
            if (arguments.size() != 2) throw TypeError("session_get_user_id() requires 2 arguments (sm, session_id)");
            void* sm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string sid = std::get<std::string>(arguments[1]);
            last_value = std::string(session_get_user_id(sm, sid.c_str()));
            return;

        // RBAC
        } else if (func_name == "__builtin_rbac_create__") {
            void* rbac = rbac_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(rbac));
            return;

        } else if (func_name == "__builtin_rbac_define_role__") {
            if (arguments.size() < 2) throw TypeError("rbac_define_role() requires 2-3 arguments (rbac, name, [parent])");
            void* rbac = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string name = std::get<std::string>(arguments[1]);
            std::string parent = arguments.size() >= 3 ? std::get<std::string>(arguments[2]) : "";
            rbac_define_role(rbac, name.c_str(), parent.empty() ? nullptr : parent.c_str());
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_rbac_add_permission__") {
            if (arguments.size() != 4) throw TypeError("rbac_add_permission() requires 4 arguments (rbac, role, resource, action)");
            void* rbac = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string role     = std::get<std::string>(arguments[1]);
            std::string resource = std::get<std::string>(arguments[2]);
            std::string action   = std::get<std::string>(arguments[3]);
            rbac_add_permission(rbac, role.c_str(), resource.c_str(), action.c_str());
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_rbac_assign_role__") {
            if (arguments.size() != 3) throw TypeError("rbac_assign_role() requires 3 arguments (rbac, user_id, role)");
            void* rbac = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string uid  = std::get<std::string>(arguments[1]);
            std::string role = std::get<std::string>(arguments[2]);
            rbac_assign_role(rbac, uid.c_str(), role.c_str());
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_rbac_revoke_role__") {
            if (arguments.size() != 3) throw TypeError("rbac_revoke_role() requires 3 arguments (rbac, user_id, role)");
            void* rbac = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string uid  = std::get<std::string>(arguments[1]);
            std::string role = std::get<std::string>(arguments[2]);
            rbac_revoke_role(rbac, uid.c_str(), role.c_str());
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_rbac_has_permission__") {
            if (arguments.size() != 4) throw TypeError("rbac_has_permission() requires 4 arguments (rbac, user_id, resource, action)");
            void* rbac = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string uid      = std::get<std::string>(arguments[1]);
            std::string resource = std::get<std::string>(arguments[2]);
            std::string action   = std::get<std::string>(arguments[3]);
            last_value = (rbac_has_permission(rbac, uid.c_str(), resource.c_str(), action.c_str()) == 1);
            return;

        } else if (func_name == "__builtin_rbac_has_role__") {
            if (arguments.size() != 3) throw TypeError("rbac_has_role() requires 3 arguments (rbac, user_id, role)");
            void* rbac = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string uid  = std::get<std::string>(arguments[1]);
            std::string role = std::get<std::string>(arguments[2]);
            last_value = (rbac_has_role(rbac, uid.c_str(), role.c_str()) == 1);
            return;

        // OAuth2
        } else if (func_name == "__builtin_oauth2_create__") {
            void* oauth2 = oauth2_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(oauth2));
            return;

        } else if (func_name == "__builtin_oauth2_register_provider__") {
            if (arguments.size() != 7) throw TypeError("oauth2_register_provider() requires 7 arguments");
            void* oauth2          = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string provider  = std::get<std::string>(arguments[1]);
            std::string client_id = std::get<std::string>(arguments[2]);
            std::string secret    = std::get<std::string>(arguments[3]);
            std::string redirect  = std::get<std::string>(arguments[4]);
            std::string scopes    = std::get<std::string>(arguments[5]);
            std::string auth_url  = std::get<std::string>(arguments[6]);
            oauth2_register_provider(oauth2, provider.c_str(), client_id.c_str(),
                                     secret.c_str(), redirect.c_str(), scopes.c_str(),
                                     auth_url.c_str(), "");
            last_value = nullptr;
            return;

        } else if (func_name == "__builtin_oauth2_get_auth_url__") {
            if (arguments.size() < 2) throw TypeError("oauth2_get_auth_url() requires 2-3 arguments (oauth2, provider, [state])");
            void* oauth2         = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string provider = std::get<std::string>(arguments[1]);
            std::string state    = arguments.size() >= 3 ? std::get<std::string>(arguments[2]) : "";
            last_value = std::string(oauth2_get_auth_url(oauth2, provider.c_str(), state.c_str()));
            return;

        } else if (func_name == "__builtin_oauth2_generate_state__") {
            if (arguments.size() != 1) throw TypeError("oauth2_generate_state() requires 1 argument (oauth2)");
            void* oauth2 = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(oauth2_generate_state(oauth2));
            return;

        } else if (func_name == "__builtin_oauth2_validate_state__") {
            if (arguments.size() != 2) throw TypeError("oauth2_validate_state() requires 2 arguments (oauth2, state)");
            void* oauth2      = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string state = std::get<std::string>(arguments[1]);
            last_value = (oauth2_validate_state(oauth2, state.c_str()) == 1);
            return;

        } else if (func_name == "__builtin_oauth2_has_provider__") {
            if (arguments.size() != 2) throw TypeError("oauth2_has_provider() requires 2 arguments (oauth2, provider)");
            void* oauth2         = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string provider = std::get<std::string>(arguments[1]);
            last_value = (oauth2_has_provider(oauth2, provider.c_str()) == 1);
            return;

        // ===== REST API Framework =====
        } else if (func_name == "__builtin_rest_router_create__") {
            std::string prefix = arguments.size() >= 1 ? std::get<std::string>(arguments[0]) : "/api/v1";
            void* router = rest_router_create(prefix.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(router));
            return;

        } else if (func_name == "__builtin_rest_router_generate_crud__") {
            if (arguments.size() != 2) throw TypeError("rest_router_generate_crud() requires 2 arguments (router, model_name)");
            void* router = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string model = std::get<std::string>(arguments[1]);
            rest_router_generate_crud(router, model.c_str());
            last_value = nullptr; return;

        } else if (func_name == "__builtin_rest_router_add_custom__") {
            if (arguments.size() < 3) throw TypeError("rest_router_add_custom() requires 3-4 arguments (router, method, path, handler, [auth])");
            void* router = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string method  = std::get<std::string>(arguments[1]);
            std::string path    = std::get<std::string>(arguments[2]);
            std::string handler = arguments.size() >= 4 ? std::get<std::string>(arguments[3]) : "";
            int auth = arguments.size() >= 5 ? (std::get<bool>(arguments[4]) ? 1 : 0) : 0;
            rest_router_add_custom(router, method.c_str(), path.c_str(), handler.c_str(), auth);
            last_value = nullptr; return;

        } else if (func_name == "__builtin_rest_router_require_auth__") {
            if (arguments.size() != 3) throw TypeError("rest_router_require_auth() requires 3 arguments (router, model, required)");
            void* router = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string model = std::get<std::string>(arguments[1]);
            bool req = std::holds_alternative<bool>(arguments[2]) ? std::get<bool>(arguments[2]) : std::get<int>(arguments[2]) != 0;
            rest_router_require_auth(router, model.c_str(), req ? 1 : 0);
            last_value = nullptr; return;

        } else if (func_name == "__builtin_rest_router_route_count__") {
            if (arguments.size() != 1) throw TypeError("rest_router_route_count() requires 1 argument");
            void* router = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = rest_router_route_count(router); return;

        } else if (func_name == "__builtin_rest_router_route_table__") {
            if (arguments.size() != 1) throw TypeError("rest_router_route_table() requires 1 argument");
            void* router = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(rest_router_route_table(router)); return;

        } else if (func_name == "__builtin_rest_router_get_prefix__") {
            if (arguments.size() != 1) throw TypeError("rest_router_get_prefix() requires 1 argument");
            void* router = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(rest_router_get_prefix(router)); return;

        } else if (func_name == "__builtin_rest_validator_create__") {
            void* v = rest_validator_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(v)); return;

        } else if (func_name == "__builtin_rest_validator_add_rule__") {
            if (arguments.size() < 3) throw TypeError("rest_validator_add_rule() requires at least 3 arguments (validator, schema, field, [type], [required], [min], [max])");
            void* v = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string schema = std::get<std::string>(arguments[1]);
            std::string field  = std::get<std::string>(arguments[2]);
            std::string type   = arguments.size() >= 4 ? std::get<std::string>(arguments[3]) : "string";
            int req     = arguments.size() >= 5 ? (std::holds_alternative<bool>(arguments[4]) ? (std::get<bool>(arguments[4]) ? 1 : 0) : std::get<int>(arguments[4])) : 0;
            int min_len = arguments.size() >= 6 ? std::get<int>(arguments[5]) : 0;
            int max_len = arguments.size() >= 7 ? std::get<int>(arguments[6]) : 0;
            rest_validator_add_rule(v, schema.c_str(), field.c_str(), type.c_str(), req, min_len, max_len);
            last_value = nullptr; return;

        } else if (func_name == "__builtin_rest_validator_validate__") {
            if (arguments.size() != 3) throw TypeError("rest_validator_validate() requires 3 arguments (validator, schema, data)");
            void* v = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string schema = std::get<std::string>(arguments[1]);
            std::string data   = std::get<std::string>(arguments[2]);
            char err_buf[2048] = {};
            int valid = rest_validator_validate(v, schema.c_str(), data.c_str(), err_buf, sizeof(err_buf));
            last_value = (valid == 1); return;

        } else if (func_name == "__builtin_rest_validator_has_schema__") {
            if (arguments.size() != 2) throw TypeError("rest_validator_has_schema() requires 2 arguments");
            void* v = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string schema = std::get<std::string>(arguments[1]);
            last_value = (rest_validator_has_schema(v, schema.c_str()) == 1); return;

        } else if (func_name == "__builtin_rest_validator_schema_count__") {
            if (arguments.size() != 1) throw TypeError("rest_validator_schema_count() requires 1 argument");
            void* v = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = rest_validator_schema_count(v); return;

        } else if (func_name == "__builtin_rest_formatter_create__") {
            std::string ver = arguments.size() >= 1 ? std::get<std::string>(arguments[0]) : "1.0";
            void* f = rest_formatter_create(ver.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(f)); return;

        } else if (func_name == "__builtin_rest_formatter_success__") {
            if (arguments.size() < 2) throw TypeError("rest_formatter_success() requires 2-3 arguments (formatter, data, [status])");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string data = std::get<std::string>(arguments[1]);
            int status = arguments.size() >= 3 ? std::get<int>(arguments[2]) : 200;
            last_value = std::string(rest_formatter_success(f, data.c_str(), status)); return;

        } else if (func_name == "__builtin_rest_formatter_created__") {
            if (arguments.size() != 2) throw TypeError("rest_formatter_created() requires 2 arguments");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string data = std::get<std::string>(arguments[1]);
            last_value = std::string(rest_formatter_created(f, data.c_str())); return;

        } else if (func_name == "__builtin_rest_formatter_error__") {
            if (arguments.size() < 2) throw TypeError("rest_formatter_error() requires 2-3 arguments");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string msg = std::get<std::string>(arguments[1]);
            int status = arguments.size() >= 3 ? std::get<int>(arguments[2]) : 400;
            last_value = std::string(rest_formatter_error(f, msg.c_str(), status)); return;

        } else if (func_name == "__builtin_rest_formatter_not_found__") {
            if (arguments.size() < 1) throw TypeError("rest_formatter_not_found() requires 1-2 arguments");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string res = arguments.size() >= 2 ? std::get<std::string>(arguments[1]) : "Resource";
            last_value = std::string(rest_formatter_not_found(f, res.c_str())); return;

        } else if (func_name == "__builtin_rest_formatter_unauthorized__") {
            if (arguments.size() < 1) throw TypeError("rest_formatter_unauthorized() requires 1-2 arguments");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string msg = arguments.size() >= 2 ? std::get<std::string>(arguments[1]) : "Unauthorized";
            last_value = std::string(rest_formatter_unauthorized(f, msg.c_str())); return;

        } else if (func_name == "__builtin_rest_formatter_forbidden__") {
            if (arguments.size() < 1) throw TypeError("rest_formatter_forbidden() requires 1-2 arguments");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string msg = arguments.size() >= 2 ? std::get<std::string>(arguments[1]) : "Forbidden";
            last_value = std::string(rest_formatter_forbidden(f, msg.c_str())); return;

        } else if (func_name == "__builtin_rest_formatter_validation_error__") {
            if (arguments.size() != 2) throw TypeError("rest_formatter_validation_error() requires 2 arguments");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string errs = std::get<std::string>(arguments[1]);
            last_value = std::string(rest_formatter_validation_error(f, errs.c_str())); return;

        } else if (func_name == "__builtin_rest_formatter_paginated__") {
            if (arguments.size() != 5) throw TypeError("rest_formatter_paginated() requires 5 arguments (formatter, data, page, per_page, total)");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string data = std::get<std::string>(arguments[1]);
            int page     = std::get<int>(arguments[2]);
            int per_page = std::get<int>(arguments[3]);
            int total    = std::get<int>(arguments[4]);
            last_value = std::string(rest_formatter_paginated(f, data.c_str(), page, per_page, total)); return;

        } else if (func_name == "__builtin_rest_formatter_pretty__") {
            if (arguments.size() != 2) throw TypeError("rest_formatter_pretty() requires 2 arguments");
            void* f = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string json = std::get<std::string>(arguments[1]);
            last_value = std::string(rest_formatter_pretty(f, json.c_str())); return;

        } else if (func_name == "__builtin_rest_docs_create__") {
            std::string title = arguments.size() >= 1 ? std::get<std::string>(arguments[0]) : "API";
            std::string ver   = arguments.size() >= 2 ? std::get<std::string>(arguments[1]) : "1.0.0";
            void* d = rest_docs_create(title.c_str(), ver.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(d)); return;

        } else if (func_name == "__builtin_rest_docs_add_endpoint__") {
            if (arguments.size() < 3) throw TypeError("rest_docs_add_endpoint() requires 3-4 arguments (docs, method, path, [summary], [auth])");
            void* d = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string method  = std::get<std::string>(arguments[1]);
            std::string path    = std::get<std::string>(arguments[2]);
            std::string summary = arguments.size() >= 4 ? std::get<std::string>(arguments[3]) : "";
            int auth = arguments.size() >= 5 ? (std::holds_alternative<bool>(arguments[4]) ? (std::get<bool>(arguments[4]) ? 1 : 0) : std::get<int>(arguments[4])) : 0;
            rest_docs_add_endpoint(d, method.c_str(), path.c_str(), summary.c_str(), auth);
            last_value = nullptr; return;

        } else if (func_name == "__builtin_rest_docs_from_router__") {
            if (arguments.size() != 2) throw TypeError("rest_docs_from_router() requires 2 arguments (docs, router)");
            void* d = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            void* r = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[1])));
            rest_docs_from_router(d, r);
            last_value = nullptr; return;

        } else if (func_name == "__builtin_rest_docs_openapi__") {
            if (arguments.size() != 1) throw TypeError("rest_docs_openapi() requires 1 argument");
            void* d = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(rest_docs_openapi(d)); return;

        } else if (func_name == "__builtin_rest_docs_markdown__") {
            if (arguments.size() != 1) throw TypeError("rest_docs_markdown() requires 1 argument");
            void* d = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(rest_docs_markdown(d)); return;

        } else if (func_name == "__builtin_rest_docs_html__") {
            if (arguments.size() != 1) throw TypeError("rest_docs_html() requires 1 argument");
            void* d = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(rest_docs_html(d)); return;

        } else if (func_name == "__builtin_rest_docs_endpoint_count__") {
            if (arguments.size() != 1) throw TypeError("rest_docs_endpoint_count() requires 1 argument");
            void* d = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = rest_docs_endpoint_count(d); return;

        // ===== GraphQL Engine =====
        } else if (func_name == "__builtin_gql_schema_create__") {
            void* s = gql_schema_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(s)); return;

        } else if (func_name == "__builtin_gql_schema_add_type__") {
            if (arguments.size() < 2) throw TypeError("gql_schema_add_type() requires 2-3 arguments (schema, name, [kind], [description])");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string name = std::get<std::string>(arguments[1]);
            std::string kind = arguments.size() >= 3 ? std::get<std::string>(arguments[2]) : "type";
            std::string desc = arguments.size() >= 4 ? std::get<std::string>(arguments[3]) : "";
            gql_schema_add_type(s, name.c_str(), kind.c_str(), desc.c_str()); last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_schema_add_field__") {
            if (arguments.size() < 4) throw TypeError("gql_schema_add_field() requires 4+ arguments (schema, type, field, field_type, [non_null], [is_list], [desc])");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string type_name  = std::get<std::string>(arguments[1]);
            std::string field_name = std::get<std::string>(arguments[2]);
            std::string field_type = std::get<std::string>(arguments[3]);
            int non_null = arguments.size() >= 5 ? (std::holds_alternative<bool>(arguments[4]) ? (std::get<bool>(arguments[4]) ? 1 : 0) : std::get<int>(arguments[4])) : 0;
            int is_list  = arguments.size() >= 6 ? (std::holds_alternative<bool>(arguments[5]) ? (std::get<bool>(arguments[5]) ? 1 : 0) : std::get<int>(arguments[5])) : 0;
            std::string desc = arguments.size() >= 7 ? std::get<std::string>(arguments[6]) : "";
            gql_schema_add_field(s, type_name.c_str(), field_name.c_str(), field_type.c_str(), non_null, is_list, desc.c_str());
            last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_schema_add_scalar__") {
            if (arguments.size() < 2) throw TypeError("gql_schema_add_scalar() requires 2 arguments");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string name = std::get<std::string>(arguments[1]);
            std::string desc = arguments.size() >= 3 ? std::get<std::string>(arguments[2]) : "";
            gql_schema_add_scalar(s, name.c_str(), desc.c_str()); last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_schema_add_enum__") {
            if (arguments.size() < 3) throw TypeError("gql_schema_add_enum() requires 3 arguments (schema, name, values_csv)");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string name = std::get<std::string>(arguments[1]);
            std::string vals = std::get<std::string>(arguments[2]);
            gql_schema_add_enum(s, name.c_str(), vals.c_str()); last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_schema_set_query__") {
            if (arguments.size() != 2) throw TypeError("gql_schema_set_query() requires 2 arguments");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            gql_schema_set_query(s, std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_schema_set_mutation__") {
            if (arguments.size() != 2) throw TypeError("gql_schema_set_mutation() requires 2 arguments");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            gql_schema_set_mutation(s, std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_schema_set_subscription__") {
            if (arguments.size() != 2) throw TypeError("gql_schema_set_subscription() requires 2 arguments");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            gql_schema_set_subscription(s, std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_schema_has_type__") {
            if (arguments.size() != 2) throw TypeError("gql_schema_has_type() requires 2 arguments");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = (gql_schema_has_type(s, std::get<std::string>(arguments[1]).c_str()) == 1); return;

        } else if (func_name == "__builtin_gql_schema_type_count__") {
            if (arguments.size() != 1) throw TypeError("gql_schema_type_count() requires 1 argument");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = gql_schema_type_count(s); return;

        } else if (func_name == "__builtin_gql_schema_validate__") {
            if (arguments.size() != 1) throw TypeError("gql_schema_validate() requires 1 argument");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            char err[512] = {};
            last_value = (gql_schema_validate(s, err, sizeof(err)) == 1); return;

        } else if (func_name == "__builtin_gql_schema_to_sdl__") {
            if (arguments.size() != 1) throw TypeError("gql_schema_to_sdl() requires 1 argument");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(gql_schema_to_sdl(s)); return;

        } else if (func_name == "__builtin_gql_schema_introspect__") {
            if (arguments.size() != 1) throw TypeError("gql_schema_introspect() requires 1 argument");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(gql_schema_introspect(s)); return;

        } else if (func_name == "__builtin_gql_query_engine_create__") {
            if (arguments.size() != 1) throw TypeError("gql_query_engine_create() requires 1 argument (schema)");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            void* e = gql_query_engine_create(s);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(e)); return;

        } else if (func_name == "__builtin_gql_query_engine_register_data__") {
            if (arguments.size() != 4) throw TypeError("gql_query_engine_register_data() requires 4 arguments (engine, type, field, json)");
            void* e = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string type_name  = std::get<std::string>(arguments[1]);
            std::string field_name = std::get<std::string>(arguments[2]);
            std::string json_data  = std::get<std::string>(arguments[3]);
            gql_query_engine_register_data(e, type_name.c_str(), field_name.c_str(), json_data.c_str());
            last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_query_execute__") {
            if (arguments.size() < 2) throw TypeError("gql_query_execute() requires 2-3 arguments (engine, query, [variables])");
            void* e = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string query = std::get<std::string>(arguments[1]);
            std::string vars  = arguments.size() >= 3 ? std::get<std::string>(arguments[2]) : "{}";
            last_value = std::string(gql_query_execute(e, query.c_str(), vars.c_str())); return;

        } else if (func_name == "__builtin_gql_query_complexity__") {
            if (arguments.size() != 2) throw TypeError("gql_query_complexity() requires 2 arguments");
            void* e = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string query = std::get<std::string>(arguments[1]);
            last_value = gql_query_complexity(e, query.c_str()); return;

        } else if (func_name == "__builtin_gql_query_resolver_count__") {
            if (arguments.size() != 1) throw TypeError("gql_query_resolver_count() requires 1 argument");
            void* e = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = gql_query_resolver_count(e); return;

        } else if (func_name == "__builtin_gql_mutation_engine_create__") {
            if (arguments.size() != 1) throw TypeError("gql_mutation_engine_create() requires 1 argument (schema)");
            void* s = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            void* m = gql_mutation_engine_create(s);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;

        } else if (func_name == "__builtin_gql_mutation_register__") {
            if (arguments.size() != 3) throw TypeError("gql_mutation_register() requires 3 arguments (engine, name, result_json)");
            void* m = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string name   = std::get<std::string>(arguments[1]);
            std::string result = std::get<std::string>(arguments[2]);
            gql_mutation_register(m, name.c_str(), result.c_str()); last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_mutation_execute__") {
            if (arguments.size() < 2) throw TypeError("gql_mutation_execute() requires 2-3 arguments");
            void* m = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string mutation = std::get<std::string>(arguments[1]);
            std::string vars     = arguments.size() >= 3 ? std::get<std::string>(arguments[2]) : "{}";
            last_value = std::string(gql_mutation_execute(m, mutation.c_str(), vars.c_str())); return;

        } else if (func_name == "__builtin_gql_mutation_count__") {
            if (arguments.size() != 1) throw TypeError("gql_mutation_count() requires 1 argument");
            void* m = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = gql_mutation_count(m); return;

        } else if (func_name == "__builtin_gql_subscription_manager_create__") {
            void* mgr = gql_subscription_manager_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mgr)); return;

        } else if (func_name == "__builtin_gql_subscribe__") {
            if (arguments.size() < 3) throw TypeError("gql_subscribe() requires 3-4 arguments (mgr, client_id, event, [query])");
            void* mgr = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string client = std::get<std::string>(arguments[1]);
            std::string event  = std::get<std::string>(arguments[2]);
            std::string query  = arguments.size() >= 4 ? std::get<std::string>(arguments[3]) : "";
            last_value = std::string(gql_subscribe(mgr, client.c_str(), event.c_str(), query.c_str())); return;

        } else if (func_name == "__builtin_gql_unsubscribe__") {
            if (arguments.size() != 2) throw TypeError("gql_unsubscribe() requires 2 arguments");
            void* mgr = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string sub_id = std::get<std::string>(arguments[1]);
            last_value = (gql_unsubscribe(mgr, sub_id.c_str()) == 1); return;

        } else if (func_name == "__builtin_gql_unsubscribe_client__") {
            if (arguments.size() != 2) throw TypeError("gql_unsubscribe_client() requires 2 arguments");
            void* mgr = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            gql_unsubscribe_client(mgr, std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;

        } else if (func_name == "__builtin_gql_publish__") {
            if (arguments.size() != 3) throw TypeError("gql_publish() requires 3 arguments (mgr, event, data_json)");
            void* mgr = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string event = std::get<std::string>(arguments[1]);
            std::string data  = std::get<std::string>(arguments[2]);
            last_value = std::string(gql_publish(mgr, event.c_str(), data.c_str())); return;

        } else if (func_name == "__builtin_gql_subscription_count__") {
            if (arguments.size() != 1) throw TypeError("gql_subscription_count() requires 1 argument");
            void* mgr = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = gql_subscription_count(mgr); return;

        } else if (func_name == "__builtin_gql_client_subscription_count__") {
            if (arguments.size() != 2) throw TypeError("gql_client_subscription_count() requires 2 arguments");
            void* mgr = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = gql_client_subscription_count(mgr, std::get<std::string>(arguments[1]).c_str()); return;

        } else if (func_name == "__builtin_gql_is_subscribed__") {
            if (arguments.size() != 3) throw TypeError("gql_is_subscribed() requires 3 arguments");
            void* mgr = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string client = std::get<std::string>(arguments[1]);
            std::string event  = std::get<std::string>(arguments[2]);
            last_value = (gql_is_subscribed(mgr, client.c_str(), event.c_str()) == 1); return;

        // ===== Milestone 6: Blockchain Builtins =====
        } else if (func_name == "__builtin_bc_sha256__") {
        if (arguments.size() != 1) throw TypeError("bc_sha256() requires 1 argument");
        std::string data = std::get<std::string>(arguments[0]);
        last_value = std::string(bc_sha256(data.c_str())); return;
    } else if (func_name == "__builtin_bc_sha256_double__") {
        if (arguments.size() != 1) throw TypeError("bc_sha256_double() requires 1 argument");
        std::string data = std::get<std::string>(arguments[0]);
        last_value = std::string(bc_sha256_double(data.c_str())); return;
    } else if (func_name == "__builtin_bc_merkle_create__") {
        void* t = bc_merkle_create();
        last_value = static_cast<int>(reinterpret_cast<intptr_t>(t)); return;
    } else if (func_name == "__builtin_bc_merkle_add_leaf__") {
        if (arguments.size() != 2) throw TypeError("bc_merkle_add_leaf() requires 2 arguments");
        void* t = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string data = std::get<std::string>(arguments[1]);
        bc_merkle_add_leaf(t, data.c_str()); last_value = nullptr; return;
    } else if (func_name == "__builtin_bc_merkle_build__") {
        if (arguments.size() != 1) throw TypeError("bc_merkle_build() requires 1 argument");
        void* t = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_merkle_build(t)); return;
    } else if (func_name == "__builtin_bc_merkle_root__") {
        if (arguments.size() != 1) throw TypeError("bc_merkle_root() requires 1 argument");
        void* t = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_merkle_root(t)); return;
    } else if (func_name == "__builtin_bc_merkle_leaf_count__") {
        if (arguments.size() != 1) throw TypeError("bc_merkle_leaf_count() requires 1 argument");
        void* t = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = bc_merkle_leaf_count(t); return;
    } else if (func_name == "__builtin_bc_tx_create__") {
        if (arguments.size() < 3) throw TypeError("bc_tx_create() requires 3-4 arguments");
        std::string sender   = std::get<std::string>(arguments[0]);
        std::string receiver = std::get<std::string>(arguments[1]);
        double amount = std::holds_alternative<double>(arguments[2])
            ? std::get<double>(arguments[2])
            : static_cast<double>(std::get<int>(arguments[2]));
        std::string data = arguments.size() > 3 ? std::get<std::string>(arguments[3]) : "";
        void* tx = bc_tx_create(sender.c_str(), receiver.c_str(), amount, data.c_str());
        last_value = static_cast<int>(reinterpret_cast<intptr_t>(tx)); return;
    } else if (func_name == "__builtin_bc_tx_id__") {
        if (arguments.size() != 1) throw TypeError("bc_tx_id() requires 1 argument");
        void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_tx_id(tx)); return;
    } else if (func_name == "__builtin_bc_tx_hash__") {
        if (arguments.size() != 1) throw TypeError("bc_tx_hash() requires 1 argument");
        void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_tx_hash(tx)); return;
    } else if (func_name == "__builtin_bc_tx_to_string__") {
        if (arguments.size() != 1) throw TypeError("bc_tx_to_string() requires 1 argument");
        void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_tx_to_string(tx)); return;
    } else if (func_name == "__builtin_bc_block_create__") {
        if (arguments.size() < 2) throw TypeError("bc_block_create() requires 2-3 arguments");
        int index = std::get<int>(arguments[0]);
        std::string prev = std::get<std::string>(arguments[1]);
        int diff = arguments.size() > 2 ? std::get<int>(arguments[2]) : 2;
        void* b = bc_block_create(index, prev.c_str(), diff);
        last_value = static_cast<int>(reinterpret_cast<intptr_t>(b)); return;
    } else if (func_name == "__builtin_bc_block_add_tx__") {
        if (arguments.size() != 2) throw TypeError("bc_block_add_tx() requires 2 arguments");
        void* b  = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[1])));
        bc_block_add_tx(b, tx); last_value = nullptr; return;
    } else if (func_name == "__builtin_bc_block_mine__") {
        if (arguments.size() != 1) throw TypeError("bc_block_mine() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        bc_block_mine(b); last_value = nullptr; return;
    } else if (func_name == "__builtin_bc_block_hash__") {
        if (arguments.size() != 1) throw TypeError("bc_block_hash() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_block_hash(b)); return;
    } else if (func_name == "__builtin_bc_block_prev_hash__") {
        if (arguments.size() != 1) throw TypeError("bc_block_prev_hash() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_block_prev_hash(b)); return;
    } else if (func_name == "__builtin_bc_block_index__") {
        if (arguments.size() != 1) throw TypeError("bc_block_index() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = bc_block_index(b); return;
    } else if (func_name == "__builtin_bc_block_nonce__") {
        if (arguments.size() != 1) throw TypeError("bc_block_nonce() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = bc_block_nonce(b); return;
    } else if (func_name == "__builtin_bc_block_tx_count__") {
        if (arguments.size() != 1) throw TypeError("bc_block_tx_count() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = bc_block_tx_count(b); return;
    } else if (func_name == "__builtin_bc_block_is_valid__") {
        if (arguments.size() != 1) throw TypeError("bc_block_is_valid() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = (bc_block_is_valid(b) == 1); return;
    } else if (func_name == "__builtin_bc_block_to_string__") {
        if (arguments.size() != 1) throw TypeError("bc_block_to_string() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_block_to_string(b)); return;
    } else if (func_name == "__builtin_bc_block_merkle_root__") {
        if (arguments.size() != 1) throw TypeError("bc_block_merkle_root() requires 1 argument");
        void* b = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_block_merkle_root(b)); return;
    } else if (func_name == "__builtin_bc_chain_create__") {
        int diff      = arguments.size() > 0 ? std::get<int>(arguments[0]) : 2;
        int consensus = arguments.size() > 1 ? std::get<int>(arguments[1]) : 0;
        void* c = bc_chain_create(diff, consensus);
        last_value = static_cast<int>(reinterpret_cast<intptr_t>(c)); return;
    } else if (func_name == "__builtin_bc_chain_add_tx__") {
        if (arguments.size() < 3) throw TypeError("bc_chain_add_tx() requires 3-4 arguments");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string sender   = std::get<std::string>(arguments[1]);
        std::string receiver = std::get<std::string>(arguments[2]);
        double amount = arguments.size() > 3
            ? (std::holds_alternative<double>(arguments[3])
                ? std::get<double>(arguments[3])
                : static_cast<double>(std::get<int>(arguments[3])))
            : 0.0;
        std::string data = arguments.size() > 4 ? std::get<std::string>(arguments[4]) : "";
        last_value = std::string(bc_chain_add_tx(c, sender.c_str(), receiver.c_str(), amount, data.c_str())); return;
    } else if (func_name == "__builtin_bc_chain_mine_block__") {
        if (arguments.size() < 1) throw TypeError("bc_chain_mine_block() requires 1-2 arguments");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string miner = arguments.size() > 1 ? std::get<std::string>(arguments[1]) : "";
        last_value = (bc_chain_mine_block(c, miner.c_str()) == 1); return;
    } else if (func_name == "__builtin_bc_chain_block_count__") {
        if (arguments.size() != 1) throw TypeError("bc_chain_block_count() requires 1 argument");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = bc_chain_block_count(c); return;
    } else if (func_name == "__builtin_bc_chain_pending_count__") {
        if (arguments.size() != 1) throw TypeError("bc_chain_pending_count() requires 1 argument");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = bc_chain_pending_count(c); return;
    } else if (func_name == "__builtin_bc_chain_is_valid__") {
        if (arguments.size() != 1) throw TypeError("bc_chain_is_valid() requires 1 argument");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = (bc_chain_is_valid(c) == 1); return;
    } else if (func_name == "__builtin_bc_chain_get_balance__") {
        if (arguments.size() != 2) throw TypeError("bc_chain_get_balance() requires 2 arguments");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string addr = std::get<std::string>(arguments[1]);
        last_value = bc_chain_get_balance(c, addr.c_str()); return;
    } else if (func_name == "__builtin_bc_chain_last_hash__") {
        if (arguments.size() != 1) throw TypeError("bc_chain_last_hash() requires 1 argument");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_chain_last_hash(c)); return;
    } else if (func_name == "__builtin_bc_chain_get_block_hash__") {
        if (arguments.size() != 2) throw TypeError("bc_chain_get_block_hash() requires 2 arguments");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        int idx = std::get<int>(arguments[1]);
        last_value = std::string(bc_chain_get_block_hash(c, idx)); return;
    } else if (func_name == "__builtin_bc_chain_stats__") {
        if (arguments.size() != 1) throw TypeError("bc_chain_stats() requires 1 argument");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_chain_stats(c)); return;
    } else if (func_name == "__builtin_bc_chain_add_validator__") {
        if (arguments.size() != 3) throw TypeError("bc_chain_add_validator() requires 3 arguments");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string addr = std::get<std::string>(arguments[1]);
        double stake = std::holds_alternative<double>(arguments[2])
            ? std::get<double>(arguments[2])
            : static_cast<double>(std::get<int>(arguments[2]));
        bc_chain_add_validator(c, addr.c_str(), stake);
        last_value = nullptr; return;
    } else if (func_name == "__builtin_bc_chain_select_validator__") {
        if (arguments.size() != 1) throw TypeError("bc_chain_select_validator() requires 1 argument");
        void* c = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = std::string(bc_chain_select_validator(c)); return;
    } else if (func_name == "__builtin_bc_p2p_create__") {
        void* net = bc_p2p_create();
        last_value = static_cast<int>(reinterpret_cast<intptr_t>(net)); return;
    } else if (func_name == "__builtin_bc_p2p_add_node__") {
        if (arguments.size() < 1) throw TypeError("bc_p2p_add_node() requires 1-3 arguments");
        void* net = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string addr = arguments.size() > 1 ? std::get<std::string>(arguments[1]) : "127.0.0.1";
        int port = arguments.size() > 2 ? std::get<int>(arguments[2]) : 0;
        last_value = std::string(bc_p2p_add_node(net, addr.c_str(), port)); return;
    } else if (func_name == "__builtin_bc_p2p_connect__") {
        if (arguments.size() != 2) throw TypeError("bc_p2p_connect() requires 2 arguments");
        void* net = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string nid = std::get<std::string>(arguments[1]);
        last_value = (bc_p2p_connect(net, nid.c_str()) == 1); return;
    } else if (func_name == "__builtin_bc_p2p_disconnect__") {
        if (arguments.size() != 2) throw TypeError("bc_p2p_disconnect() requires 2 arguments");
        void* net = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string nid = std::get<std::string>(arguments[1]);
        last_value = (bc_p2p_disconnect(net, nid.c_str()) == 1); return;
    } else if (func_name == "__builtin_bc_p2p_broadcast_block__") {
        if (arguments.size() != 3) throw TypeError("bc_p2p_broadcast_block() requires 3 arguments");
        void* net = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string nid  = std::get<std::string>(arguments[1]);
        std::string hash = std::get<std::string>(arguments[2]);
        last_value = (bc_p2p_broadcast_block(net, nid.c_str(), hash.c_str()) == 1); return;
    } else if (func_name == "__builtin_bc_p2p_broadcast_tx__") {
        if (arguments.size() != 3) throw TypeError("bc_p2p_broadcast_tx() requires 3 arguments");
        void* net = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        std::string nid = std::get<std::string>(arguments[1]);
        std::string tid = std::get<std::string>(arguments[2]);
        last_value = (bc_p2p_broadcast_tx(net, nid.c_str(), tid.c_str()) == 1); return;
    } else if (func_name == "__builtin_bc_p2p_node_count__") {
        if (arguments.size() != 1) throw TypeError("bc_p2p_node_count() requires 1 argument");
        void* net = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = bc_p2p_node_count(net); return;
    } else if (func_name == "__builtin_bc_p2p_connected_count__") {
        if (arguments.size() != 1) throw TypeError("bc_p2p_connected_count() requires 1 argument");
        void* net = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
        last_value = bc_p2p_connected_count(net); return;
        } else if (func_name == "__builtin_bc_p2p_status__") {
            if (arguments.size() != 1) throw TypeError("bc_p2p_status() requires 1 argument");
            void* net = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(bc_p2p_status(net)); return;

        // ===== Milestone 6 Phase 2: Cryptography Builtins =====
        } else if (func_name == "__builtin_crypto_sha256__") {
            if (arguments.size() != 1) throw TypeError("crypto_sha256() requires 1 argument");
            last_value = std::string(crypto_sha256(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_sha512__") {
            if (arguments.size() != 1) throw TypeError("crypto_sha512() requires 1 argument");
            last_value = std::string(crypto_sha512(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_sha1__") {
            if (arguments.size() != 1) throw TypeError("crypto_sha1() requires 1 argument");
            last_value = std::string(crypto_sha1(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_md5__") {
            if (arguments.size() != 1) throw TypeError("crypto_md5() requires 1 argument");
            last_value = std::string(crypto_md5(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_ripemd160__") {
            if (arguments.size() != 1) throw TypeError("crypto_ripemd160() requires 1 argument");
            last_value = std::string(crypto_ripemd160(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_hmac_sha256__") {
            if (arguments.size() != 2) throw TypeError("crypto_hmac_sha256() requires 2 arguments");
            last_value = std::string(crypto_hmac_sha256(
                std::get<std::string>(arguments[0]).c_str(),
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_crypto_hmac_sha512__") {
            if (arguments.size() != 2) throw TypeError("crypto_hmac_sha512() requires 2 arguments");
            last_value = std::string(crypto_hmac_sha512(
                std::get<std::string>(arguments[0]).c_str(),
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_crypto_to_base64__") {
            if (arguments.size() != 1) throw TypeError("crypto_to_base64() requires 1 argument");
            last_value = std::string(crypto_to_base64(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_from_base64__") {
            if (arguments.size() != 1) throw TypeError("crypto_from_base64() requires 1 argument");
            last_value = std::string(crypto_from_base64(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_keypair_generate__") {
            std::string curve = arguments.size() > 0 ? std::get<std::string>(arguments[0]) : "secp256k1";
            void* kp = crypto_keypair_generate(curve.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(kp)); return;
        } else if (func_name == "__builtin_crypto_keypair_from_private__") {
            if (arguments.size() < 1) throw TypeError("crypto_keypair_from_private() requires 1-2 arguments");
            std::string priv = std::get<std::string>(arguments[0]);
            std::string curve = arguments.size() > 1 ? std::get<std::string>(arguments[1]) : "secp256k1";
            void* kp = crypto_keypair_from_private(priv.c_str(), curve.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(kp)); return;
        } else if (func_name == "__builtin_crypto_keypair_destroy__") {
            if (arguments.size() != 1) throw TypeError("crypto_keypair_destroy() requires 1 argument");
            void* kp = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            crypto_keypair_destroy(kp); last_value = nullptr; return;
        } else if (func_name == "__builtin_crypto_keypair_private_pem__") {
            if (arguments.size() != 1) throw TypeError("crypto_keypair_private_pem() requires 1 argument");
            void* kp = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(crypto_keypair_private_pem(kp)); return;
        } else if (func_name == "__builtin_crypto_keypair_public_pem__") {
            if (arguments.size() != 1) throw TypeError("crypto_keypair_public_pem() requires 1 argument");
            void* kp = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(crypto_keypair_public_pem(kp)); return;
        } else if (func_name == "__builtin_crypto_keypair_private_hex__") {
            if (arguments.size() != 1) throw TypeError("crypto_keypair_private_hex() requires 1 argument");
            void* kp = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(crypto_keypair_private_hex(kp)); return;
        } else if (func_name == "__builtin_crypto_keypair_public_hex__") {
            if (arguments.size() != 1) throw TypeError("crypto_keypair_public_hex() requires 1 argument");
            void* kp = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(crypto_keypair_public_hex(kp)); return;
        } else if (func_name == "__builtin_crypto_keypair_curve__") {
            if (arguments.size() != 1) throw TypeError("crypto_keypair_curve() requires 1 argument");
            void* kp = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(crypto_keypair_curve(kp)); return;
        } else if (func_name == "__builtin_crypto_ecdsa_sign__") {
            if (arguments.size() != 2) throw TypeError("crypto_ecdsa_sign() requires 2 arguments");
            void* kp = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string msg = std::get<std::string>(arguments[1]);
            last_value = std::string(crypto_ecdsa_sign(kp, msg.c_str())); return;
        } else if (func_name == "__builtin_crypto_ecdsa_verify__") {
            if (arguments.size() != 3) throw TypeError("crypto_ecdsa_verify() requires 3 arguments");
            void* kp = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string msg = std::get<std::string>(arguments[1]);
            std::string sig = std::get<std::string>(arguments[2]);
            last_value = (crypto_ecdsa_verify(kp, msg.c_str(), sig.c_str()) == 1); return;
        } else if (func_name == "__builtin_crypto_ecdsa_verify_pubhex__") {
            if (arguments.size() != 4) throw TypeError("crypto_ecdsa_verify_pubhex() requires 4 arguments");
            std::string pub   = std::get<std::string>(arguments[0]);
            std::string curve = std::get<std::string>(arguments[1]);
            std::string msg   = std::get<std::string>(arguments[2]);
            std::string sig   = std::get<std::string>(arguments[3]);
            last_value = (crypto_ecdsa_verify_pubhex(pub.c_str(), curve.c_str(), msg.c_str(), sig.c_str()) == 1); return;
        } else if (func_name == "__builtin_crypto_pubkey_to_eth_address__") {
            if (arguments.size() != 1) throw TypeError("crypto_pubkey_to_eth_address() requires 1 argument");
            last_value = std::string(crypto_pubkey_to_eth_address(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_pubkey_to_btc_address__") {
            if (arguments.size() != 1) throw TypeError("crypto_pubkey_to_btc_address() requires 1 argument");
            last_value = std::string(crypto_pubkey_to_btc_address(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_privkey_to_wif__") {
            if (arguments.size() < 1) throw TypeError("crypto_privkey_to_wif() requires 1-2 arguments");
            std::string priv = std::get<std::string>(arguments[0]);
            int compressed = arguments.size() > 1 ? (std::get<bool>(arguments[1]) ? 1 : 0) : 1;
            last_value = std::string(crypto_privkey_to_wif(priv.c_str(), compressed)); return;
        } else if (func_name == "__builtin_crypto_wif_to_privkey__") {
            if (arguments.size() != 1) throw TypeError("crypto_wif_to_privkey() requires 1 argument");
            last_value = std::string(crypto_wif_to_privkey(std::get<std::string>(arguments[0]).c_str())); return;
        } else if (func_name == "__builtin_crypto_aes_encrypt__") {
            if (arguments.size() != 2) throw TypeError("crypto_aes_encrypt() requires 2 arguments");
            std::string key = std::get<std::string>(arguments[0]);
            std::string pt  = std::get<std::string>(arguments[1]);
            void* res = crypto_aes_encrypt(key.c_str(), pt.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(res)); return;
        } else if (func_name == "__builtin_crypto_aes_result_destroy__") {
            if (arguments.size() != 1) throw TypeError("crypto_aes_result_destroy() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            crypto_aes_result_destroy(res); last_value = nullptr; return;
        } else if (func_name == "__builtin_crypto_aes_ciphertext__") {
            if (arguments.size() != 1) throw TypeError("crypto_aes_ciphertext() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(crypto_aes_ciphertext(res)); return;
        } else if (func_name == "__builtin_crypto_aes_iv__") {
            if (arguments.size() != 1) throw TypeError("crypto_aes_iv() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(crypto_aes_iv(res)); return;
        } else if (func_name == "__builtin_crypto_aes_tag__") {
            if (arguments.size() != 1) throw TypeError("crypto_aes_tag() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(crypto_aes_tag(res)); return;
        } else if (func_name == "__builtin_crypto_aes_decrypt__") {
            if (arguments.size() != 4) throw TypeError("crypto_aes_decrypt() requires 4 arguments");
            std::string key = std::get<std::string>(arguments[0]);
            std::string ct  = std::get<std::string>(arguments[1]);
            std::string iv  = std::get<std::string>(arguments[2]);
            std::string tag = std::get<std::string>(arguments[3]);
            last_value = std::string(crypto_aes_decrypt(key.c_str(), ct.c_str(), iv.c_str(), tag.c_str())); return;
        } else if (func_name == "__builtin_crypto_pbkdf2__") {
            if (arguments.size() < 2) throw TypeError("crypto_pbkdf2() requires 2-4 arguments");
            std::string pass = std::get<std::string>(arguments[0]);
            std::string salt = std::get<std::string>(arguments[1]);
            int iters   = arguments.size() > 2 ? std::get<int>(arguments[2]) : 100000;
            int key_len = arguments.size() > 3 ? std::get<int>(arguments[3]) : 32;
            last_value = std::string(crypto_pbkdf2(pass.c_str(), salt.c_str(), iters, key_len)); return;
        } else if (func_name == "__builtin_crypto_random_bytes__") {
            int count = arguments.size() > 0 ? std::get<int>(arguments[0]) : 32;
            last_value = std::string(crypto_random_bytes(count)); return;

        // ===== Milestone 6 Phase 3: Smart Contract VM =====
        } else if (func_name == "__builtin_sc_vm_create__") {
            void* vm = sc_vm_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(vm)); return;
        } else if (func_name == "__builtin_sc_vm_destroy__") {
            if (arguments.size() != 1) throw TypeError("sc_vm_destroy() requires 1 argument");
            void* vm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_vm_destroy(vm); last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_vm_set_storage__") {
            if (arguments.size() != 4) throw TypeError("sc_vm_set_storage() requires 4 arguments");
            void* vm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_vm_set_storage(vm,
                std::get<std::string>(arguments[1]).c_str(),
                std::get<std::string>(arguments[2]).c_str(),
                std::get<std::string>(arguments[3]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_vm_get_storage__") {
            if (arguments.size() != 3) throw TypeError("sc_vm_get_storage() requires 3 arguments");
            void* vm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(sc_vm_get_storage(vm,
                std::get<std::string>(arguments[1]).c_str(),
                std::get<std::string>(arguments[2]).c_str())); return;
        } else if (func_name == "__builtin_sc_vm_set_balance__") {
            if (arguments.size() != 3) throw TypeError("sc_vm_set_balance() requires 3 arguments");
            void* vm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            int64_t bal = static_cast<int64_t>(std::get<int>(arguments[2]));
            sc_vm_set_balance(vm, std::get<std::string>(arguments[1]).c_str(), bal);
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_vm_get_balance__") {
            if (arguments.size() != 2) throw TypeError("sc_vm_get_balance() requires 2 arguments");
            void* vm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = static_cast<int>(sc_vm_get_balance(vm,
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_sc_vm_state_dump__") {
            if (arguments.size() != 1) throw TypeError("sc_vm_state_dump() requires 1 argument");
            void* vm = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(sc_vm_state_dump(vm)); return;
        } else if (func_name == "__builtin_sc_bytecode_create__") {
            void* bc = sc_bytecode_create();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(bc)); return;
        } else if (func_name == "__builtin_sc_bytecode_destroy__") {
            if (arguments.size() != 1) throw TypeError("sc_bytecode_destroy() requires 1 argument");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_bytecode_destroy(bc); last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_push_int__") {
            if (arguments.size() != 2) throw TypeError("sc_bytecode_push_int() requires 2 arguments");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_bytecode_push_int(bc, static_cast<int64_t>(std::get<int>(arguments[1])));
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_push_str__") {
            if (arguments.size() != 2) throw TypeError("sc_bytecode_push_str() requires 2 arguments");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_bytecode_push_str(bc, std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_push_bool__") {
            if (arguments.size() != 2) throw TypeError("sc_bytecode_push_bool() requires 2 arguments");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_bytecode_push_bool(bc, std::get<bool>(arguments[1]) ? 1 : 0);
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_push_double__") {
            if (arguments.size() != 2) throw TypeError("sc_bytecode_push_double() requires 2 arguments");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            double dv = std::holds_alternative<double>(arguments[1])
                ? std::get<double>(arguments[1])
                : static_cast<double>(std::get<int>(arguments[1]));
            sc_bytecode_push_double(bc, dv); last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_emit__") {
            if (arguments.size() != 2) throw TypeError("sc_bytecode_emit() requires 2 arguments");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_bytecode_emit(bc, std::get<int>(arguments[1]));
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_label__") {
            if (arguments.size() != 2) throw TypeError("sc_bytecode_label() requires 2 arguments");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_bytecode_label(bc, std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_jump__") {
            if (arguments.size() != 2) throw TypeError("sc_bytecode_jump() requires 2 arguments");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_bytecode_jump(bc, std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_jumpi__") {
            if (arguments.size() != 2) throw TypeError("sc_bytecode_jumpi() requires 2 arguments");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_bytecode_jumpi(bc, std::get<std::string>(arguments[1]).c_str());
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_bytecode_size__") {
            if (arguments.size() != 1) throw TypeError("sc_bytecode_size() requires 1 argument");
            void* bc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = sc_bytecode_size(bc); return;
        } else if (func_name == "__builtin_sc_ctx_create__") {
            if (arguments.size() < 2) throw TypeError("sc_ctx_create() requires 2-4 arguments");
            std::string caller   = std::get<std::string>(arguments[0]);
            std::string contract = std::get<std::string>(arguments[1]);
            int64_t value    = arguments.size() > 2 ? static_cast<int64_t>(std::get<int>(arguments[2])) : 0;
            uint64_t gas_lim = arguments.size() > 3 ? static_cast<uint64_t>(std::get<int>(arguments[3])) : 1000000;
            void* ctx = sc_ctx_create(caller.c_str(), contract.c_str(), value, gas_lim);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(ctx)); return;
        } else if (func_name == "__builtin_sc_ctx_destroy__") {
            if (arguments.size() != 1) throw TypeError("sc_ctx_destroy() requires 1 argument");
            void* ctx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_ctx_destroy(ctx); last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_ctx_set_block__") {
            if (arguments.size() != 3) throw TypeError("sc_ctx_set_block() requires 3 arguments");
            void* ctx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_ctx_set_block(ctx,
                static_cast<uint64_t>(std::get<int>(arguments[1])),
                static_cast<uint64_t>(std::get<int>(arguments[2])));
            last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_execute__") {
            if (arguments.size() != 3) throw TypeError("sc_execute() requires 3 arguments");
            void* vm  = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            void* bc  = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[1])));
            void* ctx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[2])));
            void* res = sc_execute(vm, bc, ctx);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(res)); return;
        } else if (func_name == "__builtin_sc_result_destroy__") {
            if (arguments.size() != 1) throw TypeError("sc_result_destroy() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            sc_result_destroy(res); last_value = nullptr; return;
        } else if (func_name == "__builtin_sc_result_success__") {
            if (arguments.size() != 1) throw TypeError("sc_result_success() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = (sc_result_success(res) == 1); return;
        } else if (func_name == "__builtin_sc_result_return_value__") {
            if (arguments.size() != 1) throw TypeError("sc_result_return_value() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(sc_result_return_value(res)); return;
        } else if (func_name == "__builtin_sc_result_revert_reason__") {
            if (arguments.size() != 1) throw TypeError("sc_result_revert_reason() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(sc_result_revert_reason(res)); return;
        } else if (func_name == "__builtin_sc_result_gas_used__") {
            if (arguments.size() != 1) throw TypeError("sc_result_gas_used() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = static_cast<int>(sc_result_gas_used(res)); return;
        } else if (func_name == "__builtin_sc_result_event_count__") {
            if (arguments.size() != 1) throw TypeError("sc_result_event_count() requires 1 argument");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = sc_result_event_count(res); return;
        } else if (func_name == "__builtin_sc_result_event_name__") {
            if (arguments.size() != 2) throw TypeError("sc_result_event_name() requires 2 arguments");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(sc_result_event_name(res, std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sc_result_event_data__") {
            if (arguments.size() != 2) throw TypeError("sc_result_event_data() requires 2 arguments");
            void* res = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(sc_result_event_data(res, std::get<int>(arguments[1]))); return;

        // ===== Milestone 6 Phase 4: DApp Framework =====
        } else if (func_name == "__builtin_dapp_wallet_create__") {
            std::string net = arguments.size() > 0 ? std::get<std::string>(arguments[0]) : "local";
            void* w = dapp_wallet_create(net.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(w)); return;
        } else if (func_name == "__builtin_dapp_wallet_from_privkey__") {
            if (arguments.size() < 1) throw TypeError("dapp_wallet_from_privkey() requires 1-2 arguments");
            std::string priv = std::get<std::string>(arguments[0]);
            std::string net  = arguments.size() > 1 ? std::get<std::string>(arguments[1]) : "local";
            void* w = dapp_wallet_from_privkey(priv.c_str(), net.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(w)); return;
        } else if (func_name == "__builtin_dapp_wallet_destroy__") {
            if (arguments.size() != 1) throw TypeError("dapp_wallet_destroy() requires 1 argument");
            void* w = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            dapp_wallet_destroy(w); last_value = nullptr; return;
        } else if (func_name == "__builtin_dapp_wallet_address__") {
            if (arguments.size() != 1) throw TypeError("dapp_wallet_address() requires 1 argument");
            void* w = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_wallet_address(w)); return;
        } else if (func_name == "__builtin_dapp_wallet_pubkey_hex__") {
            if (arguments.size() != 1) throw TypeError("dapp_wallet_pubkey_hex() requires 1 argument");
            void* w = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_wallet_pubkey_hex(w)); return;
        } else if (func_name == "__builtin_dapp_wallet_privkey_hex__") {
            if (arguments.size() != 1) throw TypeError("dapp_wallet_privkey_hex() requires 1 argument");
            void* w = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_wallet_privkey_hex(w)); return;
        } else if (func_name == "__builtin_dapp_wallet_balance__") {
            if (arguments.size() != 1) throw TypeError("dapp_wallet_balance() requires 1 argument");
            void* w = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = dapp_wallet_balance(w); return;
        } else if (func_name == "__builtin_dapp_wallet_network__") {
            if (arguments.size() != 1) throw TypeError("dapp_wallet_network() requires 1 argument");
            void* w = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_wallet_network(w)); return;
        } else if (func_name == "__builtin_dapp_wallet_sign__") {
            if (arguments.size() != 2) throw TypeError("dapp_wallet_sign() requires 2 arguments");
            void* w = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_wallet_sign(w, std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_dapp_wallet_verify__") {
            if (arguments.size() != 3) throw TypeError("dapp_wallet_verify() requires 3 arguments");
            last_value = (dapp_wallet_verify(
                std::get<std::string>(arguments[0]).c_str(),
                std::get<std::string>(arguments[1]).c_str(),
                std::get<std::string>(arguments[2]).c_str()) == 1); return;
        } else if (func_name == "__builtin_dapp_wallet_to_string__") {
            if (arguments.size() != 1) throw TypeError("dapp_wallet_to_string() requires 1 argument");
            void* w = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_wallet_to_string(w)); return;
        } else if (func_name == "__builtin_dapp_provider_create__") {
            std::string net = arguments.size() > 0 ? std::get<std::string>(arguments[0]) : "local";
            std::string url = arguments.size() > 1 ? std::get<std::string>(arguments[1]) : "http://127.0.0.1:8545";
            void* p = dapp_provider_create(net.c_str(), url.c_str());
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(p)); return;
        } else if (func_name == "__builtin_dapp_provider_destroy__") {
            if (arguments.size() != 1) throw TypeError("dapp_provider_destroy() requires 1 argument");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            dapp_provider_destroy(p); last_value = nullptr; return;
        } else if (func_name == "__builtin_dapp_provider_network__") {
            if (arguments.size() != 1) throw TypeError("dapp_provider_network() requires 1 argument");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_provider_network(p)); return;
        } else if (func_name == "__builtin_dapp_provider_chain_id__") {
            if (arguments.size() != 1) throw TypeError("dapp_provider_chain_id() requires 1 argument");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_provider_chain_id(p)); return;
        } else if (func_name == "__builtin_dapp_provider_block_number__") {
            if (arguments.size() != 1) throw TypeError("dapp_provider_block_number() requires 1 argument");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = static_cast<int>(dapp_provider_block_number(p)); return;
        } else if (func_name == "__builtin_dapp_provider_mine_block__") {
            if (arguments.size() != 1) throw TypeError("dapp_provider_mine_block() requires 1 argument");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            dapp_provider_mine_block(p); last_value = nullptr; return;
        } else if (func_name == "__builtin_dapp_provider_get_balance__") {
            if (arguments.size() != 2) throw TypeError("dapp_provider_get_balance() requires 2 arguments");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = dapp_provider_get_balance(p, std::get<std::string>(arguments[1]).c_str()); return;
        } else if (func_name == "__builtin_dapp_provider_set_balance__") {
            if (arguments.size() != 3) throw TypeError("dapp_provider_set_balance() requires 3 arguments");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            double amt = std::holds_alternative<double>(arguments[2]) ? std::get<double>(arguments[2]) : static_cast<double>(std::get<int>(arguments[2]));
            dapp_provider_set_balance(p, std::get<std::string>(arguments[1]).c_str(), amt);
            last_value = nullptr; return;
        } else if (func_name == "__builtin_dapp_provider_transfer__") {
            if (arguments.size() != 4) throw TypeError("dapp_provider_transfer() requires 4 arguments");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            double amt = std::holds_alternative<double>(arguments[3]) ? std::get<double>(arguments[3]) : static_cast<double>(std::get<int>(arguments[3]));
            dapp_provider_transfer(p, std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str(), amt);
            last_value = nullptr; return;
        } else if (func_name == "__builtin_dapp_provider_status__") {
            if (arguments.size() != 1) throw TypeError("dapp_provider_status() requires 1 argument");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_provider_status(p)); return;
        } else if (func_name == "__builtin_dapp_provider_tx_count__") {
            if (arguments.size() != 1) throw TypeError("dapp_provider_tx_count() requires 1 argument");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = dapp_provider_tx_count(p); return;
        } else if (func_name == "__builtin_dapp_provider_get_tx__") {
            if (arguments.size() != 2) throw TypeError("dapp_provider_get_tx() requires 2 arguments");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            void* tx = dapp_provider_get_tx(p, std::get<int>(arguments[1]));
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(tx)); return;
        } else if (func_name == "__builtin_dapp_provider_contract_count__") {
            if (arguments.size() != 1) throw TypeError("dapp_provider_contract_count() requires 1 argument");
            void* p = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = dapp_provider_contract_count(p); return;
        } else if (func_name == "__builtin_dapp_deploy_contract__") {
            if (arguments.size() < 4) throw TypeError("dapp_deploy_contract() requires 4-5 arguments");
            void* p  = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string name = std::get<std::string>(arguments[1]);
            std::string bc   = std::get<std::string>(arguments[2]);
            std::string abi  = std::get<std::string>(arguments[3]);
            void* w  = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[4])));
            double val = arguments.size() > 5 ? (std::holds_alternative<double>(arguments[5]) ? std::get<double>(arguments[5]) : static_cast<double>(std::get<int>(arguments[5]))) : 0.0;
            void* dc = dapp_deploy_contract(p, name.c_str(), bc.c_str(), abi.c_str(), w, val);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(dc)); return;
        } else if (func_name == "__builtin_dapp_contract_destroy__") {
            if (arguments.size() != 1) throw TypeError("dapp_contract_destroy() requires 1 argument");
            void* dc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            dapp_contract_destroy(dc); last_value = nullptr; return;
        } else if (func_name == "__builtin_dapp_contract_address__") {
            if (arguments.size() != 1) throw TypeError("dapp_contract_address() requires 1 argument");
            void* dc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_contract_address(dc)); return;
        } else if (func_name == "__builtin_dapp_contract_name__") {
            if (arguments.size() != 1) throw TypeError("dapp_contract_name() requires 1 argument");
            void* dc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_contract_name(dc)); return;
        } else if (func_name == "__builtin_dapp_contract_deployer__") {
            if (arguments.size() != 1) throw TypeError("dapp_contract_deployer() requires 1 argument");
            void* dc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_contract_deployer(dc)); return;
        } else if (func_name == "__builtin_dapp_contract_deploy_block__") {
            if (arguments.size() != 1) throw TypeError("dapp_contract_deploy_block() requires 1 argument");
            void* dc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = static_cast<int>(dapp_contract_deploy_block(dc)); return;
        } else if (func_name == "__builtin_dapp_contract_to_string__") {
            if (arguments.size() != 1) throw TypeError("dapp_contract_to_string() requires 1 argument");
            void* dc = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_contract_to_string(dc)); return;
        } else if (func_name == "__builtin_dapp_call_contract__") {
            if (arguments.size() < 4) throw TypeError("dapp_call_contract() requires 4-6 arguments");
            void* p  = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            std::string addr = std::get<std::string>(arguments[1]);
            std::string fn   = std::get<std::string>(arguments[2]);
            std::string args = std::get<std::string>(arguments[3]);
            void* w  = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[4])));
            double val = arguments.size() > 5 ? (std::holds_alternative<double>(arguments[5]) ? std::get<double>(arguments[5]) : static_cast<double>(std::get<int>(arguments[5]))) : 0.0;
            int64_t gas = arguments.size() > 6 ? static_cast<int64_t>(std::get<int>(arguments[6])) : 1000000;
            void* tx = dapp_call_contract(p, addr.c_str(), fn.c_str(), args.c_str(), w, val, gas);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(tx)); return;
        } else if (func_name == "__builtin_dapp_tx_destroy__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_destroy() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            dapp_tx_destroy(tx); last_value = nullptr; return;
        } else if (func_name == "__builtin_dapp_tx_success__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_success() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = (dapp_tx_success(tx) == 1); return;
        } else if (func_name == "__builtin_dapp_tx_hash__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_hash() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_tx_hash(tx)); return;
        } else if (func_name == "__builtin_dapp_tx_from__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_from() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_tx_from(tx)); return;
        } else if (func_name == "__builtin_dapp_tx_to__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_to() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_tx_to(tx)); return;
        } else if (func_name == "__builtin_dapp_tx_value__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_value() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = dapp_tx_value(tx); return;
        } else if (func_name == "__builtin_dapp_tx_gas_used__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_gas_used() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = static_cast<int>(dapp_tx_gas_used(tx)); return;
        } else if (func_name == "__builtin_dapp_tx_block__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_block() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = static_cast<int>(dapp_tx_block(tx)); return;
        } else if (func_name == "__builtin_dapp_tx_revert_reason__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_revert_reason() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_tx_revert_reason(tx)); return;
        } else if (func_name == "__builtin_dapp_tx_to_string__") {
            if (arguments.size() != 1) throw TypeError("dapp_tx_to_string() requires 1 argument");
            void* tx = reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])));
            last_value = std::string(dapp_tx_to_string(tx)); return;
        } else if (func_name == "__builtin_dapp_abi_encode_call__") {
            if (arguments.size() != 2) throw TypeError("dapp_abi_encode_call() requires 2 arguments");
            last_value = std::string(dapp_abi_encode_call(
                std::get<std::string>(arguments[0]).c_str(),
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_dapp_abi_selector__") {
            if (arguments.size() != 1) throw TypeError("dapp_abi_selector() requires 1 argument");
            last_value = std::string(dapp_abi_selector(std::get<std::string>(arguments[0]).c_str())); return;

        // ===== security module handlers =====
        } else if (func_name == "__builtin_sec_scanner_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_scanner_create())); return;
        } else if (func_name == "__builtin_sec_scanner_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_scanner_destroy() requires 1 argument");
            sec_scanner_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_scanner_scan_host__") {
            if (arguments.size() < 2) throw TypeError("sec_scanner_scan_host() requires at least 2 arguments");
            std::string _ports = arguments.size() >= 3 ? std::get<std::string>(arguments[2]) : "";
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_scanner_scan_host(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), _ports.c_str()))); return;
        } else if (func_name == "__builtin_sec_scanner_vuln_db_size__") {
            if (arguments.size() != 1) throw TypeError("sec_scanner_vuln_db_size() requires 1 argument");
            last_value = static_cast<int>(sec_scanner_vuln_db_size(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_scanresult_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_scanresult_destroy() requires 1 argument");
            sec_scanresult_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_scanresult_target__") {
            if (arguments.size() != 1) throw TypeError("sec_scanresult_target() requires 1 argument");
            last_value = std::string(sec_scanresult_target(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_scanresult_reachable__") {
            if (arguments.size() != 1) throw TypeError("sec_scanresult_reachable() requires 1 argument");
            last_value = static_cast<int>(sec_scanresult_reachable(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_scanresult_open_ports__") {
            if (arguments.size() != 1) throw TypeError("sec_scanresult_open_ports() requires 1 argument");
            last_value = static_cast<int>(sec_scanresult_open_ports(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_scanresult_vuln_count__") {
            if (arguments.size() != 1) throw TypeError("sec_scanresult_vuln_count() requires 1 argument");
            last_value = static_cast<int>(sec_scanresult_vuln_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_scanresult_to_json__") {
            if (arguments.size() != 1) throw TypeError("sec_scanresult_to_json() requires 1 argument");
            last_value = std::string(sec_scanresult_to_json(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_scanresult_port_service__") {
            if (arguments.size() != 2) throw TypeError("sec_scanresult_port_service() requires 2 arguments");
            last_value = std::string(sec_scanresult_port_service(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_scanresult_port_number__") {
            if (arguments.size() != 2) throw TypeError("sec_scanresult_port_number() requires 2 arguments");
            last_value = static_cast<int>(sec_scanresult_port_number(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_scanresult_vuln_name__") {
            if (arguments.size() != 2) throw TypeError("sec_scanresult_vuln_name() requires 2 arguments");
            last_value = std::string(sec_scanresult_vuln_name(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_scanresult_vuln_severity__") {
            if (arguments.size() != 2) throw TypeError("sec_scanresult_vuln_severity() requires 2 arguments");
            last_value = std::string(sec_scanresult_vuln_severity(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_scanresult_vuln_remediation__") {
            if (arguments.size() != 2) throw TypeError("sec_scanresult_vuln_remediation() requires 2 arguments");
            last_value = std::string(sec_scanresult_vuln_remediation(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_web_tester_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_web_tester_create())); return;
        } else if (func_name == "__builtin_sec_web_tester_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_web_tester_destroy() requires 1 argument");
            sec_web_tester_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_web_scan__") {
            if (arguments.size() != 2) throw TypeError("sec_web_scan() requires 2 arguments");
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_web_scan(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_sec_web_test_sqli__") {
            if (arguments.size() != 3) throw TypeError("sec_web_test_sqli() requires 3 arguments");
            last_value = static_cast<int>(sec_web_test_sqli(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str())); return;
        } else if (func_name == "__builtin_sec_web_test_xss__") {
            if (arguments.size() != 3) throw TypeError("sec_web_test_xss() requires 3 arguments");
            last_value = static_cast<int>(sec_web_test_xss(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str())); return;
        } else if (func_name == "__builtin_sec_web_test_csrf__") {
            if (arguments.size() != 2) throw TypeError("sec_web_test_csrf() requires 2 arguments");
            last_value = static_cast<int>(sec_web_test_csrf(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_sec_web_test_auth_bypass__") {
            if (arguments.size() != 2) throw TypeError("sec_web_test_auth_bypass() requires 2 arguments");
            last_value = static_cast<int>(sec_web_test_auth_bypass(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_sec_web_test_path_traversal__") {
            if (arguments.size() != 3) throw TypeError("sec_web_test_path_traversal() requires 3 arguments");
            last_value = static_cast<int>(sec_web_test_path_traversal(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str())); return;
        } else if (func_name == "__builtin_sec_web_encode_payload__") {
            if (arguments.size() != 3) throw TypeError("sec_web_encode_payload() requires 3 arguments");
            last_value = std::string(sec_web_encode_payload(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str())); return;
        } else if (func_name == "__builtin_sec_web_get_payloads__") {
            if (arguments.size() != 2) throw TypeError("sec_web_get_payloads() requires 2 arguments");
            last_value = std::string(sec_web_get_payloads(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_sec_webscan_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_webscan_destroy() requires 1 argument");
            sec_webscan_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_webscan_finding_count__") {
            if (arguments.size() != 1) throw TypeError("sec_webscan_finding_count() requires 1 argument");
            last_value = static_cast<int>(sec_webscan_finding_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_webscan_finding_name__") {
            if (arguments.size() != 2) throw TypeError("sec_webscan_finding_name() requires 2 arguments");
            last_value = std::string(sec_webscan_finding_name(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_webscan_finding_severity__") {
            if (arguments.size() != 2) throw TypeError("sec_webscan_finding_severity() requires 2 arguments");
            last_value = std::string(sec_webscan_finding_severity(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_webscan_finding_param__") {
            if (arguments.size() != 2) throw TypeError("sec_webscan_finding_param() requires 2 arguments");
            last_value = std::string(sec_webscan_finding_param(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_webscan_finding_payload__") {
            if (arguments.size() != 2) throw TypeError("sec_webscan_finding_payload() requires 2 arguments");
            last_value = std::string(sec_webscan_finding_payload(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_webscan_to_json__") {
            if (arguments.size() != 1) throw TypeError("sec_webscan_to_json() requires 1 argument");
            last_value = std::string(sec_webscan_to_json(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_malware_analyzer_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_malware_analyzer_create())); return;
        } else if (func_name == "__builtin_sec_malware_analyzer_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_malware_analyzer_destroy() requires 1 argument");
            sec_malware_analyzer_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_malware_static_analysis__") {
            if (arguments.size() < 2) throw TypeError("sec_malware_static_analysis() requires at least 2 arguments");
            std::string _hex = arguments.size() >= 3 ? std::get<std::string>(arguments[2]) : "";
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_malware_static_analysis(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), _hex.c_str()))); return;
        } else if (func_name == "__builtin_sec_malware_dynamic_analysis__") {
            if (arguments.size() < 2) throw TypeError("sec_malware_dynamic_analysis() requires at least 2 arguments");
            int _timeout = arguments.size() >= 3 ? std::get<int>(arguments[2]) : 5000;
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_malware_dynamic_analysis(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), _timeout))); return;
        } else if (func_name == "__builtin_sec_malware_add_yara_rule__") {
            if (arguments.size() != 3) throw TypeError("sec_malware_add_yara_rule() requires 3 arguments");
            last_value = static_cast<int>(sec_malware_add_yara_rule(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str())); return;
        } else if (func_name == "__builtin_sec_static_result_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_destroy() requires 1 argument");
            sec_static_result_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_static_result_threat_level__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_threat_level() requires 1 argument");
            last_value = std::string(sec_static_result_threat_level(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_static_result_file_type__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_file_type() requires 1 argument");
            last_value = std::string(sec_static_result_file_type(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_static_result_sha256__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_sha256() requires 1 argument");
            last_value = std::string(sec_static_result_sha256(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_static_result_md5__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_md5() requires 1 argument");
            last_value = std::string(sec_static_result_md5(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_static_result_entropy__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_entropy() requires 1 argument");
            last_value = sec_static_result_entropy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_sec_static_result_string_count__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_string_count() requires 1 argument");
            last_value = static_cast<int>(sec_static_result_string_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_static_result_string_value__") {
            if (arguments.size() != 2) throw TypeError("sec_static_result_string_value() requires 2 arguments");
            last_value = std::string(sec_static_result_string_value(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_static_result_string_category__") {
            if (arguments.size() != 2) throw TypeError("sec_static_result_string_category() requires 2 arguments");
            last_value = std::string(sec_static_result_string_category(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_static_result_suspicious_count__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_suspicious_count() requires 1 argument");
            last_value = static_cast<int>(sec_static_result_suspicious_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_static_result_suspicious__") {
            if (arguments.size() != 2) throw TypeError("sec_static_result_suspicious() requires 2 arguments");
            last_value = std::string(sec_static_result_suspicious(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_static_result_to_json__") {
            if (arguments.size() != 1) throw TypeError("sec_static_result_to_json() requires 1 argument");
            last_value = std::string(sec_static_result_to_json(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_dynamic_result_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_dynamic_result_destroy() requires 1 argument");
            sec_dynamic_result_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_dynamic_result_threat_level__") {
            if (arguments.size() != 1) throw TypeError("sec_dynamic_result_threat_level() requires 1 argument");
            last_value = std::string(sec_dynamic_result_threat_level(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_dynamic_result_event_count__") {
            if (arguments.size() != 1) throw TypeError("sec_dynamic_result_event_count() requires 1 argument");
            last_value = static_cast<int>(sec_dynamic_result_event_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_dynamic_result_event_type__") {
            if (arguments.size() != 2) throw TypeError("sec_dynamic_result_event_type() requires 2 arguments");
            last_value = std::string(sec_dynamic_result_event_type(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_dynamic_result_event_target__") {
            if (arguments.size() != 2) throw TypeError("sec_dynamic_result_event_target() requires 2 arguments");
            last_value = std::string(sec_dynamic_result_event_target(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_dynamic_result_ioc_count__") {
            if (arguments.size() != 1) throw TypeError("sec_dynamic_result_ioc_count() requires 1 argument");
            last_value = static_cast<int>(sec_dynamic_result_ioc_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_dynamic_result_ioc__") {
            if (arguments.size() != 2) throw TypeError("sec_dynamic_result_ioc() requires 2 arguments");
            last_value = std::string(sec_dynamic_result_ioc(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_dynamic_result_to_json__") {
            if (arguments.size() != 1) throw TypeError("sec_dynamic_result_to_json() requires 1 argument");
            last_value = std::string(sec_dynamic_result_to_json(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_edu_platform_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_edu_platform_create())); return;
        } else if (func_name == "__builtin_sec_edu_platform_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_edu_platform_destroy() requires 1 argument");
            sec_edu_platform_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_edu_challenge_count__") {
            if (arguments.size() != 1) throw TypeError("sec_edu_challenge_count() requires 1 argument");
            last_value = static_cast<int>(sec_edu_challenge_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_edu_challenge_id__") {
            if (arguments.size() != 2) throw TypeError("sec_edu_challenge_id() requires 2 arguments");
            last_value = std::string(sec_edu_challenge_id(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_edu_challenge_name__") {
            if (arguments.size() != 2) throw TypeError("sec_edu_challenge_name() requires 2 arguments");
            last_value = std::string(sec_edu_challenge_name(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_edu_challenge_category__") {
            if (arguments.size() != 2) throw TypeError("sec_edu_challenge_category() requires 2 arguments");
            last_value = std::string(sec_edu_challenge_category(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_edu_challenge_points__") {
            if (arguments.size() != 2) throw TypeError("sec_edu_challenge_points() requires 2 arguments");
            last_value = static_cast<int>(sec_edu_challenge_points(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_edu_challenge_description__") {
            if (arguments.size() != 2) throw TypeError("sec_edu_challenge_description() requires 2 arguments");
            last_value = std::string(sec_edu_challenge_description(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_edu_challenge_hint__") {
            if (arguments.size() != 2) throw TypeError("sec_edu_challenge_hint() requires 2 arguments");
            last_value = std::string(sec_edu_challenge_hint(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_sec_edu_create_session__") {
            if (arguments.size() != 2) throw TypeError("sec_edu_create_session() requires 2 arguments");
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(sec_edu_create_session(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_sec_edu_session_destroy__") {
            if (arguments.size() != 1) throw TypeError("sec_edu_session_destroy() requires 1 argument");
            sec_edu_session_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_sec_edu_session_submit__") {
            if (arguments.size() != 4) throw TypeError("sec_edu_session_submit() requires 4 arguments");
            last_value = static_cast<int>(sec_edu_session_submit(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[1]))), std::get<std::string>(arguments[2]).c_str(), std::get<std::string>(arguments[3]).c_str())); return;
        } else if (func_name == "__builtin_sec_edu_session_score__") {
            if (arguments.size() != 1) throw TypeError("sec_edu_session_score() requires 1 argument");
            last_value = static_cast<int>(sec_edu_session_score(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_edu_session_player__") {
            if (arguments.size() != 1) throw TypeError("sec_edu_session_player() requires 1 argument");
            last_value = std::string(sec_edu_session_player(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_edu_leaderboard__") {
            if (arguments.size() != 1) throw TypeError("sec_edu_leaderboard() requires 1 argument");
            last_value = std::string(sec_edu_leaderboard(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_sec_edu_get_tutorial__") {
            if (arguments.size() != 2) throw TypeError("sec_edu_get_tutorial() requires 2 arguments");
            last_value = std::string(sec_edu_get_tutorial(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str())); return;
        }
    }

    // ===== mobile module handlers =====
    if (std::holds_alternative<std::string>(callee)) {
        std::string func_name = std::get<std::string>(callee);
        if (func_name == "__builtin_mobile_app_config_create__") {
            if (arguments.size() < 3) throw TypeError("mobile_app_config_create() requires at least 3 arguments");
            int plat = arguments.size() >= 4 ? std::get<int>(arguments[3]) : 2;
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_app_config_create(
                std::get<std::string>(arguments[0]).c_str(),
                std::get<std::string>(arguments[1]).c_str(),
                std::get<std::string>(arguments[2]).c_str(), plat))); return;
        } else if (func_name == "__builtin_mobile_app_config_destroy__") {
            mobile_app_config_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_app_config_set_orientation__") {
            mobile_app_config_set_orientation(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_app_config_add_permission__") {
            mobile_app_config_add_permission(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_app_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_app_create(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))))); return;
        } else if (func_name == "__builtin_mobile_app_destroy__") {
            mobile_app_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_app_screen_count__") {
            last_value = static_cast<int>(mobile_app_screen_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_app_platform_name__") {
            last_value = std::string(mobile_app_platform_name(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_app_build_ios__") {
            last_value = std::string(mobile_app_build_ios(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_app_build_android__") {
            last_value = std::string(mobile_app_build_android(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_app_export_manifest__") {
            last_value = std::string(mobile_app_export_manifest(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_app_add_screen__") {
            mobile_app_add_screen(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[1])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_screen_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_screen_create(
                std::get<std::string>(arguments[0]).c_str(), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_mobile_screen_destroy__") {
            mobile_screen_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_screen_set_background__") {
            mobile_screen_set_background(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_screen_set_nav_bar__") {
            mobile_screen_set_nav_bar(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_screen_to_json__") {
            last_value = std::string(mobile_screen_to_json(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_screen_set_layout__") {
            mobile_screen_set_layout(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[1])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_component_create(
                std::get<int>(arguments[0]), std::get<std::string>(arguments[1]).c_str(),
                std::get<std::string>(arguments[2]).c_str()))); return;
        } else if (func_name == "__builtin_mobile_component_destroy__") {
            mobile_component_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_text__") {
            mobile_component_set_text(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_visible__") {
            mobile_component_set_visible(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_enabled__") {
            mobile_component_set_enabled(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_value__") {
            mobile_component_set_value(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<double>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_prop__") {
            mobile_component_set_prop(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_style_color__") {
            mobile_component_set_style_color(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_style_font__") {
            mobile_component_set_style_font(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_style_size__") {
            mobile_component_set_style_size(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<int>(arguments[1]), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_set_style_padding__") {
            mobile_component_set_style_padding(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_add_child__") {
            mobile_component_add_child(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[1])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_component_to_json__") {
            last_value = std::string(mobile_component_to_json(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_component_type_name__") {
            last_value = std::string(mobile_component_type_name(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_component_get_text__") {
            last_value = std::string(mobile_component_get_text(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_component_get_value__") {
            last_value = mobile_component_get_value(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_layout_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_layout_create(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_layout_destroy__") {
            mobile_layout_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_layout_set_spacing__") {
            mobile_layout_set_spacing(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_layout_set_columns__") {
            mobile_layout_set_columns(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_layout_add_item__") {
            mobile_layout_add_item(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[1])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_layout_to_json__") {
            last_value = std::string(mobile_layout_to_json(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_nav_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_nav_create())); return;
        } else if (func_name == "__builtin_mobile_nav_destroy__") {
            mobile_nav_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_nav_push__") {
            mobile_nav_push(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_nav_pop__") {
            mobile_nav_pop(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_nav_current__") {
            last_value = std::string(mobile_nav_current(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_nav_depth__") {
            last_value = static_cast<int>(mobile_nav_depth(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_device_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_device_create(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_device_destroy__") {
            mobile_device_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_device_capture_photo__") {
            std::string q = arguments.size() >= 2 ? std::get<std::string>(arguments[1]) : "high";
            last_value = std::string(mobile_device_capture_photo(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), q.c_str())); return;
        } else if (func_name == "__builtin_mobile_device_record_video__") {
            int dur = arguments.size() >= 2 ? std::get<int>(arguments[1]) : 10;
            last_value = std::string(mobile_device_record_video(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), dur)); return;
        } else if (func_name == "__builtin_mobile_device_get_location__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_device_get_location(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))))); return;
        } else if (func_name == "__builtin_mobile_location_destroy__") {
            mobile_location_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_location_latitude__") {
            last_value = mobile_location_latitude(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_location_longitude__") {
            last_value = mobile_location_longitude(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_location_altitude__") {
            last_value = mobile_location_altitude(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_location_accuracy__") {
            last_value = mobile_location_accuracy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_device_get_accelerometer__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mobile_device_get_accelerometer(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))))); return;
        } else if (func_name == "__builtin_mobile_accel_destroy__") {
            mobile_accel_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_mobile_accel_x__") {
            last_value = mobile_accel_x(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_accel_y__") {
            last_value = mobile_accel_y(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_accel_z__") {
            last_value = mobile_accel_z(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_mobile_device_storage_set__") {
            last_value = static_cast<int>(mobile_device_storage_set(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str())); return;
        } else if (func_name == "__builtin_mobile_device_storage_get__") {
            last_value = std::string(mobile_device_storage_get(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_mobile_device_storage_delete__") {
            last_value = static_cast<int>(mobile_device_storage_delete(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_mobile_device_is_connected__") {
            last_value = static_cast<int>(mobile_device_is_connected(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_device_network_type__") {
            last_value = std::string(mobile_device_network_type(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_device_model__") {
            last_value = std::string(mobile_device_model(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_device_os_version__") {
            last_value = std::string(mobile_device_os_version(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_device_id__") {
            last_value = std::string(mobile_device_id(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_device_battery_level__") {
            last_value = static_cast<int>(mobile_device_battery_level(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_device_is_charging__") {
            last_value = static_cast<int>(mobile_device_is_charging(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_mobile_device_check_permission__") {
            last_value = std::string(mobile_device_check_permission(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_mobile_device_request_permission__") {
            last_value = static_cast<int>(mobile_device_request_permission(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_mobile_device_schedule_notification__") {
            if (arguments.size() < 3) throw TypeError("mobile_device_schedule_notification() requires at least 3 arguments");
            int badge = arguments.size() >= 4 ? std::get<int>(arguments[3]) : 0;
            uint64_t delay = arguments.size() >= 5 ? static_cast<uint64_t>(std::get<int>(arguments[4])) : 0;
            last_value = std::string(mobile_device_schedule_notification(
                reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str(),
                std::get<std::string>(arguments[2]).c_str(), badge, delay)); return;
        } else if (func_name == "__builtin_mobile_device_cancel_notification__") {
            last_value = static_cast<int>(mobile_device_cancel_notification(
                reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))),
                std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_mobile_device_pending_notifications__") {
            last_value = static_cast<int>(mobile_device_pending_notifications(
                reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        }
        // game builtins
        else if (func_name == "__builtin_game_renderer_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_renderer_create(std::get<std::string>(arguments[0]).c_str(), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_game_renderer_destroy__") {
            game_renderer_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_renderer_clear__") {
            double r=0,g=0,b=0,a=1;
            if (std::holds_alternative<double>(arguments[1])) r=std::get<double>(arguments[1]); else r=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) g=std::get<double>(arguments[2]); else g=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) b=std::get<double>(arguments[3]); else b=std::get<int>(arguments[3]);
            if (std::holds_alternative<double>(arguments[4])) a=std::get<double>(arguments[4]); else a=std::get<int>(arguments[4]);
            game_renderer_clear(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)r,(float)g,(float)b,(float)a); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_renderer_present__") {
            game_renderer_present(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_renderer_width__") {
            last_value = game_renderer_width(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_renderer_height__") {
            last_value = game_renderer_height(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_renderer_title__") {
            last_value = std::string(game_renderer_title(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_game_texture_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_texture_create(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_game_texture_load__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_texture_load(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_game_texture_destroy__") {
            game_texture_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_texture_width__") {
            last_value = game_texture_width(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_texture_height__") {
            last_value = game_texture_height(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_texture_path__") {
            last_value = std::string(game_texture_path(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_game_sprite_create__") {
            double fx=0,fy=0;
            if (std::holds_alternative<double>(arguments[1])) fx=std::get<double>(arguments[1]); else fx=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) fy=std::get<double>(arguments[2]); else fy=std::get<int>(arguments[2]);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_sprite_create(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)fx,(float)fy))); return;
        } else if (func_name == "__builtin_game_sprite_destroy__") {
            game_sprite_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sprite_set_position__") {
            double fx=0,fy=0;
            if (std::holds_alternative<double>(arguments[1])) fx=std::get<double>(arguments[1]); else fx=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) fy=std::get<double>(arguments[2]); else fy=std::get<int>(arguments[2]);
            game_sprite_set_position(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)fx,(float)fy); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sprite_set_scale__") {
            double sx=1,sy=1;
            if (std::holds_alternative<double>(arguments[1])) sx=std::get<double>(arguments[1]); else sx=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) sy=std::get<double>(arguments[2]); else sy=std::get<int>(arguments[2]);
            game_sprite_set_scale(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)sx,(float)sy); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sprite_set_rotation__") {
            double a=0; if (std::holds_alternative<double>(arguments[1])) a=std::get<double>(arguments[1]); else a=std::get<int>(arguments[1]);
            game_sprite_set_rotation(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)a); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sprite_set_color__") {
            double r=1,g=1,b=1,a=1;
            if (std::holds_alternative<double>(arguments[1])) r=std::get<double>(arguments[1]); else r=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) g=std::get<double>(arguments[2]); else g=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) b=std::get<double>(arguments[3]); else b=std::get<int>(arguments[3]);
            if (std::holds_alternative<double>(arguments[4])) a=std::get<double>(arguments[4]); else a=std::get<int>(arguments[4]);
            game_sprite_set_color(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)r,(float)g,(float)b,(float)a); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sprite_set_visible__") {
            game_sprite_set_visible(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sprite_x__") {
            last_value = (double)game_sprite_x(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_sprite_y__") {
            last_value = (double)game_sprite_y(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_sprite_scale_x__") {
            last_value = (double)game_sprite_scale_x(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_sprite_scale_y__") {
            last_value = (double)game_sprite_scale_y(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_sprite_rotation__") {
            last_value = (double)game_sprite_rotation(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_sprite_to_string__") {
            last_value = std::string(game_sprite_to_string(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_game_shader_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_shader_create(std::get<std::string>(arguments[0]).c_str(), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()))); return;
        } else if (func_name == "__builtin_game_shader_destroy__") {
            game_shader_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_shader_set_float__") {
            double v=0; if (std::holds_alternative<double>(arguments[2])) v=std::get<double>(arguments[2]); else v=std::get<int>(arguments[2]);
            game_shader_set_float(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), (float)v); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_shader_set_int__") {
            game_shader_set_int(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_shader_set_vec2__") {
            double x=0,y=0;
            if (std::holds_alternative<double>(arguments[2])) x=std::get<double>(arguments[2]); else x=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) y=std::get<double>(arguments[3]); else y=std::get<int>(arguments[3]);
            game_shader_set_vec2(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), (float)x,(float)y); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_shader_set_vec3__") {
            double x=0,y=0,z=0;
            if (std::holds_alternative<double>(arguments[2])) x=std::get<double>(arguments[2]); else x=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) y=std::get<double>(arguments[3]); else y=std::get<int>(arguments[3]);
            if (std::holds_alternative<double>(arguments[4])) z=std::get<double>(arguments[4]); else z=std::get<int>(arguments[4]);
            game_shader_set_vec3(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), (float)x,(float)y,(float)z); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_shader_set_vec4__") {
            double x=0,y=0,z=0,w=0;
            if (std::holds_alternative<double>(arguments[2])) x=std::get<double>(arguments[2]); else x=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) y=std::get<double>(arguments[3]); else y=std::get<int>(arguments[3]);
            if (std::holds_alternative<double>(arguments[4])) z=std::get<double>(arguments[4]); else z=std::get<int>(arguments[4]);
            if (std::holds_alternative<double>(arguments[5])) w=std::get<double>(arguments[5]); else w=std::get<int>(arguments[5]);
            game_shader_set_vec4(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<std::string>(arguments[1]).c_str(), (float)x,(float)y,(float)z,(float)w); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_shader_name__") {
            last_value = std::string(game_shader_name(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))))); return;
        } else if (func_name == "__builtin_game_shader_uniform_count__") {
            last_value = game_shader_uniform_count(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_shader_uniform_name__") {
            last_value = std::string(game_shader_uniform_name(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_game_camera2d_create__") {
            double x=0,y=0,z=1;
            if (std::holds_alternative<double>(arguments[0])) x=std::get<double>(arguments[0]); else x=std::get<int>(arguments[0]);
            if (std::holds_alternative<double>(arguments[1])) y=std::get<double>(arguments[1]); else y=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) z=std::get<double>(arguments[2]); else z=std::get<int>(arguments[2]);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_camera2d_create((float)x,(float)y,(float)z))); return;
        } else if (func_name == "__builtin_game_camera2d_destroy__") {
            game_camera2d_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_camera2d_set_position__") {
            double x=0,y=0;
            if (std::holds_alternative<double>(arguments[1])) x=std::get<double>(arguments[1]); else x=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) y=std::get<double>(arguments[2]); else y=std::get<int>(arguments[2]);
            game_camera2d_set_position(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)x,(float)y); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_camera2d_set_zoom__") {
            double z=1; if (std::holds_alternative<double>(arguments[1])) z=std::get<double>(arguments[1]); else z=std::get<int>(arguments[1]);
            game_camera2d_set_zoom(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)z); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_camera2d_move__") {
            double dx=0,dy=0;
            if (std::holds_alternative<double>(arguments[1])) dx=std::get<double>(arguments[1]); else dx=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) dy=std::get<double>(arguments[2]); else dy=std::get<int>(arguments[2]);
            game_camera2d_move(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)dx,(float)dy); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_camera2d_x__") {
            last_value = (double)game_camera2d_x(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_camera2d_y__") {
            last_value = (double)game_camera2d_y(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_camera2d_zoom__") {
            last_value = (double)game_camera2d_zoom(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_camera3d_create__") {
            double px=0,py=0,pz=5,tx=0,ty=0,tz=0;
            if (std::holds_alternative<double>(arguments[0])) px=std::get<double>(arguments[0]); else px=std::get<int>(arguments[0]);
            if (std::holds_alternative<double>(arguments[1])) py=std::get<double>(arguments[1]); else py=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) pz=std::get<double>(arguments[2]); else pz=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) tx=std::get<double>(arguments[3]); else tx=std::get<int>(arguments[3]);
            if (std::holds_alternative<double>(arguments[4])) ty=std::get<double>(arguments[4]); else ty=std::get<int>(arguments[4]);
            if (std::holds_alternative<double>(arguments[5])) tz=std::get<double>(arguments[5]); else tz=std::get<int>(arguments[5]);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_camera3d_create((float)px,(float)py,(float)pz,(float)tx,(float)ty,(float)tz))); return;
        } else if (func_name == "__builtin_game_camera3d_destroy__") {
            game_camera3d_destroy(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_camera3d_set_position__") {
            double x=0,y=0,z=0;
            if (std::holds_alternative<double>(arguments[1])) x=std::get<double>(arguments[1]); else x=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) y=std::get<double>(arguments[2]); else y=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) z=std::get<double>(arguments[3]); else z=std::get<int>(arguments[3]);
            game_camera3d_set_position(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)x,(float)y,(float)z); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_camera3d_set_target__") {
            double x=0,y=0,z=0;
            if (std::holds_alternative<double>(arguments[1])) x=std::get<double>(arguments[1]); else x=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) y=std::get<double>(arguments[2]); else y=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) z=std::get<double>(arguments[3]); else z=std::get<int>(arguments[3]);
            game_camera3d_set_target(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)x,(float)y,(float)z); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_camera3d_set_fov__") {
            double f=60; if (std::holds_alternative<double>(arguments[1])) f=std::get<double>(arguments[1]); else f=std::get<int>(arguments[1]);
            game_camera3d_set_fov(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0]))), (float)f); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_camera3d_pos_x__") {
            last_value = (double)game_camera3d_pos_x(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_camera3d_pos_y__") {
            last_value = (double)game_camera3d_pos_y(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_camera3d_pos_z__") {
            last_value = (double)game_camera3d_pos_z(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_camera3d_fov__") {
            last_value = (double)game_camera3d_fov(reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[0])))); return;
        }
    }

    // game physics/audio/framework builtins
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_game_", 0) == 0) {
        std::string func_name = std::get<std::string>(callee);
        auto _gptr = [&](int idx) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[idx]))); };
        auto _gf = [&](int idx) -> float {
            if (std::holds_alternative<double>(arguments[idx])) return (float)std::get<double>(arguments[idx]);
            return (float)std::get<int>(arguments[idx]);
        };
        // Physics
        if (func_name == "__builtin_game_physics_world_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_physics_world_create(_gf(0),_gf(1)))); return;
        } else if (func_name == "__builtin_game_physics_world_destroy__") {
            game_physics_world_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_physics_world_step__") {
            game_physics_world_step(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_physics_world_set_gravity__") {
            game_physics_world_set_gravity(_gptr(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_physics_world_body_count__") {
            last_value = game_physics_world_body_count(_gptr(0)); return;
        } else if (func_name == "__builtin_game_rigidbody_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_rigidbody_create(_gptr(0), _gf(1), _gf(2), _gf(3), std::get<int>(arguments[4])))); return;
        } else if (func_name == "__builtin_game_rigidbody_destroy__") {
            game_rigidbody_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_rigidbody_set_position__") {
            game_rigidbody_set_position(_gptr(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_rigidbody_set_velocity__") {
            game_rigidbody_set_velocity(_gptr(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_rigidbody_apply_force__") {
            game_rigidbody_apply_force(_gptr(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_rigidbody_apply_impulse__") {
            game_rigidbody_apply_impulse(_gptr(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_rigidbody_set_restitution__") {
            game_rigidbody_set_restitution(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_rigidbody_set_friction__") {
            game_rigidbody_set_friction(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_rigidbody_x__") {
            last_value = (double)game_rigidbody_x(_gptr(0)); return;
        } else if (func_name == "__builtin_game_rigidbody_y__") {
            last_value = (double)game_rigidbody_y(_gptr(0)); return;
        } else if (func_name == "__builtin_game_rigidbody_vx__") {
            last_value = (double)game_rigidbody_vx(_gptr(0)); return;
        } else if (func_name == "__builtin_game_rigidbody_vy__") {
            last_value = (double)game_rigidbody_vy(_gptr(0)); return;
        } else if (func_name == "__builtin_game_rigidbody_mass__") {
            last_value = (double)game_rigidbody_mass(_gptr(0)); return;
        } else if (func_name == "__builtin_game_rigidbody_is_static__") {
            last_value = game_rigidbody_is_static(_gptr(0)); return;
        } else if (func_name == "__builtin_game_rigidbody_to_string__") {
            last_value = std::string(game_rigidbody_to_string(_gptr(0))); return;
        } else if (func_name == "__builtin_game_collider_box__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_collider_box(_gptr(0), _gf(1), _gf(2)))); return;
        } else if (func_name == "__builtin_game_collider_circle__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_collider_circle(_gptr(0), _gf(1)))); return;
        } else if (func_name == "__builtin_game_collider_destroy__") {
            game_collider_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_collider_check__") {
            last_value = game_collider_check(_gptr(0), _gptr(1)); return;
        } else if (func_name == "__builtin_game_collider_type__") {
            last_value = std::string(game_collider_type(_gptr(0))); return;
        } else if (func_name == "__builtin_game_particles_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_particles_create(_gf(0), _gf(1), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_game_particles_destroy__") {
            game_particles_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_particles_emit__") {
            game_particles_emit(_gptr(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_particles_update__") {
            game_particles_update(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_particles_set_velocity__") {
            game_particles_set_velocity(_gptr(0), _gf(1), _gf(2), _gf(3)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_particles_set_lifetime__") {
            game_particles_set_lifetime(_gptr(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_particles_set_color__") {
            game_particles_set_color(_gptr(0), _gf(1), _gf(2), _gf(3), _gf(4)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_particles_set_size__") {
            game_particles_set_size(_gptr(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_particles_active_count__") {
            last_value = game_particles_active_count(_gptr(0)); return;
        } else if (func_name == "__builtin_game_particles_max_count__") {
            last_value = game_particles_max_count(_gptr(0)); return;
        } else if (func_name == "__builtin_game_particles_to_string__") {
            last_value = std::string(game_particles_to_string(_gptr(0))); return;
        }
        // Audio
        else if (func_name == "__builtin_game_audio_engine_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_audio_engine_create())); return;
        } else if (func_name == "__builtin_game_audio_engine_destroy__") {
            game_audio_engine_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_audio_engine_set_master_volume__") {
            game_audio_engine_set_master_volume(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_audio_engine_master_volume__") {
            last_value = (double)game_audio_engine_master_volume(_gptr(0)); return;
        } else if (func_name == "__builtin_game_audio_engine_channel_count__") {
            last_value = game_audio_engine_channel_count(_gptr(0)); return;
        } else if (func_name == "__builtin_game_sound_load__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_sound_load(_gptr(0), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_game_sound_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_sound_create(_gptr(0), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()))); return;
        } else if (func_name == "__builtin_game_sound_destroy__") {
            game_sound_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sound_play__") {
            game_sound_play(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sound_stop__") {
            game_sound_stop(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sound_pause__") {
            game_sound_pause(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sound_resume__") {
            game_sound_resume(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sound_set_volume__") {
            game_sound_set_volume(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sound_set_pitch__") {
            game_sound_set_pitch(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sound_set_loop__") {
            game_sound_set_loop(_gptr(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_sound_is_playing__") {
            last_value = game_sound_is_playing(_gptr(0)); return;
        } else if (func_name == "__builtin_game_sound_volume__") {
            last_value = (double)game_sound_volume(_gptr(0)); return;
        } else if (func_name == "__builtin_game_sound_pitch__") {
            last_value = (double)game_sound_pitch(_gptr(0)); return;
        } else if (func_name == "__builtin_game_sound_name__") {
            last_value = std::string(game_sound_name(_gptr(0))); return;
        } else if (func_name == "__builtin_game_sound_type__") {
            last_value = std::string(game_sound_type(_gptr(0))); return;
        } else if (func_name == "__builtin_game_music_load__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_music_load(_gptr(0), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_game_music_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_music_create(_gptr(0), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_game_music_destroy__") {
            game_music_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_music_play__") {
            game_music_play(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_music_stop__") {
            game_music_stop(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_music_pause__") {
            game_music_pause(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_music_resume__") {
            game_music_resume(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_music_set_volume__") {
            game_music_set_volume(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_music_set_loop__") {
            game_music_set_loop(_gptr(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_music_is_playing__") {
            last_value = game_music_is_playing(_gptr(0)); return;
        } else if (func_name == "__builtin_game_music_name__") {
            last_value = std::string(game_music_name(_gptr(0))); return;
        } else if (func_name == "__builtin_game_audio3d_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_audio3d_create(_gptr(0)))); return;
        } else if (func_name == "__builtin_game_audio3d_destroy__") {
            game_audio3d_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_audio3d_set_listener__") {
            game_audio3d_set_listener(_gptr(0), _gf(1), _gf(2), _gf(3)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_audio3d_play_at__") {
            game_audio3d_play_at(_gptr(0), _gptr(1), _gf(2), _gf(3), _gf(4)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_audio3d_set_rolloff__") {
            game_audio3d_set_rolloff(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_mixer_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_mixer_create(_gptr(0), std::get<int>(arguments[1])))); return;
        } else if (func_name == "__builtin_game_mixer_destroy__") {
            game_mixer_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_mixer_set_channel_volume__") {
            game_mixer_set_channel_volume(_gptr(0), std::get<int>(arguments[1]), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_mixer_channel_volume__") {
            last_value = (double)game_mixer_channel_volume(_gptr(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_game_mixer_play_on_channel__") {
            game_mixer_play_on_channel(_gptr(0), _gptr(1), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_mixer_channels__") {
            last_value = game_mixer_channels(_gptr(0)); return;
        }
        // Game Framework
        else if (func_name == "__builtin_game_loop_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_loop_create(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_game_loop_destroy__") {
            game_loop_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_loop_start__") {
            game_loop_start(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_loop_stop__") {
            game_loop_stop(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_loop_tick__") {
            game_loop_tick(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_loop_is_running__") {
            last_value = game_loop_is_running(_gptr(0)); return;
        } else if (func_name == "__builtin_game_loop_delta_time__") {
            last_value = (double)game_loop_delta_time(_gptr(0)); return;
        } else if (func_name == "__builtin_game_loop_fps__") {
            last_value = game_loop_fps(_gptr(0)); return;
        } else if (func_name == "__builtin_game_loop_target_fps__") {
            last_value = game_loop_target_fps(_gptr(0)); return;
        } else if (func_name == "__builtin_game_loop_frame_count__") {
            last_value = game_loop_frame_count(_gptr(0)); return;
        } else if (func_name == "__builtin_game_loop_elapsed__") {
            last_value = game_loop_elapsed(_gptr(0)); return;
        } else if (func_name == "__builtin_game_scene_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_scene_create(std::get<std::string>(arguments[0]).c_str()))); return;
        } else if (func_name == "__builtin_game_scene_destroy__") {
            game_scene_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_scene_add_sprite__") {
            game_scene_add_sprite(_gptr(0), _gptr(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_scene_add_body__") {
            game_scene_add_body(_gptr(0), _gptr(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_scene_update__") {
            game_scene_update(_gptr(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_scene_sprite_count__") {
            last_value = game_scene_sprite_count(_gptr(0)); return;
        } else if (func_name == "__builtin_game_scene_body_count__") {
            last_value = game_scene_body_count(_gptr(0)); return;
        } else if (func_name == "__builtin_game_scene_name__") {
            last_value = std::string(game_scene_name(_gptr(0))); return;
        } else if (func_name == "__builtin_game_scene_manager_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_scene_manager_create())); return;
        } else if (func_name == "__builtin_game_scene_manager_destroy__") {
            game_scene_manager_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_scene_manager_push__") {
            game_scene_manager_push(_gptr(0), _gptr(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_scene_manager_pop__") {
            game_scene_manager_pop(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_scene_manager_current__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_scene_manager_current(_gptr(0)))); return;
        } else if (func_name == "__builtin_game_scene_manager_depth__") {
            last_value = game_scene_manager_depth(_gptr(0)); return;
        } else if (func_name == "__builtin_game_input_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_input_create())); return;
        } else if (func_name == "__builtin_game_input_destroy__") {
            game_input_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_input_key_press__") {
            game_input_key_press(_gptr(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_input_key_release__") {
            game_input_key_release(_gptr(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_input_mouse_move__") {
            game_input_mouse_move(_gptr(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_input_mouse_press__") {
            game_input_mouse_press(_gptr(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_input_mouse_release__") {
            game_input_mouse_release(_gptr(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_input_is_key_down__") {
            last_value = game_input_is_key_down(_gptr(0), std::get<std::string>(arguments[1]).c_str()); return;
        } else if (func_name == "__builtin_game_input_is_key_pressed__") {
            last_value = game_input_is_key_pressed(_gptr(0), std::get<std::string>(arguments[1]).c_str()); return;
        } else if (func_name == "__builtin_game_input_is_mouse_down__") {
            last_value = game_input_is_mouse_down(_gptr(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_game_input_mouse_x__") {
            last_value = (double)game_input_mouse_x(_gptr(0)); return;
        } else if (func_name == "__builtin_game_input_mouse_y__") {
            last_value = (double)game_input_mouse_y(_gptr(0)); return;
        } else if (func_name == "__builtin_game_input_update__") {
            game_input_update(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_assets_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(game_assets_create(std::get<std::string>(arguments[0]).c_str()))); return;
        } else if (func_name == "__builtin_game_assets_destroy__") {
            game_assets_destroy(_gptr(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_assets_register__") {
            game_assets_register(_gptr(0), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str(), std::get<std::string>(arguments[3]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_game_assets_get_path__") {
            last_value = std::string(game_assets_get_path(_gptr(0), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_game_assets_get_type__") {
            last_value = std::string(game_assets_get_type(_gptr(0), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_game_assets_count__") {
            last_value = game_assets_count(_gptr(0)); return;
        } else if (func_name == "__builtin_game_assets_name_at__") {
            last_value = std::string(game_assets_name_at(_gptr(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_game_assets_to_json__") {
            last_value = std::string(game_assets_to_json(_gptr(0))); return;
        }
    }

    // system builtins (asm / gpio / rtos / kernel)
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_asm_", 0) == 0) {
        std::string func_name = std::get<std::string>(callee);
        auto _sp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        if (func_name == "__builtin_asm_context_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(asm_context_create(std::get<std::string>(arguments[0]).c_str()))); return;
        } else if (func_name == "__builtin_asm_context_destroy__") {
            asm_context_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_asm_context_arch__") {
            last_value = std::string(asm_context_arch(_sp(0))); return;
        } else if (func_name == "__builtin_asm_assemble__") {
            last_value = std::string(asm_assemble(_sp(0), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_asm_disassemble__") {
            last_value = std::string(asm_disassemble(_sp(0), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_asm_execute__") {
            asm_execute(_sp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_asm_set_reg__") {
            int64_t v = std::holds_alternative<int>(arguments[2]) ? std::get<int>(arguments[2]) : (int64_t)std::get<double>(arguments[2]);
            asm_set_reg(_sp(0), std::get<std::string>(arguments[1]).c_str(), v); last_value = nullptr; return;
        } else if (func_name == "__builtin_asm_get_reg__") {
            last_value = (int)asm_get_reg(_sp(0), std::get<std::string>(arguments[1]).c_str()); return;
        } else if (func_name == "__builtin_asm_dump_regs__") {
            last_value = std::string(asm_dump_regs(_sp(0))); return;
        } else if (func_name == "__builtin_asm_instruction_count__") {
            last_value = asm_instruction_count(_sp(0)); return;
        } else if (func_name == "__builtin_asm_instruction_at__") {
            last_value = std::string(asm_instruction_at(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_asm_optimize__") {
            last_value = std::string(asm_optimize(_sp(0), std::get<std::string>(arguments[1]).c_str())); return;
        }
    }

    if (std::holds_alternative<std::string>(callee) && (std::get<std::string>(callee).rfind("__builtin_gpio_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_pwm_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_adc_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_dac_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_spi_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_i2c_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_uart_", 0) == 0)) {
        std::string func_name = std::get<std::string>(callee);
        auto _sp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _gf = [&](int i) -> float { return std::holds_alternative<double>(arguments[i]) ? (float)std::get<double>(arguments[i]) : (float)std::get<int>(arguments[i]); };
        if (func_name == "__builtin_gpio_controller_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gpio_controller_create(std::get<std::string>(arguments[0]).c_str()))); return;
        } else if (func_name == "__builtin_gpio_controller_destroy__") {
            gpio_controller_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gpio_controller_board__") {
            last_value = std::string(gpio_controller_board(_sp(0))); return;
        } else if (func_name == "__builtin_gpio_controller_pin_count__") {
            last_value = gpio_controller_pin_count(_sp(0)); return;
        } else if (func_name == "__builtin_gpio_pin_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gpio_pin_create(_sp(0), std::get<int>(arguments[1]), std::get<std::string>(arguments[2]).c_str()))); return;
        } else if (func_name == "__builtin_gpio_pin_destroy__") {
            gpio_pin_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gpio_pin_write__") {
            gpio_pin_write(_sp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gpio_pin_read__") {
            last_value = gpio_pin_read(_sp(0)); return;
        } else if (func_name == "__builtin_gpio_pin_number__") {
            last_value = gpio_pin_number(_sp(0)); return;
        } else if (func_name == "__builtin_gpio_pin_mode__") {
            last_value = std::string(gpio_pin_mode(_sp(0))); return;
        } else if (func_name == "__builtin_gpio_pin_set_pull__") {
            gpio_pin_set_pull(_sp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gpio_pin_to_string__") {
            last_value = std::string(gpio_pin_to_string(_sp(0))); return;
        } else if (func_name == "__builtin_pwm_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(pwm_create(_sp(0), std::get<int>(arguments[1]), _gf(2)))); return;
        } else if (func_name == "__builtin_pwm_destroy__") {
            pwm_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_pwm_set_frequency__") {
            pwm_set_frequency(_sp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_pwm_set_duty_cycle__") {
            pwm_set_duty_cycle(_sp(0), _gf(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_pwm_start__") {
            pwm_start(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_pwm_stop__") {
            pwm_stop(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_pwm_frequency__") {
            last_value = pwm_frequency(_sp(0)); return;
        } else if (func_name == "__builtin_pwm_duty_cycle__") {
            last_value = (double)pwm_duty_cycle(_sp(0)); return;
        } else if (func_name == "__builtin_pwm_is_running__") {
            last_value = pwm_is_running(_sp(0)); return;
        } else if (func_name == "__builtin_adc_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(adc_create(_sp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_adc_destroy__") {
            adc_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_adc_read_raw__") {
            last_value = adc_read_raw(_sp(0)); return;
        } else if (func_name == "__builtin_adc_read_voltage__") {
            last_value = (double)adc_read_voltage(_sp(0), _gf(1)); return;
        } else if (func_name == "__builtin_adc_resolution__") {
            last_value = adc_resolution(_sp(0)); return;
        } else if (func_name == "__builtin_adc_channel__") {
            last_value = adc_channel(_sp(0)); return;
        } else if (func_name == "__builtin_dac_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(dac_create(_sp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_dac_destroy__") {
            dac_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_dac_write_raw__") {
            dac_write_raw(_sp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_dac_write_voltage__") {
            dac_write_voltage(_sp(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_dac_resolution__") {
            last_value = dac_resolution(_sp(0)); return;
        } else if (func_name == "__builtin_dac_channel__") {
            last_value = dac_channel(_sp(0)); return;
        } else if (func_name == "__builtin_spi_bus_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(spi_bus_create(_sp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_spi_bus_destroy__") {
            spi_bus_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_spi_transfer__") {
            last_value = std::string(spi_transfer(_sp(0), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_spi_set_clock__") {
            spi_set_clock(_sp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_spi_clock__") {
            last_value = spi_clock(_sp(0)); return;
        } else if (func_name == "__builtin_spi_bus_num__") {
            last_value = spi_bus_num(_sp(0)); return;
        } else if (func_name == "__builtin_i2c_bus_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(i2c_bus_create(_sp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_i2c_bus_destroy__") {
            i2c_bus_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_i2c_write__") {
            last_value = i2c_write(_sp(0), std::get<int>(arguments[1]), std::get<std::string>(arguments[2]).c_str()); return;
        } else if (func_name == "__builtin_i2c_read__") {
            last_value = std::string(i2c_read(_sp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]))); return;
        } else if (func_name == "__builtin_i2c_scan__") {
            last_value = i2c_scan(_sp(0)); return;
        } else if (func_name == "__builtin_i2c_bus_num__") {
            last_value = i2c_bus_num(_sp(0)); return;
        } else if (func_name == "__builtin_uart_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(uart_create(_sp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_uart_destroy__") {
            uart_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_uart_write__") {
            last_value = uart_write(_sp(0), std::get<std::string>(arguments[1]).c_str()); return;
        } else if (func_name == "__builtin_uart_read__") {
            last_value = std::string(uart_read(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_uart_set_baud__") {
            uart_set_baud(_sp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_uart_baud__") {
            last_value = uart_baud(_sp(0)); return;
        } else if (func_name == "__builtin_uart_port__") {
            last_value = uart_port(_sp(0)); return;
        } else if (func_name == "__builtin_uart_bytes_available__") {
            last_value = uart_bytes_available(_sp(0)); return;
        }
    }

    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_rtos_", 0) == 0) {
        std::string func_name = std::get<std::string>(callee);
        auto _sp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        if (func_name == "__builtin_rtos_kernel_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(rtos_kernel_create(std::get<std::string>(arguments[0]).c_str()))); return;
        } else if (func_name == "__builtin_rtos_kernel_destroy__") {
            rtos_kernel_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_kernel_start__") {
            rtos_kernel_start(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_kernel_stop__") {
            rtos_kernel_stop(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_kernel_is_running__") {
            last_value = rtos_kernel_is_running(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_kernel_name__") {
            last_value = std::string(rtos_kernel_name(_sp(0))); return;
        } else if (func_name == "__builtin_rtos_kernel_tick__") {
            last_value = rtos_kernel_tick(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_kernel_task_count__") {
            last_value = rtos_kernel_task_count(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_task_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(rtos_task_create(_sp(0), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2]), std::get<int>(arguments[3])))); return;
        } else if (func_name == "__builtin_rtos_task_destroy__") {
            rtos_task_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_task_start__") {
            rtos_task_start(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_task_suspend__") {
            rtos_task_suspend(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_task_resume__") {
            rtos_task_resume(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_task_delete__") {
            rtos_task_delete(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_task_name__") {
            last_value = std::string(rtos_task_name(_sp(0))); return;
        } else if (func_name == "__builtin_rtos_task_priority__") {
            last_value = rtos_task_priority(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_task_state__") {
            last_value = std::string(rtos_task_state(_sp(0))); return;
        } else if (func_name == "__builtin_rtos_task_stack_size__") {
            last_value = rtos_task_stack_size(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_semaphore_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(rtos_semaphore_create(std::get<int>(arguments[0]), std::get<int>(arguments[1])))); return;
        } else if (func_name == "__builtin_rtos_semaphore_destroy__") {
            rtos_semaphore_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_semaphore_take__") {
            last_value = rtos_semaphore_take(_sp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_rtos_semaphore_give__") {
            last_value = rtos_semaphore_give(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_semaphore_count__") {
            last_value = rtos_semaphore_count(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_mutex_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(rtos_mutex_create())); return;
        } else if (func_name == "__builtin_rtos_mutex_destroy__") {
            rtos_mutex_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_mutex_lock__") {
            last_value = rtos_mutex_lock(_sp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_rtos_mutex_unlock__") {
            last_value = rtos_mutex_unlock(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_mutex_is_locked__") {
            last_value = rtos_mutex_is_locked(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_queue_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(rtos_queue_create(std::get<int>(arguments[0]), std::get<int>(arguments[1])))); return;
        } else if (func_name == "__builtin_rtos_queue_destroy__") {
            rtos_queue_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_queue_send__") {
            last_value = rtos_queue_send(_sp(0), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2])); return;
        } else if (func_name == "__builtin_rtos_queue_receive__") {
            last_value = std::string(rtos_queue_receive(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_rtos_queue_size__") {
            last_value = rtos_queue_size(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_queue_capacity__") {
            last_value = rtos_queue_capacity(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_queue_is_full__") {
            last_value = rtos_queue_is_full(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_queue_is_empty__") {
            last_value = rtos_queue_is_empty(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_timer_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(rtos_timer_create(std::get<std::string>(arguments[0]).c_str(), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_rtos_timer_destroy__") {
            rtos_timer_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_timer_start__") {
            rtos_timer_start(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_timer_stop__") {
            rtos_timer_stop(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_timer_reset__") {
            rtos_timer_reset(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_rtos_timer_is_active__") {
            last_value = rtos_timer_is_active(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_timer_period__") {
            last_value = rtos_timer_period(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_timer_fire_count__") {
            last_value = rtos_timer_fire_count(_sp(0)); return;
        } else if (func_name == "__builtin_rtos_timer_name__") {
            last_value = std::string(rtos_timer_name(_sp(0))); return;
        }
    }

    if (std::holds_alternative<std::string>(callee) && (std::get<std::string>(callee).rfind("__builtin_mem_map_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_interrupt_table_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_process_table_", 0) == 0 || std::get<std::string>(callee).rfind("__builtin_kernel_", 0) == 0)) {
        std::string func_name = std::get<std::string>(callee);
        auto _sp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _u64 = [&](int i) -> uint64_t { return std::holds_alternative<int>(arguments[i]) ? (uint64_t)std::get<int>(arguments[i]) : (uint64_t)std::get<double>(arguments[i]); };
        if (func_name == "__builtin_mem_map_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(mem_map_create(_u64(0), _u64(1)))); return;
        } else if (func_name == "__builtin_mem_map_destroy__") {
            mem_map_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_mem_map_add_region__") {
            mem_map_add_region(_sp(0), std::get<std::string>(arguments[1]).c_str(), _u64(2), _u64(3), std::get<std::string>(arguments[4]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_mem_map_region_count__") {
            last_value = mem_map_region_count(_sp(0)); return;
        } else if (func_name == "__builtin_mem_map_region_name__") {
            last_value = std::string(mem_map_region_name(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_mem_map_region_addr__") {
            last_value = (int)mem_map_region_addr(_sp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_mem_map_region_size__") {
            last_value = (int)mem_map_region_size(_sp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_mem_map_region_type__") {
            last_value = std::string(mem_map_region_type(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_mem_map_to_string__") {
            last_value = std::string(mem_map_to_string(_sp(0))); return;
        } else if (func_name == "__builtin_interrupt_table_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(interrupt_table_create(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_interrupt_table_destroy__") {
            interrupt_table_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_interrupt_table_register__") {
            interrupt_table_register(_sp(0), std::get<int>(arguments[1]), std::get<std::string>(arguments[2]).c_str(), std::get<std::string>(arguments[3]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_interrupt_table_fire__") {
            last_value = interrupt_table_fire(_sp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_interrupt_table_handler__") {
            last_value = std::string(interrupt_table_handler(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_interrupt_table_name__") {
            last_value = std::string(interrupt_table_name(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_interrupt_table_fire_count__") {
            last_value = interrupt_table_fire_count(_sp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_interrupt_table_size__") {
            last_value = interrupt_table_size(_sp(0)); return;
        } else if (func_name == "__builtin_process_table_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(process_table_create(std::get<int>(arguments[0])))); return;
        } else if (func_name == "__builtin_process_table_destroy__") {
            process_table_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_process_table_create_process__") {
            last_value = process_table_create_process(_sp(0), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2])); return;
        } else if (func_name == "__builtin_process_table_kill__") {
            process_table_kill(_sp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_process_table_name__") {
            last_value = std::string(process_table_name(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_process_table_priority__") {
            last_value = process_table_priority(_sp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_process_table_state__") {
            last_value = std::string(process_table_state(_sp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_process_table_count__") {
            last_value = process_table_count(_sp(0)); return;
        } else if (func_name == "__builtin_process_table_schedule__") {
            last_value = process_table_schedule(_sp(0)); return;
        } else if (func_name == "__builtin_process_table_to_string__") {
            last_value = std::string(process_table_to_string(_sp(0))); return;
        } else if (func_name == "__builtin_os_kernel_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(kernel_create(std::get<std::string>(arguments[0]).c_str(), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_os_kernel_destroy__") {
            kernel_destroy(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_kernel_boot__") {
            kernel_boot(_sp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_kernel_panic__") {
            kernel_panic(_sp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_kernel_name__") {
            last_value = std::string(kernel_name(_sp(0))); return;
        } else if (func_name == "__builtin_kernel_arch__") {
            last_value = std::string(kernel_arch(_sp(0))); return;
        } else if (func_name == "__builtin_kernel_state__") {
            last_value = std::string(kernel_state(_sp(0))); return;
        } else if (func_name == "__builtin_kernel_uptime_ms__") {
            last_value = kernel_uptime_ms(_sp(0)); return;
        } else if (func_name == "__builtin_kernel_syscall__") {
            kernel_syscall(_sp(0), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_kernel_log__") {
            last_value = std::string(kernel_log(_sp(0))); return;
        } else if (func_name == "__builtin_kernel_log_count__") {
            last_value = kernel_log_count(_sp(0)); return;
        }
    }



    // gui window/draw builtins
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_gui_", 0) == 0) {
        std::string func_name = std::get<std::string>(callee);
        auto _gp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        // Coerce int or double to int (allows physics/simulation floats to be passed to draw calls)
        auto _gi = [&](int i) -> int {
            if (std::holds_alternative<int>(arguments[i]))    return std::get<int>(arguments[i]);
            if (std::holds_alternative<double>(arguments[i])) return (int)std::get<double>(arguments[i]);
            return 0;
        };
        // Window system
        if (func_name == "__builtin_gui_init__") {
            last_value = gui_init(); return;
        } else if (func_name == "__builtin_gui_quit__") {
            gui_quit(); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_has_display__") {
            last_value = gui_has_display(); return;
        } else if (func_name == "__builtin_gui_window_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_window_create(std::get<std::string>(arguments[0]).c_str(), std::get<int>(arguments[1]), std::get<int>(arguments[2])))); return;
        } else if (func_name == "__builtin_gui_window_destroy__") {
            gui_window_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_window_show__") {
            gui_window_show(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_window_hide__") {
            gui_window_hide(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_window_set_title__") {
            gui_window_set_title(_gp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_window_title__") {
            last_value = std::string(gui_window_title(_gp(0))); return;
        } else if (func_name == "__builtin_gui_window_width__") {
            last_value = gui_window_width(_gp(0)); return;
        } else if (func_name == "__builtin_gui_window_height__") {
            last_value = gui_window_height(_gp(0)); return;
        } else if (func_name == "__builtin_gui_window_is_open__") {
            last_value = gui_window_is_open(_gp(0)); return;
        } else if (func_name == "__builtin_gui_window_close__") {
            gui_window_close(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_window_resize__") {
            gui_window_resize(_gp(0), _gi(1), _gi(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_window_set_resizable__") {
            gui_window_set_resizable(_gp(0), _gi(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_window_set_fullscreen__") {
            gui_window_set_fullscreen(_gp(0), _gi(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_window_is_fullscreen__") {
            last_value = gui_window_is_fullscreen(_gp(0)); return;
        } else if (func_name == "__builtin_gui_window_poll__") {
            last_value = gui_window_poll(_gp(0)); return;
        } else if (func_name == "__builtin_gui_key_down__") {
            last_value = gui_key_down(_gp(0), _gi(1)); return;
        } else if (func_name == "__builtin_gui_key_pressed__") {
            last_value = gui_key_pressed(_gp(0), _gi(1)); return;
        } else if (func_name == "__builtin_gui_key__") {
            last_value = gui_key(std::get<std::string>(arguments[0]).c_str()); return;
        } else if (func_name == "__builtin_gui_mouse_x__") {
            last_value = gui_mouse_x(_gp(0)); return;
        } else if (func_name == "__builtin_gui_mouse_y__") {
            last_value = gui_mouse_y(_gp(0)); return;
        } else if (func_name == "__builtin_gui_mouse_button__") {
            last_value = gui_mouse_button(_gp(0), _gi(1)); return;
        } else if (func_name == "__builtin_gui_scroll_y__") {
            last_value = gui_scroll_y(_gp(0)); return;
        } else if (func_name == "__builtin_gui_mouse_dx__") {
            last_value = gui_mouse_dx(_gp(0)); return;
        } else if (func_name == "__builtin_gui_mouse_dy__") {
            last_value = gui_mouse_dy(_gp(0)); return;
        } else if (func_name == "__builtin_gui_clear__") {
            gui_clear(_gp(0), _gi(1), _gi(2), _gi(3)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_present__") {
            gui_present(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_set_color__") {
            gui_set_color(_gp(0), _gi(1), _gi(2), _gi(3), _gi(4)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_draw_point__") {
            gui_draw_point(_gp(0), _gi(1), _gi(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_draw_line__") {
            gui_draw_line(_gp(0), _gi(1), _gi(2), _gi(3), _gi(4)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_draw_rect__") {
            gui_draw_rect(_gp(0), _gi(1), _gi(2), _gi(3), _gi(4)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_fill_rect__") {
            gui_fill_rect(_gp(0), _gi(1), _gi(2), _gi(3), _gi(4)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_draw_circle__") {
            gui_draw_circle(_gp(0), _gi(1), _gi(2), _gi(3)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_fill_circle__") {
            gui_fill_circle(_gp(0), _gi(1), _gi(2), _gi(3)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_draw_triangle__") {
            gui_draw_triangle(_gp(0), _gi(1), _gi(2), _gi(3), _gi(4), _gi(5), _gi(6)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_fill_triangle__") {
            gui_fill_triangle(_gp(0), _gi(1), _gi(2), _gi(3), _gi(4), _gi(5), _gi(6)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_draw_text__") {
            gui_draw_text(_gp(0), std::get<std::string>(arguments[1]).c_str(), _gi(2), _gi(3), _gi(4), _gi(5), _gi(6), _gi(7)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_width__") {
            last_value = gui_text_width(_gp(0), std::get<std::string>(arguments[1]).c_str(), _gi(2)); return;
        } else if (func_name == "__builtin_gui_text_height__") {
            last_value = gui_text_height(_gp(0), _gi(1)); return;
        } else if (func_name == "__builtin_gui_image_load__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_image_load(_gp(0), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_gui_image_destroy__") {
            gui_image_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_image_draw__") {
            gui_image_draw(_gp(0), _gp(1), _gi(2), _gi(3)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_image_draw_scaled__") {
            gui_image_draw_scaled(_gp(0), _gp(1), _gi(2), _gi(3), _gi(4), _gi(5)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_image_width__") {
            last_value = gui_image_width(_gp(0)); return;
        } else if (func_name == "__builtin_gui_image_height__") {
            last_value = gui_image_height(_gp(0)); return;
        } else if (func_name == "__builtin_gui_delay__") {
            gui_delay(_gi(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_ticks__") {
            last_value = gui_ticks(); return;
        } else if (func_name == "__builtin_gui_delta_time__") {
            double dt = gui_delta_time(_gp(0)); last_value = dt; return;
        }
    }

    // gui builtins
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_gui_", 0) == 0) {
        std::string func_name = std::get<std::string>(callee);
        auto _gp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        // Phase 1: Text Rendering
        if (func_name == "__builtin_gui_font_manager_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_font_manager_create())); return;
        } else if (func_name == "__builtin_gui_font_manager_destroy__") {
            gui_font_manager_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_font_load__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_font_load(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2]), std::get<std::string>(arguments[3]).c_str()))); return;
        } else if (func_name == "__builtin_gui_font_manager_count__") {
            last_value = gui_font_manager_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_font_name__") {
            last_value = std::string(gui_font_name(_gp(0))); return;
        } else if (func_name == "__builtin_gui_font_size__") {
            last_value = gui_font_size(_gp(0)); return;
        } else if (func_name == "__builtin_gui_font_style__") {
            last_value = std::string(gui_font_style(_gp(0))); return;
        } else if (func_name == "__builtin_gui_text_renderer_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_text_renderer_create())); return;
        } else if (func_name == "__builtin_gui_text_renderer_destroy__") {
            gui_text_renderer_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_renderer_set_font__") {
            gui_text_renderer_set_font(_gp(0), _gp(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_renderer_set_color__") {
            double r=1,g=1,b=1,a=1;
            if (std::holds_alternative<double>(arguments[1])) r=std::get<double>(arguments[1]); else r=std::get<int>(arguments[1]);
            if (std::holds_alternative<double>(arguments[2])) g=std::get<double>(arguments[2]); else g=std::get<int>(arguments[2]);
            if (std::holds_alternative<double>(arguments[3])) b=std::get<double>(arguments[3]); else b=std::get<int>(arguments[3]);
            if (std::holds_alternative<double>(arguments[4])) a=std::get<double>(arguments[4]); else a=std::get<int>(arguments[4]);
            gui_text_renderer_set_color(_gp(0),(float)r,(float)g,(float)b,(float)a); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_renderer_set_align__") {
            gui_text_renderer_set_align(_gp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_renderer_set_antialias__") {
            gui_text_renderer_set_antialias(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_renderer_render__") {
            last_value = std::string(gui_text_renderer_render(_gp(0), std::get<std::string>(arguments[1]).c_str())); return;
        } else if (func_name == "__builtin_gui_text_renderer_line_count__") {
            last_value = gui_text_renderer_line_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_text_renderer_line_at__") {
            last_value = std::string(gui_text_renderer_line_at(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_text_metrics_measure__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_text_metrics_measure(_gp(0), std::get<std::string>(arguments[1]).c_str()))); return;
        } else if (func_name == "__builtin_gui_text_metrics_destroy__") {
            gui_text_metrics_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_metrics_width__") {
            last_value = gui_text_metrics_width(_gp(0)); return;
        } else if (func_name == "__builtin_gui_text_metrics_height__") {
            last_value = gui_text_metrics_height(_gp(0)); return;
        } else if (func_name == "__builtin_gui_text_metrics_ascent__") {
            last_value = gui_text_metrics_ascent(_gp(0)); return;
        } else if (func_name == "__builtin_gui_text_metrics_descent__") {
            last_value = gui_text_metrics_descent(_gp(0)); return;
        } else if (func_name == "__builtin_gui_text_metrics_line_height__") {
            last_value = gui_text_metrics_line_height(_gp(0)); return;

        // Phase 2: Mouse Support
        } else if (func_name == "__builtin_gui_mouse_handler_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_mouse_handler_create())); return;
        } else if (func_name == "__builtin_gui_mouse_handler_destroy__") {
            gui_mouse_handler_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_mouse_handler_click__") {
            gui_mouse_handler_click(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]), std::get<int>(arguments[3])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_mouse_handler_move__") {
            gui_mouse_handler_move(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_mouse_handler_press__") {
            gui_mouse_handler_press(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]), std::get<int>(arguments[3])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_mouse_handler_release__") {
            gui_mouse_handler_release(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]), std::get<int>(arguments[3])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_mouse_handler_scroll__") {
            gui_mouse_handler_scroll(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]), std::get<int>(arguments[3])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_mouse_handler_event_count__") {
            last_value = gui_mouse_handler_event_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_mouse_handler_event_type__") {
            last_value = std::string(gui_mouse_handler_event_type(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_mouse_handler_event_x__") {
            last_value = gui_mouse_handler_event_x(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_mouse_handler_event_y__") {
            last_value = gui_mouse_handler_event_y(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_mouse_handler_event_button__") {
            last_value = gui_mouse_handler_event_button(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_mouse_handler_cursor_x__") {
            last_value = gui_mouse_handler_cursor_x(_gp(0)); return;
        } else if (func_name == "__builtin_gui_mouse_handler_cursor_y__") {
            last_value = gui_mouse_handler_cursor_y(_gp(0)); return;
        } else if (func_name == "__builtin_gui_mouse_handler_is_button_down__") {
            last_value = gui_mouse_handler_is_button_down(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_drag_drop_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_drag_drop_create())); return;
        } else if (func_name == "__builtin_gui_drag_drop_destroy__") {
            gui_drag_drop_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_drag_drop_start__") {
            gui_drag_drop_start(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]), std::get<std::string>(arguments[3]).c_str(), std::get<std::string>(arguments[4]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_drag_drop_move__") {
            gui_drag_drop_move(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_drag_drop_drop__") {
            last_value = gui_drag_drop_drop(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])); return;
        } else if (func_name == "__builtin_gui_drag_drop_is_dragging__") {
            last_value = gui_drag_drop_is_dragging(_gp(0)); return;
        } else if (func_name == "__builtin_gui_drag_drop_data__") {
            last_value = std::string(gui_drag_drop_data(_gp(0))); return;
        } else if (func_name == "__builtin_gui_drag_drop_mime__") {
            last_value = std::string(gui_drag_drop_mime(_gp(0))); return;
        } else if (func_name == "__builtin_gui_drag_drop_start_x__") {
            last_value = gui_drag_drop_start_x(_gp(0)); return;
        } else if (func_name == "__builtin_gui_drag_drop_start_y__") {
            last_value = gui_drag_drop_start_y(_gp(0)); return;
        } else if (func_name == "__builtin_gui_context_menu_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_context_menu_create())); return;
        } else if (func_name == "__builtin_gui_context_menu_destroy__") {
            gui_context_menu_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_context_menu_add_item__") {
            gui_context_menu_add_item(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_context_menu_add_separator__") {
            gui_context_menu_add_separator(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_context_menu_show__") {
            gui_context_menu_show(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_context_menu_hide__") {
            gui_context_menu_hide(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_context_menu_item_count__") {
            last_value = gui_context_menu_item_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_context_menu_item_label__") {
            last_value = std::string(gui_context_menu_item_label(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_context_menu_item_action__") {
            last_value = std::string(gui_context_menu_item_action(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_context_menu_is_visible__") {
            last_value = gui_context_menu_is_visible(_gp(0)); return;
        } else if (func_name == "__builtin_gui_context_menu_x__") {
            last_value = gui_context_menu_x(_gp(0)); return;
        } else if (func_name == "__builtin_gui_context_menu_y__") {
            last_value = gui_context_menu_y(_gp(0)); return;

        // Phase 3: Advanced Widgets
        } else if (func_name == "__builtin_gui_text_input_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_text_input_create())); return;
        } else if (func_name == "__builtin_gui_text_input_destroy__") {
            gui_text_input_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_set_text__") {
            gui_text_input_set_text(_gp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_set_placeholder__") {
            gui_text_input_set_placeholder(_gp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_set_max_length__") {
            gui_text_input_set_max_length(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_set_password__") {
            gui_text_input_set_password(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_set_multiline__") {
            gui_text_input_set_multiline(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_insert__") {
            gui_text_input_insert(_gp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_clear__") {
            gui_text_input_clear(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_text__") {
            last_value = std::string(gui_text_input_text(_gp(0))); return;
        } else if (func_name == "__builtin_gui_text_input_length__") {
            last_value = gui_text_input_length(_gp(0)); return;
        } else if (func_name == "__builtin_gui_text_input_cursor_pos__") {
            last_value = gui_text_input_cursor_pos(_gp(0)); return;
        } else if (func_name == "__builtin_gui_text_input_is_focused__") {
            last_value = gui_text_input_is_focused(_gp(0)); return;
        } else if (func_name == "__builtin_gui_text_input_focus__") {
            gui_text_input_focus(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_text_input_blur__") {
            gui_text_input_blur(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_dropdown_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_dropdown_create())); return;
        } else if (func_name == "__builtin_gui_dropdown_destroy__") {
            gui_dropdown_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_dropdown_add_option__") {
            gui_dropdown_add_option(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_dropdown_set_selected__") {
            gui_dropdown_set_selected(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_dropdown_set_placeholder__") {
            gui_dropdown_set_placeholder(_gp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_dropdown_open__") {
            gui_dropdown_open(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_dropdown_close__") {
            gui_dropdown_close(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_dropdown_option_count__") {
            last_value = gui_dropdown_option_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_dropdown_option_label__") {
            last_value = std::string(gui_dropdown_option_label(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_dropdown_option_value__") {
            last_value = std::string(gui_dropdown_option_value(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_dropdown_selected_index__") {
            last_value = gui_dropdown_selected_index(_gp(0)); return;
        } else if (func_name == "__builtin_gui_dropdown_selected_value__") {
            last_value = std::string(gui_dropdown_selected_value(_gp(0))); return;
        } else if (func_name == "__builtin_gui_dropdown_is_open__") {
            last_value = gui_dropdown_is_open(_gp(0)); return;
        } else if (func_name == "__builtin_gui_table_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_table_create())); return;
        } else if (func_name == "__builtin_gui_table_destroy__") {
            gui_table_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_table_add_column__") {
            gui_table_add_column(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_table_add_row__") {
            gui_table_add_row(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_table_set_cell__") {
            gui_table_set_cell(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]), std::get<std::string>(arguments[3]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_table_get_cell__") {
            last_value = std::string(gui_table_get_cell(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]))); return;
        } else if (func_name == "__builtin_gui_table_row_count__") {
            last_value = gui_table_row_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_table_col_count__") {
            last_value = gui_table_col_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_table_column_header__") {
            last_value = std::string(gui_table_column_header(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_table_column_width__") {
            last_value = gui_table_column_width(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_table_set_selected_row__") {
            gui_table_set_selected_row(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_table_selected_row__") {
            last_value = gui_table_selected_row(_gp(0)); return;
        } else if (func_name == "__builtin_gui_table_sort_by_column__") {
            gui_table_sort_by_column(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_table_to_csv__") {
            last_value = std::string(gui_table_to_csv(_gp(0))); return;

        } else if (func_name == "__builtin_gui_tree_view_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_tree_view_create())); return;
        } else if (func_name == "__builtin_gui_tree_view_destroy__") {
            gui_tree_view_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_tree_view_add_node__") {
            last_value = gui_tree_view_add_node(_gp(0), std::get<int>(arguments[1]), std::get<std::string>(arguments[2]).c_str(), std::get<std::string>(arguments[3]).c_str()); return;
        } else if (func_name == "__builtin_gui_tree_view_expand__") {
            gui_tree_view_expand(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_tree_view_collapse__") {
            gui_tree_view_collapse(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_tree_view_select__") {
            gui_tree_view_select(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_tree_view_node_count__") {
            last_value = gui_tree_view_node_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_tree_view_node_label__") {
            last_value = std::string(gui_tree_view_node_label(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_tree_view_node_data__") {
            last_value = std::string(gui_tree_view_node_data(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_tree_view_node_parent__") {
            last_value = gui_tree_view_node_parent(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_tree_view_node_is_expanded__") {
            last_value = gui_tree_view_node_is_expanded(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_tree_view_selected_node__") {
            last_value = gui_tree_view_selected_node(_gp(0)); return;
        } else if (func_name == "__builtin_gui_tree_view_child_count__") {
            last_value = gui_tree_view_child_count(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_tabs_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_tabs_create())); return;
        } else if (func_name == "__builtin_gui_tabs_destroy__") {
            gui_tabs_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_tabs_add_tab__") {
            gui_tabs_add_tab(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_tabs_set_active__") {
            gui_tabs_set_active(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_tabs_remove_tab__") {
            gui_tabs_remove_tab(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_tabs_count__") {
            last_value = gui_tabs_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_tabs_label__") {
            last_value = std::string(gui_tabs_label(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_tabs_content_id__") {
            last_value = std::string(gui_tabs_content_id(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_tabs_active_index__") {
            last_value = gui_tabs_active_index(_gp(0)); return;
        } else if (func_name == "__builtin_gui_tabs_active_label__") {
            last_value = std::string(gui_tabs_active_label(_gp(0))); return;
        // Phase 4: Layout Management
        } else if (func_name == "__builtin_gui_flex_layout_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_flex_layout_create(std::get<std::string>(arguments[0]).c_str()))); return;
        } else if (func_name == "__builtin_gui_flex_layout_destroy__") {
            gui_flex_layout_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_flex_layout_add_item__") {
            gui_flex_layout_add_item(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2]), std::get<int>(arguments[3]), std::get<int>(arguments[4])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_flex_layout_set_gap__") {
            gui_flex_layout_set_gap(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_flex_layout_set_padding__") {
            gui_flex_layout_set_padding(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_flex_layout_set_wrap__") {
            gui_flex_layout_set_wrap(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_flex_layout_set_align__") {
            gui_flex_layout_set_align(_gp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_flex_layout_set_justify__") {
            gui_flex_layout_set_justify(_gp(0), std::get<std::string>(arguments[1]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_flex_layout_item_count__") {
            last_value = gui_flex_layout_item_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_flex_layout_item_id__") {
            last_value = std::string(gui_flex_layout_item_id(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_flex_layout_item_flex__") {
            last_value = gui_flex_layout_item_flex(_gp(0), std::get<int>(arguments[1])); return;
        } else if (func_name == "__builtin_gui_flex_layout_compute__") {
            last_value = std::string(gui_flex_layout_compute(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]))); return;
        } else if (func_name == "__builtin_gui_flex_layout_direction__") {
            last_value = std::string(gui_flex_layout_direction(_gp(0))); return;
        } else if (func_name == "__builtin_gui_grid_layout_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_grid_layout_create(std::get<int>(arguments[0]), std::get<int>(arguments[1])))); return;
        } else if (func_name == "__builtin_gui_grid_layout_destroy__") {
            gui_grid_layout_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_grid_layout_place__") {
            gui_grid_layout_place(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2]), std::get<int>(arguments[3]), std::get<int>(arguments[4]), std::get<int>(arguments[5])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_grid_layout_set_col_width__") {
            gui_grid_layout_set_col_width(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_grid_layout_set_row_height__") {
            gui_grid_layout_set_row_height(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_grid_layout_set_gap__") {
            gui_grid_layout_set_gap(_gp(0), std::get<int>(arguments[1])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_grid_layout_cols__") {
            last_value = gui_grid_layout_cols(_gp(0)); return;
        } else if (func_name == "__builtin_gui_grid_layout_rows__") {
            last_value = gui_grid_layout_rows(_gp(0)); return;
        } else if (func_name == "__builtin_gui_grid_layout_item_count__") {
            last_value = gui_grid_layout_item_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_grid_layout_item_id__") {
            last_value = std::string(gui_grid_layout_item_id(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_grid_layout_compute__") {
            last_value = std::string(gui_grid_layout_compute(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]))); return;
        } else if (func_name == "__builtin_gui_anchor_layout_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(gui_anchor_layout_create())); return;
        } else if (func_name == "__builtin_gui_anchor_layout_destroy__") {
            gui_anchor_layout_destroy(_gp(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_anchor_layout_add_item__") {
            gui_anchor_layout_add_item(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<int>(arguments[2]), std::get<int>(arguments[3]), std::get<int>(arguments[4]), std::get<int>(arguments[5])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_anchor_layout_set_anchor__") {
            gui_anchor_layout_set_anchor(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str(), std::get<int>(arguments[3])); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_anchor_layout_set_relative__") {
            gui_anchor_layout_set_relative(_gp(0), std::get<std::string>(arguments[1]).c_str(), std::get<std::string>(arguments[2]).c_str(), std::get<std::string>(arguments[3]).c_str()); last_value = nullptr; return;
        } else if (func_name == "__builtin_gui_anchor_layout_item_count__") {
            last_value = gui_anchor_layout_item_count(_gp(0)); return;
        } else if (func_name == "__builtin_gui_anchor_layout_item_id__") {
            last_value = std::string(gui_anchor_layout_item_id(_gp(0), std::get<int>(arguments[1]))); return;
        } else if (func_name == "__builtin_gui_anchor_layout_compute__") {
            last_value = std::string(gui_anchor_layout_compute(_gp(0), std::get<int>(arguments[1]), std::get<int>(arguments[2]))); return;
        }
    }

    // network builtins (Milestone 12)
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_net_", 0) == 0) {
        std::string func_name = std::get<std::string>(callee);
        auto _np = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _ns = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _ni = [&](int i) -> int { return std::get<int>(arguments[i]); };
        // TCP
        if (func_name == "__builtin_net_tcp_socket_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_tcp_socket_create())); return;
        } else if (func_name == "__builtin_net_tcp_socket_destroy__") {
            net_tcp_socket_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_tcp_connect__") {
            last_value = net_tcp_connect(_np(0), _ns(1), _ni(2)); return;
        } else if (func_name == "__builtin_net_tcp_bind__") {
            last_value = net_tcp_bind(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_tcp_listen__") {
            last_value = net_tcp_listen(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_tcp_accept__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_tcp_accept(_np(0)))); return;
        } else if (func_name == "__builtin_net_tcp_send__") {
            last_value = net_tcp_send(_np(0), _ns(1), -1); return;
        } else if (func_name == "__builtin_net_tcp_recv__") {
            last_value = net_tcp_recv(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_tcp_recv_data__") {
            last_value = std::string(net_tcp_recv_data(_np(0))); return;
        } else if (func_name == "__builtin_net_tcp_close__") {
            last_value = net_tcp_close(_np(0)); return;
        } else if (func_name == "__builtin_net_tcp_is_connected__") {
            last_value = net_tcp_is_connected(_np(0)); return;
        } else if (func_name == "__builtin_net_tcp_remote_addr__") {
            last_value = std::string(net_tcp_remote_addr(_np(0))); return;
        } else if (func_name == "__builtin_net_tcp_remote_port__") {
            last_value = net_tcp_remote_port(_np(0)); return;
        } else if (func_name == "__builtin_net_tcp_set_nonblocking__") {
            last_value = net_tcp_set_nonblocking(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_tcp_set_timeout__") {
            last_value = net_tcp_set_timeout(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_tcp_bytes_available__") {
            last_value = net_tcp_bytes_available(_np(0)); return;
        // UDP
        } else if (func_name == "__builtin_net_udp_socket_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_udp_socket_create())); return;
        } else if (func_name == "__builtin_net_udp_socket_destroy__") {
            net_udp_socket_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_udp_bind__") {
            last_value = net_udp_bind(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_udp_send_to__") {
            // args: sock, data, len, host, port
            last_value = net_udp_send_to(_np(0), _ns(1), _ni(2), _ns(3), _ni(4)); return;
        } else if (func_name == "__builtin_net_udp_recv_from__") {
            last_value = net_udp_recv_from(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_udp_recv_data__") {
            last_value = std::string(net_udp_recv_data(_np(0))); return;
        } else if (func_name == "__builtin_net_udp_recv_addr__") {
            last_value = std::string(net_udp_recv_addr(_np(0))); return;
        } else if (func_name == "__builtin_net_udp_recv_port__") {
            last_value = net_udp_recv_port(_np(0)); return;
        } else if (func_name == "__builtin_net_udp_close__") {
            last_value = net_udp_close(_np(0)); return;
        } else if (func_name == "__builtin_net_udp_set_broadcast__") {
            last_value = net_udp_set_broadcast(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_udp_set_timeout__") {
            last_value = net_udp_set_timeout(_np(0), _ni(1)); return;
        // Address helpers
        } else if (func_name == "__builtin_net_resolve_host__") {
            last_value = std::string(net_resolve_host(_ns(0))); return;
        } else if (func_name == "__builtin_net_local_ip__") {
            last_value = std::string(net_local_ip()); return;
        } else if (func_name == "__builtin_net_hostname__") {
            last_value = std::string(net_hostname()); return;
        // DNS
        } else if (func_name == "__builtin_net_dns_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_dns_create())); return;
        } else if (func_name == "__builtin_net_dns_destroy__") {
            net_dns_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_dns_lookup__") {
            last_value = std::string(net_dns_lookup(_np(0), _ns(1))); return;
        } else if (func_name == "__builtin_net_dns_reverse__") {
            last_value = std::string(net_dns_reverse(_np(0), _ns(1))); return;
        } else if (func_name == "__builtin_net_dns_lookup_count__") {
            last_value = net_dns_lookup_count(_np(0)); return;
        } else if (func_name == "__builtin_net_dns_lookup_result__") {
            last_value = std::string(net_dns_lookup_result(_np(0), _ni(1))); return;
        } else if (func_name == "__builtin_net_dns_set_server__") {
            net_dns_set_server(_np(0), _ns(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_dns_server__") {
            last_value = std::string(net_dns_server(_np(0))); return;
        // SMTP
        } else if (func_name == "__builtin_net_smtp_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_smtp_create(_ns(0), _ni(1)))); return;
        } else if (func_name == "__builtin_net_smtp_destroy__") {
            net_smtp_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_smtp_connect__") {
            last_value = net_smtp_connect(_np(0)); return;
        } else if (func_name == "__builtin_net_smtp_auth__") {
            last_value = net_smtp_auth(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_smtp_send_mail__") {
            last_value = net_smtp_send_mail(_np(0), _ns(1), _ns(2), _ns(3), _ns(4)); return;
        } else if (func_name == "__builtin_net_smtp_disconnect__") {
            last_value = net_smtp_disconnect(_np(0)); return;
        } else if (func_name == "__builtin_net_smtp_is_connected__") {
            last_value = net_smtp_is_connected(_np(0)); return;
        } else if (func_name == "__builtin_net_smtp_last_response__") {
            last_value = std::string(net_smtp_last_response(_np(0))); return;
        } else if (func_name == "__builtin_net_smtp_set_tls__") {
            net_smtp_set_tls(_np(0), _ni(1)); last_value = nullptr; return;
        // FTP
        } else if (func_name == "__builtin_net_ftp_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_ftp_create(_ns(0), _ni(1)))); return;
        } else if (func_name == "__builtin_net_ftp_destroy__") {
            net_ftp_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_ftp_connect__") {
            last_value = net_ftp_connect(_np(0)); return;
        } else if (func_name == "__builtin_net_ftp_login__") {
            last_value = net_ftp_login(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_ftp_list__") {
            last_value = net_ftp_list(_np(0), _ns(1)); return;
        } else if (func_name == "__builtin_net_ftp_file_count__") {
            last_value = net_ftp_file_count(_np(0)); return;
        } else if (func_name == "__builtin_net_ftp_file_name__") {
            last_value = std::string(net_ftp_file_name(_np(0), _ni(1))); return;
        } else if (func_name == "__builtin_net_ftp_file_size__") {
            last_value = net_ftp_file_size(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_ftp_download__") {
            last_value = net_ftp_download(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_ftp_upload__") {
            last_value = net_ftp_upload(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_ftp_mkdir__") {
            last_value = net_ftp_mkdir(_np(0), _ns(1)); return;
        } else if (func_name == "__builtin_net_ftp_delete__") {
            last_value = net_ftp_delete(_np(0), _ns(1)); return;
        } else if (func_name == "__builtin_net_ftp_disconnect__") {
            last_value = net_ftp_disconnect(_np(0)); return;
        } else if (func_name == "__builtin_net_ftp_cwd__") {
            last_value = std::string(net_ftp_cwd(_np(0))); return;
        // SSH
        } else if (func_name == "__builtin_net_ssh_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_ssh_create(_ns(0), _ni(1)))); return;
        } else if (func_name == "__builtin_net_ssh_destroy__") {
            net_ssh_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_ssh_connect__") {
            last_value = net_ssh_connect(_np(0)); return;
        } else if (func_name == "__builtin_net_ssh_auth_password__") {
            last_value = net_ssh_auth_password(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_ssh_auth_key__") {
            last_value = net_ssh_auth_key(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_ssh_exec__") {
            last_value = net_ssh_exec(_np(0), _ns(1)); return;
        } else if (func_name == "__builtin_net_ssh_output__") {
            last_value = std::string(net_ssh_output(_np(0))); return;
        } else if (func_name == "__builtin_net_ssh_stderr__") {
            last_value = std::string(net_ssh_stderr(_np(0))); return;
        } else if (func_name == "__builtin_net_ssh_exit_code__") {
            last_value = net_ssh_exit_code(_np(0)); return;
        } else if (func_name == "__builtin_net_ssh_upload__") {
            last_value = net_ssh_upload(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_ssh_download__") {
            last_value = net_ssh_download(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_ssh_disconnect__") {
            last_value = net_ssh_disconnect(_np(0)); return;
        } else if (func_name == "__builtin_net_ssh_is_connected__") {
            last_value = net_ssh_is_connected(_np(0)); return;
        // Packet capture
        } else if (func_name == "__builtin_net_capture_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_capture_create(_ns(0)))); return;
        } else if (func_name == "__builtin_net_capture_destroy__") {
            net_capture_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_capture_start__") {
            last_value = net_capture_start(_np(0)); return;
        } else if (func_name == "__builtin_net_capture_stop__") {
            last_value = net_capture_stop(_np(0)); return;
        } else if (func_name == "__builtin_net_capture_packet_count__") {
            last_value = net_capture_packet_count(_np(0)); return;
        } else if (func_name == "__builtin_net_capture_packet_src__") {
            last_value = std::string(net_capture_packet_src(_np(0), _ni(1))); return;
        } else if (func_name == "__builtin_net_capture_packet_dst__") {
            last_value = std::string(net_capture_packet_dst(_np(0), _ni(1))); return;
        } else if (func_name == "__builtin_net_capture_packet_proto__") {
            last_value = std::string(net_capture_packet_proto(_np(0), _ni(1))); return;
        } else if (func_name == "__builtin_net_capture_packet_size__") {
            last_value = net_capture_packet_size(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_capture_packet_data__") {
            last_value = std::string(net_capture_packet_data(_np(0), _ni(1))); return;
        } else if (func_name == "__builtin_net_capture_set_filter__") {
            net_capture_set_filter(_np(0), _ns(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_capture_to_json__") {
            last_value = std::string(net_capture_to_json(_np(0))); return;
        // Network monitor
        } else if (func_name == "__builtin_net_monitor_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_monitor_create())); return;
        } else if (func_name == "__builtin_net_monitor_destroy__") {
            net_monitor_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_monitor_add_interface__") {
            net_monitor_add_interface(_np(0), _ns(1)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_monitor_sample__") {
            last_value = net_monitor_sample(_np(0)); return;
        } else if (func_name == "__builtin_net_monitor_interface_count__") {
            last_value = net_monitor_interface_count(_np(0)); return;
        } else if (func_name == "__builtin_net_monitor_interface_name__") {
            last_value = std::string(net_monitor_interface_name(_np(0), _ni(1))); return;
        } else if (func_name == "__builtin_net_monitor_bytes_sent__") {
            last_value = net_monitor_bytes_sent(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_monitor_bytes_recv__") {
            last_value = net_monitor_bytes_recv(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_monitor_packets_sent__") {
            last_value = net_monitor_packets_sent(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_monitor_packets_recv__") {
            last_value = net_monitor_packets_recv(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_monitor_to_json__") {
            last_value = std::string(net_monitor_to_json(_np(0))); return;
        // Bandwidth
        } else if (func_name == "__builtin_net_bandwidth_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_bandwidth_create())); return;
        } else if (func_name == "__builtin_net_bandwidth_destroy__") {
            net_bandwidth_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_bandwidth_test_upload__") {
            last_value = net_bandwidth_test_upload(_np(0), _ns(1), _ni(2), _ni(3)); return;
        } else if (func_name == "__builtin_net_bandwidth_test_download__") {
            last_value = net_bandwidth_test_download(_np(0), _ns(1), _ni(2), _ni(3)); return;
        } else if (func_name == "__builtin_net_bandwidth_last_upload_mbps__") {
            last_value = net_bandwidth_last_upload_mbps(_np(0)); return;
        } else if (func_name == "__builtin_net_bandwidth_last_download_mbps__") {
            last_value = net_bandwidth_last_download_mbps(_np(0)); return;
        } else if (func_name == "__builtin_net_bandwidth_report__") {
            last_value = std::string(net_bandwidth_report(_np(0))); return;
        // Ping
        } else if (func_name == "__builtin_net_ping_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_ping_create())); return;
        } else if (func_name == "__builtin_net_ping_destroy__") {
            net_ping_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_ping_host__") {
            last_value = net_ping_host(_np(0), _ns(1), _ni(2)); return;
        } else if (func_name == "__builtin_net_ping_min__") {
            last_value = net_ping_min(_np(0)); return;
        } else if (func_name == "__builtin_net_ping_max__") {
            last_value = net_ping_max(_np(0)); return;
        } else if (func_name == "__builtin_net_ping_avg__") {
            last_value = net_ping_avg(_np(0)); return;
        } else if (func_name == "__builtin_net_ping_jitter__") {
            last_value = net_ping_jitter(_np(0)); return;
        } else if (func_name == "__builtin_net_ping_packet_loss__") {
            last_value = net_ping_packet_loss(_np(0)); return;
        } else if (func_name == "__builtin_net_ping_report__") {
            last_value = std::string(net_ping_report(_np(0))); return;
        // Event loop
        } else if (func_name == "__builtin_net_event_loop_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_event_loop_create())); return;
        } else if (func_name == "__builtin_net_event_loop_destroy__") {
            net_event_loop_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_event_loop_add_socket__") {
            last_value = net_event_loop_add_socket(_np(0), _np(1), _ns(2), _ns(3)); return;
        } else if (func_name == "__builtin_net_event_loop_remove_socket__") {
            last_value = net_event_loop_remove_socket(_np(0), _np(1)); return;
        } else if (func_name == "__builtin_net_event_loop_run_once__") {
            last_value = net_event_loop_run_once(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_event_loop_run__") {
            last_value = net_event_loop_run(_np(0), _ni(1)); return;
        } else if (func_name == "__builtin_net_event_loop_stop__") {
            last_value = net_event_loop_stop(_np(0)); return;
        } else if (func_name == "__builtin_net_event_loop_pending_count__") {
            last_value = net_event_loop_pending_count(_np(0)); return;
        } else if (func_name == "__builtin_net_event_loop_next_event__") {
            last_value = std::string(net_event_loop_next_event(_np(0))); return;
        } else if (func_name == "__builtin_net_event_loop_is_running__") {
            last_value = net_event_loop_is_running(_np(0)); return;
        // Connection pool
        } else if (func_name == "__builtin_net_pool_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_pool_create(_ns(0), _ni(1), _ni(2)))); return;
        } else if (func_name == "__builtin_net_pool_destroy__") {
            net_pool_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_pool_acquire__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_pool_acquire(_np(0)))); return;
        } else if (func_name == "__builtin_net_pool_release__") {
            last_value = net_pool_release(_np(0), _np(1)); return;
        } else if (func_name == "__builtin_net_pool_size__") {
            last_value = net_pool_size(_np(0)); return;
        } else if (func_name == "__builtin_net_pool_active__") {
            last_value = net_pool_active(_np(0)); return;
        } else if (func_name == "__builtin_net_pool_idle__") {
            last_value = net_pool_idle(_np(0)); return;
        } else if (func_name == "__builtin_net_pool_host__") {
            last_value = std::string(net_pool_host(_np(0))); return;
        } else if (func_name == "__builtin_net_pool_port__") {
            last_value = net_pool_port(_np(0)); return;
        } else if (func_name == "__builtin_net_pool_set_timeout__") {
            net_pool_set_timeout(_np(0), _ni(1)); last_value = nullptr; return;
        // Async request
        } else if (func_name == "__builtin_net_async_request_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_async_request_create())); return;
        } else if (func_name == "__builtin_net_async_request_destroy__") {
            net_async_request_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_async_get__") {
            last_value = net_async_get(_np(0), _ns(1)); return;
        } else if (func_name == "__builtin_net_async_post__") {
            last_value = net_async_post(_np(0), _ns(1), _ns(2)); return;
        } else if (func_name == "__builtin_net_async_is_done__") {
            last_value = net_async_is_done(_np(0)); return;
        } else if (func_name == "__builtin_net_async_status_code__") {
            last_value = net_async_status_code(_np(0)); return;
        } else if (func_name == "__builtin_net_async_response__") {
            last_value = std::string(net_async_response(_np(0))); return;
        } else if (func_name == "__builtin_net_async_error__") {
            last_value = std::string(net_async_error(_np(0))); return;
        } else if (func_name == "__builtin_net_async_elapsed_ms__") {
            last_value = net_async_elapsed_ms(_np(0)); return;
        // Load balancer
        } else if (func_name == "__builtin_net_lb_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(net_lb_create(_ns(0)))); return;
        } else if (func_name == "__builtin_net_lb_destroy__") {
            net_lb_destroy(_np(0)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_lb_add_backend__") {
            net_lb_add_backend(_np(0), _ns(1), _ni(2), _ni(3)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_lb_next_host__") {
            last_value = std::string(net_lb_next_host(_np(0))); return;
        } else if (func_name == "__builtin_net_lb_next_port__") {
            last_value = net_lb_next_port(_np(0)); return;
        } else if (func_name == "__builtin_net_lb_backend_count__") {
            last_value = net_lb_backend_count(_np(0)); return;
        } else if (func_name == "__builtin_net_lb_mark_down__") {
            net_lb_mark_down(_np(0), _ns(1), _ni(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_lb_mark_up__") {
            net_lb_mark_up(_np(0), _ns(1), _ni(2)); last_value = nullptr; return;
        } else if (func_name == "__builtin_net_lb_strategy__") {
            last_value = std::string(net_lb_strategy(_np(0))); return;
        } else if (func_name == "__builtin_net_lb_stats__") {
            last_value = std::string(net_lb_stats(_np(0))); return;
        }
    }

    // Milestone 13: texture builtins
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_texture_", 0) == 0
     || std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_spritesheet_", 0) == 0
     || std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_atlas_", 0) == 0) {
        std::string fn = std::get<std::string>(callee);
        auto _tp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _ts = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _ti = [&](int i) -> int { return std::get<int>(arguments[i]); };
        if (fn == "__builtin_texture_load__") {
            void* h = texture_load(_ts(0));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_texture_destroy__") {
            texture_destroy(_tp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_texture_width__") {
            last_value = texture_width(_tp(0)); return;
        } else if (fn == "__builtin_texture_height__") {
            last_value = texture_height(_tp(0)); return;
        } else if (fn == "__builtin_texture_bind__") {
            texture_bind(_tp(0), _ti(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_texture_error__") {
            last_value = std::string(texture_error()); return;
        } else if (fn == "__builtin_spritesheet_create__") {
            void* h = spritesheet_create(_tp(0), _ti(1), _ti(2), _ti(3));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_spritesheet_destroy__") {
            spritesheet_destroy(_tp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_spritesheet_frame_uv__") {
            float u0=0,v0=0,u1=0,v1=0;
            spritesheet_frame_uv(_tp(0), _ti(1), &u0, &v0, &u1, &v1);
            last_value = std::string(std::to_string(u0)+","+std::to_string(v0)+","+std::to_string(u1)+","+std::to_string(v1)); return;
        } else if (fn == "__builtin_atlas_load__") {
            void* h = atlas_load(_tp(0), _ts(1));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_atlas_destroy__") {
            atlas_destroy(_tp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_atlas_region_uv__") {
            float u0=0,v0=0,u1=0,v1=0;
            atlas_region_uv(_tp(0), _ts(1), &u0, &v0, &u1, &v1);
            last_value = std::string(std::to_string(u0)+","+std::to_string(v0)+","+std::to_string(u1)+","+std::to_string(v1)); return;
        } else if (fn == "__builtin_atlas_error__") {
            last_value = std::string(atlas_error()); return;
        }
    }

    // Milestone 13: shader builtins
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_shader_", 0) == 0) {
        std::string fn = std::get<std::string>(callee);
        auto _sp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _ss = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _sf = [&](int i) -> float {
            if (std::holds_alternative<double>(arguments[i])) return (float)std::get<double>(arguments[i]);
            if (std::holds_alternative<int>(arguments[i])) return (float)std::get<int>(arguments[i]);
            return 0.0f;
        };
        if (fn == "__builtin_shader_create__") {
            void* h = shader_create(_ss(0), _ss(1));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_shader_destroy__") {
            shader_destroy(_sp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_shader_use__") {
            shader_use(_sp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_shader_set_uniform_float__") {
            shader_set_uniform_float(_sp(0), _ss(1), _sf(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_shader_set_uniform_vec3__") {
            shader_set_uniform_vec3(_sp(0), _ss(1), _sf(2), _sf(3), _sf(4)); last_value = nullptr; return;
        } else if (fn == "__builtin_shader_set_uniform_mat4__") {
            // mat4: 16 floats passed as a string "f0,f1,...,f15" or individual args — use string form
            last_value = nullptr; return;
        } else if (fn == "__builtin_shader_error__") {
            last_value = std::string(shader_error()); return;
        }
    }

    // Milestone 13: model builtins
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_model_", 0) == 0) {
        std::string fn = std::get<std::string>(callee);
        auto _mp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _ms = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        if (fn == "__builtin_model_load__") {
            void* h = model_load(_ms(0));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_model_destroy__") {
            model_destroy(_mp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_model_draw__") {
            model_draw(_mp(0), arguments.size() > 1 ? _mp(1) : nullptr); last_value = nullptr; return;
        } else if (fn == "__builtin_model_vertex_count__") {
            last_value = model_vertex_count(_mp(0)); return;
        } else if (fn == "__builtin_model_face_count__") {
            last_value = model_face_count(_mp(0)); return;
        } else if (fn == "__builtin_model_error__") {
            last_value = std::string(model_error()); return;
        }
    }

    // Milestone 13: physics builtins
    if (std::holds_alternative<std::string>(callee) && (
        std::get<std::string>(callee).rfind("__builtin_physics_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_rigidbody_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_collider_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_collision_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_constraint_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_particles_", 0) == 0)) {
        std::string fn = std::get<std::string>(callee);
        auto _pp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _pi = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _pf = [&](int i) -> float {
            if (std::holds_alternative<double>(arguments[i])) return (float)std::get<double>(arguments[i]);
            if (std::holds_alternative<int>(arguments[i])) return (float)std::get<int>(arguments[i]);
            return 0.0f;
        };
        if (fn == "__builtin_physics_world_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(physics_world_create(_pf(0), _pf(1)))); return;
        } else if (fn == "__builtin_physics_world_destroy__") {
            physics_world_destroy(_pp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_physics_world_step__") {
            physics_world_step(_pp(0), _pf(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_rigidbody_create__") {
            void* h = rigidbody_create(_pp(0), _pf(1), _pf(2), _pf(3));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_rigidbody_destroy__") {
            rigidbody_destroy(_pp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_rigidbody_apply_force__") {
            rigidbody_apply_force(_pp(0), _pf(1), _pf(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_rigidbody_apply_impulse__") {
            rigidbody_apply_impulse(_pp(0), _pf(1), _pf(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_rigidbody_get_x__") {
            last_value = (double)rigidbody_get_x(_pp(0)); return;
        } else if (fn == "__builtin_rigidbody_get_y__") {
            last_value = (double)rigidbody_get_y(_pp(0)); return;
        } else if (fn == "__builtin_rigidbody_get_vx__") {
            last_value = (double)rigidbody_get_vx(_pp(0)); return;
        } else if (fn == "__builtin_rigidbody_get_vy__") {
            last_value = (double)rigidbody_get_vy(_pp(0)); return;
        } else if (fn == "__builtin_collider_add_box__") {
            collider_add_box(_pp(0), _pf(1), _pf(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_collider_add_circle__") {
            collider_add_circle(_pp(0), _pf(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_collider_add_convex_hull__") {
            last_value = nullptr; return; // hull passed as array — skip for now
        } else if (fn == "__builtin_collision_query_pair__") {
            last_value = collision_query_pair(_pp(0), _pp(1)); return;
        } else if (fn == "__builtin_constraint_add_distance__") {
            void* h = constraint_add_distance(_pp(0), _pp(1), _pp(2), _pf(3));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_constraint_add_spring__") {
            void* h = constraint_add_spring(_pp(0), _pp(1), _pp(2), _pf(3), _pf(4));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_constraint_add_hinge__") {
            void* h = constraint_add_hinge(_pp(0), _pp(1), _pp(2), _pf(3), _pf(4));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_constraint_destroy__") {
            constraint_destroy(_pp(0), _pp(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_particles_create__") {
            void* h = particles_create(_pp(0), _pf(1), _pf(2), _pi(3));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_particles_destroy__") {
            particles_destroy(_pp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_particles_emit__") {
            particles_emit(_pp(0), _pi(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_particles_update__") {
            particles_update(_pp(0), _pf(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_particles_set_color_gradient__") {
            particles_set_color_gradient(_pp(0), _pf(1), _pf(2), _pf(3), _pf(4), _pf(5), _pf(6));
            last_value = nullptr; return;
        } else if (fn == "__builtin_particles_active_count__") {
            last_value = particles_active_count(_pp(0)); return;
        } else if (fn == "__builtin_physics_error__") {
            last_value = std::string(physics_error()); return;
        }
    }

    // Milestone 13: simulation builtins
    if (std::holds_alternative<std::string>(callee) && (
        std::get<std::string>(callee).rfind("__builtin_nbody_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_fluid_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_blackhole_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_field_sim_", 0) == 0)) {
        std::string fn = std::get<std::string>(callee);
        auto _simp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _simi = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _simd = [&](int i) -> double {
            if (std::holds_alternative<double>(arguments[i])) return std::get<double>(arguments[i]);
            if (std::holds_alternative<int>(arguments[i])) return (double)std::get<int>(arguments[i]);
            return 0.0;
        };
        if (fn == "__builtin_nbody_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(nbody_create(_simd(0), _simd(1)))); return;
        } else if (fn == "__builtin_nbody_destroy__") {
            nbody_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_nbody_add_body__") {
            last_value = nbody_add_body(_simp(0), _simd(1), _simd(2), _simd(3), _simd(4), _simd(5), _simd(6), _simd(7)); return;
        } else if (fn == "__builtin_nbody_step__") {
            nbody_step(_simp(0), _simd(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_nbody_get_position__") {
            double x=0, y=0, z=0;
            nbody_get_position(_simp(0), _simi(1), &x, &y, &z);
            last_value = std::string(std::to_string(x)+","+std::to_string(y)+","+std::to_string(z)); return;
        } else if (fn == "__builtin_nbody_get_x__") {
            last_value = nbody_get_x(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_nbody_get_y__") {
            last_value = nbody_get_y(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_nbody_total_energy__") {
            last_value = nbody_total_energy(_simp(0)); return;
        } else if (fn == "__builtin_fluid_create__") {
            void* h = fluid_create(_simd(0), _simd(1), _simi(2));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_fluid_destroy__") {
            fluid_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_fluid_step__") {
            fluid_step(_simp(0), _simd(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_fluid_get_particle_position__") {
            double x=0, y=0;
            fluid_get_particle_position(_simp(0), _simi(1), &x, &y);
            last_value = std::string(std::to_string(x)+","+std::to_string(y)); return;
        } else if (fn == "__builtin_fluid_get_particle_x__") {
            last_value = fluid_get_particle_x(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_fluid_get_particle_y__") {
            last_value = fluid_get_particle_y(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_fluid_set_viscosity__") {
            fluid_set_viscosity(_simp(0), _simd(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_blackhole_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(blackhole_create(_simd(0), _simd(1)))); return;
        } else if (fn == "__builtin_blackhole_destroy__") {
            blackhole_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_blackhole_add_particle__") {
            last_value = blackhole_add_particle(_simp(0), _simd(1), _simd(2), _simd(3), _simd(4)); return;
        } else if (fn == "__builtin_blackhole_step__") {
            blackhole_step(_simp(0), _simd(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_blackhole_get_particle_position__") {
            double x=0, y=0;
            blackhole_get_particle_position(_simp(0), _simi(1), &x, &y);
            last_value = std::string(std::to_string(x)+","+std::to_string(y)); return;
        } else if (fn == "__builtin_blackhole_get_particle_x__") {
            last_value = blackhole_get_particle_x(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_blackhole_get_particle_y__") {
            last_value = blackhole_get_particle_y(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_blackhole_captured_count__") {
            last_value = blackhole_captured_count(_simp(0)); return;
        } else if (fn == "__builtin_field_sim_create__") {
            void* h = field_sim_create(_simi(0), _simd(1), _simd(2), _simd(3), _simd(4));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_field_sim_destroy__") {
            field_sim_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_field_sim_add_attractor__") {
            field_sim_add_attractor(_simp(0), _simd(1), _simd(2), _simd(3)); last_value = nullptr; return;
        } else if (fn == "__builtin_field_sim_add_repulsor__") {
            field_sim_add_repulsor(_simp(0), _simd(1), _simd(2), _simd(3)); last_value = nullptr; return;
        } else if (fn == "__builtin_field_sim_step__") {
            field_sim_step(_simp(0), _simd(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_field_sim_get_particle_position__") {
            double x=0, y=0;
            field_sim_get_particle_position(_simp(0), _simi(1), &x, &y);
            last_value = std::string(std::to_string(x)+","+std::to_string(y)); return;
        } else if (fn == "__builtin_field_sim_get_particle_x__") {
            last_value = field_sim_get_particle_x(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_field_sim_get_particle_y__") {
            last_value = field_sim_get_particle_y(_simp(0), _simi(1)); return;
        }
    }

    // gl3d: OpenGL 3D engine builtins
    if (std::holds_alternative<std::string>(callee) &&
        std::get<std::string>(callee).rfind("__builtin_gl3d_", 0) == 0) {
        std::string fn = std::get<std::string>(callee);
        auto _gp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _gi = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _gf = [&](int i) -> float {
            if (std::holds_alternative<double>(arguments[i])) return (float)std::get<double>(arguments[i]);
            if (std::holds_alternative<int>(arguments[i])) return (float)std::get<int>(arguments[i]);
            return 0.f;
        };
        auto _gs = [&](int i) -> std::string { return std::get<std::string>(arguments[i]); };
        // Window
        if (fn == "__builtin_gl3d_window_create__") {
            void* h = gl3d_window_create(_gs(0).c_str(), _gi(1), _gi(2));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_window_destroy__") {
            gl3d_window_destroy(_gp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_window_poll__") {
            last_value = gl3d_window_poll(_gp(0)); return;
        } else if (fn == "__builtin_gl3d_window_swap__") {
            gl3d_window_swap(_gp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_window_set_title__") {
            gl3d_window_set_title(_gp(0), _gs(1).c_str()); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_window_width__") {
            last_value = gl3d_window_width(_gp(0)); return;
        } else if (fn == "__builtin_gl3d_window_height__") {
            last_value = gl3d_window_height(_gp(0)); return;
        // Input
        } else if (fn == "__builtin_gl3d_key__") {
            last_value = gl3d_key(_gp(0), _gs(1).c_str()); return;
        } else if (fn == "__builtin_gl3d_key_pressed__") {
            last_value = gl3d_key_pressed(_gp(0), _gs(1).c_str()); return;
        } else if (fn == "__builtin_gl3d_mouse_dx__") {
            last_value = (double)gl3d_mouse_dx(_gp(0)); return;
        } else if (fn == "__builtin_gl3d_mouse_dy__") {
            last_value = (double)gl3d_mouse_dy(_gp(0)); return;
        } else if (fn == "__builtin_gl3d_mouse_capture__") {
            gl3d_mouse_capture(_gp(0), _gi(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_delta__") {
            last_value = (double)gl3d_delta(_gp(0)); return;
        } else if (fn == "__builtin_gl3d_time__") {
            last_value = (double)gl3d_time(_gp(0)); return;
        // Mesh
        } else if (fn == "__builtin_gl3d_mesh_destroy__") {
            gl3d_mesh_destroy(_gp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_mesh_draw__") {
            gl3d_mesh_draw(_gp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_mesh_box__") {
            void* h = gl3d_mesh_box(_gf(0), _gf(1), _gf(2));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_mesh_plane__") {
            void* h = gl3d_mesh_plane(_gf(0), _gf(1), _gi(2));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_mesh_sphere__") {
            void* h = gl3d_mesh_sphere(_gf(0), _gi(1), _gi(2));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_mesh_cylinder__") {
            void* h = gl3d_mesh_cylinder(_gf(0), _gf(1), _gi(2));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_mesh_terrain__") {
            // arg0 = Sapphire list of floats, arg1 = subdiv, arg2 = size, arg3 = hscale
            std::vector<float> hdata;
            if (std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                auto& arr = std::get<std::shared_ptr<ArrayValue>>(arguments[0])->elements;
                for (auto& v : arr) {
                    if (std::holds_alternative<double>(v)) hdata.push_back((float)std::get<double>(v));
                    else if (std::holds_alternative<int>(v)) hdata.push_back((float)std::get<int>(v));
                    else hdata.push_back(0.f);
                }
            }
            void* h = gl3d_mesh_terrain(hdata.data(), _gi(1), _gf(2), _gf(3));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        // Line mesh
        } else if (fn == "__builtin_gl3d_mesh_lines_create__") {
            std::vector<float> vdata;
            if (std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[0])) {
                auto& arr = std::get<std::shared_ptr<ArrayValue>>(arguments[0])->elements;
                for (auto& v : arr) {
                    if (std::holds_alternative<double>(v)) vdata.push_back((float)std::get<double>(v));
                    else if (std::holds_alternative<int>(v)) vdata.push_back((float)std::get<int>(v));
                    else vdata.push_back(0.f);
                }
            }
            int vc = (int)(vdata.size() / 3);
            void* h = gl3d_mesh_lines_create(vdata.data(), vc);
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_mesh_lines_update__") {
            std::vector<float> vdata;
            if (std::holds_alternative<std::shared_ptr<ArrayValue>>(arguments[1])) {
                auto& arr = std::get<std::shared_ptr<ArrayValue>>(arguments[1])->elements;
                for (auto& v : arr) {
                    if (std::holds_alternative<double>(v)) vdata.push_back((float)std::get<double>(v));
                    else if (std::holds_alternative<int>(v)) vdata.push_back((float)std::get<int>(v));
                    else vdata.push_back(0.f);
                }
            }
            int vc = (int)(vdata.size() / 3);
            gl3d_mesh_lines_update(_gp(0), vdata.data(), vc); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_mesh_lines_draw__") {
            gl3d_mesh_lines_draw(_gp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_mesh_lines_destroy__") {
            gl3d_mesh_lines_destroy(_gp(0)); last_value = nullptr; return;
        // Shader
        } else if (fn == "__builtin_gl3d_shader_create__") {
            void* h = gl3d_shader_create(_gs(0).c_str(), _gs(1).c_str());
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_shader_destroy__") {
            gl3d_shader_destroy(_gp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_shader_use__") {
            gl3d_shader_use(_gp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_shader_set_float__") {
            gl3d_shader_set_float(_gp(0), _gs(1).c_str(), _gf(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_shader_set_vec3__") {
            gl3d_shader_set_vec3(_gp(0), _gs(1).c_str(), _gf(2), _gf(3), _gf(4)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_shader_set_vec4__") {
            gl3d_shader_set_vec4(_gp(0), _gs(1).c_str(), _gf(2), _gf(3), _gf(4), _gf(5)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_shader_set_mat4__") {
            // mat4 passed as int handle (pointer to float[16])
            float* m = reinterpret_cast<float*>(static_cast<intptr_t>(_gi(2)));
            gl3d_shader_set_mat4(_gp(0), _gs(1).c_str(), m); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_shader_set_int__") {
            gl3d_shader_set_int(_gp(0), _gs(1).c_str(), _gi(2)); last_value = nullptr; return;
        // Texture
        } else if (fn == "__builtin_gl3d_texture_load__") {
            void* h = gl3d_texture_load(_gs(0).c_str());
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_texture_solid__") {
            void* h = gl3d_texture_solid((unsigned char)_gi(0),(unsigned char)_gi(1),(unsigned char)_gi(2));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_texture_destroy__") {
            gl3d_texture_destroy(_gp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_texture_bind__") {
            gl3d_texture_bind(_gp(0), _gi(1)); last_value = nullptr; return;
        // Math
        } else if (fn == "__builtin_gl3d_mat4_identity__") {
            float* m = gl3d_mat4_identity();
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_perspective__") {
            float* m = gl3d_mat4_perspective(_gf(0), _gf(1), _gf(2), _gf(3));
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_lookat__") {
            float* m = gl3d_mat4_lookat(_gf(0),_gf(1),_gf(2),_gf(3),_gf(4),_gf(5),_gf(6),_gf(7),_gf(8));
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_translate__") {
            float* m = gl3d_mat4_translate(_gf(0), _gf(1), _gf(2));
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_scale__") {
            float* m = gl3d_mat4_scale(_gf(0), _gf(1), _gf(2));
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_rotate_y__") {
            float* m = gl3d_mat4_rotate_y(_gf(0));
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_rotate_x__") {
            float* m = gl3d_mat4_rotate_x(_gf(0));
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_rotate_z__") {
            float* m = gl3d_mat4_rotate_z(_gf(0));
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_mul__") {
            float* a = reinterpret_cast<float*>(static_cast<intptr_t>(_gi(0)));
            float* b = reinterpret_cast<float*>(static_cast<intptr_t>(_gi(1)));
            float* m = gl3d_mat4_mul(a, b);
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(m)); return;
        } else if (fn == "__builtin_gl3d_mat4_free__") {
            float* m = reinterpret_cast<float*>(static_cast<intptr_t>(_gi(0)));
            gl3d_mat4_free(m); last_value = nullptr; return;
        // Render state
        } else if (fn == "__builtin_gl3d_clear__") {
            gl3d_clear(_gf(0), _gf(1), _gf(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_depth_test__") {
            gl3d_depth_test(_gi(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_wireframe__") {
            gl3d_wireframe(_gi(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_viewport__") {
            gl3d_viewport(_gi(0), _gi(1), _gi(2), _gi(3)); last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_error__") {
            last_value = std::string(gl3d_error()); return;
        } else if (fn == "__builtin_gl3d_world4d_build__") {
            // arg0: cam list (20 floats), arg1: cells list (ncells*4 floats), arg2: w_dist
            std::vector<float> cam_data, cell_data;
            auto list_to_floats = [](const Value& v, std::vector<float>& out) {
                if (std::holds_alternative<std::shared_ptr<ArrayValue>>(v)) {
                    for (auto& e : std::get<std::shared_ptr<ArrayValue>>(v)->elements) {
                        if (std::holds_alternative<double>(e)) out.push_back((float)std::get<double>(e));
                        else if (std::holds_alternative<int>(e)) out.push_back((float)std::get<int>(e));
                        else out.push_back(0.f);
                    }
                }
            };
            list_to_floats(arguments[0], cam_data);
            list_to_floats(arguments[1], cell_data);
            while (cam_data.size() < 20) cam_data.push_back(0.f);
            int ncells = (int)(cell_data.size() / 4);
            void* h = gl3d_world4d_build(cam_data.data(), cell_data.data(), ncells, _gf(2));
            last_value = h ? static_cast<int>(reinterpret_cast<intptr_t>(h)) : 0; return;
        } else if (fn == "__builtin_gl3d_world4d_update__") {
            std::vector<float> cam_data, cell_data;
            auto list_to_floats = [](const Value& v, std::vector<float>& out) {
                if (std::holds_alternative<std::shared_ptr<ArrayValue>>(v)) {
                    for (auto& e : std::get<std::shared_ptr<ArrayValue>>(v)->elements) {
                        if (std::holds_alternative<double>(e)) out.push_back((float)std::get<double>(e));
                        else if (std::holds_alternative<int>(e)) out.push_back((float)std::get<int>(e));
                        else out.push_back(0.f);
                    }
                }
            };
            list_to_floats(arguments[1], cam_data);
            list_to_floats(arguments[2], cell_data);
            while (cam_data.size() < 20) cam_data.push_back(0.f);
            int ncells = (int)(cell_data.size() / 4);
            gl3d_world4d_update(_gp(0), cam_data.data(), cell_data.data(), ncells, _gf(3));
            last_value = nullptr; return;
        } else if (fn == "__builtin_gl3d_car_step__") {
            // args: state_list, input_list, heights_list, subdiv, tsize, hscale,
            //       tree_xs_list, tree_zs_list, dt
            auto lf = [](const Value& v, std::vector<float>& out) {
                if (std::holds_alternative<std::shared_ptr<ArrayValue>>(v)) {
                    for (auto& e : std::get<std::shared_ptr<ArrayValue>>(v)->elements) {
                        if (std::holds_alternative<double>(e)) out.push_back((float)std::get<double>(e));
                        else if (std::holds_alternative<int>(e)) out.push_back((float)std::get<int>(e));
                        else out.push_back(0.f);
                    }
                }
            };
            std::vector<float> state, inp, hts, txs, tzs;
            lf(arguments[0], state); while(state.size()<12) state.push_back(0.f);
            lf(arguments[1], inp);   while(inp.size()<5)    inp.push_back(0.f);
            lf(arguments[2], hts);
            lf(arguments[6], txs);
            lf(arguments[7], tzs);
            int subdiv = std::holds_alternative<int>(arguments[3]) ? std::get<int>(arguments[3]) : (int)std::get<double>(arguments[3]);
            float tsize  = std::holds_alternative<int>(arguments[4]) ? (float)std::get<int>(arguments[4]) : (float)std::get<double>(arguments[4]);
            float hscale = std::holds_alternative<int>(arguments[5]) ? (float)std::get<int>(arguments[5]) : (float)std::get<double>(arguments[5]);
            float dt_val = std::holds_alternative<int>(arguments[8]) ? (float)std::get<int>(arguments[8]) : (float)std::get<double>(arguments[8]);
            int ntrees = (int)txs.size();
            float out_state[12] = {};
            gl3d_car_step(state.data(), inp.data(), hts.data(), subdiv, tsize, hscale,
                          txs.data(), tzs.data(), ntrees, dt_val, out_state);
            auto result = std::make_shared<ArrayValue>();
            for (int i = 0; i < 12; i++) result->elements.push_back((double)out_state[i]);
            last_value = result; return;
        }
    }

    // Milestone 14: OS Development
    if (std::holds_alternative<std::string>(callee) && (
        std::get<std::string>(callee).rfind("__builtin_boot_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_os_kernel_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_pmm_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_vmm_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_heap_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_scheduler_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_syscall_table_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_driver_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_os_vga_", 0) == 0)) {
        std::string fn = std::get<std::string>(callee);
        auto _simp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _simi = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _sims = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _simd = [&](int i) -> double {
            if (std::holds_alternative<double>(arguments[i])) return std::get<double>(arguments[i]);
            if (std::holds_alternative<int>(arguments[i])) return (double)std::get<int>(arguments[i]);
            return 0.0;
        };

        if (fn == "__builtin_boot_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(boot_create(_sims(0)))); return;
        } else if (fn == "__builtin_boot_destroy__") {
            boot_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_boot_load_mbr__") {
            last_value = boot_load_mbr(_simp(0)); return;
        } else if (fn == "__builtin_boot_load_stage2__") {
            last_value = boot_load_stage2(_simp(0), (uint32_t)_simi(1), (uint32_t)_simi(2)); return;
        } else if (fn == "__builtin_boot_load_kernel__") {
            last_value = boot_load_kernel(_simp(0), (uint32_t)_simi(1), (uint32_t)_simi(2)); return;
        } else if (fn == "__builtin_boot_status__") {
            last_value = std::string(boot_status(_simp(0))); return;
        } else if (fn == "__builtin_boot_get_memory_map_count__") {
            last_value = boot_get_memory_map_count(_simp(0)); return;
        } else if (fn == "__builtin_boot_get_memory_map_entry__") {
            last_value = std::string(boot_get_memory_map_entry(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_boot_enter_protected_mode__") {
            boot_enter_protected_mode(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_boot_enter_long_mode__") {
            boot_enter_long_mode(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_boot_mode__") {
            last_value = std::string(boot_mode(_simp(0))); return;
        } else if (fn == "__builtin_os_kernel_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(os_kernel_create(_sims(0)))); return;
        } else if (fn == "__builtin_os_kernel_destroy__") {
            os_kernel_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_init_idt__") {
            os_kernel_init_idt(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_init_gdt__") {
            os_kernel_init_gdt(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_init_pic__") {
            os_kernel_init_pic(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_init_pit__") {
            os_kernel_init_pit(_simp(0), (uint32_t)_simi(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_register_isr__") {
            os_kernel_register_isr(_simp(0), _simi(1), _sims(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_isr_count__") {
            last_value = os_kernel_isr_count(_simp(0)); return;
        } else if (fn == "__builtin_os_kernel_isr_name__") {
            last_value = std::string(os_kernel_isr_name(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_os_kernel_trigger_interrupt__") {
            os_kernel_trigger_interrupt(_simp(0), _simi(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_interrupt_count__") {
            last_value = os_kernel_interrupt_count(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_os_kernel_vga_write__") {
            os_kernel_vga_write(_simp(0), _simi(1), _simi(2), _sims(3), _simi(4)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_vga_read__") {
            last_value = std::string(os_kernel_vga_read(_simp(0), _simi(1), _simi(2))); return;
        } else if (fn == "__builtin_os_kernel_vga_clear__") {
            os_kernel_vga_clear(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_kernel_status__") {
            last_value = std::string(os_kernel_status(_simp(0))); return;
        } else if (fn == "__builtin_pmm_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(pmm_create((uint64_t)_simd(0), (uint32_t)_simi(1)))); return;
        } else if (fn == "__builtin_pmm_destroy__") {
            pmm_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_pmm_mark_free__") {
            pmm_mark_free(_simp(0), (uint64_t)_simd(1), (uint64_t)_simd(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_pmm_mark_used__") {
            pmm_mark_used(_simp(0), (uint64_t)_simd(1), (uint64_t)_simd(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_pmm_alloc_page__") {
            last_value = std::to_string(pmm_alloc_page(_simp(0))); return;
        } else if (fn == "__builtin_pmm_free_page__") {
            pmm_free_page(_simp(0), (uint64_t)_simd(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_pmm_alloc_pages__") {
            last_value = std::to_string(pmm_alloc_pages(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_pmm_free_pages_count__") {
            last_value = pmm_free_pages_count(_simp(0)); return;
        } else if (fn == "__builtin_pmm_used_pages_count__") {
            last_value = pmm_used_pages_count(_simp(0)); return;
        } else if (fn == "__builtin_pmm_total_pages__") {
            last_value = pmm_total_pages(_simp(0)); return;
        } else if (fn == "__builtin_pmm_page_size__") {
            last_value = (int)pmm_page_size(_simp(0)); return;
        } else if (fn == "__builtin_vmm_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(vmm_create(_simp(0)))); return;
        } else if (fn == "__builtin_vmm_destroy__") {
            vmm_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_vmm_map__") {
            last_value = vmm_map(_simp(0), (uint64_t)_simd(1), (uint64_t)_simd(2), _simi(3)); return;
        } else if (fn == "__builtin_vmm_unmap__") {
            last_value = vmm_unmap(_simp(0), (uint64_t)_simd(1)); return;
        } else if (fn == "__builtin_vmm_translate__") {
            last_value = std::to_string(vmm_translate(_simp(0), (uint64_t)_simd(1))); return;
        } else if (fn == "__builtin_vmm_is_mapped__") {
            last_value = vmm_is_mapped(_simp(0), (uint64_t)_simd(1)); return;
        } else if (fn == "__builtin_vmm_page_fault_count__") {
            last_value = vmm_page_fault_count(_simp(0)); return;
        } else if (fn == "__builtin_vmm_dump_table__") {
            last_value = std::string(vmm_dump_table(_simp(0))); return;
        } else if (fn == "__builtin_heap_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(heap_create((uint64_t)_simd(0), (uint64_t)_simd(1)))); return;
        } else if (fn == "__builtin_heap_destroy__") {
            heap_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_heap_alloc__") {
            last_value = std::to_string(heap_alloc(_simp(0), (uint64_t)_simd(1))); return;
        } else if (fn == "__builtin_heap_free__") {
            heap_free(_simp(0), (uint64_t)_simd(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_heap_used__") {
            last_value = std::to_string(heap_used(_simp(0))); return;
        } else if (fn == "__builtin_heap_free_space__") {
            last_value = std::to_string(heap_free_space(_simp(0))); return;
        } else if (fn == "__builtin_heap_block_count__") {
            last_value = heap_block_count(_simp(0)); return;
        } else if (fn == "__builtin_scheduler_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(scheduler_create(_sims(0)))); return;
        } else if (fn == "__builtin_scheduler_destroy__") {
            scheduler_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_scheduler_add_process__") {
            last_value = scheduler_add_process(_simp(0), _sims(1), _simi(2), (uint64_t)_simd(3)); return;
        } else if (fn == "__builtin_scheduler_terminate__") {
            scheduler_terminate(_simp(0), _simi(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_scheduler_tick__") {
            last_value = scheduler_tick(_simp(0)); return;
        } else if (fn == "__builtin_scheduler_current_pid__") {
            last_value = scheduler_current_pid(_simp(0)); return;
        } else if (fn == "__builtin_scheduler_current_name__") {
            last_value = std::string(scheduler_current_name(_simp(0))); return;
        } else if (fn == "__builtin_scheduler_process_count__") {
            last_value = scheduler_process_count(_simp(0)); return;
        } else if (fn == "__builtin_scheduler_process_name__") {
            last_value = std::string(scheduler_process_name(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_scheduler_process_state__") {
            last_value = std::string(scheduler_process_state(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_scheduler_process_priority__") {
            last_value = scheduler_process_priority(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_scheduler_process_ticks__") {
            last_value = (int)scheduler_process_ticks(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_scheduler_block__") {
            scheduler_block(_simp(0), _simi(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_scheduler_unblock__") {
            scheduler_unblock(_simp(0), _simi(1)); last_value = nullptr; return;
        } else if (fn == "__builtin_syscall_table_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(syscall_table_create())); return;
        } else if (fn == "__builtin_syscall_table_destroy__") {
            syscall_table_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_syscall_table_register__") {
            syscall_table_register(_simp(0), _simi(1), _sims(2), _simi(3)); last_value = nullptr; return;
        } else if (fn == "__builtin_syscall_table_invoke__") {
            last_value = std::string(syscall_table_invoke(_simp(0), _simi(1), _sims(2))); return;
        } else if (fn == "__builtin_syscall_table_count__") {
            last_value = syscall_table_count(_simp(0)); return;
        } else if (fn == "__builtin_syscall_table_name__") {
            last_value = std::string(syscall_table_name(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_driver_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(driver_create(_sims(0), _sims(1)))); return;
        } else if (fn == "__builtin_driver_destroy__") {
            driver_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_driver_write__") {
            driver_write(_simp(0), (uint64_t)_simd(1), (uint64_t)_simd(2), _simi(3)); last_value = nullptr; return;
        } else if (fn == "__builtin_driver_read__") {
            last_value = (int)driver_read(_simp(0), (uint64_t)_simd(1), _simi(2)); return;
        } else if (fn == "__builtin_driver_irq_fire__") {
            driver_irq_fire(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_driver_irq_count__") {
            last_value = driver_irq_count(_simp(0)); return;
        } else if (fn == "__builtin_driver_name__") {
            last_value = std::string(driver_name(_simp(0))); return;
        } else if (fn == "__builtin_driver_type__") {
            last_value = std::string(driver_type(_simp(0))); return;
        } else if (fn == "__builtin_driver_is_ready__") {
            last_value = driver_is_ready(_simp(0)); return;
        } else if (fn == "__builtin_os_vga_create__") {
            last_value = static_cast<int>(reinterpret_cast<intptr_t>(os_vga_create(_simi(0), _simi(1)))); return;
        } else if (fn == "__builtin_os_vga_destroy__") {
            os_vga_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_vga_putchar__") {
            os_vga_putchar(_simp(0), (char)_simi(1), _simi(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_vga_puts__") {
            os_vga_puts(_simp(0), _sims(1), _simi(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_vga_set_cursor__") {
            os_vga_set_cursor(_simp(0), _simi(1), _simi(2)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_vga_clear__") {
            os_vga_clear(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_os_vga_cursor_row__") {
            last_value = os_vga_cursor_row(_simp(0)); return;
        } else if (fn == "__builtin_os_vga_cursor_col__") {
            last_value = os_vga_cursor_col(_simp(0)); return;
        } else if (fn == "__builtin_os_vga_get_line__") {
            last_value = std::string(os_vga_get_line(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_os_vga_cols__") {
            last_value = os_vga_cols(_simp(0)); return;
        } else if (fn == "__builtin_os_vga_rows__") {
            last_value = os_vga_rows(_simp(0)); return;
        }
    }

    // Milestone 15: Advanced Cryptography
    if (std::holds_alternative<std::string>(callee) && (
        std::get<std::string>(callee).rfind("__builtin_kyber_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_sphincs_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_mceliece_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_rainbow_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_zksnark_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_zkstark_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_bulletproof_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_plonk_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_bgv_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_bfv_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_ckks_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_tfhe_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_shamir_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_smpc_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_threshold_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_ot_", 0) == 0)) {
        std::string fn = std::get<std::string>(callee);
        auto _simp = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _simi = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _sims = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _simd = [&](int i) -> double { return std::holds_alternative<double>(arguments[i]) ? std::get<double>(arguments[i]) : (double)std::get<int>(arguments[i]); };
        auto _simll = [&](int i) -> long long { return std::holds_alternative<int>(arguments[i]) ? (long long)std::get<int>(arguments[i]) : (long long)std::get<double>(arguments[i]); };
        auto _ptr2int = [&](void* p) -> int { return (int)(intptr_t)p; };
        // ---- Kyber ----
        if (fn == "__builtin_kyber_keygen__") {
            last_value = _ptr2int(advcrypto_kyber_keygen(_simi(0))); return;
        } else if (fn == "__builtin_kyber_destroy_keypair__") {
            advcrypto_kyber_destroy_keypair(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_kyber_public_key__") {
            last_value = std::string(advcrypto_kyber_public_key(_simp(0))); return;
        } else if (fn == "__builtin_kyber_secret_key__") {
            last_value = std::string(advcrypto_kyber_secret_key(_simp(0))); return;
        } else if (fn == "__builtin_kyber_security_level__") {
            last_value = advcrypto_kyber_security_level(_simp(0)); return;
        } else if (fn == "__builtin_kyber_encapsulate__") {
            last_value = _ptr2int(advcrypto_kyber_encapsulate(_sims(0), _simi(1))); return;
        } else if (fn == "__builtin_kyber_destroy_encap__") {
            advcrypto_kyber_destroy_encap(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_kyber_ciphertext__") {
            last_value = std::string(advcrypto_kyber_ciphertext(_simp(0))); return;
        } else if (fn == "__builtin_kyber_shared_secret__") {
            last_value = std::string(advcrypto_kyber_shared_secret(_simp(0))); return;
        } else if (fn == "__builtin_kyber_decapsulate__") {
            last_value = std::string(advcrypto_kyber_decapsulate(_sims(0), _sims(1), _simi(2))); return;
        // ---- SPHINCS+ ----
        } else if (fn == "__builtin_sphincs_keygen__") {
            last_value = _ptr2int(advcrypto_sphincs_keygen(_sims(0))); return;
        } else if (fn == "__builtin_sphincs_destroy_keypair__") {
            advcrypto_sphincs_destroy_keypair(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_sphincs_public_key__") {
            last_value = std::string(advcrypto_sphincs_public_key(_simp(0))); return;
        } else if (fn == "__builtin_sphincs_secret_key__") {
            last_value = std::string(advcrypto_sphincs_secret_key(_simp(0))); return;
        } else if (fn == "__builtin_sphincs_sign__") {
            last_value = std::string(advcrypto_sphincs_sign(_sims(0), _sims(1), _sims(2))); return;
        } else if (fn == "__builtin_sphincs_verify__") {
            last_value = advcrypto_sphincs_verify(_sims(0), _sims(1), _sims(2), _sims(3)); return;
        // ---- McEliece ----
        } else if (fn == "__builtin_mceliece_keygen__") {
            last_value = _ptr2int(advcrypto_mceliece_keygen(_simi(0), _simi(1), _simi(2))); return;
        } else if (fn == "__builtin_mceliece_destroy_keypair__") {
            advcrypto_mceliece_destroy_keypair(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_mceliece_public_key__") {
            last_value = std::string(advcrypto_mceliece_public_key(_simp(0))); return;
        } else if (fn == "__builtin_mceliece_secret_key__") {
            last_value = std::string(advcrypto_mceliece_secret_key(_simp(0))); return;
        } else if (fn == "__builtin_mceliece_encrypt__") {
            last_value = std::string(advcrypto_mceliece_encrypt(_sims(0), _sims(1))); return;
        } else if (fn == "__builtin_mceliece_decrypt__") {
            last_value = std::string(advcrypto_mceliece_decrypt(_sims(0), _sims(1))); return;
        // ---- Rainbow ----
        } else if (fn == "__builtin_rainbow_keygen__") {
            last_value = _ptr2int(advcrypto_rainbow_keygen(_sims(0))); return;
        } else if (fn == "__builtin_rainbow_destroy_keypair__") {
            advcrypto_rainbow_destroy_keypair(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_rainbow_public_key__") {
            last_value = std::string(advcrypto_rainbow_public_key(_simp(0))); return;
        } else if (fn == "__builtin_rainbow_secret_key__") {
            last_value = std::string(advcrypto_rainbow_secret_key(_simp(0))); return;
        } else if (fn == "__builtin_rainbow_sign__") {
            last_value = std::string(advcrypto_rainbow_sign(_sims(0), _sims(1), _sims(2))); return;
        } else if (fn == "__builtin_rainbow_verify__") {
            last_value = advcrypto_rainbow_verify(_sims(0), _sims(1), _sims(2), _sims(3)); return;
        // ---- zk-SNARK ----
        } else if (fn == "__builtin_zksnark_setup__") {
            last_value = _ptr2int(advcrypto_zksnark_setup(_sims(0))); return;
        } else if (fn == "__builtin_zksnark_destroy_keys__") {
            advcrypto_zksnark_destroy_keys(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_zksnark_proving_key__") {
            last_value = std::string(advcrypto_zksnark_proving_key(_simp(0))); return;
        } else if (fn == "__builtin_zksnark_verification_key__") {
            last_value = std::string(advcrypto_zksnark_verification_key(_simp(0))); return;
        } else if (fn == "__builtin_zksnark_circuit_id__") {
            last_value = std::string(advcrypto_zksnark_circuit_id(_simp(0))); return;
        } else if (fn == "__builtin_zksnark_prove__") {
            last_value = _ptr2int(advcrypto_zksnark_prove(_sims(0), _sims(1), _sims(2), _sims(3))); return;
        } else if (fn == "__builtin_zksnark_destroy_proof__") {
            advcrypto_zksnark_destroy_proof(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_zksnark_proof_hex__") {
            last_value = std::string(advcrypto_zksnark_proof_hex(_simp(0))); return;
        } else if (fn == "__builtin_zksnark_public_inputs__") {
            last_value = std::string(advcrypto_zksnark_public_inputs(_simp(0))); return;
        } else if (fn == "__builtin_zksnark_verify__") {
            last_value = advcrypto_zksnark_verify(_sims(0), _sims(1), _sims(2), _sims(3)); return;
        // ---- zk-STARK ----
        } else if (fn == "__builtin_zkstark_prove__") {
            last_value = _ptr2int(advcrypto_zkstark_prove(_sims(0), _simi(1))); return;
        } else if (fn == "__builtin_zkstark_destroy_proof__") {
            advcrypto_zkstark_destroy_proof(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_zkstark_proof_hex__") {
            last_value = std::string(advcrypto_zkstark_proof_hex(_simp(0))); return;
        } else if (fn == "__builtin_zkstark_trace_commitment__") {
            last_value = std::string(advcrypto_zkstark_trace_commitment(_simp(0))); return;
        } else if (fn == "__builtin_zkstark_verify__") {
            last_value = advcrypto_zkstark_verify(_sims(0), _sims(1), _simi(2)); return;
        // ---- Bulletproofs ----
        } else if (fn == "__builtin_bulletproof_prove_range__") {
            last_value = _ptr2int(advcrypto_bulletproof_prove_range(_simll(0), _simi(1))); return;
        } else if (fn == "__builtin_bulletproof_destroy__") {
            advcrypto_bulletproof_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_bulletproof_proof_hex__") {
            last_value = std::string(advcrypto_bulletproof_proof_hex(_simp(0))); return;
        } else if (fn == "__builtin_bulletproof_commitment_hex__") {
            last_value = std::string(advcrypto_bulletproof_commitment_hex(_simp(0))); return;
        } else if (fn == "__builtin_bulletproof_verify_range__") {
            last_value = advcrypto_bulletproof_verify_range(_sims(0), _sims(1), _simi(2)); return;
        // ---- PLONK ----
        } else if (fn == "__builtin_plonk_setup__") {
            last_value = _ptr2int(advcrypto_plonk_setup(_simi(0))); return;
        } else if (fn == "__builtin_plonk_destroy_srs__") {
            advcrypto_plonk_destroy_srs(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_plonk_srs_hex__") {
            last_value = std::string(advcrypto_plonk_srs_hex(_simp(0))); return;
        } else if (fn == "__builtin_plonk_srs_id__") {
            last_value = std::string(advcrypto_plonk_srs_id(_simp(0))); return;
        } else if (fn == "__builtin_plonk_prove__") {
            last_value = _ptr2int(advcrypto_plonk_prove(_sims(0), _sims(1), _sims(2), _sims(3))); return;
        } else if (fn == "__builtin_plonk_destroy_proof__") {
            advcrypto_plonk_destroy_proof(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_plonk_proof_hex__") {
            last_value = std::string(advcrypto_plonk_proof_hex(_simp(0))); return;
        } else if (fn == "__builtin_plonk_verify__") {
            last_value = advcrypto_plonk_verify(_sims(0), _sims(1), _sims(2), _sims(3)); return;
        // ---- BGV ----
        } else if (fn == "__builtin_bgv_create_context__") {
            last_value = _ptr2int(advcrypto_bgv_create_context(_simi(0), _simi(1))); return;
        } else if (fn == "__builtin_bgv_destroy_context__") {
            advcrypto_bgv_destroy_context(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_bgv_context_id__") {
            last_value = std::string(advcrypto_bgv_context_id(_simp(0))); return;
        } else if (fn == "__builtin_bgv_keygen__") {
            last_value = _ptr2int(advcrypto_bgv_keygen(_sims(0))); return;
        } else if (fn == "__builtin_bgv_destroy_keypair__") {
            advcrypto_bgv_destroy_keypair(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_bgv_public_key__") {
            last_value = std::string(advcrypto_bgv_public_key(_simp(0))); return;
        } else if (fn == "__builtin_bgv_secret_key__") {
            last_value = std::string(advcrypto_bgv_secret_key(_simp(0))); return;
        } else if (fn == "__builtin_bgv_encrypt__") {
            last_value = _ptr2int(advcrypto_bgv_encrypt(_sims(0), _sims(1), _simll(2))); return;
        } else if (fn == "__builtin_bgv_destroy_ciphertext__") {
            advcrypto_bgv_destroy_ciphertext(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_bgv_ciphertext_hex__") {
            last_value = std::string(advcrypto_bgv_ciphertext_hex(_simp(0))); return;
        } else if (fn == "__builtin_bgv_decrypt__") {
            last_value = (int)advcrypto_bgv_decrypt(_sims(0), _sims(1), _sims(2)); return;
        } else if (fn == "__builtin_bgv_add__") {
            last_value = _ptr2int(advcrypto_bgv_add(_sims(0), _sims(1), _sims(2))); return;
        } else if (fn == "__builtin_bgv_multiply__") {
            last_value = _ptr2int(advcrypto_bgv_multiply(_sims(0), _sims(1), _sims(2))); return;
        // ---- BFV ----
        } else if (fn == "__builtin_bfv_create_context__") {
            last_value = _ptr2int(advcrypto_bfv_create_context(_simi(0), _simi(1))); return;
        } else if (fn == "__builtin_bfv_destroy_context__") {
            advcrypto_bfv_destroy_context(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_bfv_context_id__") {
            last_value = std::string(advcrypto_bfv_context_id(_simp(0))); return;
        } else if (fn == "__builtin_bfv_keygen__") {
            last_value = _ptr2int(advcrypto_bfv_keygen(_sims(0))); return;
        } else if (fn == "__builtin_bfv_destroy_keypair__") {
            advcrypto_bfv_destroy_keypair(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_bfv_public_key__") {
            last_value = std::string(advcrypto_bfv_public_key(_simp(0))); return;
        } else if (fn == "__builtin_bfv_secret_key__") {
            last_value = std::string(advcrypto_bfv_secret_key(_simp(0))); return;
        } else if (fn == "__builtin_bfv_encrypt__") {
            last_value = _ptr2int(advcrypto_bfv_encrypt(_sims(0), _sims(1), _simll(2))); return;
        } else if (fn == "__builtin_bfv_destroy_ciphertext__") {
            advcrypto_bfv_destroy_ciphertext(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_bfv_ciphertext_hex__") {
            last_value = std::string(advcrypto_bfv_ciphertext_hex(_simp(0))); return;
        } else if (fn == "__builtin_bfv_decrypt__") {
            last_value = (int)advcrypto_bfv_decrypt(_sims(0), _sims(1), _sims(2)); return;
        } else if (fn == "__builtin_bfv_add__") {
            last_value = _ptr2int(advcrypto_bfv_add(_sims(0), _sims(1), _sims(2))); return;
        } else if (fn == "__builtin_bfv_multiply__") {
            last_value = _ptr2int(advcrypto_bfv_multiply(_sims(0), _sims(1), _sims(2))); return;
        // ---- CKKS ----
        } else if (fn == "__builtin_ckks_create_context__") {
            last_value = _ptr2int(advcrypto_ckks_create_context(_simi(0), _simi(1))); return;
        } else if (fn == "__builtin_ckks_destroy_context__") {
            advcrypto_ckks_destroy_context(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_ckks_context_id__") {
            last_value = std::string(advcrypto_ckks_context_id(_simp(0))); return;
        } else if (fn == "__builtin_ckks_keygen__") {
            last_value = _ptr2int(advcrypto_ckks_keygen(_sims(0))); return;
        } else if (fn == "__builtin_ckks_destroy_keypair__") {
            advcrypto_ckks_destroy_keypair(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_ckks_public_key__") {
            last_value = std::string(advcrypto_ckks_public_key(_simp(0))); return;
        } else if (fn == "__builtin_ckks_secret_key__") {
            last_value = std::string(advcrypto_ckks_secret_key(_simp(0))); return;
        } else if (fn == "__builtin_ckks_encrypt__") {
            last_value = _ptr2int(advcrypto_ckks_encrypt(_sims(0), _sims(1), _simd(2))); return;
        } else if (fn == "__builtin_ckks_destroy_ciphertext__") {
            advcrypto_ckks_destroy_ciphertext(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_ckks_ciphertext_hex__") {
            last_value = std::string(advcrypto_ckks_ciphertext_hex(_simp(0))); return;
        } else if (fn == "__builtin_ckks_decrypt__") {
            last_value = advcrypto_ckks_decrypt(_sims(0), _sims(1), _sims(2)); return;
        } else if (fn == "__builtin_ckks_add__") {
            last_value = _ptr2int(advcrypto_ckks_add(_sims(0), _sims(1), _sims(2))); return;
        } else if (fn == "__builtin_ckks_multiply__") {
            last_value = _ptr2int(advcrypto_ckks_multiply(_sims(0), _sims(1), _sims(2))); return;
        // ---- TFHE ----
        } else if (fn == "__builtin_tfhe_create_context__") {
            last_value = _ptr2int(advcrypto_tfhe_create_context(_simi(0))); return;
        } else if (fn == "__builtin_tfhe_destroy_context__") {
            advcrypto_tfhe_destroy_context(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_tfhe_context_id__") {
            last_value = std::string(advcrypto_tfhe_context_id(_simp(0))); return;
        } else if (fn == "__builtin_tfhe_keygen__") {
            last_value = _ptr2int(advcrypto_tfhe_keygen(_sims(0))); return;
        } else if (fn == "__builtin_tfhe_destroy_keypair__") {
            advcrypto_tfhe_destroy_keypair(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_tfhe_secret_key__") {
            last_value = std::string(advcrypto_tfhe_secret_key(_simp(0))); return;
        } else if (fn == "__builtin_tfhe_cloud_key__") {
            last_value = std::string(advcrypto_tfhe_cloud_key(_simp(0))); return;
        } else if (fn == "__builtin_tfhe_encrypt_bit__") {
            last_value = _ptr2int(advcrypto_tfhe_encrypt_bit(_sims(0), _sims(1), _simi(2))); return;
        } else if (fn == "__builtin_tfhe_destroy_ciphertext__") {
            advcrypto_tfhe_destroy_ciphertext(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_tfhe_ciphertext_hex__") {
            last_value = std::string(advcrypto_tfhe_ciphertext_hex(_simp(0))); return;
        } else if (fn == "__builtin_tfhe_decrypt_bit__") {
            last_value = advcrypto_tfhe_decrypt_bit(_sims(0), _sims(1), _sims(2)); return;
        } else if (fn == "__builtin_tfhe_gate_and__") {
            last_value = _ptr2int(advcrypto_tfhe_gate_and(_sims(0), _sims(1), _sims(2), _sims(3))); return;
        } else if (fn == "__builtin_tfhe_gate_or__") {
            last_value = _ptr2int(advcrypto_tfhe_gate_or(_sims(0), _sims(1), _sims(2), _sims(3))); return;
        } else if (fn == "__builtin_tfhe_gate_xor__") {
            last_value = _ptr2int(advcrypto_tfhe_gate_xor(_sims(0), _sims(1), _sims(2), _sims(3))); return;
        } else if (fn == "__builtin_tfhe_gate_not__") {
            last_value = _ptr2int(advcrypto_tfhe_gate_not(_sims(0), _sims(1), _sims(2))); return;
        // ---- Shamir ----
        } else if (fn == "__builtin_shamir_split__") {
            last_value = _ptr2int(advcrypto_shamir_split(_simll(0), _simi(1), _simi(2))); return;
        } else if (fn == "__builtin_shamir_destroy_shares__") {
            advcrypto_shamir_destroy_shares(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_shamir_share_count__") {
            last_value = advcrypto_shamir_share_count(_simp(0)); return;
        } else if (fn == "__builtin_shamir_share_x__") {
            last_value = advcrypto_shamir_share_x(_simp(0), _simi(1)); return;
        } else if (fn == "__builtin_shamir_share_y__") {
            last_value = std::string(advcrypto_shamir_share_y(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_shamir_reconstruct__") {
            last_value = (int)advcrypto_shamir_reconstruct(_simp(0), _simi(1)); return;
        // ---- SMPC ----
        } else if (fn == "__builtin_smpc_share__") {
            last_value = _ptr2int(advcrypto_smpc_share(_simll(0), _simi(1), _simi(2))); return;
        } else if (fn == "__builtin_smpc_destroy_shares__") {
            advcrypto_smpc_destroy_shares(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_smpc_share_count__") {
            last_value = advcrypto_smpc_share_count(_simp(0)); return;
        } else if (fn == "__builtin_smpc_share_hex__") {
            last_value = std::string(advcrypto_smpc_share_hex(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_smpc_reconstruct__") {
            last_value = (int)advcrypto_smpc_reconstruct(_simp(0), _simi(1)); return;
        // ---- Threshold ----
        } else if (fn == "__builtin_threshold_keygen__") {
            last_value = _ptr2int(advcrypto_threshold_keygen(_simi(0), _simi(1))); return;
        } else if (fn == "__builtin_threshold_destroy__") {
            advcrypto_threshold_destroy(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_threshold_public_key__") {
            last_value = std::string(advcrypto_threshold_public_key(_simp(0))); return;
        } else if (fn == "__builtin_threshold_share_count__") {
            last_value = advcrypto_threshold_share_count(_simp(0)); return;
        } else if (fn == "__builtin_threshold_share_hex__") {
            last_value = std::string(advcrypto_threshold_share_hex(_simp(0), _simi(1))); return;
        } else if (fn == "__builtin_threshold_partial_sign__") {
            last_value = std::string(advcrypto_threshold_partial_sign(_sims(0), _sims(1), _simi(2))); return;
        } else if (fn == "__builtin_threshold_combine__") {
            last_value = std::string(advcrypto_threshold_combine(_sims(0), _sims(1), _simi(2))); return;
        } else if (fn == "__builtin_threshold_verify__") {
            last_value = advcrypto_threshold_verify(_sims(0), _sims(1), _sims(2)); return;
        // ---- Oblivious Transfer ----
        } else if (fn == "__builtin_ot_sender_init__") {
            last_value = _ptr2int(advcrypto_ot_sender_init(_sims(0), _sims(1))); return;
        } else if (fn == "__builtin_ot_destroy_sender__") {
            advcrypto_ot_destroy_sender(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_ot_sender_state__") {
            last_value = std::string(advcrypto_ot_sender_state(_simp(0))); return;
        } else if (fn == "__builtin_ot_receiver_init__") {
            last_value = _ptr2int(advcrypto_ot_receiver_init(_simi(0))); return;
        } else if (fn == "__builtin_ot_destroy_receiver__") {
            advcrypto_ot_destroy_receiver(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_ot_receiver_query__") {
            last_value = std::string(advcrypto_ot_receiver_query(_simp(0))); return;
        } else if (fn == "__builtin_ot_sender_respond__") {
            last_value = _ptr2int(advcrypto_ot_sender_respond(_sims(0), _sims(1))); return;
        } else if (fn == "__builtin_ot_destroy_message__") {
            advcrypto_ot_destroy_message(_simp(0)); last_value = nullptr; return;
        } else if (fn == "__builtin_ot_message_hex__") {
            last_value = std::string(advcrypto_ot_message_hex(_simp(0))); return;
        } else if (fn == "__builtin_ot_receiver_extract__") {
            last_value = std::string(advcrypto_ot_receiver_extract(_sims(0), _sims(1), _simi(2))); return;
        }
    }

    // Milestone 16: Mathematical Computing
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_mathx_", 0) == 0) {
        std::string fn = std::get<std::string>(callee);
        auto _simp  = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _simi  = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _simll = [&](int i) -> long long { return std::holds_alternative<int>(arguments[i]) ? (long long)std::get<int>(arguments[i]) : (long long)std::get<double>(arguments[i]); };
        auto _simd  = [&](int i) -> double { return std::holds_alternative<double>(arguments[i]) ? std::get<double>(arguments[i]) : (double)std::get<int>(arguments[i]); };
        auto _sims  = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _ptr2int = [&](void* p) -> int { return (int)(intptr_t)p; };
        // Number Theory
        if (fn == "__builtin_mathx_sieve__") { last_value = _ptr2int(mathx_sieve(_simi(0))); return; }
        else if (fn == "__builtin_mathx_sieve_destroy__") { mathx_sieve_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_mathx_sieve_count__") { last_value = mathx_sieve_count(_simp(0)); return; }
        else if (fn == "__builtin_mathx_sieve_get__") { last_value = mathx_sieve_get(_simp(0), _simi(1)); return; }
        else if (fn == "__builtin_mathx_is_prime__") { last_value = mathx_is_prime(_simll(0)); return; }
        else if (fn == "__builtin_mathx_prime_factors__") { last_value = _ptr2int(mathx_prime_factors(_simll(0))); return; }
        else if (fn == "__builtin_mathx_vec_destroy__") { mathx_vec_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_mathx_vec_count__") { last_value = mathx_vec_count(_simp(0)); return; }
        else if (fn == "__builtin_mathx_vec_get__") { last_value = (int)mathx_vec_get(_simp(0), _simi(1)); return; }
        else if (fn == "__builtin_mathx_gcd__") { last_value = (int)mathx_gcd(_simll(0), _simll(1)); return; }
        else if (fn == "__builtin_mathx_lcm__") { last_value = (int)mathx_lcm(_simll(0), _simll(1)); return; }
        else if (fn == "__builtin_mathx_mod_pow__") { last_value = (int)mathx_mod_pow(_simll(0), _simll(1), _simll(2)); return; }
        else if (fn == "__builtin_mathx_mod_inverse__") { last_value = (int)mathx_mod_inverse(_simll(0), _simll(1)); return; }
        else if (fn == "__builtin_mathx_euler_totient__") { last_value = (int)mathx_euler_totient(_simll(0)); return; }
        else if (fn == "__builtin_mathx_is_perfect__") { last_value = mathx_is_perfect(_simll(0)); return; }
        else if (fn == "__builtin_mathx_is_abundant__") { last_value = mathx_is_abundant(_simll(0)); return; }
        else if (fn == "__builtin_mathx_divisors__") { last_value = _ptr2int(mathx_divisors(_simll(0))); return; }
        else if (fn == "__builtin_mathx_sum_divisors__") { last_value = (int)mathx_sum_divisors(_simll(0)); return; }
        else if (fn == "__builtin_mathx_nth_prime__") { last_value = (int)mathx_nth_prime(_simi(0)); return; }
        else if (fn == "__builtin_mathx_collatz_length__") { last_value = (int)mathx_collatz_length(_simll(0)); return; }
        else if (fn == "__builtin_mathx_digital_root__") { last_value = (int)mathx_digital_root(_simll(0)); return; }
        else if (fn == "__builtin_mathx_is_palindrome_num__") { last_value = mathx_is_palindrome_num(_simll(0)); return; }
        else if (fn == "__builtin_mathx_reverse_num__") { last_value = (int)mathx_reverse_num(_simll(0)); return; }
        else if (fn == "__builtin_mathx_sum_digits__") { last_value = (int)mathx_sum_digits(_simll(0)); return; }
        else if (fn == "__builtin_mathx_is_pandigital__") { last_value = mathx_is_pandigital(_simll(0), _simi(1)); return; }
        // Matrix
        else if (fn == "__builtin_mathx_mat_create__") { last_value = _ptr2int(mathx_mat_create(_simi(0), _simi(1))); return; }
        else if (fn == "__builtin_mathx_mat_identity__") { last_value = _ptr2int(mathx_mat_identity(_simi(0))); return; }
        else if (fn == "__builtin_mathx_mat_destroy__") { mathx_mat_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_mathx_mat_rows__") { last_value = mathx_mat_rows(_simp(0)); return; }
        else if (fn == "__builtin_mathx_mat_cols__") { last_value = mathx_mat_cols(_simp(0)); return; }
        else if (fn == "__builtin_mathx_mat_get__") { last_value = mathx_mat_get(_simp(0), _simi(1), _simi(2)); return; }
        else if (fn == "__builtin_mathx_mat_set__") { mathx_mat_set(_simp(0), _simi(1), _simi(2), _simd(3)); last_value = nullptr; return; }
        else if (fn == "__builtin_mathx_mat_add__") { last_value = _ptr2int(mathx_mat_add(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_mat_sub__") { last_value = _ptr2int(mathx_mat_sub(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_mat_mul__") { last_value = _ptr2int(mathx_mat_mul(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_mat_scale__") { last_value = _ptr2int(mathx_mat_scale(_simp(0), _simd(1))); return; }
        else if (fn == "__builtin_mathx_mat_transpose__") { last_value = _ptr2int(mathx_mat_transpose(_simp(0))); return; }
        else if (fn == "__builtin_mathx_mat_det__") { last_value = mathx_mat_det(_simp(0)); return; }
        else if (fn == "__builtin_mathx_mat_inverse__") { last_value = _ptr2int(mathx_mat_inverse(_simp(0))); return; }
        else if (fn == "__builtin_mathx_mat_solve__") { last_value = _ptr2int(mathx_mat_solve(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_mat_trace__") { last_value = mathx_mat_trace(_simp(0)); return; }
        else if (fn == "__builtin_mathx_mat_to_string__") { last_value = std::string(mathx_mat_to_string(_simp(0))); return; }
        else if (fn == "__builtin_mathx_mat_dominant_eigenvalue__") { last_value = mathx_mat_dominant_eigenvalue(_simp(0), _simi(1)); return; }
        else if (fn == "__builtin_mathx_mat_dominant_eigenvector__") { last_value = _ptr2int(mathx_mat_dominant_eigenvector(_simp(0), _simi(1))); return; }
        // Symbolic
        else if (fn == "__builtin_mathx_sym_num__") { last_value = _ptr2int(mathx_sym_num(_simd(0))); return; }
        else if (fn == "__builtin_mathx_sym_var__") { last_value = _ptr2int(mathx_sym_var(_sims(0))); return; }
        else if (fn == "__builtin_mathx_sym_add__") { last_value = _ptr2int(mathx_sym_add(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_sym_sub__") { last_value = _ptr2int(mathx_sym_sub(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_sym_mul__") { last_value = _ptr2int(mathx_sym_mul(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_sym_div__") { last_value = _ptr2int(mathx_sym_div(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_sym_pow__") { last_value = _ptr2int(mathx_sym_pow(_simp(0), _simp(1))); return; }
        else if (fn == "__builtin_mathx_sym_neg__") { last_value = _ptr2int(mathx_sym_neg(_simp(0))); return; }
        else if (fn == "__builtin_mathx_sym_sin__") { last_value = _ptr2int(mathx_sym_sin(_simp(0))); return; }
        else if (fn == "__builtin_mathx_sym_cos__") { last_value = _ptr2int(mathx_sym_cos(_simp(0))); return; }
        else if (fn == "__builtin_mathx_sym_exp__") { last_value = _ptr2int(mathx_sym_exp(_simp(0))); return; }
        else if (fn == "__builtin_mathx_sym_ln__") { last_value = _ptr2int(mathx_sym_ln(_simp(0))); return; }
        else if (fn == "__builtin_mathx_sym_simplify__") { last_value = _ptr2int(mathx_sym_simplify(_simp(0))); return; }
        else if (fn == "__builtin_mathx_sym_diff__") { last_value = _ptr2int(mathx_sym_diff(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_mathx_sym_eval__") { last_value = mathx_sym_eval(_simp(0), _sims(1), _simd(2)); return; }
        else if (fn == "__builtin_mathx_sym_to_string__") { last_value = std::string(mathx_sym_to_string(_simp(0))); return; }
        else if (fn == "__builtin_mathx_sym_destroy__") { mathx_sym_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_mathx_integrate_expr__") { last_value = mathx_integrate_expr(_simp(0), _sims(1), _simd(2), _simd(3), _simi(4)); return; }
        else if (fn == "__builtin_mathx_find_root__") { last_value = mathx_find_root(_simp(0), _sims(1), _simd(2), _simi(3)); return; }
        // Statistics
        else if (fn == "__builtin_mathx_dvec_create__") { last_value = _ptr2int(mathx_dvec_create()); return; }
        else if (fn == "__builtin_mathx_dvec_push__") { mathx_dvec_push(_simp(0), _simd(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_mathx_dvec_destroy__") { mathx_dvec_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_mathx_dvec_count__") { last_value = mathx_dvec_count(_simp(0)); return; }
        else if (fn == "__builtin_mathx_dvec_get__") { last_value = mathx_dvec_get(_simp(0), _simi(1)); return; }
        else if (fn == "__builtin_mathx_stat_mean__") { last_value = mathx_stat_mean(_simp(0)); return; }
        else if (fn == "__builtin_mathx_stat_variance__") { last_value = mathx_stat_variance(_simp(0)); return; }
        else if (fn == "__builtin_mathx_stat_stddev__") { last_value = mathx_stat_stddev(_simp(0)); return; }
        else if (fn == "__builtin_mathx_stat_median__") { last_value = mathx_stat_median(_simp(0)); return; }
        else if (fn == "__builtin_mathx_stat_correlation__") { last_value = mathx_stat_correlation(_simp(0), _simp(1)); return; }
        else if (fn == "__builtin_mathx_stat_linear_regression__") { last_value = _ptr2int(mathx_stat_linear_regression(_simp(0), _simp(1))); return; }
    }

    // Milestone 17: Polish & Optimization
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_polish_", 0) == 0) {
        std::string fn = std::get<std::string>(callee);
        auto _simp    = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _simi    = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _simll   = [&](int i) -> long long { return std::holds_alternative<int>(arguments[i]) ? (long long)std::get<int>(arguments[i]) : (long long)std::get<double>(arguments[i]); };
        auto _simd    = [&](int i) -> double { return std::holds_alternative<double>(arguments[i]) ? std::get<double>(arguments[i]) : (double)std::get<int>(arguments[i]); };
        auto _sims    = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _ptr2int = [&](void* p) -> int { return (int)(intptr_t)p; };
        // Profiler
        if (fn == "__builtin_polish_profiler_reset__") { polish_profiler_reset(); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_profiler_start__") { polish_profiler_start(_sims(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_profiler_stop__") { polish_profiler_stop(_sims(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_profiler_entry_count__") { last_value = polish_profiler_entry_count(); return; }
        else if (fn == "__builtin_polish_profiler_entry_name__") { last_value = std::string(polish_profiler_entry_name(_simi(0))); return; }
        else if (fn == "__builtin_polish_profiler_entry_calls__") { last_value = (int)polish_profiler_entry_calls(_simi(0)); return; }
        else if (fn == "__builtin_polish_profiler_entry_total_ms__") { last_value = polish_profiler_entry_total_ms(_simi(0)); return; }
        else if (fn == "__builtin_polish_profiler_entry_mean_ms__") { last_value = polish_profiler_entry_mean_ms(_simi(0)); return; }
        else if (fn == "__builtin_polish_profiler_entry_min_ms__") { last_value = polish_profiler_entry_min_ms(_simi(0)); return; }
        else if (fn == "__builtin_polish_profiler_entry_max_ms__") { last_value = polish_profiler_entry_max_ms(_simi(0)); return; }
        else if (fn == "__builtin_polish_profiler_report__") { last_value = std::string(polish_profiler_report()); return; }
        // Benchmark
        else if (fn == "__builtin_polish_bench_create__") { last_value = _ptr2int(polish_bench_create(_sims(0), _simi(1))); return; }
        else if (fn == "__builtin_polish_bench_destroy__") { polish_bench_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_bench_iter_start__") { polish_bench_iter_start(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_bench_iter_stop__") { polish_bench_iter_stop(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_bench_iterations__") { last_value = polish_bench_iterations(_simp(0)); return; }
        else if (fn == "__builtin_polish_bench_mean_ms__") { last_value = polish_bench_mean_ms(_simp(0)); return; }
        else if (fn == "__builtin_polish_bench_min_ms__") { last_value = polish_bench_min_ms(_simp(0)); return; }
        else if (fn == "__builtin_polish_bench_max_ms__") { last_value = polish_bench_max_ms(_simp(0)); return; }
        else if (fn == "__builtin_polish_bench_ops_per_sec__") { last_value = polish_bench_ops_per_sec(_simp(0)); return; }
        else if (fn == "__builtin_polish_bench_report__") { last_value = std::string(polish_bench_report(_simp(0))); return; }
        // Logger
        else if (fn == "__builtin_polish_logger_set_level__") { polish_logger_set_level(_simi(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_logger_get_level__") { last_value = polish_logger_get_level(); return; }
        else if (fn == "__builtin_polish_logger_set_file__") { polish_logger_set_file(_sims(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_logger_close_file__") { polish_logger_close_file(); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_logger_log__") { polish_logger_log(_simi(0), _sims(1), _sims(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_logger_enable_timestamps__") { polish_logger_enable_timestamps(_simi(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_logger_enable_colors__") { polish_logger_enable_colors(_simi(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_logger_message_count__") { last_value = polish_logger_message_count(); return; }
        else if (fn == "__builtin_polish_logger_get_message__") { last_value = std::string(polish_logger_get_message(_simi(0))); return; }
        else if (fn == "__builtin_polish_logger_clear__") { polish_logger_clear(); last_value = nullptr; return; }
        // Stack trace
        else if (fn == "__builtin_polish_stacktrace_capture__") { last_value = _ptr2int(polish_stacktrace_capture()); return; }
        else if (fn == "__builtin_polish_stacktrace_destroy__") { polish_stacktrace_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_stacktrace_depth__") { last_value = polish_stacktrace_depth(_simp(0)); return; }
        else if (fn == "__builtin_polish_stacktrace_frame_function__") { last_value = std::string(polish_stacktrace_frame_function(_simp(0), _simi(1))); return; }
        else if (fn == "__builtin_polish_stacktrace_frame_file__") { last_value = std::string(polish_stacktrace_frame_file(_simp(0), _simi(1))); return; }
        else if (fn == "__builtin_polish_stacktrace_frame_line__") { last_value = polish_stacktrace_frame_line(_simp(0), _simi(1)); return; }
        else if (fn == "__builtin_polish_stacktrace_to_string__") { last_value = std::string(polish_stacktrace_to_string(_simp(0))); return; }
        // Memory tracker
        else if (fn == "__builtin_polish_memtrack_reset__") { polish_memtrack_reset(); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_memtrack_alloc__") { polish_memtrack_alloc(_sims(0), _simll(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_memtrack_free__") { polish_memtrack_free(_sims(0), _simll(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_polish_memtrack_current_bytes__") { last_value = (int)polish_memtrack_current_bytes(); return; }
        else if (fn == "__builtin_polish_memtrack_peak_bytes__") { last_value = (int)polish_memtrack_peak_bytes(); return; }
        else if (fn == "__builtin_polish_memtrack_alloc_count__") { last_value = polish_memtrack_alloc_count(); return; }
        else if (fn == "__builtin_polish_memtrack_report__") { last_value = std::string(polish_memtrack_report()); return; }
    }

    // Milestone 18: Community & Ecosystem
    if (std::holds_alternative<std::string>(callee) && std::get<std::string>(callee).rfind("__builtin_eco_", 0) == 0) {
        std::string fn = std::get<std::string>(callee);
        auto _simp    = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _simi    = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _sims    = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _ptr2int = [&](void* p) -> int { return (int)(intptr_t)p; };
        // Semver
        if      (fn == "__builtin_eco_semver_parse__")        { last_value = _ptr2int(eco_semver_parse(_sims(0))); return; }
        else if (fn == "__builtin_eco_semver_destroy__")      { eco_semver_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_semver_major__")        { last_value = eco_semver_major(_simp(0)); return; }
        else if (fn == "__builtin_eco_semver_minor__")        { last_value = eco_semver_minor(_simp(0)); return; }
        else if (fn == "__builtin_eco_semver_patch__")        { last_value = eco_semver_patch(_simp(0)); return; }
        else if (fn == "__builtin_eco_semver_pre__")          { last_value = std::string(eco_semver_pre(_simp(0))); return; }
        else if (fn == "__builtin_eco_semver_to_string__")    { last_value = std::string(eco_semver_to_string(_simp(0))); return; }
        else if (fn == "__builtin_eco_semver_compare__")      { last_value = eco_semver_compare(_simp(0), _simp(1)); return; }
        else if (fn == "__builtin_eco_semver_satisfies__")    { last_value = eco_semver_satisfies(_simp(0), _sims(1)); return; }
        else if (fn == "__builtin_eco_semver_bump_major__")   { last_value = _ptr2int(eco_semver_bump_major(_simp(0))); return; }
        else if (fn == "__builtin_eco_semver_bump_minor__")   { last_value = _ptr2int(eco_semver_bump_minor(_simp(0))); return; }
        else if (fn == "__builtin_eco_semver_bump_patch__")   { last_value = _ptr2int(eco_semver_bump_patch(_simp(0))); return; }
        else if (fn == "__builtin_eco_semver_is_valid__")     { last_value = eco_semver_is_valid(_sims(0)); return; }
        else if (fn == "__builtin_eco_semver_is_prerelease__"){ last_value = eco_semver_is_prerelease(_simp(0)); return; }
        // Manifest
        else if (fn == "__builtin_eco_manifest_create__")     { last_value = _ptr2int(eco_manifest_create(_sims(0), _sims(1))); return; }
        else if (fn == "__builtin_eco_manifest_destroy__")    { eco_manifest_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_manifest_set_description__") { eco_manifest_set_description(_simp(0), _sims(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_manifest_set_author__") { eco_manifest_set_author(_simp(0), _sims(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_manifest_set_license__"){ eco_manifest_set_license(_simp(0), _sims(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_manifest_add_dep__")    { eco_manifest_add_dep(_simp(0), _sims(1), _sims(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_manifest_add_keyword__"){ eco_manifest_add_keyword(_simp(0), _sims(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_manifest_name__")       { last_value = std::string(eco_manifest_name(_simp(0))); return; }
        else if (fn == "__builtin_eco_manifest_version__")    { last_value = std::string(eco_manifest_version(_simp(0))); return; }
        else if (fn == "__builtin_eco_manifest_description__"){ last_value = std::string(eco_manifest_description(_simp(0))); return; }
        else if (fn == "__builtin_eco_manifest_dep_count__")  { last_value = eco_manifest_dep_count(_simp(0)); return; }
        else if (fn == "__builtin_eco_manifest_dep_name__")   { last_value = std::string(eco_manifest_dep_name(_simp(0), _simi(1))); return; }
        else if (fn == "__builtin_eco_manifest_dep_range__")  { last_value = std::string(eco_manifest_dep_range(_simp(0), _simi(1))); return; }
        else if (fn == "__builtin_eco_manifest_to_toml__")    { last_value = std::string(eco_manifest_to_toml(_simp(0))); return; }
        // Registry
        else if (fn == "__builtin_eco_registry_reset__")      { eco_registry_reset(); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_registry_publish__")    { last_value = _ptr2int(eco_registry_publish(_sims(0), _sims(1), _sims(2), _sims(3), _sims(4), _simi(5))); return; }
        else if (fn == "__builtin_eco_registry_pkg_destroy__"){ eco_registry_pkg_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_registry_exists__")     { last_value = eco_registry_exists(_sims(0)); return; }
        else if (fn == "__builtin_eco_registry_get__")        { last_value = _ptr2int(eco_registry_get(_sims(0))); return; }
        else if (fn == "__builtin_eco_registry_pkg_name__")   { last_value = std::string(eco_registry_pkg_name(_simp(0))); return; }
        else if (fn == "__builtin_eco_registry_pkg_latest__") { last_value = std::string(eco_registry_pkg_latest(_simp(0))); return; }
        else if (fn == "__builtin_eco_registry_pkg_description__") { last_value = std::string(eco_registry_pkg_description(_simp(0))); return; }
        else if (fn == "__builtin_eco_registry_pkg_author__") { last_value = std::string(eco_registry_pkg_author(_simp(0))); return; }
        else if (fn == "__builtin_eco_registry_pkg_downloads__") { last_value = eco_registry_pkg_downloads(_simp(0)); return; }
        else if (fn == "__builtin_eco_registry_package_count__") { last_value = eco_registry_package_count(); return; }
        else if (fn == "__builtin_eco_registry_search__")     { last_value = _ptr2int(eco_registry_search(_sims(0))); return; }
        else if (fn == "__builtin_eco_registry_list_all__")   { last_value = _ptr2int(eco_registry_list_all()); return; }
        // StrVec
        else if (fn == "__builtin_eco_strvec_destroy__")      { eco_strvec_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_strvec_count__")        { last_value = eco_strvec_count(_simp(0)); return; }
        else if (fn == "__builtin_eco_strvec_get__")          { last_value = std::string(eco_strvec_get(_simp(0), _simi(1))); return; }
        // Package manager
        else if (fn == "__builtin_eco_pkgmgr_reset__")        { eco_pkgmgr_reset(); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_pkgmgr_install__")      { last_value = eco_pkgmgr_install(_sims(0), _sims(1)); return; }
        else if (fn == "__builtin_eco_pkgmgr_install_message__") { last_value = std::string(eco_pkgmgr_install_message()); return; }
        else if (fn == "__builtin_eco_pkgmgr_install_manifest__") { last_value = eco_pkgmgr_install_manifest(_simp(0)); return; }
        else if (fn == "__builtin_eco_pkgmgr_is_installed__") { last_value = eco_pkgmgr_is_installed(_sims(0)); return; }
        else if (fn == "__builtin_eco_pkgmgr_installed_version__") { last_value = std::string(eco_pkgmgr_installed_version(_sims(0))); return; }
        else if (fn == "__builtin_eco_pkgmgr_list_installed__") { last_value = _ptr2int(eco_pkgmgr_list_installed()); return; }
        else if (fn == "__builtin_eco_pkgmgr_uninstall__")    { last_value = eco_pkgmgr_uninstall(_sims(0)); return; }
        else if (fn == "__builtin_eco_pkgmgr_lockfile__")     { last_value = std::string(eco_pkgmgr_lockfile(_simp(0))); return; }
        // Template engine
        else if (fn == "__builtin_eco_tmpl_vars_create__")    { last_value = _ptr2int(eco_tmpl_vars_create()); return; }
        else if (fn == "__builtin_eco_tmpl_vars_destroy__")   { eco_tmpl_vars_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_tmpl_vars_set__")       { eco_tmpl_vars_set(_simp(0), _sims(1), _sims(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_eco_tmpl_render__")         { last_value = std::string(eco_tmpl_render(_sims(0), _simp(1))); return; }
        // Formatter
        else if (fn == "__builtin_eco_fmt_indent__")          { last_value = std::string(eco_fmt_indent(_sims(0), _simi(1))); return; }
        else if (fn == "__builtin_eco_fmt_trim__")            { last_value = std::string(eco_fmt_trim(_sims(0))); return; }
        else if (fn == "__builtin_eco_fmt_normalize__")       { last_value = std::string(eco_fmt_normalize(_sims(0))); return; }
        else if (fn == "__builtin_eco_fmt_count_lines__")     { last_value = eco_fmt_count_lines(_sims(0)); return; }
        else if (fn == "__builtin_eco_fmt_count_chars__")     { last_value = eco_fmt_count_chars(_sims(0)); return; }
    }

    // Milestone 19: Database & Storage
    if (std::holds_alternative<std::string>(callee) && (
        std::get<std::string>(callee).rfind("__builtin_db_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_kv_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_doc_", 0) == 0 ||
        std::get<std::string>(callee).rfind("__builtin_orm_", 0) == 0)) {
        std::string fn = std::get<std::string>(callee);
        auto _simp    = [&](int i) { return reinterpret_cast<void*>(static_cast<intptr_t>(std::get<int>(arguments[i]))); };
        auto _simi    = [&](int i) -> int { return std::get<int>(arguments[i]); };
        auto _simll   = [&](int i) -> long long { return std::holds_alternative<int>(arguments[i]) ? (long long)std::get<int>(arguments[i]) : (long long)std::get<double>(arguments[i]); };
        auto _sims    = [&](int i) -> const char* { return std::get<std::string>(arguments[i]).c_str(); };
        auto _ptr2int = [&](void* p) -> int { return (int)(intptr_t)p; };
        // DB core
        if      (fn == "__builtin_db_open__")               { last_value = _ptr2int(db_open_c(_sims(0))); return; }
        else if (fn == "__builtin_db_close__")              { db_close_c(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_exec__")               { last_value = _ptr2int(db_exec_c(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_db_result_destroy__")     { db_result_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_result_ok__")          { last_value = db_result_ok(_simp(0)); return; }
        else if (fn == "__builtin_db_result_error__")       { last_value = std::string(db_result_error(_simp(0))); return; }
        else if (fn == "__builtin_db_result_row_count__")   { last_value = db_result_row_count(_simp(0)); return; }
        else if (fn == "__builtin_db_result_col_count__")   { last_value = db_result_col_count(_simp(0)); return; }
        else if (fn == "__builtin_db_result_col_name__")    { last_value = std::string(db_result_col_name(_simp(0), _simi(1))); return; }
        else if (fn == "__builtin_db_result_get__")         { last_value = std::string(db_result_get(_simp(0), _simi(1), _simi(2))); return; }
        else if (fn == "__builtin_db_result_rows_affected__") { last_value = (int)db_result_rows_affected(_simp(0)); return; }
        else if (fn == "__builtin_db_begin__")              { last_value = db_begin(_simp(0)); return; }
        else if (fn == "__builtin_db_commit__")             { last_value = db_commit(_simp(0)); return; }
        else if (fn == "__builtin_db_rollback__")           { last_value = db_rollback(_simp(0)); return; }
        // Query builder
        else if (fn == "__builtin_db_qb_select__")          { last_value = _ptr2int(db_qb_select(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_db_qb_insert__")          { last_value = _ptr2int(db_qb_insert(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_db_qb_update__")          { last_value = _ptr2int(db_qb_update(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_db_qb_delete__")          { last_value = _ptr2int(db_qb_delete(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_db_qb_create_table__")    { last_value = _ptr2int(db_qb_create_table(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_db_qb_destroy__")         { db_qb_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_cols__")            { db_qb_cols(_simp(0), _sims(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_where__")           { db_qb_where(_simp(0), _sims(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_order__")           { db_qb_order(_simp(0), _sims(1), _simi(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_limit__")           { db_qb_limit(_simp(0), _simi(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_offset__")          { db_qb_offset(_simp(0), _simi(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_set__")             { db_qb_set(_simp(0), _sims(1), _sims(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_value__")           { db_qb_value(_simp(0), _sims(1)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_add_col__")         { db_qb_add_col(_simp(0), _sims(1), _sims(2), _simi(3), _simi(4)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_qb_exec__")            { last_value = _ptr2int(db_qb_exec(_simp(0))); return; }
        else if (fn == "__builtin_db_qb_sql__")             { last_value = std::string(db_qb_sql(_simp(0))); return; }
        // KV Store
        else if (fn == "__builtin_kv_open__")               { last_value = _ptr2int(kv_open_c()); return; }
        else if (fn == "__builtin_kv_close__")              { kv_close_c(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_kv_set__")                { kv_set_c(_simp(0), _sims(1), _sims(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_kv_setex__")              { kv_setex_c(_simp(0), _sims(1), _sims(2), _simll(3)); last_value = nullptr; return; }
        else if (fn == "__builtin_kv_get__")                { last_value = std::string(kv_get_c(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_kv_exists__")             { last_value = kv_exists_c(_simp(0), _sims(1)); return; }
        else if (fn == "__builtin_kv_del__")                { last_value = kv_del_c(_simp(0), _sims(1)); return; }
        else if (fn == "__builtin_kv_incr__")               { last_value = (int)kv_incr_c(_simp(0), _sims(1)); return; }
        else if (fn == "__builtin_kv_decr__")               { last_value = (int)kv_decr_c(_simp(0), _sims(1)); return; }
        else if (fn == "__builtin_kv_keys__")               { last_value = _ptr2int(kv_keys_c(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_kv_flush__")              { kv_flush_c(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_kv_count__")              { last_value = kv_count_c(_simp(0)); return; }
        else if (fn == "__builtin_kv_lpush__")              { kv_lpush_c(_simp(0), _sims(1), _sims(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_kv_rpush__")              { kv_rpush_c(_simp(0), _sims(1), _sims(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_kv_lpop__")               { last_value = std::string(kv_lpop_c(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_kv_rpop__")               { last_value = std::string(kv_rpop_c(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_kv_llen__")               { last_value = kv_llen_c(_simp(0), _sims(1)); return; }
        else if (fn == "__builtin_kv_lindex__")             { last_value = std::string(kv_lindex_c(_simp(0), _sims(1), _simi(2))); return; }
        else if (fn == "__builtin_kv_hset__")               { kv_hset_c(_simp(0), _sims(1), _sims(2), _sims(3)); last_value = nullptr; return; }
        else if (fn == "__builtin_kv_hget__")               { last_value = std::string(kv_hget_c(_simp(0), _sims(1), _sims(2))); return; }
        else if (fn == "__builtin_kv_hexists__")            { last_value = kv_hexists_c(_simp(0), _sims(1), _sims(2)); return; }
        else if (fn == "__builtin_kv_hlen__")               { last_value = kv_hlen_c(_simp(0), _sims(1)); return; }
        else if (fn == "__builtin_kv_hkeys__")              { last_value = _ptr2int(kv_hkeys_c(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_db_strvec_destroy__")     { db_strvec_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_db_strvec_count__")       { last_value = db_strvec_count(_simp(0)); return; }
        else if (fn == "__builtin_db_strvec_get__")         { last_value = std::string(db_strvec_get(_simp(0), _simi(1))); return; }
        // Document store
        else if (fn == "__builtin_doc_open__")              { last_value = _ptr2int(doc_open_c()); return; }
        else if (fn == "__builtin_doc_close__")             { doc_close_c(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_doc_map_create__")        { last_value = _ptr2int(doc_map_create()); return; }
        else if (fn == "__builtin_doc_map_destroy__")       { doc_map_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_doc_map_set__")           { doc_map_set(_simp(0), _sims(1), _sims(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_doc_map_get__")           { last_value = std::string(doc_map_get(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_doc_map_keys__")          { last_value = _ptr2int(doc_map_keys(_simp(0))); return; }
        else if (fn == "__builtin_doc_insert__")            { last_value = std::string(doc_insert_c(_simp(0), _sims(1), _simp(2))); return; }
        else if (fn == "__builtin_doc_find__")              { last_value = _ptr2int(doc_find_c(_simp(0), _sims(1), _sims(2), _sims(3))); return; }
        else if (fn == "__builtin_doc_find_all__")          { last_value = _ptr2int(doc_find_all_c(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_doc_update__")            { last_value = doc_update_c(_simp(0), _sims(1), _sims(2), _sims(3), _sims(4)); return; }
        else if (fn == "__builtin_doc_delete__")            { last_value = doc_delete_c(_simp(0), _sims(1), _sims(2)); return; }
        else if (fn == "__builtin_doc_count__")             { last_value = doc_count_c(_simp(0), _sims(1)); return; }
        else if (fn == "__builtin_doc_list_destroy__")      { doc_list_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_doc_list_count__")        { last_value = doc_list_count(_simp(0)); return; }
        else if (fn == "__builtin_doc_list_get_map__")      { last_value = _ptr2int(doc_list_get_map(_simp(0), _simi(1))); return; }
        // ORM
        else if (fn == "__builtin_orm_create__")            { last_value = _ptr2int(orm_create_c(_simp(0))); return; }
        else if (fn == "__builtin_orm_destroy__")           { orm_destroy_c(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_orm_fields_create__")     { last_value = _ptr2int(orm_fields_create()); return; }
        else if (fn == "__builtin_orm_fields_destroy__")    { orm_fields_destroy(_simp(0)); last_value = nullptr; return; }
        else if (fn == "__builtin_orm_fields_add__")        { orm_fields_add(_simp(0), _sims(1), _sims(2), _simi(3), _simi(4)); last_value = nullptr; return; }
        else if (fn == "__builtin_orm_register__")          { orm_register_c(_simp(0), _sims(1), _simp(2)); last_value = nullptr; return; }
        else if (fn == "__builtin_orm_migrate__")           { last_value = orm_migrate_c(_simp(0)); return; }
        else if (fn == "__builtin_orm_find_all__")          { last_value = _ptr2int(orm_find_all_c(_simp(0), _sims(1))); return; }
        else if (fn == "__builtin_orm_find_by__")           { last_value = _ptr2int(orm_find_by_c(_simp(0), _sims(1), _sims(2), _sims(3))); return; }
        else if (fn == "__builtin_orm_insert__")            { last_value = _ptr2int(orm_insert_c(_simp(0), _sims(1), _simp(2))); return; }
        else if (fn == "__builtin_orm_update__")            { last_value = _ptr2int(orm_update_c(_simp(0), _sims(1), _sims(2), _sims(3), _simp(4))); return; }
        else if (fn == "__builtin_orm_delete__")            { last_value = _ptr2int(orm_delete_c(_simp(0), _sims(1), _sims(2), _sims(3))); return; }
    }

    if (std::holds_alternative<std::shared_ptr<WaitGroupMethod>>(callee)) {
        auto wgm = std::get<std::shared_ptr<WaitGroupMethod>>(callee);
        
        if (wgm->method_name == "add") {
            if (arguments.size() != 1) {
                throw TypeError("WaitGroup.add() requires 1 argument");
            }
            int delta = std::get<int>(arguments[0]);
            wgm->waitgroup->add(delta);
            last_value = nullptr;
            return;
        } else if (wgm->method_name == "done") {
            if (!arguments.empty()) {
                throw TypeError("WaitGroup.done() takes no arguments");
            }
            wgm->waitgroup->done();
            last_value = nullptr;
            return;
        } else if (wgm->method_name == "wait") {
            if (!arguments.empty()) {
                throw TypeError("WaitGroup.wait() takes no arguments");
            }
            wgm->waitgroup->wait();
            last_value = nullptr;
            return;
        } else if (wgm->method_name == "count") {
            if (!arguments.empty()) {
                throw TypeError("WaitGroup.count() takes no arguments");
            }
            last_value = wgm->waitgroup->count();
            return;
        }
        
        throw RuntimeError("Unknown WaitGroup method: " + wgm->method_name);
    }
    
    // Handle Option methods
    if (std::holds_alternative<std::shared_ptr<OptionMethod>>(callee)) {
        auto om = std::get<std::shared_ptr<OptionMethod>>(callee);
        
        if (om->method_name == "is_some") {
            if (!arguments.empty()) {
                throw TypeError("Option.is_some() takes no arguments");
            }
            last_value = om->option->isSome();
            return;
        } else if (om->method_name == "is_none") {
            if (!arguments.empty()) {
                throw TypeError("Option.is_none() takes no arguments");
            }
            last_value = om->option->isNone();
            return;
        } else if (om->method_name == "unwrap") {
            if (!arguments.empty()) {
                throw TypeError("Option.unwrap() takes no arguments");
            }
            last_value = om->option->unwrap();
            return;
        } else if (om->method_name == "unwrap_or") {
            if (arguments.size() != 1) {
                throw TypeError("Option.unwrap_or() requires 1 argument");
            }
            last_value = om->option->unwrapOr(arguments[0]);
            return;
        } else if (om->method_name == "map") {
            if (arguments.size() != 1) {
                throw TypeError("Option.map() requires 1 argument (function)");
            }
            
            if (om->option->isNone()) {
                // None.map(f) = None
                last_value = om->option;
                return;
            }
            
            // Some(x).map(f) = Some(f(x))
            if (!std::holds_alternative<std::shared_ptr<Function>>(arguments[0])) {
                throw TypeError("Option.map() requires a function argument");
            }
            
            auto func = std::get<std::shared_ptr<Function>>(arguments[0]);
            std::vector<Value> func_args = {om->option->value};
            
            // Call the function with the wrapped value
            if (func_args.size() != func->parameters.size()) {
                throw TypeError("Function expects " + std::to_string(func->parameters.size()) + 
                                " arguments but got " + std::to_string(func_args.size()));
            }
            
            auto func_env = std::make_shared<Environment>(func->closure);
            for (size_t i = 0; i < func->parameters.size(); i++) {
                func_env->define(func->parameters[i].name, func_args[i]);
            }
            
            auto previous_env = environment;
            environment = func_env;
            
            Value result = nullptr;
            try {
                for (auto& stmt : *func->getBody()) {
                    stmt->accept(*this);
                }
            } catch (const ReturnException& ret) {
                result = ret.value;
            }
            
            environment = previous_env;
            
            // Wrap result in Some
            auto new_option = std::make_shared<OptionValue>(result);
            last_value = new_option;
            return;
        } else if (om->method_name == "and_then") {
            if (arguments.size() != 1) {
                throw TypeError("Option.and_then() requires 1 argument (function)");
            }
            
            if (om->option->isNone()) {
                // None.and_then(f) = None
                last_value = om->option;
                return;
            }
            
            // Some(x).and_then(f) = f(x) (f must return Option)
            if (!std::holds_alternative<std::shared_ptr<Function>>(arguments[0])) {
                throw TypeError("Option.and_then() requires a function argument");
            }
            
            auto func = std::get<std::shared_ptr<Function>>(arguments[0]);
            std::vector<Value> func_args = {om->option->value};
            
            if (func_args.size() != func->parameters.size()) {
                throw TypeError("Function expects " + std::to_string(func->parameters.size()) + 
                                " arguments but got " + std::to_string(func_args.size()));
            }
            
            auto func_env = std::make_shared<Environment>(func->closure);
            for (size_t i = 0; i < func->parameters.size(); i++) {
                func_env->define(func->parameters[i].name, func_args[i]);
            }
            
            auto previous_env = environment;
            environment = func_env;
            
            Value result = nullptr;
            try {
                for (auto& stmt : *func->getBody()) {
                    stmt->accept(*this);
                }
            } catch (const ReturnException& ret) {
                result = ret.value;
            }
            
            environment = previous_env;
            
            // Result should already be an Option
            last_value = result;
            return;
        }
        
        throw RuntimeError("Unknown Option method: " + om->method_name);
    }
    
    // Handle Result methods
    if (std::holds_alternative<std::shared_ptr<ResultMethod>>(callee)) {
        auto rm = std::get<std::shared_ptr<ResultMethod>>(callee);
        
        if (rm->method_name == "is_ok") {
            if (!arguments.empty()) {
                throw TypeError("Result.is_ok() takes no arguments");
            }
            last_value = rm->result->isOk();
            return;
        } else if (rm->method_name == "is_err") {
            if (!arguments.empty()) {
                throw TypeError("Result.is_err() takes no arguments");
            }
            last_value = rm->result->isErr();
            return;
        } else if (rm->method_name == "unwrap") {
            if (!arguments.empty()) {
                throw TypeError("Result.unwrap() takes no arguments");
            }
            last_value = rm->result->unwrap();
            return;
        } else if (rm->method_name == "unwrap_or") {
            if (arguments.size() != 1) {
                throw TypeError("Result.unwrap_or() requires 1 argument");
            }
            last_value = rm->result->unwrapOr(arguments[0]);
            return;
        } else if (rm->method_name == "unwrap_err") {
            if (!arguments.empty()) {
                throw TypeError("Result.unwrap_err() takes no arguments");
            }
            last_value = rm->result->unwrapErr();
            return;
        } else if (rm->method_name == "map") {
            if (arguments.size() != 1) {
                throw TypeError("Result.map() requires 1 argument (function)");
            }
            
            if (rm->result->isErr()) {
                // Err.map(f) = Err (unchanged)
                last_value = rm->result;
                return;
            }
            
            // Ok(x).map(f) = Ok(f(x))
            if (!std::holds_alternative<std::shared_ptr<Function>>(arguments[0])) {
                throw TypeError("Result.map() requires a function argument");
            }
            
            auto func = std::get<std::shared_ptr<Function>>(arguments[0]);
            std::vector<Value> func_args = {rm->result->value};
            
            if (func_args.size() != func->parameters.size()) {
                throw TypeError("Function expects " + std::to_string(func->parameters.size()) + 
                                " arguments but got " + std::to_string(func_args.size()));
            }
            
            auto func_env = std::make_shared<Environment>(func->closure);
            for (size_t i = 0; i < func->parameters.size(); i++) {
                func_env->define(func->parameters[i].name, func_args[i]);
            }
            
            auto previous_env = environment;
            environment = func_env;
            
            Value result = nullptr;
            try {
                for (auto& stmt : *func->getBody()) {
                    stmt->accept(*this);
                }
            } catch (const ReturnException& ret) {
                result = ret.value;
            }
            
            environment = previous_env;
            
            // Wrap result in Ok
            auto new_result = std::make_shared<ResultValue>(true, result);
            last_value = new_result;
            return;
        } else if (rm->method_name == "map_err") {
            if (arguments.size() != 1) {
                throw TypeError("Result.map_err() requires 1 argument (function)");
            }
            
            if (rm->result->isOk()) {
                // Ok.map_err(f) = Ok (unchanged)
                last_value = rm->result;
                return;
            }
            
            // Err(e).map_err(f) = Err(f(e))
            if (!std::holds_alternative<std::shared_ptr<Function>>(arguments[0])) {
                throw TypeError("Result.map_err() requires a function argument");
            }
            
            auto func = std::get<std::shared_ptr<Function>>(arguments[0]);
            std::vector<Value> func_args = {rm->result->value};
            
            if (func_args.size() != func->parameters.size()) {
                throw TypeError("Function expects " + std::to_string(func->parameters.size()) + 
                                " arguments but got " + std::to_string(func_args.size()));
            }
            
            auto func_env = std::make_shared<Environment>(func->closure);
            for (size_t i = 0; i < func->parameters.size(); i++) {
                func_env->define(func->parameters[i].name, func_args[i]);
            }
            
            auto previous_env = environment;
            environment = func_env;
            
            Value result = nullptr;
            try {
                for (auto& stmt : *func->getBody()) {
                    stmt->accept(*this);
                }
            } catch (const ReturnException& ret) {
                result = ret.value;
            }
            
            environment = previous_env;
            
            // Wrap result in Err
            auto new_result = std::make_shared<ResultValue>(false, result);
            last_value = new_result;
            return;
        } else if (rm->method_name == "and_then") {
            if (arguments.size() != 1) {
                throw TypeError("Result.and_then() requires 1 argument (function)");
            }
            
            if (rm->result->isErr()) {
                // Err.and_then(f) = Err (unchanged)
                last_value = rm->result;
                return;
            }
            
            // Ok(x).and_then(f) = f(x) (f must return Result)
            if (!std::holds_alternative<std::shared_ptr<Function>>(arguments[0])) {
                throw TypeError("Result.and_then() requires a function argument");
            }
            
            auto func = std::get<std::shared_ptr<Function>>(arguments[0]);
            std::vector<Value> func_args = {rm->result->value};
            
            if (func_args.size() != func->parameters.size()) {
                throw TypeError("Function expects " + std::to_string(func->parameters.size()) + 
                                " arguments but got " + std::to_string(func_args.size()));
            }
            
            auto func_env = std::make_shared<Environment>(func->closure);
            for (size_t i = 0; i < func->parameters.size(); i++) {
                func_env->define(func->parameters[i].name, func_args[i]);
            }
            
            auto previous_env = environment;
            environment = func_env;
            
            Value result = nullptr;
            try {
                for (auto& stmt : *func->getBody()) {
                    stmt->accept(*this);
                }
            } catch (const ReturnException& ret) {
                result = ret.value;
            }
            
            environment = previous_env;
            
            // Result should already be a Result
            last_value = result;
            return;
        }
        
        throw RuntimeError("Unknown Result method: " + rm->method_name);
    }
    
    // Handle Rc methods
    if (std::holds_alternative<std::shared_ptr<RcMethod>>(callee)) {
        auto rcm = std::get<std::shared_ptr<RcMethod>>(callee);
        
        if (rcm->method_name == "borrow") {
            if (!arguments.empty()) {
                throw TypeError("Rc.borrow() takes no arguments");
            }
            last_value = rcm->rc->borrow();
            return;
        } else if (rcm->method_name == "use_count") {
            if (!arguments.empty()) {
                throw TypeError("Rc.use_count() takes no arguments");
            }
            last_value = static_cast<int>(rcm->rc->use_count());
            return;
        } else if (rcm->method_name == "clone") {
            if (!arguments.empty()) {
                throw TypeError("Rc.clone() takes no arguments");
            }
            auto new_rc = std::make_shared<RcValue>(rcm->rc->inner);
            last_value = new_rc;
            return;
        }
        
        throw RuntimeError("Unknown Rc method: " + rcm->method_name);
    }
    
    // Handle Arc methods
    if (std::holds_alternative<std::shared_ptr<ArcMethod>>(callee)) {
        auto acm = std::get<std::shared_ptr<ArcMethod>>(callee);
        
        if (acm->method_name == "borrow") {
            if (!arguments.empty()) {
                throw TypeError("Arc.borrow() takes no arguments");
            }
            last_value = acm->arc->borrow();
            return;
        } else if (acm->method_name == "use_count") {
            if (!arguments.empty()) {
                throw TypeError("Arc.use_count() takes no arguments");
            }
            last_value = static_cast<int>(acm->arc->use_count());
            return;
        } else if (acm->method_name == "clone") {
            if (!arguments.empty()) {
                throw TypeError("Arc.clone() takes no arguments");
            }
            auto new_arc = std::make_shared<ArcValue>(acm->arc->inner);
            last_value = new_arc;
            return;
        }
        
        throw RuntimeError("Unknown Arc method: " + acm->method_name);
    }
    
    // Handle Weak methods
    if (std::holds_alternative<std::shared_ptr<WeakMethod>>(callee)) {
        auto wkm = std::get<std::shared_ptr<WeakMethod>>(callee);
        
        if (wkm->method_name == "upgrade_rc") {
            if (!arguments.empty()) {
                throw TypeError("Weak.upgrade_rc() takes no arguments");
            }
            if (!wkm->weak->expired()) {
                auto upgraded = wkm->weak->upgrade_rc();
                if (upgraded) {
                    auto rc = std::make_shared<RcValue>(std::move(upgraded));
                    last_value = std::make_shared<OptionValue>(Value(rc));
                    return;
                }
            }
            last_value = std::make_shared<OptionValue>();
            return;
        } else if (wkm->method_name == "upgrade_arc") {
            if (!arguments.empty()) {
                throw TypeError("Weak.upgrade_arc() takes no arguments");
            }
            if (!wkm->weak->expired()) {
                auto upgraded = wkm->weak->upgrade_arc();
                if (upgraded) {
                    auto arc = std::make_shared<ArcValue>(std::move(upgraded));
                    last_value = std::make_shared<OptionValue>(Value(arc));
                    return;
                }
            }
            last_value = std::make_shared<OptionValue>();
            return;
        } else if (wkm->method_name == "expired") {
            if (!arguments.empty()) {
                throw TypeError("Weak.expired() takes no arguments");
            }
            last_value = wkm->weak->expired();
            return;
        } else if (wkm->method_name == "use_count") {
            if (!arguments.empty()) {
                throw TypeError("Weak.use_count() takes no arguments");
            }
            last_value = static_cast<int>(wkm->weak->use_count());
            return;
        }
        
        throw RuntimeError("Unknown Weak method: " + wkm->method_name);
    }
}

void Interpreter::visitListExpr(ListExpr& expr) {
    auto array = std::make_shared<ArrayValue>();
    
    for (auto& element : expr.elements) {
        array->elements.push_back(evaluateExpr(*element));
    }
    
    last_value = array;
}

void Interpreter::visitIndexExpr(IndexExpr& expr) {
    Value object = evaluateExpr(*expr.object);
    Value index = evaluateExpr(*expr.index);
    
    if (std::holds_alternative<std::shared_ptr<ArrayValue>>(object)) {
        auto array = std::get<std::shared_ptr<ArrayValue>>(object);
        
        // Auto-coerce whole-number doubles to int (e.g. from math_floor result)
        if (std::holds_alternative<double>(index)) {
            double d = std::get<double>(index);
            index = static_cast<int>(d);
        }
        if (!std::holds_alternative<int>(index)) {
            throw TypeError("Array index must be an integer");
        }
        
        int idx = std::get<int>(index);
        
        if (idx < 0 || idx >= static_cast<int>(array->elements.size())) {
            throw IndexError("Array index out of bounds: " + std::to_string(idx));
        }
        
        last_value = array->elements[idx];
    } else if (std::holds_alternative<std::string>(object)) {
        // String indexing
        std::string str = std::get<std::string>(object);
        
        if (!std::holds_alternative<int>(index)) {
            throw TypeError("String index must be an integer");
        }
        
        int idx = std::get<int>(index);
        
        if (idx < 0 || idx >= static_cast<int>(str.length())) {
            throw IndexError("String index out of bounds: " + std::to_string(idx));
        }
        
        last_value = std::string(1, str[idx]);
    } else {
        throw TypeError("Cannot index non-array/non-string type");
    }
}

void Interpreter::visitGetExpr(GetExpr& expr) {
    Value object = evaluateExpr(*expr.object);
    
    // Handle channel methods
    if (std::holds_alternative<std::shared_ptr<ChannelValue>>(object)) {
        auto channel = std::get<std::shared_ptr<ChannelValue>>(object);
        
        if (expr.name == "close" || expr.name == "is_closed" || 
            expr.name == "empty" || expr.name == "size") {
            // Return a bound channel method
            last_value = std::make_shared<ChannelMethod>(channel, expr.name);
            return;
        }
        
        throw RuntimeError("Undefined method '" + expr.name + "' on channel");
    }
    
    // Handle WaitGroup methods
    if (std::holds_alternative<std::shared_ptr<WaitGroupValue>>(object)) {
        auto wg = std::get<std::shared_ptr<WaitGroupValue>>(object);
        
        if (expr.name == "add" || expr.name == "done" || 
            expr.name == "wait" || expr.name == "count") {
            // Return a bound WaitGroup method
            last_value = std::make_shared<WaitGroupMethod>(wg, expr.name);
            return;
        }
        
        throw RuntimeError("Undefined method '" + expr.name + "' on WaitGroup");
    }
    
    // Handle Option methods
    if (std::holds_alternative<std::shared_ptr<OptionValue>>(object)) {
        auto option = std::get<std::shared_ptr<OptionValue>>(object);
        
        if (expr.name == "is_some" || expr.name == "is_none" || 
            expr.name == "unwrap" || expr.name == "unwrap_or" ||
            expr.name == "map" || expr.name == "and_then") {
            // Return a bound Option method
            last_value = std::make_shared<OptionMethod>(option, expr.name);
            return;
        }
        
        throw RuntimeError("Undefined method '" + expr.name + "' on Option");
    }
    
    // Handle Result methods
    if (std::holds_alternative<std::shared_ptr<ResultValue>>(object)) {
        auto result = std::get<std::shared_ptr<ResultValue>>(object);
        
        if (expr.name == "is_ok" || expr.name == "is_err" || 
            expr.name == "unwrap" || expr.name == "unwrap_or" ||
            expr.name == "unwrap_err" || expr.name == "map" || 
            expr.name == "map_err" || expr.name == "and_then") {
            // Return a bound Result method
            last_value = std::make_shared<ResultMethod>(result, expr.name);
            return;
        }
        
        throw RuntimeError("Undefined method '" + expr.name + "' on Result");
    }
    
    // Handle Rc methods
    if (std::holds_alternative<std::shared_ptr<RcValue>>(object)) {
        auto rc = std::get<std::shared_ptr<RcValue>>(object);
        
        if (expr.name == "borrow" || expr.name == "use_count" || expr.name == "clone") {
            last_value = std::make_shared<RcMethod>(rc, expr.name);
            return;
        }
        
        throw RuntimeError("Undefined method '" + expr.name + "' on Rc");
    }
    
    // Handle Arc methods
    if (std::holds_alternative<std::shared_ptr<ArcValue>>(object)) {
        auto arc = std::get<std::shared_ptr<ArcValue>>(object);
        
        if (expr.name == "borrow" || expr.name == "use_count" || expr.name == "clone") {
            last_value = std::make_shared<ArcMethod>(arc, expr.name);
            return;
        }
        
        throw RuntimeError("Undefined method '" + expr.name + "' on Arc");
    }
    
    // Handle Weak methods
    if (std::holds_alternative<std::shared_ptr<WeakValue>>(object)) {
        auto weak = std::get<std::shared_ptr<WeakValue>>(object);
        
        if (expr.name == "upgrade_rc" || expr.name == "upgrade_arc" || 
            expr.name == "expired" || expr.name == "use_count") {
            last_value = std::make_shared<WeakMethod>(weak, expr.name);
            return;
        }
        
        throw RuntimeError("Undefined method '" + expr.name + "' on Weak");
    }
    
    // Handle static method access on Class (e.g., MyClass.static_method)
    if (std::holds_alternative<std::shared_ptr<Class>>(object)) {
        auto cls = std::get<std::shared_ptr<Class>>(object);
        
        // Look for method
        auto method = cls->findMethod(expr.name);
        if (method) {
            // Check if it's a static or class method
            bool is_static = cls->static_methods.find(expr.name) != cls->static_methods.end();
            bool is_classmethod = cls->class_methods.find(expr.name) != cls->class_methods.end();
            
            if (is_static || is_classmethod) {
                last_value = std::make_shared<StaticMethod>(cls, method, is_classmethod);
                return;
            }
            
            throw RuntimeError("Cannot access instance method '" + expr.name + "' on class '" + cls->name + "'");
        }
        
        throw RuntimeError("Undefined method '" + expr.name + "' on class '" + cls->name + "'");
    }
    
    // Handle array methods
    if (std::holds_alternative<std::shared_ptr<ArrayValue>>(object)) {
        auto array = std::get<std::shared_ptr<ArrayValue>>(object);
        
        if (expr.name == "push") {
            last_value = std::make_shared<ArrayMethod>(array, "push");
            return;
        } else if (expr.name == "pop") {
            last_value = std::make_shared<ArrayMethod>(array, "pop");
            return;
        } else if (expr.name == "length") {
            last_value = static_cast<int>(array->elements.size());
            return;
        } else if (expr.name == "size") {
            last_value = static_cast<int>(array->elements.size());
            return;
        }
        
        // Check extension methods for Array
        auto arr_ext = extensions.find("Array");
        if (arr_ext != extensions.end()) {
            auto meth_it = arr_ext->second.find(expr.name);
            if (meth_it != arr_ext->second.end()) {
                last_value = std::make_shared<ExtensionMethod>(object, meth_it->second);
                return;
            }
        }
        
        throw RuntimeError("Unknown array property '" + expr.name + "'");
    }
    
    // Handle string methods
    if (std::holds_alternative<std::string>(object)) {
        std::string str = std::get<std::string>(object);
        
        if (expr.name == "length") {
            last_value = static_cast<int>(str.length());
            return;
        } else if (expr.name == "size") {
            last_value = static_cast<int>(str.length());
            return;
        } else if (expr.name == "upper") {
            last_value = std::make_shared<StringMethod>(str, "upper");
            return;
        } else if (expr.name == "lower") {
            last_value = std::make_shared<StringMethod>(str, "lower");
            return;
        } else if (expr.name == "split") {
            last_value = std::make_shared<StringMethod>(str, "split");
            return;
        } else if (expr.name == "trim") {
            last_value = std::make_shared<StringMethod>(str, "trim");
            return;
        } else if (expr.name == "contains") {
            last_value = std::make_shared<StringMethod>(str, "contains");
            return;
        }
        
        // Check extension methods for String
        auto str_ext = extensions.find("String");
        if (str_ext != extensions.end()) {
            auto meth_it = str_ext->second.find(expr.name);
            if (meth_it != str_ext->second.end()) {
                last_value = std::make_shared<ExtensionMethod>(object, meth_it->second);
                return;
            }
        }
        
        throw RuntimeError("Unknown string property '" + expr.name + "'");
    }
    
    // Handle hash map methods
    if (std::holds_alternative<std::shared_ptr<HashMapValue>>(object)) {
        auto hashmap = std::get<std::shared_ptr<HashMapValue>>(object);
        
        if (expr.name == "get") {
            last_value = std::make_shared<HashMapMethod>(hashmap, "get");
            return;
        } else if (expr.name == "set") {
            last_value = std::make_shared<HashMapMethod>(hashmap, "set");
            return;
        } else if (expr.name == "has") {
            last_value = std::make_shared<HashMapMethod>(hashmap, "has");
            return;
        } else if (expr.name == "remove") {
            last_value = std::make_shared<HashMapMethod>(hashmap, "remove");
            return;
        } else if (expr.name == "keys") {
            last_value = std::make_shared<HashMapMethod>(hashmap, "keys");
            return;
        } else if (expr.name == "values") {
            last_value = std::make_shared<HashMapMethod>(hashmap, "values");
            return;
        } else if (expr.name == "size") {
            last_value = static_cast<int>(hashmap->pairs.size());
            return;
        } else if (expr.name == "length") {
            last_value = static_cast<int>(hashmap->pairs.size());
            return;
        }
        
        // Check extension methods for HashMap
        auto hm_ext = extensions.find("HashMap");
        if (hm_ext != extensions.end()) {
            auto meth_it = hm_ext->second.find(expr.name);
            if (meth_it != hm_ext->second.end()) {
                last_value = std::make_shared<ExtensionMethod>(object, meth_it->second);
                return;
            }
        }
        
        throw RuntimeError("Unknown hash map property '" + expr.name + "'");
    }
    
    if (std::holds_alternative<std::shared_ptr<Instance>>(object)) {
        auto instance = std::get<std::shared_ptr<Instance>>(object);
        
        // Check instance fields first
        auto it = instance->fields.find(expr.name);
        if (it != instance->fields.end()) {
            last_value = it->second;
            return;
        }
        
        // Then check methods on the class (and superclasses)
        auto method = instance->klass->findMethod(expr.name);
        if (method) {
            // Check if it's a static or class method
            bool is_static = instance->klass->static_methods.find(expr.name) != instance->klass->static_methods.end();
            bool is_classmethod = instance->klass->class_methods.find(expr.name) != instance->klass->class_methods.end();
            
            if (is_static || is_classmethod) {
                // Static/class methods can be called on instances too
                last_value = std::make_shared<StaticMethod>(instance->klass, method, is_classmethod);
                return;
            }
            
            // Regular instance method
            last_value = std::make_shared<BoundMethod>(instance, method);
            return;
        }
        
        // Finally check trait methods
        auto trait_method = findTraitMethod(instance->klass->name, expr.name);
        if (trait_method) {
            last_value = std::make_shared<BoundMethod>(instance, trait_method);
            return;
        }
        
        throw RuntimeError("Undefined property '" + expr.name + "' on instance of '" + instance->klass->name + "'");
    }
    
    throw TypeError("Only instances and classes have properties");
}

void Interpreter::visitSetExpr(SetExpr& expr) {
    Value object = evaluateExpr(*expr.object);
    Value value = evaluateExpr(*expr.value);
    
    if (std::holds_alternative<std::shared_ptr<Instance>>(object)) {
        auto instance = std::get<std::shared_ptr<Instance>>(object);
        instance->fields[expr.name] = value;
        last_value = value;
        return;
    }
    
    throw TypeError("Only instances have settable properties");
}

void Interpreter::visitIndexAssignExpr(IndexAssignExpr& expr) {
    Value object = evaluateExpr(*expr.object);
    Value index = evaluateExpr(*expr.index);
    Value value = evaluateExpr(*expr.value);
    
    // Handle array index assignment
    if (std::holds_alternative<std::shared_ptr<ArrayValue>>(object)) {
        auto array = std::get<std::shared_ptr<ArrayValue>>(object);
        
        // Auto-coerce whole-number doubles to int
        if (std::holds_alternative<double>(index)) {
            index = static_cast<int>(std::get<double>(index));
        }
        if (std::holds_alternative<int>(index)) {
            int idx = std::get<int>(index);
            if (idx >= 0 && idx < static_cast<int>(array->elements.size())) {
                array->elements[idx] = value;
                last_value = value;
                return;
            } else {
                throw RuntimeError("Array index out of bounds");
            }
        } else {
            throw TypeError("Array index must be an integer");
        }
    }
    
    throw TypeError("Only arrays support index assignment");
}

void Interpreter::visitHashMapExpr(HashMapExpr& expr) {
    auto hashmap = std::make_shared<HashMapValue>();
    
    // Evaluate all key-value pairs
    for (const auto& pair : expr.pairs) {
        Value key = evaluateExpr(*pair.first);
        Value value = evaluateExpr(*pair.second);
        
        // Convert key to string
        std::string keyStr;
        if (std::holds_alternative<std::string>(key)) {
            keyStr = std::get<std::string>(key);
        } else {
            keyStr = valueToString(key);
        }
        
        hashmap->pairs[keyStr] = value;
    }
    
    last_value = hashmap;
}

// Statement visitors

void Interpreter::visitExprStmt(ExprStmt& stmt) {
    evaluateExpr(*stmt.expression);
}

void Interpreter::visitVarDeclStmt(VarDeclStmt& stmt) {
    Value value = nullptr;
    if (stmt.initializer) {
        value = evaluateExpr(*stmt.initializer);
    }
    environment->define(stmt.name, value, stmt.ownership);
}

void Interpreter::visitFunctionDecl(FunctionDecl& stmt) {
    // Create a function object and store it in the environment
    auto func = std::make_shared<Function>(
        stmt.name,
        stmt.parameters,
        stmt.return_type,
        &stmt.body,
        environment,  // Capture current environment as closure
        stmt.is_async  // Pass async flag
    );
    
    // If no decorators, just define the function
    if (stmt.decorators.empty()) {
        environment->define(stmt.name, func);
        return;
    }
    
    // Apply decorators in reverse order (bottom to top)
    Value decorated_func = func;
    for (auto it = stmt.decorators.rbegin(); it != stmt.decorators.rend(); ++it) {
        auto& decorator = *it;
        
        // Look up decorator function
        Value decorator_func = environment->get(decorator.name);
        
        // Handle built-in decorators
        if (std::holds_alternative<std::string>(decorator_func)) {
            std::string builtin_name = std::get<std::string>(decorator_func);
            
            if (builtin_name == "__builtin_cache__") {
                // Mark function as cached - we'll handle caching in visitCallExpr
                if (!std::holds_alternative<std::shared_ptr<Function>>(decorated_func)) {
                    throw RuntimeError("@cache can only be applied to functions");
                }
                auto f = std::get<std::shared_ptr<Function>>(decorated_func);
                // Store a marker that this function should be cached
                // We'll use a special naming convention: prepend "__cached__" to the name
                f->name = "__cached__" + f->name;
                continue;
            }
            
            if (builtin_name == "__builtin_timing__") {
                // Mark function for timing - we'll handle timing in visitCallExpr
                if (!std::holds_alternative<std::shared_ptr<Function>>(decorated_func)) {
                    throw RuntimeError("@timing can only be applied to functions");
                }
                auto f = std::get<std::shared_ptr<Function>>(decorated_func);
                f->name = "__timed__" + f->name;
                continue;
            }
            
            if (builtin_name == "__builtin_deprecated__") {
                // Print deprecation warning
                if (!std::holds_alternative<std::shared_ptr<Function>>(decorated_func)) {
                    throw RuntimeError("@deprecated can only be applied to functions");
                }
                auto f = std::get<std::shared_ptr<Function>>(decorated_func);
                
                // Get deprecation message if provided
                std::string message = "Function '" + stmt.name + "' is deprecated";
                if (!decorator.arguments.empty()) {
                    Value msg_val = evaluateExpr(*decorator.arguments[0]);
                    if (std::holds_alternative<std::string>(msg_val)) {
                        message = std::get<std::string>(msg_val);
                    }
                }
                
                // Mark function as deprecated
                f->name = "__deprecated__" + f->name + "__" + message;
                continue;
            }

            if (builtin_name == "__builtin_constexpr__") {
                // Evaluate the function immediately with no arguments and store the result value.
                // The name is then bound to the computed value, not the function.
                if (!std::holds_alternative<std::shared_ptr<Function>>(decorated_func)) {
                    throw RuntimeError("@constexpr can only be applied to functions");
                }
                auto f = std::get<std::shared_ptr<Function>>(decorated_func);
                auto fn_env = std::make_shared<Environment>(f->closure);
                auto saved_env = environment;
                environment = fn_env;
                Value result = nullptr;
                try {
                    for (auto& s : *f->getBody()) s->accept(*this);
                } catch (const ReturnException& ret) {
                    result = ret.value;
                }
                environment = saved_env;
                // Replace the function with its computed value
                decorated_func = result;
                continue;
            }

            if (builtin_name == "__builtin_singleton__") {
                // Wrap the function so it only ever creates one instance.
                if (!std::holds_alternative<std::shared_ptr<Function>>(decorated_func)) {
                    throw RuntimeError("@singleton can only be applied to functions");
                }
                auto f = std::get<std::shared_ptr<Function>>(decorated_func);
                f->name = "__singleton__" + f->name;
                continue;
            }
        }
        
        // Evaluate decorator arguments
        std::vector<Value> args;
        args.push_back(decorated_func);  // First argument is the function being decorated
        
        for (auto& arg_expr : decorator.arguments) {
            args.push_back(evaluateExpr(*arg_expr));
        }
        
        // Apply decorator (call it with the function and arguments)
        if (std::holds_alternative<std::shared_ptr<Function>>(decorator_func)) {
            auto dec_fn = std::get<std::shared_ptr<Function>>(decorator_func);
            
            // Create environment for decorator call
            auto dec_env = std::make_shared<Environment>(dec_fn->closure);
            
            // Bind parameters
            if (args.size() != dec_fn->parameters.size()) {
                throw RuntimeError("Decorator '" + decorator.name + "' expects " +
                                   std::to_string(dec_fn->parameters.size()) + " arguments but got " +
                                   std::to_string(args.size()));
            }
            
            for (size_t i = 0; i < dec_fn->parameters.size(); i++) {
                dec_env->define(dec_fn->parameters[i].name, args[i], dec_fn->parameters[i].ownership);
            }
            
            auto previous_env = environment;
            environment = dec_env;
            
            try {
                for (auto& s : *dec_fn->getBody()) {
                    s->accept(*this);
                }
                decorated_func = nullptr;  // No return
            } catch (const ReturnException& ret) {
                decorated_func = ret.value;
            }
            
            environment = previous_env;
        } else {
            throw RuntimeError("'" + decorator.name + "' is not a function and cannot be used as a decorator");
        }
    }
    
    environment->define(stmt.name, decorated_func);
}

void Interpreter::visitReturnStmt(ReturnStmt& stmt) {
    Value value = nullptr;
    if (stmt.value) {
        value = evaluateExpr(*stmt.value);
    }
    throw ReturnException(value);
}

void Interpreter::visitIfStmt(IfStmt& stmt) {
    Value condition = evaluateExpr(*stmt.condition);
    
    if (isTruthy(condition)) {
        for (auto& s : stmt.then_branch) {
            s->accept(*this);
        }
    } else if (!stmt.else_branch.empty()) {
        for (auto& s : stmt.else_branch) {
            s->accept(*this);
        }
    }
}

void Interpreter::visitWhileStmt(WhileStmt& stmt) {
    while (isTruthy(evaluateExpr(*stmt.condition))) {
        try {
            for (auto& s : stmt.body) {
                s->accept(*this);
            }
        } catch (const BreakException&) {
            break;
        } catch (const ContinueException&) {
            continue;
        }
    }
}

void Interpreter::visitForStmt(ForStmt& stmt) {
    Value iterable = evaluateExpr(*stmt.iterable);
    
    // Handle channel iteration
    if (std::holds_alternative<std::shared_ptr<ChannelValue>>(iterable)) {
        auto channel = std::get<std::shared_ptr<ChannelValue>>(iterable);
        
        // Iterate until channel is closed and empty
        while (true) {
            if (channel->empty()) {
                if (channel->is_closed()) {
                    break;  // Channel closed and empty, stop iteration
                }
                // Channel empty but not closed, could wait or break
                // For now, break (non-blocking iteration)
                break;
            }
            
            // Receive value from channel
            Value value = channel->receive();
            environment->define(stmt.variable, value);
            
            // Execute loop body
            try {
                for (auto& s : stmt.body) {
                    s->accept(*this);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
        return;
    }
    
    // Handle string iteration
    if (std::holds_alternative<std::string>(iterable)) {
        std::string str = std::get<std::string>(iterable);
        for (size_t i = 0; i < str.size(); i++) {
            environment->define(stmt.variable, std::string(1, str[i]));
            try {
                for (auto& s : stmt.body) {
                    s->accept(*this);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
        return;
    }
    
    // Handle array/list iteration
    if (std::holds_alternative<std::shared_ptr<ArrayValue>>(iterable)) {
        auto array = std::get<std::shared_ptr<ArrayValue>>(iterable);
        for (size_t i = 0; i < array->elements.size(); i++) {
            environment->define(stmt.variable, array->elements[i]);
            try {
                for (auto& s : stmt.body) {
                    s->accept(*this);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
        return;
    }
    
    // Handle hash map iteration (iterate over keys)
    if (std::holds_alternative<std::shared_ptr<HashMapValue>>(iterable)) {
        auto hashmap = std::get<std::shared_ptr<HashMapValue>>(iterable);
        for (const auto& pair : hashmap->pairs) {
            environment->define(stmt.variable, pair.first);
            try {
                for (auto& s : stmt.body) {
                    s->accept(*this);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
        return;
    }
    
    // Handle integer iteration (range)
    if (std::holds_alternative<int>(iterable)) {
        int end = std::get<int>(iterable);
        for (int i = 0; i < end; i++) {
            environment->define(stmt.variable, i);
            try {
                for (auto& s : stmt.body) {
                    s->accept(*this);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
        return;
    }
    
    throwException("TypeError", "not iterable");
}

void Interpreter::visitBreakStmt(BreakStmt& stmt) {
    (void)stmt;
    throw BreakException();
}

void Interpreter::visitContinueStmt(ContinueStmt& stmt) {
    (void)stmt;
    throw ContinueException();
}

void Interpreter::visitTryStmt(TryStmt& stmt) {
    std::exception_ptr pending_exception;
    
    try {
        for (auto& s : stmt.try_body) {
            s->accept(*this);
        }
    } catch (const SapphireException& ex) {
        bool handled = false;
        
        for (auto& clause : stmt.catch_clauses) {
            if (!clause.exception_type.empty() &&
                clause.exception_type != ex.getTypeName()) {
                continue;
            }
            
            // Bind exception variable if present
            if (!clause.variable_name.empty()) {
                std::string value = ex.getTypeName() + ": " + ex.getMessage();
                environment->define(clause.variable_name, value);
            }
            
            for (auto& s : clause.body) {
                s->accept(*this);
            }
            
            handled = true;
            break;
        }
        
        if (!handled) {
            pending_exception = std::current_exception();
        }
    } catch (const ReturnException&) {
        pending_exception = std::current_exception();
    }
    
    // Finally block always runs
    if (!stmt.finally_body.empty()) {
        for (auto& s : stmt.finally_body) {
            s->accept(*this);
        }
    }
    
    // Rethrow if not handled
    if (pending_exception) {
        std::rethrow_exception(pending_exception);
    }
}

void Interpreter::visitThrowStmt(ThrowStmt& stmt) {
    std::string message;
    if (stmt.message) {
        Value value = evaluateExpr(*stmt.message);
        message = valueToString(value);
    }
    
    throwException(stmt.exception_type, message);
}

void Interpreter::visitClassDecl(ClassDecl& stmt) {
    std::shared_ptr<Class> superclass = nullptr;
    
    if (!stmt.superclass_name.empty()) {
        Value value = environment->get(stmt.superclass_name);
        if (!std::holds_alternative<std::shared_ptr<Class>>(value)) {
            throw TypeError("Superclass '" + stmt.superclass_name + "' is not a class");
        }
        superclass = std::get<std::shared_ptr<Class>>(value);
    }
    
    // Check for decorators
    bool is_dataclass = false;
    bool is_singleton = false;
    
    for (const auto& decorator : stmt.decorators) {
        Value dec_val = environment->get(decorator.name);
        if (std::holds_alternative<std::string>(dec_val)) {
            std::string builtin_name = std::get<std::string>(dec_val);
            if (builtin_name == "__builtin_dataclass__") {
                is_dataclass = true;
            } else if (builtin_name == "__builtin_singleton__") {
                is_singleton = true;
            }
        }
    }
    
    // For @singleton on a class, note it — instance caching is handled at call site
    if (is_singleton) {
        // No-op here; the __singleton__ prefix on the constructor handles it
    }
    
    // Map methods by name
    std::map<std::string, std::shared_ptr<Function>> methods;
    std::set<std::string> static_methods;
    std::set<std::string> class_methods;
    
    for (auto& method : stmt.methods) {
        // Check for method decorators
        bool is_static = false;
        bool is_classmethod = false;
        bool is_property = false;
        
        for (const auto& decorator : method->decorators) {
            Value dec_val = environment->get(decorator.name);
            if (std::holds_alternative<std::string>(dec_val)) {
                std::string builtin_name = std::get<std::string>(dec_val);
                if (builtin_name == "__builtin_staticmethod__") {
                    is_static = true;
                } else if (builtin_name == "__builtin_classmethod__") {
                    is_classmethod = true;
                } else if (builtin_name == "__builtin_property__") {
                    is_property = true;
                }
            }
        }
        
        auto func = std::make_shared<Function>(
            method->name,
            method->parameters,
            method->return_type,
            &method->body,
            environment  // methods share the current environment as closure
        );
        methods[method->name] = func;
        
        if (is_static) {
            static_methods.insert(method->name);
        }
        if (is_classmethod) {
            class_methods.insert(method->name);
        }
        if (is_property) {
            // Properties are just regular methods for now
            // In a full implementation, they would be callable without ()
            std::cout << "Note: @property on '" << method->name << "' - call with () for now" << std::endl;
        }
    }
    
    auto cls = std::make_shared<Class>(stmt.name, superclass, methods, false, is_dataclass);
    cls->static_methods = static_methods;
    cls->class_methods = class_methods;
    environment->define(stmt.name, cls);
}

void Interpreter::visitImportStmt(ImportStmt& stmt) {
    std::string mod = stmt.module_name;

    // ===== stdlib module registration =====

    // Milestone 1: Collections (arrays)
    if (mod == "collections") {
        environment->define("array_create", std::string("__builtin_array_create__"));
        environment->define("array_push", std::string("__builtin_array_push__"));
        environment->define("array_get", std::string("__builtin_array_get__"));
        environment->define("array_set", std::string("__builtin_array_set__"));
        environment->define("array_pop", std::string("__builtin_array_pop__"));
        environment->define("array_size", std::string("__builtin_array_size__"));
        environment->define("array_clear", std::string("__builtin_array_clear__"));
    }

    // Milestone 1: Math Library
    if (mod == "math") {
        environment->define("math_sqrt", std::string("__builtin_math_sqrt__"));
        environment->define("math_pow", std::string("__builtin_math_pow__"));
        environment->define("math_sin", std::string("__builtin_math_sin__"));
        environment->define("math_cos", std::string("__builtin_math_cos__"));
        environment->define("math_tan", std::string("__builtin_math_tan__"));
        environment->define("math_exp", std::string("__builtin_math_exp__"));
        environment->define("math_log", std::string("__builtin_math_log__"));
        environment->define("math_log10", std::string("__builtin_math_log10__"));
        environment->define("math_abs", std::string("__builtin_math_abs__"));
        environment->define("math_floor", std::string("__builtin_math_floor__"));
        environment->define("math_ceil", std::string("__builtin_math_ceil__"));
        environment->define("math_round", std::string("__builtin_math_round__"));
        environment->define("math_pi", std::string("__builtin_math_pi__"));
        environment->define("math_e", std::string("__builtin_math_e__"));
        environment->define("math_atan2", std::string("__builtin_math_atan2__"));
        environment->define("math_clamp", std::string("__builtin_math_clamp__"));
        environment->define("math_min",   std::string("__builtin_math_min__"));
        environment->define("math_max",   std::string("__builtin_math_max__"));
    }

    // Milestone 1: CSV I/O
    if (mod == "csv") {
        environment->define("csv_create", std::string("__builtin_csv_create__"));
        environment->define("csv_read_file", std::string("__builtin_csv_read_file__"));
        environment->define("csv_write_file", std::string("__builtin_csv_write_file__"));
        environment->define("csv_get", std::string("__builtin_csv_get__"));
        environment->define("csv_set", std::string("__builtin_csv_set__"));
        environment->define("csv_row_count", std::string("__builtin_csv_row_count__"));
        environment->define("csv_col_count", std::string("__builtin_csv_col_count__"));
        environment->define("csv_add_row", std::string("__builtin_csv_add_row__"));
    }

    // Milestone 2: JSON Support
    if (mod == "json") {
        environment->define("json_parse", std::string("__builtin_json_parse__"));
        environment->define("json_stringify", std::string("__builtin_json_stringify__"));
        environment->define("json_create_object", std::string("__builtin_json_create_object__"));
        environment->define("json_create_array", std::string("__builtin_json_create_array__"));
        environment->define("json_get", std::string("__builtin_json_get__"));
        environment->define("json_set", std::string("__builtin_json_set__"));
        environment->define("json_push", std::string("__builtin_json_push__"));
        environment->define("json_size", std::string("__builtin_json_size__"));
        environment->define("json_loads", std::string("__builtin_json_loads__"));
        environment->define("json_dumps", std::string("__builtin_json_dumps__"));
    }

    // Milestone 2: Random Numbers
    if (mod == "random") {
        environment->define("random_seed", std::string("__builtin_random_seed__"));
        environment->define("random", std::string("__builtin_random__"));
        environment->define("random_int", std::string("__builtin_random_int__"));
        environment->define("random_normal", std::string("__builtin_random_normal__"));
        environment->define("random_boolean", std::string("__builtin_random_boolean__"));
        environment->define("crypto_random", std::string("__builtin_crypto_random__"));
        environment->define("crypto_random_int", std::string("__builtin_crypto_random_int__"));
    }

    // Milestone 3: Data Visualization
    if (mod == "plotting") {
        environment->define("plot_create", std::string("__builtin_plot_create__"));
        environment->define("plot_add_line", std::string("__builtin_plot_add_line__"));
        environment->define("plot_add_scatter", std::string("__builtin_plot_add_scatter__"));
        environment->define("plot_add_bar", std::string("__builtin_plot_add_bar__"));
        environment->define("plot_add_histogram", std::string("__builtin_plot_add_histogram__"));
        environment->define("plot_set_title", std::string("__builtin_plot_set_title__"));
        environment->define("plot_set_xlabel", std::string("__builtin_plot_set_xlabel__"));
        environment->define("plot_set_ylabel", std::string("__builtin_plot_set_ylabel__"));
        environment->define("plot_show", std::string("__builtin_plot_show__"));
        environment->define("plot_save_svg", std::string("__builtin_plot_save_svg__"));
        environment->define("quick_plot", std::string("__builtin_quick_plot__"));
        environment->define("quick_scatter", std::string("__builtin_quick_scatter__"));
        environment->define("quick_histogram", std::string("__builtin_quick_histogram__"));
    }

    // 3D Plotting (browser-based via Plotly.js HTML output)
    if (mod == "plot3d") {
        environment->define("plot3d_create",      std::string("__builtin_plot3d_create__"));
        environment->define("plot3d_add_surface",  std::string("__builtin_plot3d_add_surface__"));
        environment->define("plot3d_add_scatter",  std::string("__builtin_plot3d_add_scatter__"));
        environment->define("plot3d_add_line",     std::string("__builtin_plot3d_add_line__"));
        environment->define("plot3d_set_title",    std::string("__builtin_plot3d_set_title__"));
        environment->define("plot3d_set_xlabel",   std::string("__builtin_plot3d_set_xlabel__"));
        environment->define("plot3d_set_ylabel",   std::string("__builtin_plot3d_set_ylabel__"));
        environment->define("plot3d_set_zlabel",   std::string("__builtin_plot3d_set_zlabel__"));
        environment->define("plot3d_save_html",    std::string("__builtin_plot3d_save_html__"));
    }

    // Milestone 4: HTTP Server/Client
    if (mod == "http") {
        environment->define("http_server_create", std::string("__builtin_http_server_create__"));
        environment->define("http_server_start", std::string("__builtin_http_server_start__"));
        environment->define("http_server_stop", std::string("__builtin_http_server_stop__"));
        environment->define("http_server_get", std::string("__builtin_http_server_get__"));
        environment->define("http_server_post", std::string("__builtin_http_server_post__"));
        environment->define("http_server_put", std::string("__builtin_http_server_put__"));
        environment->define("http_server_delete", std::string("__builtin_http_server_delete__"));
        environment->define("http_client_create", std::string("__builtin_http_client_create__"));
        environment->define("http_client_get", std::string("__builtin_http_client_get__"));
        environment->define("http_client_post", std::string("__builtin_http_client_post__"));
        environment->define("http_response_status", std::string("__builtin_http_response_status__"));
        environment->define("http_response_body", std::string("__builtin_http_response_body__"));
        environment->define("http_response_header", std::string("__builtin_http_response_header__"));
        environment->define("http_url_encode", std::string("__builtin_http_url_encode__"));
        environment->define("http_url_decode", std::string("__builtin_http_url_decode__"));
        environment->define("http_html_escape", std::string("__builtin_http_html_escape__"));
    }

    // File System Module
    if (mod == "fs") {
        environment->define("fs_exists", std::string("__builtin_fs_exists__"));
        environment->define("fs_is_file", std::string("__builtin_fs_is_file__"));
        environment->define("fs_is_dir", std::string("__builtin_fs_is_dir__"));
        environment->define("fs_create_dir", std::string("__builtin_fs_create_dir__"));
        environment->define("fs_read_file", std::string("__builtin_fs_read_file__"));
        environment->define("fs_write_file", std::string("__builtin_fs_write_file__"));
        environment->define("fs_delete", std::string("__builtin_fs_delete__"));
        environment->define("fs_abs_path", std::string("__builtin_fs_abs_path__"));
    }

    // Milestone 4: WebSocket Support
    if (mod == "websocket") {
        environment->define("websocket_server_create", std::string("__builtin_websocket_server_create__"));
        environment->define("websocket_server_start", std::string("__builtin_websocket_server_start__"));
        environment->define("websocket_server_stop", std::string("__builtin_websocket_server_stop__"));
        environment->define("websocket_server_broadcast_text", std::string("__builtin_websocket_server_broadcast_text__"));
        environment->define("websocket_server_broadcast_binary", std::string("__builtin_websocket_server_broadcast_binary__"));
        environment->define("websocket_server_connection_count", std::string("__builtin_websocket_server_connection_count__"));
        environment->define("websocket_client_create", std::string("__builtin_websocket_client_create__"));
        environment->define("websocket_client_connect", std::string("__builtin_websocket_client_connect__"));
        environment->define("websocket_client_disconnect", std::string("__builtin_websocket_client_disconnect__"));
        environment->define("websocket_client_send_text", std::string("__builtin_websocket_client_send_text__"));
        environment->define("websocket_client_send_binary", std::string("__builtin_websocket_client_send_binary__"));
        environment->define("websocket_client_is_connected", std::string("__builtin_websocket_client_is_connected__"));
        environment->define("websocket_base64_encode", std::string("__builtin_websocket_base64_encode__"));
        environment->define("websocket_base64_decode", std::string("__builtin_websocket_base64_decode__"));
    }

    // Milestone 4: Template Engine
    if (mod == "template") {
        environment->define("html_template_engine_create", std::string("__builtin_html_template_engine_create__"));
        environment->define("html_template_render", std::string("__builtin_html_template_render__"));
        environment->define("html_template_render_file", std::string("__builtin_html_template_render_file__"));
        environment->define("html_template_set_dir", std::string("__builtin_html_template_set_dir__"));
        environment->define("html_template_cache", std::string("__builtin_html_template_cache__"));
    }

    // Milestone 4: Database Drivers
    if (mod == "database") {
        environment->define("postgresql_create", std::string("__builtin_postgresql_create__"));
        environment->define("postgresql_connect", std::string("__builtin_postgresql_connect__"));
        environment->define("postgresql_disconnect", std::string("__builtin_postgresql_disconnect__"));
        environment->define("postgresql_query", std::string("__builtin_postgresql_query__"));
        environment->define("postgresql_execute", std::string("__builtin_postgresql_execute__"));
        environment->define("mysql_create", std::string("__builtin_mysql_create__"));
        environment->define("mysql_connect", std::string("__builtin_mysql_connect__"));
        environment->define("mysql_disconnect", std::string("__builtin_mysql_disconnect__"));
        environment->define("mysql_query", std::string("__builtin_mysql_query__"));
        environment->define("mysql_execute", std::string("__builtin_mysql_execute__"));
    }

    // Milestone 5: ORM Framework
    if (mod == "orm") {
        environment->define("orm_model_create", std::string("__builtin_orm_model_create__"));
        environment->define("orm_model_add_field", std::string("__builtin_orm_model_add_field__"));
        environment->define("orm_model_add_primary_key", std::string("__builtin_orm_model_add_primary_key__"));
        environment->define("orm_model_get_sql", std::string("__builtin_orm_model_get_sql__"));
        environment->define("orm_record_create", std::string("__builtin_orm_record_create__"));
        environment->define("orm_record_set", std::string("__builtin_orm_record_set__"));
        environment->define("orm_record_get", std::string("__builtin_orm_record_get__"));
        environment->define("orm_record_to_json", std::string("__builtin_orm_record_to_json__"));
        environment->define("orm_query_create", std::string("__builtin_orm_query_create__"));
        environment->define("orm_query_where", std::string("__builtin_orm_query_where__"));
        environment->define("orm_query_limit", std::string("__builtin_orm_query_limit__"));
        environment->define("orm_query_offset", std::string("__builtin_orm_query_offset__"));
        environment->define("orm_query_order_by", std::string("__builtin_orm_query_order_by__"));
        environment->define("orm_query_to_sql", std::string("__builtin_orm_query_to_sql__"));
        environment->define("orm_db_create", std::string("__builtin_orm_db_create__"));
        environment->define("orm_db_connect", std::string("__builtin_orm_db_connect__"));
        environment->define("orm_db_execute", std::string("__builtin_orm_db_execute__"));
    }

    // Milestone 5: Auth Framework
    if (mod == "auth") {
        environment->define("jwt_create", std::string("__builtin_jwt_create__"));
        environment->define("jwt_generate", std::string("__builtin_jwt_generate__"));
        environment->define("jwt_validate", std::string("__builtin_jwt_validate__"));
        environment->define("jwt_get_subject", std::string("__builtin_jwt_get_subject__"));
        environment->define("jwt_get_claim", std::string("__builtin_jwt_get_claim__"));
        environment->define("jwt_refresh", std::string("__builtin_jwt_refresh__"));
        environment->define("jwt_blacklist", std::string("__builtin_jwt_blacklist__"));
        environment->define("jwt_is_blacklisted", std::string("__builtin_jwt_is_blacklisted__"));
        environment->define("session_manager_create", std::string("__builtin_session_manager_create__"));
        environment->define("session_create", std::string("__builtin_session_create__"));
        environment->define("session_is_valid", std::string("__builtin_session_is_valid__"));
        environment->define("session_set_data", std::string("__builtin_session_set_data__"));
        environment->define("session_get_data", std::string("__builtin_session_get_data__"));
        environment->define("session_destroy", std::string("__builtin_session_destroy__"));
        environment->define("session_regenerate", std::string("__builtin_session_regenerate__"));
        environment->define("session_cleanup", std::string("__builtin_session_cleanup__"));
        environment->define("session_get_user_id", std::string("__builtin_session_get_user_id__"));
        environment->define("rbac_create", std::string("__builtin_rbac_create__"));
        environment->define("rbac_define_role", std::string("__builtin_rbac_define_role__"));
        environment->define("rbac_add_permission", std::string("__builtin_rbac_add_permission__"));
        environment->define("rbac_assign_role", std::string("__builtin_rbac_assign_role__"));
        environment->define("rbac_revoke_role", std::string("__builtin_rbac_revoke_role__"));
        environment->define("rbac_has_permission", std::string("__builtin_rbac_has_permission__"));
        environment->define("rbac_has_role", std::string("__builtin_rbac_has_role__"));
        environment->define("oauth2_create", std::string("__builtin_oauth2_create__"));
        environment->define("oauth2_register_provider", std::string("__builtin_oauth2_register_provider__"));
        environment->define("oauth2_get_auth_url", std::string("__builtin_oauth2_get_auth_url__"));
        environment->define("oauth2_generate_state", std::string("__builtin_oauth2_generate_state__"));
        environment->define("oauth2_validate_state", std::string("__builtin_oauth2_validate_state__"));
        environment->define("oauth2_has_provider", std::string("__builtin_oauth2_has_provider__"));
    }

    // Milestone 5: REST API Framework
    if (mod == "rest") {
        environment->define("rest_router_create", std::string("__builtin_rest_router_create__"));
        environment->define("rest_router_generate_crud", std::string("__builtin_rest_router_generate_crud__"));
        environment->define("rest_router_add_custom", std::string("__builtin_rest_router_add_custom__"));
        environment->define("rest_router_require_auth", std::string("__builtin_rest_router_require_auth__"));
        environment->define("rest_router_route_count", std::string("__builtin_rest_router_route_count__"));
        environment->define("rest_router_route_table", std::string("__builtin_rest_router_route_table__"));
        environment->define("rest_router_get_prefix", std::string("__builtin_rest_router_get_prefix__"));
        environment->define("rest_validator_create", std::string("__builtin_rest_validator_create__"));
        environment->define("rest_validator_add_rule", std::string("__builtin_rest_validator_add_rule__"));
        environment->define("rest_validator_validate", std::string("__builtin_rest_validator_validate__"));
        environment->define("rest_validator_has_schema", std::string("__builtin_rest_validator_has_schema__"));
        environment->define("rest_validator_schema_count", std::string("__builtin_rest_validator_schema_count__"));
        environment->define("rest_formatter_create", std::string("__builtin_rest_formatter_create__"));
        environment->define("rest_formatter_success", std::string("__builtin_rest_formatter_success__"));
        environment->define("rest_formatter_created", std::string("__builtin_rest_formatter_created__"));
        environment->define("rest_formatter_error", std::string("__builtin_rest_formatter_error__"));
        environment->define("rest_formatter_not_found", std::string("__builtin_rest_formatter_not_found__"));
        environment->define("rest_formatter_unauthorized", std::string("__builtin_rest_formatter_unauthorized__"));
        environment->define("rest_formatter_forbidden", std::string("__builtin_rest_formatter_forbidden__"));
        environment->define("rest_formatter_validation_error", std::string("__builtin_rest_formatter_validation_error__"));
        environment->define("rest_formatter_paginated", std::string("__builtin_rest_formatter_paginated__"));
        environment->define("rest_formatter_pretty", std::string("__builtin_rest_formatter_pretty__"));
        environment->define("rest_docs_create", std::string("__builtin_rest_docs_create__"));
        environment->define("rest_docs_add_endpoint", std::string("__builtin_rest_docs_add_endpoint__"));
        environment->define("rest_docs_from_router", std::string("__builtin_rest_docs_from_router__"));
        environment->define("rest_docs_openapi", std::string("__builtin_rest_docs_openapi__"));
        environment->define("rest_docs_markdown", std::string("__builtin_rest_docs_markdown__"));
        environment->define("rest_docs_html", std::string("__builtin_rest_docs_html__"));
        environment->define("rest_docs_endpoint_count", std::string("__builtin_rest_docs_endpoint_count__"));
    }

    // Milestone 5: GraphQL Engine
    if (mod == "graphql") {
        environment->define("gql_schema_create", std::string("__builtin_gql_schema_create__"));
        environment->define("gql_schema_add_type", std::string("__builtin_gql_schema_add_type__"));
        environment->define("gql_schema_add_field", std::string("__builtin_gql_schema_add_field__"));
        environment->define("gql_schema_add_scalar", std::string("__builtin_gql_schema_add_scalar__"));
        environment->define("gql_schema_add_enum", std::string("__builtin_gql_schema_add_enum__"));
        environment->define("gql_schema_set_query", std::string("__builtin_gql_schema_set_query__"));
        environment->define("gql_schema_set_mutation", std::string("__builtin_gql_schema_set_mutation__"));
        environment->define("gql_schema_set_subscription", std::string("__builtin_gql_schema_set_subscription__"));
        environment->define("gql_schema_has_type", std::string("__builtin_gql_schema_has_type__"));
        environment->define("gql_schema_type_count", std::string("__builtin_gql_schema_type_count__"));
        environment->define("gql_schema_validate", std::string("__builtin_gql_schema_validate__"));
        environment->define("gql_schema_to_sdl", std::string("__builtin_gql_schema_to_sdl__"));
        environment->define("gql_schema_introspect", std::string("__builtin_gql_schema_introspect__"));
        environment->define("gql_query_engine_create", std::string("__builtin_gql_query_engine_create__"));
        environment->define("gql_query_engine_register_data", std::string("__builtin_gql_query_engine_register_data__"));
        environment->define("gql_query_execute", std::string("__builtin_gql_query_execute__"));
        environment->define("gql_query_complexity", std::string("__builtin_gql_query_complexity__"));
        environment->define("gql_query_resolver_count", std::string("__builtin_gql_query_resolver_count__"));
        environment->define("gql_mutation_engine_create", std::string("__builtin_gql_mutation_engine_create__"));
        environment->define("gql_mutation_register", std::string("__builtin_gql_mutation_register__"));
        environment->define("gql_mutation_execute", std::string("__builtin_gql_mutation_execute__"));
        environment->define("gql_mutation_count", std::string("__builtin_gql_mutation_count__"));
        environment->define("gql_subscription_manager_create", std::string("__builtin_gql_subscription_manager_create__"));
        environment->define("gql_subscribe", std::string("__builtin_gql_subscribe__"));
        environment->define("gql_unsubscribe", std::string("__builtin_gql_unsubscribe__"));
        environment->define("gql_unsubscribe_client", std::string("__builtin_gql_unsubscribe_client__"));
        environment->define("gql_publish", std::string("__builtin_gql_publish__"));
        environment->define("gql_subscription_count", std::string("__builtin_gql_subscription_count__"));
        environment->define("gql_client_subscription_count", std::string("__builtin_gql_client_subscription_count__"));
        environment->define("gql_is_subscribed", std::string("__builtin_gql_is_subscribed__"));
    }

    if (mod == "blockchain") {
        environment->define("bc_sha256",              std::string("__builtin_bc_sha256__"));
        environment->define("bc_sha256_double",       std::string("__builtin_bc_sha256_double__"));
        environment->define("bc_merkle_create",       std::string("__builtin_bc_merkle_create__"));
        environment->define("bc_merkle_add_leaf",     std::string("__builtin_bc_merkle_add_leaf__"));
        environment->define("bc_merkle_build",        std::string("__builtin_bc_merkle_build__"));
        environment->define("bc_merkle_root",         std::string("__builtin_bc_merkle_root__"));
        environment->define("bc_merkle_leaf_count",   std::string("__builtin_bc_merkle_leaf_count__"));
        environment->define("bc_tx_create",           std::string("__builtin_bc_tx_create__"));
        environment->define("bc_tx_id",               std::string("__builtin_bc_tx_id__"));
        environment->define("bc_tx_hash",             std::string("__builtin_bc_tx_hash__"));
        environment->define("bc_tx_to_string",        std::string("__builtin_bc_tx_to_string__"));
        environment->define("bc_block_create",        std::string("__builtin_bc_block_create__"));
        environment->define("bc_block_add_tx",        std::string("__builtin_bc_block_add_tx__"));
        environment->define("bc_block_mine",          std::string("__builtin_bc_block_mine__"));
        environment->define("bc_block_hash",          std::string("__builtin_bc_block_hash__"));
        environment->define("bc_block_prev_hash",     std::string("__builtin_bc_block_prev_hash__"));
        environment->define("bc_block_index",         std::string("__builtin_bc_block_index__"));
        environment->define("bc_block_nonce",         std::string("__builtin_bc_block_nonce__"));
        environment->define("bc_block_tx_count",      std::string("__builtin_bc_block_tx_count__"));
        environment->define("bc_block_is_valid",      std::string("__builtin_bc_block_is_valid__"));
        environment->define("bc_block_to_string",     std::string("__builtin_bc_block_to_string__"));
        environment->define("bc_block_merkle_root",   std::string("__builtin_bc_block_merkle_root__"));
        environment->define("bc_chain_create",        std::string("__builtin_bc_chain_create__"));
        environment->define("bc_chain_add_tx",        std::string("__builtin_bc_chain_add_tx__"));
        environment->define("bc_chain_mine_block",    std::string("__builtin_bc_chain_mine_block__"));
        environment->define("bc_chain_block_count",   std::string("__builtin_bc_chain_block_count__"));
        environment->define("bc_chain_pending_count", std::string("__builtin_bc_chain_pending_count__"));
        environment->define("bc_chain_is_valid",      std::string("__builtin_bc_chain_is_valid__"));
        environment->define("bc_chain_get_balance",   std::string("__builtin_bc_chain_get_balance__"));
        environment->define("bc_chain_last_hash",     std::string("__builtin_bc_chain_last_hash__"));
        environment->define("bc_chain_get_block_hash",std::string("__builtin_bc_chain_get_block_hash__"));
        environment->define("bc_chain_stats",         std::string("__builtin_bc_chain_stats__"));
        environment->define("bc_chain_add_validator", std::string("__builtin_bc_chain_add_validator__"));
        environment->define("bc_chain_select_validator",std::string("__builtin_bc_chain_select_validator__"));
        environment->define("bc_p2p_create",          std::string("__builtin_bc_p2p_create__"));
        environment->define("bc_p2p_add_node",        std::string("__builtin_bc_p2p_add_node__"));
        environment->define("bc_p2p_connect",         std::string("__builtin_bc_p2p_connect__"));
        environment->define("bc_p2p_disconnect",      std::string("__builtin_bc_p2p_disconnect__"));
        environment->define("bc_p2p_broadcast_block", std::string("__builtin_bc_p2p_broadcast_block__"));
        environment->define("bc_p2p_broadcast_tx",    std::string("__builtin_bc_p2p_broadcast_tx__"));
        environment->define("bc_p2p_node_count",      std::string("__builtin_bc_p2p_node_count__"));
        environment->define("bc_p2p_connected_count", std::string("__builtin_bc_p2p_connected_count__"));
        environment->define("bc_p2p_status",          std::string("__builtin_bc_p2p_status__"));
        return;
    }

    if (mod == "crypto") {
        environment->define("crypto_sha256",             std::string("__builtin_crypto_sha256__"));
        environment->define("crypto_sha512",             std::string("__builtin_crypto_sha512__"));
        environment->define("crypto_sha1",               std::string("__builtin_crypto_sha1__"));
        environment->define("crypto_md5",                std::string("__builtin_crypto_md5__"));
        environment->define("crypto_ripemd160",          std::string("__builtin_crypto_ripemd160__"));
        environment->define("crypto_hmac_sha256",        std::string("__builtin_crypto_hmac_sha256__"));
        environment->define("crypto_hmac_sha512",        std::string("__builtin_crypto_hmac_sha512__"));
        environment->define("crypto_to_base64",          std::string("__builtin_crypto_to_base64__"));
        environment->define("crypto_from_base64",        std::string("__builtin_crypto_from_base64__"));
        environment->define("crypto_keypair_generate",   std::string("__builtin_crypto_keypair_generate__"));
        environment->define("crypto_keypair_from_private",std::string("__builtin_crypto_keypair_from_private__"));
        environment->define("crypto_keypair_destroy",    std::string("__builtin_crypto_keypair_destroy__"));
        environment->define("crypto_keypair_private_pem",std::string("__builtin_crypto_keypair_private_pem__"));
        environment->define("crypto_keypair_public_pem", std::string("__builtin_crypto_keypair_public_pem__"));
        environment->define("crypto_keypair_private_hex",std::string("__builtin_crypto_keypair_private_hex__"));
        environment->define("crypto_keypair_public_hex", std::string("__builtin_crypto_keypair_public_hex__"));
        environment->define("crypto_keypair_curve",      std::string("__builtin_crypto_keypair_curve__"));
        environment->define("crypto_ecdsa_sign",         std::string("__builtin_crypto_ecdsa_sign__"));
        environment->define("crypto_ecdsa_verify",       std::string("__builtin_crypto_ecdsa_verify__"));
        environment->define("crypto_ecdsa_verify_pubhex",std::string("__builtin_crypto_ecdsa_verify_pubhex__"));
        environment->define("crypto_pubkey_to_eth_address",std::string("__builtin_crypto_pubkey_to_eth_address__"));
        environment->define("crypto_pubkey_to_btc_address",std::string("__builtin_crypto_pubkey_to_btc_address__"));
        environment->define("crypto_privkey_to_wif",     std::string("__builtin_crypto_privkey_to_wif__"));
        environment->define("crypto_wif_to_privkey",     std::string("__builtin_crypto_wif_to_privkey__"));
        environment->define("crypto_aes_encrypt",        std::string("__builtin_crypto_aes_encrypt__"));
        environment->define("crypto_aes_result_destroy", std::string("__builtin_crypto_aes_result_destroy__"));
        environment->define("crypto_aes_ciphertext",     std::string("__builtin_crypto_aes_ciphertext__"));
        environment->define("crypto_aes_iv",             std::string("__builtin_crypto_aes_iv__"));
        environment->define("crypto_aes_tag",            std::string("__builtin_crypto_aes_tag__"));
        environment->define("crypto_aes_decrypt",        std::string("__builtin_crypto_aes_decrypt__"));
        environment->define("crypto_pbkdf2",             std::string("__builtin_crypto_pbkdf2__"));
        environment->define("crypto_random_bytes",       std::string("__builtin_crypto_random_bytes__"));
        return;
    }

    if (mod == "smartcontract") {
        environment->define("sc_vm_create",            std::string("__builtin_sc_vm_create__"));
        environment->define("sc_vm_destroy",           std::string("__builtin_sc_vm_destroy__"));
        environment->define("sc_vm_set_storage",       std::string("__builtin_sc_vm_set_storage__"));
        environment->define("sc_vm_get_storage",       std::string("__builtin_sc_vm_get_storage__"));
        environment->define("sc_vm_set_balance",       std::string("__builtin_sc_vm_set_balance__"));
        environment->define("sc_vm_get_balance",       std::string("__builtin_sc_vm_get_balance__"));
        environment->define("sc_vm_state_dump",        std::string("__builtin_sc_vm_state_dump__"));
        environment->define("sc_bytecode_create",      std::string("__builtin_sc_bytecode_create__"));
        environment->define("sc_bytecode_destroy",     std::string("__builtin_sc_bytecode_destroy__"));
        environment->define("sc_bytecode_push_int",    std::string("__builtin_sc_bytecode_push_int__"));
        environment->define("sc_bytecode_push_str",    std::string("__builtin_sc_bytecode_push_str__"));
        environment->define("sc_bytecode_push_bool",   std::string("__builtin_sc_bytecode_push_bool__"));
        environment->define("sc_bytecode_push_double", std::string("__builtin_sc_bytecode_push_double__"));
        environment->define("sc_bytecode_emit",        std::string("__builtin_sc_bytecode_emit__"));
        environment->define("sc_bytecode_label",       std::string("__builtin_sc_bytecode_label__"));
        environment->define("sc_bytecode_jump",        std::string("__builtin_sc_bytecode_jump__"));
        environment->define("sc_bytecode_jumpi",       std::string("__builtin_sc_bytecode_jumpi__"));
        environment->define("sc_bytecode_size",        std::string("__builtin_sc_bytecode_size__"));
        environment->define("sc_ctx_create",           std::string("__builtin_sc_ctx_create__"));
        environment->define("sc_ctx_destroy",          std::string("__builtin_sc_ctx_destroy__"));
        environment->define("sc_ctx_set_block",        std::string("__builtin_sc_ctx_set_block__"));
        environment->define("sc_execute",              std::string("__builtin_sc_execute__"));
        environment->define("sc_result_destroy",       std::string("__builtin_sc_result_destroy__"));
        environment->define("sc_result_success",       std::string("__builtin_sc_result_success__"));
        environment->define("sc_result_return_value",  std::string("__builtin_sc_result_return_value__"));
        environment->define("sc_result_revert_reason", std::string("__builtin_sc_result_revert_reason__"));
        environment->define("sc_result_gas_used",      std::string("__builtin_sc_result_gas_used__"));
        environment->define("sc_result_event_count",   std::string("__builtin_sc_result_event_count__"));
        environment->define("sc_result_event_name",    std::string("__builtin_sc_result_event_name__"));
        environment->define("sc_result_event_data",    std::string("__builtin_sc_result_event_data__"));
        return;
    }

    if (mod == "dapp") {
        // Wallet
        environment->define("dapp_wallet_create",        std::string("__builtin_dapp_wallet_create__"));
        environment->define("dapp_wallet_from_privkey",  std::string("__builtin_dapp_wallet_from_privkey__"));
        environment->define("dapp_wallet_destroy",       std::string("__builtin_dapp_wallet_destroy__"));
        environment->define("dapp_wallet_address",       std::string("__builtin_dapp_wallet_address__"));
        environment->define("dapp_wallet_pubkey_hex",    std::string("__builtin_dapp_wallet_pubkey_hex__"));
        environment->define("dapp_wallet_privkey_hex",   std::string("__builtin_dapp_wallet_privkey_hex__"));
        environment->define("dapp_wallet_balance",       std::string("__builtin_dapp_wallet_balance__"));
        environment->define("dapp_wallet_network",       std::string("__builtin_dapp_wallet_network__"));
        environment->define("dapp_wallet_sign",          std::string("__builtin_dapp_wallet_sign__"));
        environment->define("dapp_wallet_verify",        std::string("__builtin_dapp_wallet_verify__"));
        environment->define("dapp_wallet_to_string",     std::string("__builtin_dapp_wallet_to_string__"));
        // Provider
        environment->define("dapp_provider_create",          std::string("__builtin_dapp_provider_create__"));
        environment->define("dapp_provider_destroy",         std::string("__builtin_dapp_provider_destroy__"));
        environment->define("dapp_provider_network",         std::string("__builtin_dapp_provider_network__"));
        environment->define("dapp_provider_chain_id",        std::string("__builtin_dapp_provider_chain_id__"));
        environment->define("dapp_provider_block_number",    std::string("__builtin_dapp_provider_block_number__"));
        environment->define("dapp_provider_mine_block",      std::string("__builtin_dapp_provider_mine_block__"));
        environment->define("dapp_provider_get_balance",     std::string("__builtin_dapp_provider_get_balance__"));
        environment->define("dapp_provider_set_balance",     std::string("__builtin_dapp_provider_set_balance__"));
        environment->define("dapp_provider_transfer",        std::string("__builtin_dapp_provider_transfer__"));
        environment->define("dapp_provider_status",          std::string("__builtin_dapp_provider_status__"));
        environment->define("dapp_provider_tx_count",        std::string("__builtin_dapp_provider_tx_count__"));
        environment->define("dapp_provider_get_tx",          std::string("__builtin_dapp_provider_get_tx__"));
        environment->define("dapp_provider_contract_count",  std::string("__builtin_dapp_provider_contract_count__"));
        // Contract deployment
        environment->define("dapp_deploy_contract",      std::string("__builtin_dapp_deploy_contract__"));
        environment->define("dapp_contract_destroy",     std::string("__builtin_dapp_contract_destroy__"));
        environment->define("dapp_contract_address",     std::string("__builtin_dapp_contract_address__"));
        environment->define("dapp_contract_name",        std::string("__builtin_dapp_contract_name__"));
        environment->define("dapp_contract_deployer",    std::string("__builtin_dapp_contract_deployer__"));
        environment->define("dapp_contract_deploy_block",std::string("__builtin_dapp_contract_deploy_block__"));
        environment->define("dapp_contract_to_string",   std::string("__builtin_dapp_contract_to_string__"));
        // Contract call / tx
        environment->define("dapp_call_contract",        std::string("__builtin_dapp_call_contract__"));
        environment->define("dapp_tx_destroy",           std::string("__builtin_dapp_tx_destroy__"));
        environment->define("dapp_tx_success",           std::string("__builtin_dapp_tx_success__"));
        environment->define("dapp_tx_hash",              std::string("__builtin_dapp_tx_hash__"));
        environment->define("dapp_tx_from",              std::string("__builtin_dapp_tx_from__"));
        environment->define("dapp_tx_to",                std::string("__builtin_dapp_tx_to__"));
        environment->define("dapp_tx_value",             std::string("__builtin_dapp_tx_value__"));
        environment->define("dapp_tx_gas_used",          std::string("__builtin_dapp_tx_gas_used__"));
        environment->define("dapp_tx_block",             std::string("__builtin_dapp_tx_block__"));
        environment->define("dapp_tx_revert_reason",     std::string("__builtin_dapp_tx_revert_reason__"));
        environment->define("dapp_tx_to_string",         std::string("__builtin_dapp_tx_to_string__"));
        // ABI
        environment->define("dapp_abi_encode_call",      std::string("__builtin_dapp_abi_encode_call__"));
        environment->define("dapp_abi_selector",         std::string("__builtin_dapp_abi_selector__"));
        return;
    }

    if (mod == "security") {
        // VulnScanner
        environment->define("sec_scanner_create",           std::string("__builtin_sec_scanner_create__"));
        environment->define("sec_scanner_destroy",          std::string("__builtin_sec_scanner_destroy__"));
        environment->define("sec_scanner_scan_host",        std::string("__builtin_sec_scanner_scan_host__"));
        environment->define("sec_scanner_vuln_db_size",     std::string("__builtin_sec_scanner_vuln_db_size__"));
        // ScanResult
        environment->define("sec_scanresult_destroy",       std::string("__builtin_sec_scanresult_destroy__"));
        environment->define("sec_scanresult_target",        std::string("__builtin_sec_scanresult_target__"));
        environment->define("sec_scanresult_reachable",     std::string("__builtin_sec_scanresult_reachable__"));
        environment->define("sec_scanresult_open_ports",    std::string("__builtin_sec_scanresult_open_ports__"));
        environment->define("sec_scanresult_vuln_count",    std::string("__builtin_sec_scanresult_vuln_count__"));
        environment->define("sec_scanresult_to_json",       std::string("__builtin_sec_scanresult_to_json__"));
        environment->define("sec_scanresult_port_service",  std::string("__builtin_sec_scanresult_port_service__"));
        environment->define("sec_scanresult_port_number",   std::string("__builtin_sec_scanresult_port_number__"));
        environment->define("sec_scanresult_vuln_name",     std::string("__builtin_sec_scanresult_vuln_name__"));
        environment->define("sec_scanresult_vuln_severity", std::string("__builtin_sec_scanresult_vuln_severity__"));
        environment->define("sec_scanresult_vuln_remediation", std::string("__builtin_sec_scanresult_vuln_remediation__"));
        // WebSecurityTester
        environment->define("sec_web_tester_create",        std::string("__builtin_sec_web_tester_create__"));
        environment->define("sec_web_tester_destroy",       std::string("__builtin_sec_web_tester_destroy__"));
        environment->define("sec_web_scan",                 std::string("__builtin_sec_web_scan__"));
        environment->define("sec_web_test_sqli",            std::string("__builtin_sec_web_test_sqli__"));
        environment->define("sec_web_test_xss",             std::string("__builtin_sec_web_test_xss__"));
        environment->define("sec_web_test_csrf",            std::string("__builtin_sec_web_test_csrf__"));
        environment->define("sec_web_test_auth_bypass",     std::string("__builtin_sec_web_test_auth_bypass__"));
        environment->define("sec_web_test_path_traversal",  std::string("__builtin_sec_web_test_path_traversal__"));
        environment->define("sec_web_encode_payload",       std::string("__builtin_sec_web_encode_payload__"));
        environment->define("sec_web_get_payloads",         std::string("__builtin_sec_web_get_payloads__"));
        // WebScanResult
        environment->define("sec_webscan_destroy",          std::string("__builtin_sec_webscan_destroy__"));
        environment->define("sec_webscan_finding_count",    std::string("__builtin_sec_webscan_finding_count__"));
        environment->define("sec_webscan_finding_name",     std::string("__builtin_sec_webscan_finding_name__"));
        environment->define("sec_webscan_finding_severity", std::string("__builtin_sec_webscan_finding_severity__"));
        environment->define("sec_webscan_finding_param",    std::string("__builtin_sec_webscan_finding_param__"));
        environment->define("sec_webscan_finding_payload",  std::string("__builtin_sec_webscan_finding_payload__"));
        environment->define("sec_webscan_to_json",          std::string("__builtin_sec_webscan_to_json__"));
        // MalwareAnalyzer
        environment->define("sec_malware_analyzer_create",  std::string("__builtin_sec_malware_analyzer_create__"));
        environment->define("sec_malware_analyzer_destroy", std::string("__builtin_sec_malware_analyzer_destroy__"));
        environment->define("sec_malware_static_analysis",  std::string("__builtin_sec_malware_static_analysis__"));
        environment->define("sec_malware_dynamic_analysis", std::string("__builtin_sec_malware_dynamic_analysis__"));
        environment->define("sec_malware_add_yara_rule",    std::string("__builtin_sec_malware_add_yara_rule__"));
        // StaticAnalysisResult
        environment->define("sec_static_result_destroy",       std::string("__builtin_sec_static_result_destroy__"));
        environment->define("sec_static_result_threat_level",  std::string("__builtin_sec_static_result_threat_level__"));
        environment->define("sec_static_result_file_type",     std::string("__builtin_sec_static_result_file_type__"));
        environment->define("sec_static_result_sha256",        std::string("__builtin_sec_static_result_sha256__"));
        environment->define("sec_static_result_md5",           std::string("__builtin_sec_static_result_md5__"));
        environment->define("sec_static_result_entropy",       std::string("__builtin_sec_static_result_entropy__"));
        environment->define("sec_static_result_string_count",  std::string("__builtin_sec_static_result_string_count__"));
        environment->define("sec_static_result_string_value",  std::string("__builtin_sec_static_result_string_value__"));
        environment->define("sec_static_result_string_category", std::string("__builtin_sec_static_result_string_category__"));
        environment->define("sec_static_result_suspicious_count", std::string("__builtin_sec_static_result_suspicious_count__"));
        environment->define("sec_static_result_suspicious",    std::string("__builtin_sec_static_result_suspicious__"));
        environment->define("sec_static_result_to_json",       std::string("__builtin_sec_static_result_to_json__"));
        // DynamicAnalysisResult
        environment->define("sec_dynamic_result_destroy",      std::string("__builtin_sec_dynamic_result_destroy__"));
        environment->define("sec_dynamic_result_threat_level", std::string("__builtin_sec_dynamic_result_threat_level__"));
        environment->define("sec_dynamic_result_event_count",  std::string("__builtin_sec_dynamic_result_event_count__"));
        environment->define("sec_dynamic_result_event_type",   std::string("__builtin_sec_dynamic_result_event_type__"));
        environment->define("sec_dynamic_result_event_target", std::string("__builtin_sec_dynamic_result_event_target__"));
        environment->define("sec_dynamic_result_ioc_count",    std::string("__builtin_sec_dynamic_result_ioc_count__"));
        environment->define("sec_dynamic_result_ioc",          std::string("__builtin_sec_dynamic_result_ioc__"));
        environment->define("sec_dynamic_result_to_json",      std::string("__builtin_sec_dynamic_result_to_json__"));
        // EducationalPlatform
        environment->define("sec_edu_platform_create",         std::string("__builtin_sec_edu_platform_create__"));
        environment->define("sec_edu_platform_destroy",        std::string("__builtin_sec_edu_platform_destroy__"));
        environment->define("sec_edu_challenge_count",         std::string("__builtin_sec_edu_challenge_count__"));
        environment->define("sec_edu_challenge_id",            std::string("__builtin_sec_edu_challenge_id__"));
        environment->define("sec_edu_challenge_name",          std::string("__builtin_sec_edu_challenge_name__"));
        environment->define("sec_edu_challenge_category",      std::string("__builtin_sec_edu_challenge_category__"));
        environment->define("sec_edu_challenge_points",        std::string("__builtin_sec_edu_challenge_points__"));
        environment->define("sec_edu_challenge_description",   std::string("__builtin_sec_edu_challenge_description__"));
        environment->define("sec_edu_challenge_hint",          std::string("__builtin_sec_edu_challenge_hint__"));
        environment->define("sec_edu_create_session",          std::string("__builtin_sec_edu_create_session__"));
        environment->define("sec_edu_session_destroy",         std::string("__builtin_sec_edu_session_destroy__"));
        environment->define("sec_edu_session_submit",          std::string("__builtin_sec_edu_session_submit__"));
        environment->define("sec_edu_session_score",           std::string("__builtin_sec_edu_session_score__"));
        environment->define("sec_edu_session_player",          std::string("__builtin_sec_edu_session_player__"));
        environment->define("sec_edu_leaderboard",             std::string("__builtin_sec_edu_leaderboard__"));
        environment->define("sec_edu_get_tutorial",            std::string("__builtin_sec_edu_get_tutorial__"));
        return;
    }

    if (mod == "mobile") {
        // AppConfig
        environment->define("mobile_app_config_create",          std::string("__builtin_mobile_app_config_create__"));
        environment->define("mobile_app_config_destroy",         std::string("__builtin_mobile_app_config_destroy__"));
        environment->define("mobile_app_config_set_orientation", std::string("__builtin_mobile_app_config_set_orientation__"));
        environment->define("mobile_app_config_add_permission",  std::string("__builtin_mobile_app_config_add_permission__"));
        // MobileApp
        environment->define("mobile_app_create",                 std::string("__builtin_mobile_app_create__"));
        environment->define("mobile_app_destroy",                std::string("__builtin_mobile_app_destroy__"));
        environment->define("mobile_app_screen_count",           std::string("__builtin_mobile_app_screen_count__"));
        environment->define("mobile_app_platform_name",          std::string("__builtin_mobile_app_platform_name__"));
        environment->define("mobile_app_build_ios",              std::string("__builtin_mobile_app_build_ios__"));
        environment->define("mobile_app_build_android",          std::string("__builtin_mobile_app_build_android__"));
        environment->define("mobile_app_export_manifest",        std::string("__builtin_mobile_app_export_manifest__"));
        environment->define("mobile_app_add_screen",             std::string("__builtin_mobile_app_add_screen__"));
        // Screen
        environment->define("mobile_screen_create",              std::string("__builtin_mobile_screen_create__"));
        environment->define("mobile_screen_destroy",             std::string("__builtin_mobile_screen_destroy__"));
        environment->define("mobile_screen_set_background",      std::string("__builtin_mobile_screen_set_background__"));
        environment->define("mobile_screen_set_nav_bar",         std::string("__builtin_mobile_screen_set_nav_bar__"));
        environment->define("mobile_screen_to_json",             std::string("__builtin_mobile_screen_to_json__"));
        environment->define("mobile_screen_set_layout",          std::string("__builtin_mobile_screen_set_layout__"));
        // UIComponent
        environment->define("mobile_component_create",           std::string("__builtin_mobile_component_create__"));
        environment->define("mobile_component_destroy",          std::string("__builtin_mobile_component_destroy__"));
        environment->define("mobile_component_set_text",         std::string("__builtin_mobile_component_set_text__"));
        environment->define("mobile_component_set_visible",      std::string("__builtin_mobile_component_set_visible__"));
        environment->define("mobile_component_set_enabled",      std::string("__builtin_mobile_component_set_enabled__"));
        environment->define("mobile_component_set_value",        std::string("__builtin_mobile_component_set_value__"));
        environment->define("mobile_component_set_prop",         std::string("__builtin_mobile_component_set_prop__"));
        environment->define("mobile_component_set_style_color",  std::string("__builtin_mobile_component_set_style_color__"));
        environment->define("mobile_component_set_style_font",   std::string("__builtin_mobile_component_set_style_font__"));
        environment->define("mobile_component_set_style_size",   std::string("__builtin_mobile_component_set_style_size__"));
        environment->define("mobile_component_set_style_padding",std::string("__builtin_mobile_component_set_style_padding__"));
        environment->define("mobile_component_add_child",        std::string("__builtin_mobile_component_add_child__"));
        environment->define("mobile_component_to_json",          std::string("__builtin_mobile_component_to_json__"));
        environment->define("mobile_component_type_name",        std::string("__builtin_mobile_component_type_name__"));
        environment->define("mobile_component_get_text",         std::string("__builtin_mobile_component_get_text__"));
        environment->define("mobile_component_get_value",        std::string("__builtin_mobile_component_get_value__"));
        // UILayout
        environment->define("mobile_layout_create",              std::string("__builtin_mobile_layout_create__"));
        environment->define("mobile_layout_destroy",             std::string("__builtin_mobile_layout_destroy__"));
        environment->define("mobile_layout_set_spacing",         std::string("__builtin_mobile_layout_set_spacing__"));
        environment->define("mobile_layout_set_columns",         std::string("__builtin_mobile_layout_set_columns__"));
        environment->define("mobile_layout_add_item",            std::string("__builtin_mobile_layout_add_item__"));
        environment->define("mobile_layout_to_json",             std::string("__builtin_mobile_layout_to_json__"));
        // Navigation
        environment->define("mobile_nav_create",                 std::string("__builtin_mobile_nav_create__"));
        environment->define("mobile_nav_destroy",                std::string("__builtin_mobile_nav_destroy__"));
        environment->define("mobile_nav_push",                   std::string("__builtin_mobile_nav_push__"));
        environment->define("mobile_nav_pop",                    std::string("__builtin_mobile_nav_pop__"));
        environment->define("mobile_nav_current",                std::string("__builtin_mobile_nav_current__"));
        environment->define("mobile_nav_depth",                  std::string("__builtin_mobile_nav_depth__"));
        // DeviceAPI
        environment->define("mobile_device_create",              std::string("__builtin_mobile_device_create__"));
        environment->define("mobile_device_destroy",             std::string("__builtin_mobile_device_destroy__"));
        environment->define("mobile_device_capture_photo",       std::string("__builtin_mobile_device_capture_photo__"));
        environment->define("mobile_device_record_video",        std::string("__builtin_mobile_device_record_video__"));
        environment->define("mobile_device_get_location",        std::string("__builtin_mobile_device_get_location__"));
        environment->define("mobile_location_destroy",           std::string("__builtin_mobile_location_destroy__"));
        environment->define("mobile_location_latitude",          std::string("__builtin_mobile_location_latitude__"));
        environment->define("mobile_location_longitude",         std::string("__builtin_mobile_location_longitude__"));
        environment->define("mobile_location_altitude",          std::string("__builtin_mobile_location_altitude__"));
        environment->define("mobile_location_accuracy",          std::string("__builtin_mobile_location_accuracy__"));
        environment->define("mobile_device_get_accelerometer",   std::string("__builtin_mobile_device_get_accelerometer__"));
        environment->define("mobile_accel_destroy",              std::string("__builtin_mobile_accel_destroy__"));
        environment->define("mobile_accel_x",                    std::string("__builtin_mobile_accel_x__"));
        environment->define("mobile_accel_y",                    std::string("__builtin_mobile_accel_y__"));
        environment->define("mobile_accel_z",                    std::string("__builtin_mobile_accel_z__"));
        environment->define("mobile_device_storage_set",         std::string("__builtin_mobile_device_storage_set__"));
        environment->define("mobile_device_storage_get",         std::string("__builtin_mobile_device_storage_get__"));
        environment->define("mobile_device_storage_delete",      std::string("__builtin_mobile_device_storage_delete__"));
        environment->define("mobile_device_is_connected",        std::string("__builtin_mobile_device_is_connected__"));
        environment->define("mobile_device_network_type",        std::string("__builtin_mobile_device_network_type__"));
        environment->define("mobile_device_model",               std::string("__builtin_mobile_device_model__"));
        environment->define("mobile_device_os_version",          std::string("__builtin_mobile_device_os_version__"));
        environment->define("mobile_device_id",                  std::string("__builtin_mobile_device_id__"));
        environment->define("mobile_device_battery_level",       std::string("__builtin_mobile_device_battery_level__"));
        environment->define("mobile_device_is_charging",         std::string("__builtin_mobile_device_is_charging__"));
        environment->define("mobile_device_check_permission",    std::string("__builtin_mobile_device_check_permission__"));
        environment->define("mobile_device_request_permission",  std::string("__builtin_mobile_device_request_permission__"));
        environment->define("mobile_device_schedule_notification",std::string("__builtin_mobile_device_schedule_notification__"));
        environment->define("mobile_device_cancel_notification", std::string("__builtin_mobile_device_cancel_notification__"));
        environment->define("mobile_device_pending_notifications",std::string("__builtin_mobile_device_pending_notifications__"));
        return;
    }

    if (mod == "game") {
        // Phase 1: Graphics Engine
        environment->define("game_renderer_create",       std::string("__builtin_game_renderer_create__"));
        environment->define("game_renderer_destroy",      std::string("__builtin_game_renderer_destroy__"));
        environment->define("game_renderer_clear",        std::string("__builtin_game_renderer_clear__"));
        environment->define("game_renderer_present",      std::string("__builtin_game_renderer_present__"));
        environment->define("game_renderer_width",        std::string("__builtin_game_renderer_width__"));
        environment->define("game_renderer_height",       std::string("__builtin_game_renderer_height__"));
        environment->define("game_renderer_title",        std::string("__builtin_game_renderer_title__"));
        environment->define("game_texture_create",        std::string("__builtin_game_texture_create__"));
        environment->define("game_texture_load",          std::string("__builtin_game_texture_load__"));
        environment->define("game_texture_destroy",       std::string("__builtin_game_texture_destroy__"));
        environment->define("game_texture_width",         std::string("__builtin_game_texture_width__"));
        environment->define("game_texture_height",        std::string("__builtin_game_texture_height__"));
        environment->define("game_texture_path",          std::string("__builtin_game_texture_path__"));
        environment->define("game_sprite_create",         std::string("__builtin_game_sprite_create__"));
        environment->define("game_sprite_destroy",        std::string("__builtin_game_sprite_destroy__"));
        environment->define("game_sprite_set_position",   std::string("__builtin_game_sprite_set_position__"));
        environment->define("game_sprite_set_scale",      std::string("__builtin_game_sprite_set_scale__"));
        environment->define("game_sprite_set_rotation",   std::string("__builtin_game_sprite_set_rotation__"));
        environment->define("game_sprite_set_color",      std::string("__builtin_game_sprite_set_color__"));
        environment->define("game_sprite_set_visible",    std::string("__builtin_game_sprite_set_visible__"));
        environment->define("game_sprite_x",              std::string("__builtin_game_sprite_x__"));
        environment->define("game_sprite_y",              std::string("__builtin_game_sprite_y__"));
        environment->define("game_sprite_scale_x",        std::string("__builtin_game_sprite_scale_x__"));
        environment->define("game_sprite_scale_y",        std::string("__builtin_game_sprite_scale_y__"));
        environment->define("game_sprite_rotation",       std::string("__builtin_game_sprite_rotation__"));
        environment->define("game_sprite_to_string",      std::string("__builtin_game_sprite_to_string__"));
        environment->define("game_shader_create",         std::string("__builtin_game_shader_create__"));
        environment->define("game_shader_destroy",        std::string("__builtin_game_shader_destroy__"));
        environment->define("game_shader_set_float",      std::string("__builtin_game_shader_set_float__"));
        environment->define("game_shader_set_int",        std::string("__builtin_game_shader_set_int__"));
        environment->define("game_shader_set_vec2",       std::string("__builtin_game_shader_set_vec2__"));
        environment->define("game_shader_set_vec3",       std::string("__builtin_game_shader_set_vec3__"));
        environment->define("game_shader_set_vec4",       std::string("__builtin_game_shader_set_vec4__"));
        environment->define("game_shader_name",           std::string("__builtin_game_shader_name__"));
        environment->define("game_shader_uniform_count",  std::string("__builtin_game_shader_uniform_count__"));
        environment->define("game_shader_uniform_name",   std::string("__builtin_game_shader_uniform_name__"));
        environment->define("game_camera2d_create",       std::string("__builtin_game_camera2d_create__"));
        environment->define("game_camera2d_destroy",      std::string("__builtin_game_camera2d_destroy__"));
        environment->define("game_camera2d_set_position", std::string("__builtin_game_camera2d_set_position__"));
        environment->define("game_camera2d_set_zoom",     std::string("__builtin_game_camera2d_set_zoom__"));
        environment->define("game_camera2d_move",         std::string("__builtin_game_camera2d_move__"));
        environment->define("game_camera2d_x",            std::string("__builtin_game_camera2d_x__"));
        environment->define("game_camera2d_y",            std::string("__builtin_game_camera2d_y__"));
        environment->define("game_camera2d_zoom",         std::string("__builtin_game_camera2d_zoom__"));
        environment->define("game_camera3d_create",       std::string("__builtin_game_camera3d_create__"));
        environment->define("game_camera3d_destroy",      std::string("__builtin_game_camera3d_destroy__"));
        environment->define("game_camera3d_set_position", std::string("__builtin_game_camera3d_set_position__"));
        environment->define("game_camera3d_set_target",   std::string("__builtin_game_camera3d_set_target__"));
        environment->define("game_camera3d_set_fov",      std::string("__builtin_game_camera3d_set_fov__"));
        environment->define("game_camera3d_pos_x",        std::string("__builtin_game_camera3d_pos_x__"));
        environment->define("game_camera3d_pos_y",        std::string("__builtin_game_camera3d_pos_y__"));
        environment->define("game_camera3d_pos_z",        std::string("__builtin_game_camera3d_pos_z__"));
        environment->define("game_camera3d_fov",          std::string("__builtin_game_camera3d_fov__"));
        // Phase 2: Physics Engine
        environment->define("game_physics_world_create",  std::string("__builtin_game_physics_world_create__"));
        environment->define("game_physics_world_destroy", std::string("__builtin_game_physics_world_destroy__"));
        environment->define("game_physics_world_step",    std::string("__builtin_game_physics_world_step__"));
        environment->define("game_physics_world_set_gravity", std::string("__builtin_game_physics_world_set_gravity__"));
        environment->define("game_physics_world_body_count", std::string("__builtin_game_physics_world_body_count__"));
        environment->define("game_rigidbody_create",      std::string("__builtin_game_rigidbody_create__"));
        environment->define("game_rigidbody_destroy",     std::string("__builtin_game_rigidbody_destroy__"));
        environment->define("game_rigidbody_set_position",std::string("__builtin_game_rigidbody_set_position__"));
        environment->define("game_rigidbody_set_velocity",std::string("__builtin_game_rigidbody_set_velocity__"));
        environment->define("game_rigidbody_apply_force", std::string("__builtin_game_rigidbody_apply_force__"));
        environment->define("game_rigidbody_apply_impulse",std::string("__builtin_game_rigidbody_apply_impulse__"));
        environment->define("game_rigidbody_set_restitution",std::string("__builtin_game_rigidbody_set_restitution__"));
        environment->define("game_rigidbody_set_friction",std::string("__builtin_game_rigidbody_set_friction__"));
        environment->define("game_rigidbody_x",           std::string("__builtin_game_rigidbody_x__"));
        environment->define("game_rigidbody_y",           std::string("__builtin_game_rigidbody_y__"));
        environment->define("game_rigidbody_vx",          std::string("__builtin_game_rigidbody_vx__"));
        environment->define("game_rigidbody_vy",          std::string("__builtin_game_rigidbody_vy__"));
        environment->define("game_rigidbody_mass",        std::string("__builtin_game_rigidbody_mass__"));
        environment->define("game_rigidbody_is_static",   std::string("__builtin_game_rigidbody_is_static__"));
        environment->define("game_rigidbody_to_string",   std::string("__builtin_game_rigidbody_to_string__"));
        environment->define("game_collider_box",          std::string("__builtin_game_collider_box__"));
        environment->define("game_collider_circle",       std::string("__builtin_game_collider_circle__"));
        environment->define("game_collider_destroy",      std::string("__builtin_game_collider_destroy__"));
        environment->define("game_collider_check",        std::string("__builtin_game_collider_check__"));
        environment->define("game_collider_type",         std::string("__builtin_game_collider_type__"));
        environment->define("game_particles_create",      std::string("__builtin_game_particles_create__"));
        environment->define("game_particles_destroy",     std::string("__builtin_game_particles_destroy__"));
        environment->define("game_particles_emit",        std::string("__builtin_game_particles_emit__"));
        environment->define("game_particles_update",      std::string("__builtin_game_particles_update__"));
        environment->define("game_particles_set_velocity",std::string("__builtin_game_particles_set_velocity__"));
        environment->define("game_particles_set_lifetime",std::string("__builtin_game_particles_set_lifetime__"));
        environment->define("game_particles_set_color",   std::string("__builtin_game_particles_set_color__"));
        environment->define("game_particles_set_size",    std::string("__builtin_game_particles_set_size__"));
        environment->define("game_particles_active_count",std::string("__builtin_game_particles_active_count__"));
        environment->define("game_particles_max_count",   std::string("__builtin_game_particles_max_count__"));
        environment->define("game_particles_to_string",   std::string("__builtin_game_particles_to_string__"));
        // Phase 3: Audio System
        environment->define("game_audio_engine_create",   std::string("__builtin_game_audio_engine_create__"));
        environment->define("game_audio_engine_destroy",  std::string("__builtin_game_audio_engine_destroy__"));
        environment->define("game_audio_engine_set_master_volume", std::string("__builtin_game_audio_engine_set_master_volume__"));
        environment->define("game_audio_engine_master_volume", std::string("__builtin_game_audio_engine_master_volume__"));
        environment->define("game_audio_engine_channel_count", std::string("__builtin_game_audio_engine_channel_count__"));
        environment->define("game_sound_load",            std::string("__builtin_game_sound_load__"));
        environment->define("game_sound_create",          std::string("__builtin_game_sound_create__"));
        environment->define("game_sound_destroy",         std::string("__builtin_game_sound_destroy__"));
        environment->define("game_sound_play",            std::string("__builtin_game_sound_play__"));
        environment->define("game_sound_stop",            std::string("__builtin_game_sound_stop__"));
        environment->define("game_sound_pause",           std::string("__builtin_game_sound_pause__"));
        environment->define("game_sound_resume",          std::string("__builtin_game_sound_resume__"));
        environment->define("game_sound_set_volume",      std::string("__builtin_game_sound_set_volume__"));
        environment->define("game_sound_set_pitch",       std::string("__builtin_game_sound_set_pitch__"));
        environment->define("game_sound_set_loop",        std::string("__builtin_game_sound_set_loop__"));
        environment->define("game_sound_is_playing",      std::string("__builtin_game_sound_is_playing__"));
        environment->define("game_sound_volume",          std::string("__builtin_game_sound_volume__"));
        environment->define("game_sound_pitch",           std::string("__builtin_game_sound_pitch__"));
        environment->define("game_sound_name",            std::string("__builtin_game_sound_name__"));
        environment->define("game_sound_type",            std::string("__builtin_game_sound_type__"));
        environment->define("game_music_load",            std::string("__builtin_game_music_load__"));
        environment->define("game_music_create",          std::string("__builtin_game_music_create__"));
        environment->define("game_music_destroy",         std::string("__builtin_game_music_destroy__"));
        environment->define("game_music_play",            std::string("__builtin_game_music_play__"));
        environment->define("game_music_stop",            std::string("__builtin_game_music_stop__"));
        environment->define("game_music_pause",           std::string("__builtin_game_music_pause__"));
        environment->define("game_music_resume",          std::string("__builtin_game_music_resume__"));
        environment->define("game_music_set_volume",      std::string("__builtin_game_music_set_volume__"));
        environment->define("game_music_set_loop",        std::string("__builtin_game_music_set_loop__"));
        environment->define("game_music_is_playing",      std::string("__builtin_game_music_is_playing__"));
        environment->define("game_music_name",            std::string("__builtin_game_music_name__"));
        environment->define("game_audio3d_create",        std::string("__builtin_game_audio3d_create__"));
        environment->define("game_audio3d_destroy",       std::string("__builtin_game_audio3d_destroy__"));
        environment->define("game_audio3d_set_listener",  std::string("__builtin_game_audio3d_set_listener__"));
        environment->define("game_audio3d_play_at",       std::string("__builtin_game_audio3d_play_at__"));
        environment->define("game_audio3d_set_rolloff",   std::string("__builtin_game_audio3d_set_rolloff__"));
        environment->define("game_mixer_create",          std::string("__builtin_game_mixer_create__"));
        environment->define("game_mixer_destroy",         std::string("__builtin_game_mixer_destroy__"));
        environment->define("game_mixer_set_channel_volume", std::string("__builtin_game_mixer_set_channel_volume__"));
        environment->define("game_mixer_channel_volume",  std::string("__builtin_game_mixer_channel_volume__"));
        environment->define("game_mixer_play_on_channel", std::string("__builtin_game_mixer_play_on_channel__"));
        environment->define("game_mixer_channels",        std::string("__builtin_game_mixer_channels__"));
        // Phase 4: Game Framework
        environment->define("game_loop_create",           std::string("__builtin_game_loop_create__"));
        environment->define("game_loop_destroy",          std::string("__builtin_game_loop_destroy__"));
        environment->define("game_loop_start",            std::string("__builtin_game_loop_start__"));
        environment->define("game_loop_stop",             std::string("__builtin_game_loop_stop__"));
        environment->define("game_loop_tick",             std::string("__builtin_game_loop_tick__"));
        environment->define("game_loop_is_running",       std::string("__builtin_game_loop_is_running__"));
        environment->define("game_loop_delta_time",       std::string("__builtin_game_loop_delta_time__"));
        environment->define("game_loop_fps",              std::string("__builtin_game_loop_fps__"));
        environment->define("game_loop_target_fps",       std::string("__builtin_game_loop_target_fps__"));
        environment->define("game_loop_frame_count",      std::string("__builtin_game_loop_frame_count__"));
        environment->define("game_loop_elapsed",          std::string("__builtin_game_loop_elapsed__"));
        environment->define("game_scene_create",          std::string("__builtin_game_scene_create__"));
        environment->define("game_scene_destroy",         std::string("__builtin_game_scene_destroy__"));
        environment->define("game_scene_add_sprite",      std::string("__builtin_game_scene_add_sprite__"));
        environment->define("game_scene_add_body",        std::string("__builtin_game_scene_add_body__"));
        environment->define("game_scene_update",          std::string("__builtin_game_scene_update__"));
        environment->define("game_scene_sprite_count",    std::string("__builtin_game_scene_sprite_count__"));
        environment->define("game_scene_body_count",      std::string("__builtin_game_scene_body_count__"));
        environment->define("game_scene_name",            std::string("__builtin_game_scene_name__"));
        environment->define("game_scene_manager_create",  std::string("__builtin_game_scene_manager_create__"));
        environment->define("game_scene_manager_destroy", std::string("__builtin_game_scene_manager_destroy__"));
        environment->define("game_scene_manager_push",    std::string("__builtin_game_scene_manager_push__"));
        environment->define("game_scene_manager_pop",     std::string("__builtin_game_scene_manager_pop__"));
        environment->define("game_scene_manager_current", std::string("__builtin_game_scene_manager_current__"));
        environment->define("game_scene_manager_depth",   std::string("__builtin_game_scene_manager_depth__"));
        environment->define("game_input_create",          std::string("__builtin_game_input_create__"));
        environment->define("game_input_destroy",         std::string("__builtin_game_input_destroy__"));
        environment->define("game_input_key_press",       std::string("__builtin_game_input_key_press__"));
        environment->define("game_input_key_release",     std::string("__builtin_game_input_key_release__"));
        environment->define("game_input_mouse_move",      std::string("__builtin_game_input_mouse_move__"));
        environment->define("game_input_mouse_press",     std::string("__builtin_game_input_mouse_press__"));
        environment->define("game_input_mouse_release",   std::string("__builtin_game_input_mouse_release__"));
        environment->define("game_input_is_key_down",     std::string("__builtin_game_input_is_key_down__"));
        environment->define("game_input_is_key_pressed",  std::string("__builtin_game_input_is_key_pressed__"));
        environment->define("game_input_is_mouse_down",   std::string("__builtin_game_input_is_mouse_down__"));
        environment->define("game_input_mouse_x",         std::string("__builtin_game_input_mouse_x__"));
        environment->define("game_input_mouse_y",         std::string("__builtin_game_input_mouse_y__"));
        environment->define("game_input_update",          std::string("__builtin_game_input_update__"));
        environment->define("game_assets_create",         std::string("__builtin_game_assets_create__"));
        environment->define("game_assets_destroy",        std::string("__builtin_game_assets_destroy__"));
        environment->define("game_assets_register",       std::string("__builtin_game_assets_register__"));
        environment->define("game_assets_get_path",       std::string("__builtin_game_assets_get_path__"));
        environment->define("game_assets_get_type",       std::string("__builtin_game_assets_get_type__"));
        environment->define("game_assets_count",          std::string("__builtin_game_assets_count__"));
        environment->define("game_assets_name_at",        std::string("__builtin_game_assets_name_at__"));
        environment->define("game_assets_to_json",        std::string("__builtin_game_assets_to_json__"));
        return;
    }

    if (mod == "system") {
        // Phase 1: Inline Assembly
        environment->define("asm_context_create",       std::string("__builtin_asm_context_create__"));
        environment->define("asm_context_destroy",      std::string("__builtin_asm_context_destroy__"));
        environment->define("asm_context_arch",         std::string("__builtin_asm_context_arch__"));
        environment->define("asm_assemble",             std::string("__builtin_asm_assemble__"));
        environment->define("asm_disassemble",          std::string("__builtin_asm_disassemble__"));
        environment->define("asm_execute",              std::string("__builtin_asm_execute__"));
        environment->define("asm_set_reg",              std::string("__builtin_asm_set_reg__"));
        environment->define("asm_get_reg",              std::string("__builtin_asm_get_reg__"));
        environment->define("asm_dump_regs",            std::string("__builtin_asm_dump_regs__"));
        environment->define("asm_instruction_count",    std::string("__builtin_asm_instruction_count__"));
        environment->define("asm_instruction_at",       std::string("__builtin_asm_instruction_at__"));
        environment->define("asm_optimize",             std::string("__builtin_asm_optimize__"));
        // Phase 2: GPIO & Hardware
        environment->define("gpio_controller_create",   std::string("__builtin_gpio_controller_create__"));
        environment->define("gpio_controller_destroy",  std::string("__builtin_gpio_controller_destroy__"));
        environment->define("gpio_controller_board",    std::string("__builtin_gpio_controller_board__"));
        environment->define("gpio_controller_pin_count",std::string("__builtin_gpio_controller_pin_count__"));
        environment->define("gpio_pin_create",          std::string("__builtin_gpio_pin_create__"));
        environment->define("gpio_pin_destroy",         std::string("__builtin_gpio_pin_destroy__"));
        environment->define("gpio_pin_write",           std::string("__builtin_gpio_pin_write__"));
        environment->define("gpio_pin_read",            std::string("__builtin_gpio_pin_read__"));
        environment->define("gpio_pin_number",          std::string("__builtin_gpio_pin_number__"));
        environment->define("gpio_pin_mode",            std::string("__builtin_gpio_pin_mode__"));
        environment->define("gpio_pin_set_pull",        std::string("__builtin_gpio_pin_set_pull__"));
        environment->define("gpio_pin_to_string",       std::string("__builtin_gpio_pin_to_string__"));
        environment->define("pwm_create",               std::string("__builtin_pwm_create__"));
        environment->define("pwm_destroy",              std::string("__builtin_pwm_destroy__"));
        environment->define("pwm_set_frequency",        std::string("__builtin_pwm_set_frequency__"));
        environment->define("pwm_set_duty_cycle",       std::string("__builtin_pwm_set_duty_cycle__"));
        environment->define("pwm_start",                std::string("__builtin_pwm_start__"));
        environment->define("pwm_stop",                 std::string("__builtin_pwm_stop__"));
        environment->define("pwm_frequency",            std::string("__builtin_pwm_frequency__"));
        environment->define("pwm_duty_cycle",           std::string("__builtin_pwm_duty_cycle__"));
        environment->define("pwm_is_running",           std::string("__builtin_pwm_is_running__"));
        environment->define("adc_create",               std::string("__builtin_adc_create__"));
        environment->define("adc_destroy",              std::string("__builtin_adc_destroy__"));
        environment->define("adc_read_raw",             std::string("__builtin_adc_read_raw__"));
        environment->define("adc_read_voltage",         std::string("__builtin_adc_read_voltage__"));
        environment->define("adc_resolution",           std::string("__builtin_adc_resolution__"));
        environment->define("adc_channel",              std::string("__builtin_adc_channel__"));
        environment->define("dac_create",               std::string("__builtin_dac_create__"));
        environment->define("dac_destroy",              std::string("__builtin_dac_destroy__"));
        environment->define("dac_write_raw",            std::string("__builtin_dac_write_raw__"));
        environment->define("dac_write_voltage",        std::string("__builtin_dac_write_voltage__"));
        environment->define("dac_resolution",           std::string("__builtin_dac_resolution__"));
        environment->define("dac_channel",              std::string("__builtin_dac_channel__"));
        environment->define("spi_bus_create",           std::string("__builtin_spi_bus_create__"));
        environment->define("spi_bus_destroy",          std::string("__builtin_spi_bus_destroy__"));
        environment->define("spi_transfer",             std::string("__builtin_spi_transfer__"));
        environment->define("spi_set_clock",            std::string("__builtin_spi_set_clock__"));
        environment->define("spi_clock",                std::string("__builtin_spi_clock__"));
        environment->define("spi_bus_num",              std::string("__builtin_spi_bus_num__"));
        environment->define("i2c_bus_create",           std::string("__builtin_i2c_bus_create__"));
        environment->define("i2c_bus_destroy",          std::string("__builtin_i2c_bus_destroy__"));
        environment->define("i2c_write",                std::string("__builtin_i2c_write__"));
        environment->define("i2c_read",                 std::string("__builtin_i2c_read__"));
        environment->define("i2c_scan",                 std::string("__builtin_i2c_scan__"));
        environment->define("i2c_bus_num",              std::string("__builtin_i2c_bus_num__"));
        environment->define("uart_create",              std::string("__builtin_uart_create__"));
        environment->define("uart_destroy",             std::string("__builtin_uart_destroy__"));
        environment->define("uart_write",               std::string("__builtin_uart_write__"));
        environment->define("uart_read",                std::string("__builtin_uart_read__"));
        environment->define("uart_set_baud",            std::string("__builtin_uart_set_baud__"));
        environment->define("uart_baud",                std::string("__builtin_uart_baud__"));
        environment->define("uart_port",                std::string("__builtin_uart_port__"));
        environment->define("uart_bytes_available",     std::string("__builtin_uart_bytes_available__"));
        // Phase 3: RTOS
        environment->define("rtos_kernel_create",       std::string("__builtin_rtos_kernel_create__"));
        environment->define("rtos_kernel_destroy",      std::string("__builtin_rtos_kernel_destroy__"));
        environment->define("rtos_kernel_start",        std::string("__builtin_rtos_kernel_start__"));
        environment->define("rtos_kernel_stop",         std::string("__builtin_rtos_kernel_stop__"));
        environment->define("rtos_kernel_is_running",   std::string("__builtin_rtos_kernel_is_running__"));
        environment->define("rtos_kernel_name",         std::string("__builtin_rtos_kernel_name__"));
        environment->define("rtos_kernel_tick",         std::string("__builtin_rtos_kernel_tick__"));
        environment->define("rtos_kernel_task_count",   std::string("__builtin_rtos_kernel_task_count__"));
        environment->define("rtos_task_create",         std::string("__builtin_rtos_task_create__"));
        environment->define("rtos_task_destroy",        std::string("__builtin_rtos_task_destroy__"));
        environment->define("rtos_task_start",          std::string("__builtin_rtos_task_start__"));
        environment->define("rtos_task_suspend",        std::string("__builtin_rtos_task_suspend__"));
        environment->define("rtos_task_resume",         std::string("__builtin_rtos_task_resume__"));
        environment->define("rtos_task_delete",         std::string("__builtin_rtos_task_delete__"));
        environment->define("rtos_task_name",           std::string("__builtin_rtos_task_name__"));
        environment->define("rtos_task_priority",       std::string("__builtin_rtos_task_priority__"));
        environment->define("rtos_task_state",          std::string("__builtin_rtos_task_state__"));
        environment->define("rtos_task_stack_size",     std::string("__builtin_rtos_task_stack_size__"));
        environment->define("rtos_semaphore_create",    std::string("__builtin_rtos_semaphore_create__"));
        environment->define("rtos_semaphore_destroy",   std::string("__builtin_rtos_semaphore_destroy__"));
        environment->define("rtos_semaphore_take",      std::string("__builtin_rtos_semaphore_take__"));
        environment->define("rtos_semaphore_give",      std::string("__builtin_rtos_semaphore_give__"));
        environment->define("rtos_semaphore_count",     std::string("__builtin_rtos_semaphore_count__"));
        environment->define("rtos_mutex_create",        std::string("__builtin_rtos_mutex_create__"));
        environment->define("rtos_mutex_destroy",       std::string("__builtin_rtos_mutex_destroy__"));
        environment->define("rtos_mutex_lock",          std::string("__builtin_rtos_mutex_lock__"));
        environment->define("rtos_mutex_unlock",        std::string("__builtin_rtos_mutex_unlock__"));
        environment->define("rtos_mutex_is_locked",     std::string("__builtin_rtos_mutex_is_locked__"));
        environment->define("rtos_queue_create",        std::string("__builtin_rtos_queue_create__"));
        environment->define("rtos_queue_destroy",       std::string("__builtin_rtos_queue_destroy__"));
        environment->define("rtos_queue_send",          std::string("__builtin_rtos_queue_send__"));
        environment->define("rtos_queue_receive",       std::string("__builtin_rtos_queue_receive__"));
        environment->define("rtos_queue_size",          std::string("__builtin_rtos_queue_size__"));
        environment->define("rtos_queue_capacity",      std::string("__builtin_rtos_queue_capacity__"));
        environment->define("rtos_queue_is_full",       std::string("__builtin_rtos_queue_is_full__"));
        environment->define("rtos_queue_is_empty",      std::string("__builtin_rtos_queue_is_empty__"));
        environment->define("rtos_timer_create",        std::string("__builtin_rtos_timer_create__"));
        environment->define("rtos_timer_destroy",       std::string("__builtin_rtos_timer_destroy__"));
        environment->define("rtos_timer_start",         std::string("__builtin_rtos_timer_start__"));
        environment->define("rtos_timer_stop",          std::string("__builtin_rtos_timer_stop__"));
        environment->define("rtos_timer_reset",         std::string("__builtin_rtos_timer_reset__"));
        environment->define("rtos_timer_is_active",     std::string("__builtin_rtos_timer_is_active__"));
        environment->define("rtos_timer_period",        std::string("__builtin_rtos_timer_period__"));
        environment->define("rtos_timer_fire_count",    std::string("__builtin_rtos_timer_fire_count__"));
        environment->define("rtos_timer_name",          std::string("__builtin_rtos_timer_name__"));
        // Phase 4: Bootloader & Kernel
        environment->define("mem_map_create",           std::string("__builtin_mem_map_create__"));
        environment->define("mem_map_destroy",          std::string("__builtin_mem_map_destroy__"));
        environment->define("mem_map_add_region",       std::string("__builtin_mem_map_add_region__"));
        environment->define("mem_map_region_count",     std::string("__builtin_mem_map_region_count__"));
        environment->define("mem_map_region_name",      std::string("__builtin_mem_map_region_name__"));
        environment->define("mem_map_region_addr",      std::string("__builtin_mem_map_region_addr__"));
        environment->define("mem_map_region_size",      std::string("__builtin_mem_map_region_size__"));
        environment->define("mem_map_region_type",      std::string("__builtin_mem_map_region_type__"));
        environment->define("mem_map_to_string",        std::string("__builtin_mem_map_to_string__"));
        environment->define("interrupt_table_create",   std::string("__builtin_interrupt_table_create__"));
        environment->define("interrupt_table_destroy",  std::string("__builtin_interrupt_table_destroy__"));
        environment->define("interrupt_table_register", std::string("__builtin_interrupt_table_register__"));
        environment->define("interrupt_table_fire",     std::string("__builtin_interrupt_table_fire__"));
        environment->define("interrupt_table_handler",  std::string("__builtin_interrupt_table_handler__"));
        environment->define("interrupt_table_name",     std::string("__builtin_interrupt_table_name__"));
        environment->define("interrupt_table_fire_count",std::string("__builtin_interrupt_table_fire_count__"));
        environment->define("interrupt_table_size",     std::string("__builtin_interrupt_table_size__"));
        environment->define("process_table_create",     std::string("__builtin_process_table_create__"));
        environment->define("process_table_destroy",    std::string("__builtin_process_table_destroy__"));
        environment->define("process_table_create_process",std::string("__builtin_process_table_create_process__"));
        environment->define("process_table_kill",       std::string("__builtin_process_table_kill__"));
        environment->define("process_table_name",       std::string("__builtin_process_table_name__"));
        environment->define("process_table_priority",   std::string("__builtin_process_table_priority__"));
        environment->define("process_table_state",      std::string("__builtin_process_table_state__"));
        environment->define("process_table_count",      std::string("__builtin_process_table_count__"));
        environment->define("process_table_schedule",   std::string("__builtin_process_table_schedule__"));
        environment->define("process_table_to_string",  std::string("__builtin_process_table_to_string__"));
        environment->define("kernel_create",            std::string("__builtin_os_kernel_create__"));
        environment->define("kernel_destroy",           std::string("__builtin_os_kernel_destroy__"));
        environment->define("kernel_boot",              std::string("__builtin_kernel_boot__"));
        environment->define("kernel_panic",             std::string("__builtin_kernel_panic__"));
        environment->define("kernel_name",              std::string("__builtin_kernel_name__"));
        environment->define("kernel_arch",              std::string("__builtin_kernel_arch__"));
        environment->define("kernel_state",             std::string("__builtin_kernel_state__"));
        environment->define("kernel_uptime_ms",         std::string("__builtin_kernel_uptime_ms__"));
        environment->define("kernel_syscall",           std::string("__builtin_kernel_syscall__"));
        environment->define("kernel_log",               std::string("__builtin_kernel_log__"));
        environment->define("kernel_log_count",         std::string("__builtin_kernel_log_count__"));
        return;
    }
    if (mod == "gui") {
        // Phase 1: Text Rendering
        environment->define("gui_font_manager_create",    std::string("__builtin_gui_font_manager_create__"));
        environment->define("gui_font_manager_destroy",   std::string("__builtin_gui_font_manager_destroy__"));
        environment->define("gui_font_load",              std::string("__builtin_gui_font_load__"));
        environment->define("gui_font_manager_count",     std::string("__builtin_gui_font_manager_count__"));
        environment->define("gui_font_name",              std::string("__builtin_gui_font_name__"));
        environment->define("gui_font_size",              std::string("__builtin_gui_font_size__"));
        environment->define("gui_font_style",             std::string("__builtin_gui_font_style__"));
        environment->define("gui_text_renderer_create",   std::string("__builtin_gui_text_renderer_create__"));
        environment->define("gui_text_renderer_destroy",  std::string("__builtin_gui_text_renderer_destroy__"));
        environment->define("gui_text_renderer_set_font", std::string("__builtin_gui_text_renderer_set_font__"));
        environment->define("gui_text_renderer_set_color",std::string("__builtin_gui_text_renderer_set_color__"));
        environment->define("gui_text_renderer_set_align",std::string("__builtin_gui_text_renderer_set_align__"));
        environment->define("gui_text_renderer_set_antialias", std::string("__builtin_gui_text_renderer_set_antialias__"));
        environment->define("gui_text_renderer_render",   std::string("__builtin_gui_text_renderer_render__"));
        environment->define("gui_text_renderer_line_count",std::string("__builtin_gui_text_renderer_line_count__"));
        environment->define("gui_text_renderer_line_at",  std::string("__builtin_gui_text_renderer_line_at__"));
        environment->define("gui_text_metrics_measure",   std::string("__builtin_gui_text_metrics_measure__"));
        environment->define("gui_text_metrics_destroy",   std::string("__builtin_gui_text_metrics_destroy__"));
        environment->define("gui_text_metrics_width",     std::string("__builtin_gui_text_metrics_width__"));
        environment->define("gui_text_metrics_height",    std::string("__builtin_gui_text_metrics_height__"));
        environment->define("gui_text_metrics_ascent",    std::string("__builtin_gui_text_metrics_ascent__"));
        environment->define("gui_text_metrics_descent",   std::string("__builtin_gui_text_metrics_descent__"));
        environment->define("gui_text_metrics_line_height",std::string("__builtin_gui_text_metrics_line_height__"));
        // Phase 2: Mouse Support
        environment->define("gui_mouse_handler_create",   std::string("__builtin_gui_mouse_handler_create__"));
        environment->define("gui_mouse_handler_destroy",  std::string("__builtin_gui_mouse_handler_destroy__"));
        environment->define("gui_mouse_handler_click",    std::string("__builtin_gui_mouse_handler_click__"));
        environment->define("gui_mouse_handler_move",     std::string("__builtin_gui_mouse_handler_move__"));
        environment->define("gui_mouse_handler_press",    std::string("__builtin_gui_mouse_handler_press__"));
        environment->define("gui_mouse_handler_release",  std::string("__builtin_gui_mouse_handler_release__"));
        environment->define("gui_mouse_handler_scroll",   std::string("__builtin_gui_mouse_handler_scroll__"));
        environment->define("gui_mouse_handler_event_count",  std::string("__builtin_gui_mouse_handler_event_count__"));
        environment->define("gui_mouse_handler_event_type",   std::string("__builtin_gui_mouse_handler_event_type__"));
        environment->define("gui_mouse_handler_event_x",      std::string("__builtin_gui_mouse_handler_event_x__"));
        environment->define("gui_mouse_handler_event_y",      std::string("__builtin_gui_mouse_handler_event_y__"));
        environment->define("gui_mouse_handler_event_button", std::string("__builtin_gui_mouse_handler_event_button__"));
        environment->define("gui_mouse_handler_cursor_x",     std::string("__builtin_gui_mouse_handler_cursor_x__"));
        environment->define("gui_mouse_handler_cursor_y",     std::string("__builtin_gui_mouse_handler_cursor_y__"));
        environment->define("gui_mouse_handler_is_button_down",std::string("__builtin_gui_mouse_handler_is_button_down__"));
        environment->define("gui_drag_drop_create",       std::string("__builtin_gui_drag_drop_create__"));
        environment->define("gui_drag_drop_destroy",      std::string("__builtin_gui_drag_drop_destroy__"));
        environment->define("gui_drag_drop_start",        std::string("__builtin_gui_drag_drop_start__"));
        environment->define("gui_drag_drop_move",         std::string("__builtin_gui_drag_drop_move__"));
        environment->define("gui_drag_drop_drop",         std::string("__builtin_gui_drag_drop_drop__"));
        environment->define("gui_drag_drop_is_dragging",  std::string("__builtin_gui_drag_drop_is_dragging__"));
        environment->define("gui_drag_drop_data",         std::string("__builtin_gui_drag_drop_data__"));
        environment->define("gui_drag_drop_mime",         std::string("__builtin_gui_drag_drop_mime__"));
        environment->define("gui_drag_drop_start_x",      std::string("__builtin_gui_drag_drop_start_x__"));
        environment->define("gui_drag_drop_start_y",      std::string("__builtin_gui_drag_drop_start_y__"));
        environment->define("gui_context_menu_create",    std::string("__builtin_gui_context_menu_create__"));
        environment->define("gui_context_menu_destroy",   std::string("__builtin_gui_context_menu_destroy__"));
        environment->define("gui_context_menu_add_item",  std::string("__builtin_gui_context_menu_add_item__"));
        environment->define("gui_context_menu_add_separator", std::string("__builtin_gui_context_menu_add_separator__"));
        environment->define("gui_context_menu_show",      std::string("__builtin_gui_context_menu_show__"));
        environment->define("gui_context_menu_hide",      std::string("__builtin_gui_context_menu_hide__"));
        environment->define("gui_context_menu_item_count",std::string("__builtin_gui_context_menu_item_count__"));
        environment->define("gui_context_menu_item_label",std::string("__builtin_gui_context_menu_item_label__"));
        environment->define("gui_context_menu_item_action",std::string("__builtin_gui_context_menu_item_action__"));
        environment->define("gui_context_menu_is_visible",std::string("__builtin_gui_context_menu_is_visible__"));
        environment->define("gui_context_menu_x",         std::string("__builtin_gui_context_menu_x__"));
        environment->define("gui_context_menu_y",         std::string("__builtin_gui_context_menu_y__"));
        // Phase 3: Advanced Widgets
        environment->define("gui_text_input_create",      std::string("__builtin_gui_text_input_create__"));
        environment->define("gui_text_input_destroy",     std::string("__builtin_gui_text_input_destroy__"));
        environment->define("gui_text_input_set_text",    std::string("__builtin_gui_text_input_set_text__"));
        environment->define("gui_text_input_set_placeholder", std::string("__builtin_gui_text_input_set_placeholder__"));
        environment->define("gui_text_input_set_max_length",  std::string("__builtin_gui_text_input_set_max_length__"));
        environment->define("gui_text_input_set_password",    std::string("__builtin_gui_text_input_set_password__"));
        environment->define("gui_text_input_set_multiline",   std::string("__builtin_gui_text_input_set_multiline__"));
        environment->define("gui_text_input_insert",      std::string("__builtin_gui_text_input_insert__"));
        environment->define("gui_text_input_clear",       std::string("__builtin_gui_text_input_clear__"));
        environment->define("gui_text_input_text",        std::string("__builtin_gui_text_input_text__"));
        environment->define("gui_text_input_length",      std::string("__builtin_gui_text_input_length__"));
        environment->define("gui_text_input_cursor_pos",  std::string("__builtin_gui_text_input_cursor_pos__"));
        environment->define("gui_text_input_is_focused",  std::string("__builtin_gui_text_input_is_focused__"));
        environment->define("gui_text_input_focus",       std::string("__builtin_gui_text_input_focus__"));
        environment->define("gui_text_input_blur",        std::string("__builtin_gui_text_input_blur__"));
        environment->define("gui_dropdown_create",        std::string("__builtin_gui_dropdown_create__"));
        environment->define("gui_dropdown_destroy",       std::string("__builtin_gui_dropdown_destroy__"));
        environment->define("gui_dropdown_add_option",    std::string("__builtin_gui_dropdown_add_option__"));
        environment->define("gui_dropdown_set_selected",  std::string("__builtin_gui_dropdown_set_selected__"));
        environment->define("gui_dropdown_set_placeholder",std::string("__builtin_gui_dropdown_set_placeholder__"));
        environment->define("gui_dropdown_open",          std::string("__builtin_gui_dropdown_open__"));
        environment->define("gui_dropdown_close",         std::string("__builtin_gui_dropdown_close__"));
        environment->define("gui_dropdown_option_count",  std::string("__builtin_gui_dropdown_option_count__"));
        environment->define("gui_dropdown_option_label",  std::string("__builtin_gui_dropdown_option_label__"));
        environment->define("gui_dropdown_option_value",  std::string("__builtin_gui_dropdown_option_value__"));
        environment->define("gui_dropdown_selected_index",std::string("__builtin_gui_dropdown_selected_index__"));
        environment->define("gui_dropdown_selected_value",std::string("__builtin_gui_dropdown_selected_value__"));
        environment->define("gui_dropdown_is_open",       std::string("__builtin_gui_dropdown_is_open__"));
        environment->define("gui_table_create",           std::string("__builtin_gui_table_create__"));
        environment->define("gui_table_destroy",          std::string("__builtin_gui_table_destroy__"));
        environment->define("gui_table_add_column",       std::string("__builtin_gui_table_add_column__"));
        environment->define("gui_table_add_row",          std::string("__builtin_gui_table_add_row__"));
        environment->define("gui_table_set_cell",         std::string("__builtin_gui_table_set_cell__"));
        environment->define("gui_table_get_cell",         std::string("__builtin_gui_table_get_cell__"));
        environment->define("gui_table_row_count",        std::string("__builtin_gui_table_row_count__"));
        environment->define("gui_table_col_count",        std::string("__builtin_gui_table_col_count__"));
        environment->define("gui_table_column_header",    std::string("__builtin_gui_table_column_header__"));
        environment->define("gui_table_column_width",     std::string("__builtin_gui_table_column_width__"));
        environment->define("gui_table_set_selected_row", std::string("__builtin_gui_table_set_selected_row__"));
        environment->define("gui_table_selected_row",     std::string("__builtin_gui_table_selected_row__"));
        environment->define("gui_table_sort_by_column",   std::string("__builtin_gui_table_sort_by_column__"));
        environment->define("gui_table_to_csv",           std::string("__builtin_gui_table_to_csv__"));
        environment->define("gui_tree_view_create",       std::string("__builtin_gui_tree_view_create__"));
        environment->define("gui_tree_view_destroy",      std::string("__builtin_gui_tree_view_destroy__"));
        environment->define("gui_tree_view_add_node",     std::string("__builtin_gui_tree_view_add_node__"));
        environment->define("gui_tree_view_expand",       std::string("__builtin_gui_tree_view_expand__"));
        environment->define("gui_tree_view_collapse",     std::string("__builtin_gui_tree_view_collapse__"));
        environment->define("gui_tree_view_select",       std::string("__builtin_gui_tree_view_select__"));
        environment->define("gui_tree_view_node_count",   std::string("__builtin_gui_tree_view_node_count__"));
        environment->define("gui_tree_view_node_label",   std::string("__builtin_gui_tree_view_node_label__"));
        environment->define("gui_tree_view_node_data",    std::string("__builtin_gui_tree_view_node_data__"));
        environment->define("gui_tree_view_node_parent",  std::string("__builtin_gui_tree_view_node_parent__"));
        environment->define("gui_tree_view_node_is_expanded", std::string("__builtin_gui_tree_view_node_is_expanded__"));
        environment->define("gui_tree_view_selected_node",std::string("__builtin_gui_tree_view_selected_node__"));
        environment->define("gui_tree_view_child_count",  std::string("__builtin_gui_tree_view_child_count__"));
        environment->define("gui_tabs_create",            std::string("__builtin_gui_tabs_create__"));
        environment->define("gui_tabs_destroy",           std::string("__builtin_gui_tabs_destroy__"));
        environment->define("gui_tabs_add_tab",           std::string("__builtin_gui_tabs_add_tab__"));
        environment->define("gui_tabs_set_active",        std::string("__builtin_gui_tabs_set_active__"));
        environment->define("gui_tabs_remove_tab",        std::string("__builtin_gui_tabs_remove_tab__"));
        environment->define("gui_tabs_count",             std::string("__builtin_gui_tabs_count__"));
        environment->define("gui_tabs_label",             std::string("__builtin_gui_tabs_label__"));
        environment->define("gui_tabs_content_id",        std::string("__builtin_gui_tabs_content_id__"));
        environment->define("gui_tabs_active_index",      std::string("__builtin_gui_tabs_active_index__"));
        environment->define("gui_tabs_active_label",      std::string("__builtin_gui_tabs_active_label__"));
        // Phase 4: Layout Management
        environment->define("gui_flex_layout_create",     std::string("__builtin_gui_flex_layout_create__"));
        environment->define("gui_flex_layout_destroy",    std::string("__builtin_gui_flex_layout_destroy__"));
        environment->define("gui_flex_layout_add_item",   std::string("__builtin_gui_flex_layout_add_item__"));
        environment->define("gui_flex_layout_set_gap",    std::string("__builtin_gui_flex_layout_set_gap__"));
        environment->define("gui_flex_layout_set_padding",std::string("__builtin_gui_flex_layout_set_padding__"));
        environment->define("gui_flex_layout_set_wrap",   std::string("__builtin_gui_flex_layout_set_wrap__"));
        environment->define("gui_flex_layout_set_align",  std::string("__builtin_gui_flex_layout_set_align__"));
        environment->define("gui_flex_layout_set_justify",std::string("__builtin_gui_flex_layout_set_justify__"));
        environment->define("gui_flex_layout_item_count", std::string("__builtin_gui_flex_layout_item_count__"));
        environment->define("gui_flex_layout_item_id",    std::string("__builtin_gui_flex_layout_item_id__"));
        environment->define("gui_flex_layout_item_flex",  std::string("__builtin_gui_flex_layout_item_flex__"));
        environment->define("gui_flex_layout_compute",    std::string("__builtin_gui_flex_layout_compute__"));
        environment->define("gui_flex_layout_direction",  std::string("__builtin_gui_flex_layout_direction__"));
        environment->define("gui_grid_layout_create",     std::string("__builtin_gui_grid_layout_create__"));
        environment->define("gui_grid_layout_destroy",    std::string("__builtin_gui_grid_layout_destroy__"));
        environment->define("gui_grid_layout_place",      std::string("__builtin_gui_grid_layout_place__"));
        environment->define("gui_grid_layout_set_col_width",  std::string("__builtin_gui_grid_layout_set_col_width__"));
        environment->define("gui_grid_layout_set_row_height", std::string("__builtin_gui_grid_layout_set_row_height__"));
        environment->define("gui_grid_layout_set_gap",    std::string("__builtin_gui_grid_layout_set_gap__"));
        environment->define("gui_grid_layout_cols",       std::string("__builtin_gui_grid_layout_cols__"));
        environment->define("gui_grid_layout_rows",       std::string("__builtin_gui_grid_layout_rows__"));
        environment->define("gui_grid_layout_item_count", std::string("__builtin_gui_grid_layout_item_count__"));
        environment->define("gui_grid_layout_item_id",    std::string("__builtin_gui_grid_layout_item_id__"));
        environment->define("gui_grid_layout_compute",    std::string("__builtin_gui_grid_layout_compute__"));
        environment->define("gui_anchor_layout_create",   std::string("__builtin_gui_anchor_layout_create__"));
        environment->define("gui_anchor_layout_destroy",  std::string("__builtin_gui_anchor_layout_destroy__"));
        environment->define("gui_anchor_layout_add_item", std::string("__builtin_gui_anchor_layout_add_item__"));
        environment->define("gui_anchor_layout_set_anchor",   std::string("__builtin_gui_anchor_layout_set_anchor__"));
        environment->define("gui_anchor_layout_set_relative", std::string("__builtin_gui_anchor_layout_set_relative__"));
        environment->define("gui_anchor_layout_item_count",   std::string("__builtin_gui_anchor_layout_item_count__"));
        environment->define("gui_anchor_layout_item_id",      std::string("__builtin_gui_anchor_layout_item_id__"));
        environment->define("gui_anchor_layout_compute",      std::string("__builtin_gui_anchor_layout_compute__"));
        // Window system
        environment->define("gui_init",                    std::string("__builtin_gui_init__"));
        environment->define("gui_quit",                    std::string("__builtin_gui_quit__"));
        environment->define("gui_has_display",             std::string("__builtin_gui_has_display__"));
        environment->define("gui_window_create",           std::string("__builtin_gui_window_create__"));
        environment->define("gui_window_destroy",          std::string("__builtin_gui_window_destroy__"));
        environment->define("gui_window_show",             std::string("__builtin_gui_window_show__"));
        environment->define("gui_window_hide",             std::string("__builtin_gui_window_hide__"));
        environment->define("gui_window_set_title",        std::string("__builtin_gui_window_set_title__"));
        environment->define("gui_window_title",            std::string("__builtin_gui_window_title__"));
        environment->define("gui_window_width",            std::string("__builtin_gui_window_width__"));
        environment->define("gui_window_height",           std::string("__builtin_gui_window_height__"));
        environment->define("gui_window_is_open",          std::string("__builtin_gui_window_is_open__"));
        environment->define("gui_window_close",            std::string("__builtin_gui_window_close__"));
        environment->define("gui_window_resize",           std::string("__builtin_gui_window_resize__"));
        environment->define("gui_window_set_resizable",    std::string("__builtin_gui_window_set_resizable__"));
        environment->define("gui_window_set_fullscreen",   std::string("__builtin_gui_window_set_fullscreen__"));
        environment->define("gui_window_is_fullscreen",    std::string("__builtin_gui_window_is_fullscreen__"));
        environment->define("gui_window_poll",             std::string("__builtin_gui_window_poll__"));
        environment->define("gui_key_down",                std::string("__builtin_gui_key_down__"));
        environment->define("gui_key_pressed",             std::string("__builtin_gui_key_pressed__"));
        environment->define("gui_key",                     std::string("__builtin_gui_key__"));
        environment->define("gui_mouse_x",                 std::string("__builtin_gui_mouse_x__"));
        environment->define("gui_mouse_y",                 std::string("__builtin_gui_mouse_y__"));
        environment->define("gui_mouse_button",            std::string("__builtin_gui_mouse_button__"));
        environment->define("gui_scroll_y",                std::string("__builtin_gui_scroll_y__"));
        environment->define("gui_mouse_dx",                std::string("__builtin_gui_mouse_dx__"));
        environment->define("gui_mouse_dy",                std::string("__builtin_gui_mouse_dy__"));
        environment->define("gui_clear",                   std::string("__builtin_gui_clear__"));
        environment->define("gui_present",                 std::string("__builtin_gui_present__"));
        environment->define("gui_set_color",               std::string("__builtin_gui_set_color__"));
        environment->define("gui_draw_point",              std::string("__builtin_gui_draw_point__"));
        environment->define("gui_draw_line",               std::string("__builtin_gui_draw_line__"));
        environment->define("gui_draw_rect",               std::string("__builtin_gui_draw_rect__"));
        environment->define("gui_fill_rect",               std::string("__builtin_gui_fill_rect__"));
        environment->define("gui_draw_circle",             std::string("__builtin_gui_draw_circle__"));
        environment->define("gui_fill_circle",             std::string("__builtin_gui_fill_circle__"));
        environment->define("gui_draw_triangle",           std::string("__builtin_gui_draw_triangle__"));
        environment->define("gui_fill_triangle",           std::string("__builtin_gui_fill_triangle__"));
        environment->define("gui_draw_text",               std::string("__builtin_gui_draw_text__"));
        environment->define("gui_text_width",              std::string("__builtin_gui_text_width__"));
        environment->define("gui_text_height",             std::string("__builtin_gui_text_height__"));
        environment->define("gui_image_load",              std::string("__builtin_gui_image_load__"));
        environment->define("gui_image_destroy",           std::string("__builtin_gui_image_destroy__"));
        environment->define("gui_image_draw",              std::string("__builtin_gui_image_draw__"));
        environment->define("gui_image_draw_scaled",       std::string("__builtin_gui_image_draw_scaled__"));
        environment->define("gui_image_width",             std::string("__builtin_gui_image_width__"));
        environment->define("gui_image_height",            std::string("__builtin_gui_image_height__"));
        environment->define("gui_delay",                   std::string("__builtin_gui_delay__"));
        environment->define("gui_ticks",                   std::string("__builtin_gui_ticks__"));
        environment->define("gui_delta_time",              std::string("__builtin_gui_delta_time__"));
        return;
    }

    if (mod == "network") {
        // Phase 1: TCP sockets
        environment->define("net_tcp_socket_create",      std::string("__builtin_net_tcp_socket_create__"));
        environment->define("net_tcp_socket_destroy",     std::string("__builtin_net_tcp_socket_destroy__"));
        environment->define("net_tcp_connect",            std::string("__builtin_net_tcp_connect__"));
        environment->define("net_tcp_bind",               std::string("__builtin_net_tcp_bind__"));
        environment->define("net_tcp_listen",             std::string("__builtin_net_tcp_listen__"));
        environment->define("net_tcp_accept",             std::string("__builtin_net_tcp_accept__"));
        environment->define("net_tcp_send",               std::string("__builtin_net_tcp_send__"));
        environment->define("net_tcp_recv",               std::string("__builtin_net_tcp_recv__"));
        environment->define("net_tcp_recv_data",          std::string("__builtin_net_tcp_recv_data__"));
        environment->define("net_tcp_close",              std::string("__builtin_net_tcp_close__"));
        environment->define("net_tcp_is_connected",       std::string("__builtin_net_tcp_is_connected__"));
        environment->define("net_tcp_remote_addr",        std::string("__builtin_net_tcp_remote_addr__"));
        environment->define("net_tcp_remote_port",        std::string("__builtin_net_tcp_remote_port__"));
        environment->define("net_tcp_set_nonblocking",    std::string("__builtin_net_tcp_set_nonblocking__"));
        environment->define("net_tcp_set_timeout",        std::string("__builtin_net_tcp_set_timeout__"));
        environment->define("net_tcp_bytes_available",    std::string("__builtin_net_tcp_bytes_available__"));
        // UDP sockets
        environment->define("net_udp_socket_create",      std::string("__builtin_net_udp_socket_create__"));
        environment->define("net_udp_socket_destroy",     std::string("__builtin_net_udp_socket_destroy__"));
        environment->define("net_udp_bind",               std::string("__builtin_net_udp_bind__"));
        environment->define("net_udp_send_to",            std::string("__builtin_net_udp_send_to__"));
        environment->define("net_udp_recv_from",          std::string("__builtin_net_udp_recv_from__"));
        environment->define("net_udp_recv_data",          std::string("__builtin_net_udp_recv_data__"));
        environment->define("net_udp_recv_addr",          std::string("__builtin_net_udp_recv_addr__"));
        environment->define("net_udp_recv_port",          std::string("__builtin_net_udp_recv_port__"));
        environment->define("net_udp_close",              std::string("__builtin_net_udp_close__"));
        environment->define("net_udp_set_broadcast",      std::string("__builtin_net_udp_set_broadcast__"));
        environment->define("net_udp_set_timeout",        std::string("__builtin_net_udp_set_timeout__"));
        // Address helpers
        environment->define("net_resolve_host",           std::string("__builtin_net_resolve_host__"));
        environment->define("net_local_ip",               std::string("__builtin_net_local_ip__"));
        environment->define("net_hostname",               std::string("__builtin_net_hostname__"));
        // Phase 2: DNS
        environment->define("net_dns_create",             std::string("__builtin_net_dns_create__"));
        environment->define("net_dns_destroy",            std::string("__builtin_net_dns_destroy__"));
        environment->define("net_dns_lookup",             std::string("__builtin_net_dns_lookup__"));
        environment->define("net_dns_reverse",            std::string("__builtin_net_dns_reverse__"));
        environment->define("net_dns_lookup_count",       std::string("__builtin_net_dns_lookup_count__"));
        environment->define("net_dns_lookup_result",      std::string("__builtin_net_dns_lookup_result__"));
        environment->define("net_dns_set_server",         std::string("__builtin_net_dns_set_server__"));
        environment->define("net_dns_server",             std::string("__builtin_net_dns_server__"));
        // SMTP
        environment->define("net_smtp_create",            std::string("__builtin_net_smtp_create__"));
        environment->define("net_smtp_destroy",           std::string("__builtin_net_smtp_destroy__"));
        environment->define("net_smtp_connect",           std::string("__builtin_net_smtp_connect__"));
        environment->define("net_smtp_auth",              std::string("__builtin_net_smtp_auth__"));
        environment->define("net_smtp_send_mail",         std::string("__builtin_net_smtp_send_mail__"));
        environment->define("net_smtp_disconnect",        std::string("__builtin_net_smtp_disconnect__"));
        environment->define("net_smtp_is_connected",      std::string("__builtin_net_smtp_is_connected__"));
        environment->define("net_smtp_last_response",     std::string("__builtin_net_smtp_last_response__"));
        environment->define("net_smtp_set_tls",           std::string("__builtin_net_smtp_set_tls__"));
        // FTP
        environment->define("net_ftp_create",             std::string("__builtin_net_ftp_create__"));
        environment->define("net_ftp_destroy",            std::string("__builtin_net_ftp_destroy__"));
        environment->define("net_ftp_connect",            std::string("__builtin_net_ftp_connect__"));
        environment->define("net_ftp_login",              std::string("__builtin_net_ftp_login__"));
        environment->define("net_ftp_list",               std::string("__builtin_net_ftp_list__"));
        environment->define("net_ftp_file_count",         std::string("__builtin_net_ftp_file_count__"));
        environment->define("net_ftp_file_name",          std::string("__builtin_net_ftp_file_name__"));
        environment->define("net_ftp_file_size",          std::string("__builtin_net_ftp_file_size__"));
        environment->define("net_ftp_download",           std::string("__builtin_net_ftp_download__"));
        environment->define("net_ftp_upload",             std::string("__builtin_net_ftp_upload__"));
        environment->define("net_ftp_mkdir",              std::string("__builtin_net_ftp_mkdir__"));
        environment->define("net_ftp_delete",             std::string("__builtin_net_ftp_delete__"));
        environment->define("net_ftp_disconnect",         std::string("__builtin_net_ftp_disconnect__"));
        environment->define("net_ftp_cwd",                std::string("__builtin_net_ftp_cwd__"));
        // SSH
        environment->define("net_ssh_create",             std::string("__builtin_net_ssh_create__"));
        environment->define("net_ssh_destroy",            std::string("__builtin_net_ssh_destroy__"));
        environment->define("net_ssh_connect",            std::string("__builtin_net_ssh_connect__"));
        environment->define("net_ssh_auth_password",      std::string("__builtin_net_ssh_auth_password__"));
        environment->define("net_ssh_auth_key",           std::string("__builtin_net_ssh_auth_key__"));
        environment->define("net_ssh_exec",               std::string("__builtin_net_ssh_exec__"));
        environment->define("net_ssh_output",             std::string("__builtin_net_ssh_output__"));
        environment->define("net_ssh_stderr",             std::string("__builtin_net_ssh_stderr__"));
        environment->define("net_ssh_exit_code",          std::string("__builtin_net_ssh_exit_code__"));
        environment->define("net_ssh_upload",             std::string("__builtin_net_ssh_upload__"));
        environment->define("net_ssh_download",           std::string("__builtin_net_ssh_download__"));
        environment->define("net_ssh_disconnect",         std::string("__builtin_net_ssh_disconnect__"));
        environment->define("net_ssh_is_connected",       std::string("__builtin_net_ssh_is_connected__"));
        // Phase 3: Packet capture
        environment->define("net_capture_create",         std::string("__builtin_net_capture_create__"));
        environment->define("net_capture_destroy",        std::string("__builtin_net_capture_destroy__"));
        environment->define("net_capture_start",          std::string("__builtin_net_capture_start__"));
        environment->define("net_capture_stop",           std::string("__builtin_net_capture_stop__"));
        environment->define("net_capture_packet_count",   std::string("__builtin_net_capture_packet_count__"));
        environment->define("net_capture_packet_src",     std::string("__builtin_net_capture_packet_src__"));
        environment->define("net_capture_packet_dst",     std::string("__builtin_net_capture_packet_dst__"));
        environment->define("net_capture_packet_proto",   std::string("__builtin_net_capture_packet_proto__"));
        environment->define("net_capture_packet_size",    std::string("__builtin_net_capture_packet_size__"));
        environment->define("net_capture_packet_data",    std::string("__builtin_net_capture_packet_data__"));
        environment->define("net_capture_set_filter",     std::string("__builtin_net_capture_set_filter__"));
        environment->define("net_capture_to_json",        std::string("__builtin_net_capture_to_json__"));
        // Network monitor
        environment->define("net_monitor_create",         std::string("__builtin_net_monitor_create__"));
        environment->define("net_monitor_destroy",        std::string("__builtin_net_monitor_destroy__"));
        environment->define("net_monitor_add_interface",  std::string("__builtin_net_monitor_add_interface__"));
        environment->define("net_monitor_sample",         std::string("__builtin_net_monitor_sample__"));
        environment->define("net_monitor_interface_count",std::string("__builtin_net_monitor_interface_count__"));
        environment->define("net_monitor_interface_name", std::string("__builtin_net_monitor_interface_name__"));
        environment->define("net_monitor_bytes_sent",     std::string("__builtin_net_monitor_bytes_sent__"));
        environment->define("net_monitor_bytes_recv",     std::string("__builtin_net_monitor_bytes_recv__"));
        environment->define("net_monitor_packets_sent",   std::string("__builtin_net_monitor_packets_sent__"));
        environment->define("net_monitor_packets_recv",   std::string("__builtin_net_monitor_packets_recv__"));
        environment->define("net_monitor_to_json",        std::string("__builtin_net_monitor_to_json__"));
        // Bandwidth
        environment->define("net_bandwidth_create",       std::string("__builtin_net_bandwidth_create__"));
        environment->define("net_bandwidth_destroy",      std::string("__builtin_net_bandwidth_destroy__"));
        environment->define("net_bandwidth_test_upload",  std::string("__builtin_net_bandwidth_test_upload__"));
        environment->define("net_bandwidth_test_download",std::string("__builtin_net_bandwidth_test_download__"));
        environment->define("net_bandwidth_last_upload_mbps",  std::string("__builtin_net_bandwidth_last_upload_mbps__"));
        environment->define("net_bandwidth_last_download_mbps",std::string("__builtin_net_bandwidth_last_download_mbps__"));
        environment->define("net_bandwidth_report",       std::string("__builtin_net_bandwidth_report__"));
        // Ping
        environment->define("net_ping_create",            std::string("__builtin_net_ping_create__"));
        environment->define("net_ping_destroy",           std::string("__builtin_net_ping_destroy__"));
        environment->define("net_ping_host",              std::string("__builtin_net_ping_host__"));
        environment->define("net_ping_min",               std::string("__builtin_net_ping_min__"));
        environment->define("net_ping_max",               std::string("__builtin_net_ping_max__"));
        environment->define("net_ping_avg",               std::string("__builtin_net_ping_avg__"));
        environment->define("net_ping_jitter",            std::string("__builtin_net_ping_jitter__"));
        environment->define("net_ping_packet_loss",       std::string("__builtin_net_ping_packet_loss__"));
        environment->define("net_ping_report",            std::string("__builtin_net_ping_report__"));
        // Phase 4: Event loop
        environment->define("net_event_loop_create",      std::string("__builtin_net_event_loop_create__"));
        environment->define("net_event_loop_destroy",     std::string("__builtin_net_event_loop_destroy__"));
        environment->define("net_event_loop_add_socket",  std::string("__builtin_net_event_loop_add_socket__"));
        environment->define("net_event_loop_remove_socket",std::string("__builtin_net_event_loop_remove_socket__"));
        environment->define("net_event_loop_run_once",    std::string("__builtin_net_event_loop_run_once__"));
        environment->define("net_event_loop_run",         std::string("__builtin_net_event_loop_run__"));
        environment->define("net_event_loop_stop",        std::string("__builtin_net_event_loop_stop__"));
        environment->define("net_event_loop_pending_count",std::string("__builtin_net_event_loop_pending_count__"));
        environment->define("net_event_loop_next_event",  std::string("__builtin_net_event_loop_next_event__"));
        environment->define("net_event_loop_is_running",  std::string("__builtin_net_event_loop_is_running__"));
        // Connection pool
        environment->define("net_pool_create",            std::string("__builtin_net_pool_create__"));
        environment->define("net_pool_destroy",           std::string("__builtin_net_pool_destroy__"));
        environment->define("net_pool_acquire",           std::string("__builtin_net_pool_acquire__"));
        environment->define("net_pool_release",           std::string("__builtin_net_pool_release__"));
        environment->define("net_pool_size",              std::string("__builtin_net_pool_size__"));
        environment->define("net_pool_active",            std::string("__builtin_net_pool_active__"));
        environment->define("net_pool_idle",              std::string("__builtin_net_pool_idle__"));
        environment->define("net_pool_host",              std::string("__builtin_net_pool_host__"));
        environment->define("net_pool_port",              std::string("__builtin_net_pool_port__"));
        environment->define("net_pool_set_timeout",       std::string("__builtin_net_pool_set_timeout__"));
        // Async request
        environment->define("net_async_request_create",   std::string("__builtin_net_async_request_create__"));
        environment->define("net_async_request_destroy",  std::string("__builtin_net_async_request_destroy__"));
        environment->define("net_async_get",              std::string("__builtin_net_async_get__"));
        environment->define("net_async_post",             std::string("__builtin_net_async_post__"));
        environment->define("net_async_is_done",          std::string("__builtin_net_async_is_done__"));
        environment->define("net_async_status_code",      std::string("__builtin_net_async_status_code__"));
        environment->define("net_async_response",         std::string("__builtin_net_async_response__"));
        environment->define("net_async_error",            std::string("__builtin_net_async_error__"));
        environment->define("net_async_elapsed_ms",       std::string("__builtin_net_async_elapsed_ms__"));
        // Load balancer
        environment->define("net_lb_create",              std::string("__builtin_net_lb_create__"));
        environment->define("net_lb_destroy",             std::string("__builtin_net_lb_destroy__"));
        environment->define("net_lb_add_backend",         std::string("__builtin_net_lb_add_backend__"));
        environment->define("net_lb_next_host",           std::string("__builtin_net_lb_next_host__"));
        environment->define("net_lb_next_port",           std::string("__builtin_net_lb_next_port__"));
        environment->define("net_lb_backend_count",       std::string("__builtin_net_lb_backend_count__"));
        environment->define("net_lb_mark_down",           std::string("__builtin_net_lb_mark_down__"));
        environment->define("net_lb_mark_up",             std::string("__builtin_net_lb_mark_up__"));
        environment->define("net_lb_strategy",            std::string("__builtin_net_lb_strategy__"));
        environment->define("net_lb_stats",               std::string("__builtin_net_lb_stats__"));
        return;
    }

    // Milestone 13: texture
    if (mod == "texture") {
        environment->define("texture_load",             std::string("__builtin_texture_load__"));
        environment->define("texture_destroy",          std::string("__builtin_texture_destroy__"));
        environment->define("texture_width",            std::string("__builtin_texture_width__"));
        environment->define("texture_height",           std::string("__builtin_texture_height__"));
        environment->define("texture_bind",             std::string("__builtin_texture_bind__"));
        environment->define("texture_error",            std::string("__builtin_texture_error__"));
        environment->define("spritesheet_create",       std::string("__builtin_spritesheet_create__"));
        environment->define("spritesheet_destroy",      std::string("__builtin_spritesheet_destroy__"));
        environment->define("spritesheet_frame_uv",     std::string("__builtin_spritesheet_frame_uv__"));
        environment->define("atlas_load",               std::string("__builtin_atlas_load__"));
        environment->define("atlas_destroy",            std::string("__builtin_atlas_destroy__"));
        environment->define("atlas_region_uv",          std::string("__builtin_atlas_region_uv__"));
        environment->define("atlas_error",              std::string("__builtin_atlas_error__"));
        return;
    }

    // gl3d: OpenGL 3D engine
    if (mod == "gl3d") {
        environment->define("gl3d_window_create",       std::string("__builtin_gl3d_window_create__"));
        environment->define("gl3d_window_destroy",      std::string("__builtin_gl3d_window_destroy__"));
        environment->define("gl3d_window_poll",         std::string("__builtin_gl3d_window_poll__"));
        environment->define("gl3d_window_swap",         std::string("__builtin_gl3d_window_swap__"));
        environment->define("gl3d_window_set_title",    std::string("__builtin_gl3d_window_set_title__"));
        environment->define("gl3d_window_width",        std::string("__builtin_gl3d_window_width__"));
        environment->define("gl3d_window_height",       std::string("__builtin_gl3d_window_height__"));
        environment->define("gl3d_key",                 std::string("__builtin_gl3d_key__"));
        environment->define("gl3d_key_pressed",         std::string("__builtin_gl3d_key_pressed__"));
        environment->define("gl3d_mouse_dx",            std::string("__builtin_gl3d_mouse_dx__"));
        environment->define("gl3d_mouse_dy",            std::string("__builtin_gl3d_mouse_dy__"));
        environment->define("gl3d_mouse_capture",       std::string("__builtin_gl3d_mouse_capture__"));
        environment->define("gl3d_delta",               std::string("__builtin_gl3d_delta__"));
        environment->define("gl3d_time",                std::string("__builtin_gl3d_time__"));
        environment->define("gl3d_mesh_create",         std::string("__builtin_gl3d_mesh_create__"));
        environment->define("gl3d_mesh_destroy",        std::string("__builtin_gl3d_mesh_destroy__"));
        environment->define("gl3d_mesh_draw",           std::string("__builtin_gl3d_mesh_draw__"));
        environment->define("gl3d_mesh_box",            std::string("__builtin_gl3d_mesh_box__"));
        environment->define("gl3d_mesh_plane",          std::string("__builtin_gl3d_mesh_plane__"));
        environment->define("gl3d_mesh_sphere",         std::string("__builtin_gl3d_mesh_sphere__"));
        environment->define("gl3d_mesh_cylinder",       std::string("__builtin_gl3d_mesh_cylinder__"));
        environment->define("gl3d_mesh_terrain",        std::string("__builtin_gl3d_mesh_terrain__"));
        environment->define("gl3d_mesh_lines_create",   std::string("__builtin_gl3d_mesh_lines_create__"));
        environment->define("gl3d_mesh_lines_update",   std::string("__builtin_gl3d_mesh_lines_update__"));
        environment->define("gl3d_mesh_lines_draw",     std::string("__builtin_gl3d_mesh_lines_draw__"));
        environment->define("gl3d_mesh_lines_destroy",  std::string("__builtin_gl3d_mesh_lines_destroy__"));
        environment->define("gl3d_shader_create",       std::string("__builtin_gl3d_shader_create__"));
        environment->define("gl3d_shader_destroy",      std::string("__builtin_gl3d_shader_destroy__"));
        environment->define("gl3d_shader_use",          std::string("__builtin_gl3d_shader_use__"));
        environment->define("gl3d_shader_set_float",    std::string("__builtin_gl3d_shader_set_float__"));
        environment->define("gl3d_shader_set_vec3",     std::string("__builtin_gl3d_shader_set_vec3__"));
        environment->define("gl3d_shader_set_vec4",     std::string("__builtin_gl3d_shader_set_vec4__"));
        environment->define("gl3d_shader_set_mat4",     std::string("__builtin_gl3d_shader_set_mat4__"));
        environment->define("gl3d_shader_set_int",      std::string("__builtin_gl3d_shader_set_int__"));
        environment->define("gl3d_texture_load",        std::string("__builtin_gl3d_texture_load__"));
        environment->define("gl3d_texture_solid",       std::string("__builtin_gl3d_texture_solid__"));
        environment->define("gl3d_texture_destroy",     std::string("__builtin_gl3d_texture_destroy__"));
        environment->define("gl3d_texture_bind",        std::string("__builtin_gl3d_texture_bind__"));
        environment->define("gl3d_mat4_identity",       std::string("__builtin_gl3d_mat4_identity__"));
        environment->define("gl3d_mat4_perspective",    std::string("__builtin_gl3d_mat4_perspective__"));
        environment->define("gl3d_mat4_lookat",         std::string("__builtin_gl3d_mat4_lookat__"));
        environment->define("gl3d_mat4_translate",      std::string("__builtin_gl3d_mat4_translate__"));
        environment->define("gl3d_mat4_scale",          std::string("__builtin_gl3d_mat4_scale__"));
        environment->define("gl3d_mat4_rotate_y",       std::string("__builtin_gl3d_mat4_rotate_y__"));
        environment->define("gl3d_mat4_rotate_x",       std::string("__builtin_gl3d_mat4_rotate_x__"));
        environment->define("gl3d_mat4_rotate_z",       std::string("__builtin_gl3d_mat4_rotate_z__"));
        environment->define("gl3d_mat4_mul",            std::string("__builtin_gl3d_mat4_mul__"));
        environment->define("gl3d_mat4_free",           std::string("__builtin_gl3d_mat4_free__"));
        environment->define("gl3d_clear",               std::string("__builtin_gl3d_clear__"));
        environment->define("gl3d_depth_test",          std::string("__builtin_gl3d_depth_test__"));
        environment->define("gl3d_wireframe",           std::string("__builtin_gl3d_wireframe__"));
        environment->define("gl3d_viewport",            std::string("__builtin_gl3d_viewport__"));
        environment->define("gl3d_error",               std::string("__builtin_gl3d_error__"));
        environment->define("gl3d_world4d_build",       std::string("__builtin_gl3d_world4d_build__"));
        environment->define("gl3d_world4d_update",      std::string("__builtin_gl3d_world4d_update__"));
        environment->define("gl3d_car_step",            std::string("__builtin_gl3d_car_step__"));
        return;
    }

    // Milestone 13: shader
    if (mod == "shader") {
        environment->define("shader_create",                std::string("__builtin_shader_create__"));
        environment->define("shader_destroy",               std::string("__builtin_shader_destroy__"));
        environment->define("shader_use",                   std::string("__builtin_shader_use__"));
        environment->define("shader_set_uniform_float",     std::string("__builtin_shader_set_uniform_float__"));
        environment->define("shader_set_uniform_vec3",      std::string("__builtin_shader_set_uniform_vec3__"));
        environment->define("shader_set_uniform_mat4",      std::string("__builtin_shader_set_uniform_mat4__"));
        environment->define("shader_error",                 std::string("__builtin_shader_error__"));
        return;
    }

    // Milestone 13: model
    if (mod == "model") {
        environment->define("model_load",           std::string("__builtin_model_load__"));
        environment->define("model_destroy",        std::string("__builtin_model_destroy__"));
        environment->define("model_draw",           std::string("__builtin_model_draw__"));
        environment->define("model_vertex_count",   std::string("__builtin_model_vertex_count__"));
        environment->define("model_face_count",     std::string("__builtin_model_face_count__"));
        environment->define("model_error",          std::string("__builtin_model_error__"));
        return;
    }

    // Milestone 13: physics
    if (mod == "physics") {
        environment->define("physics_world_create",     std::string("__builtin_physics_world_create__"));
        environment->define("physics_world_destroy",    std::string("__builtin_physics_world_destroy__"));
        environment->define("physics_world_step",       std::string("__builtin_physics_world_step__"));
        environment->define("rigidbody_create",         std::string("__builtin_rigidbody_create__"));
        environment->define("rigidbody_destroy",        std::string("__builtin_rigidbody_destroy__"));
        environment->define("rigidbody_apply_force",    std::string("__builtin_rigidbody_apply_force__"));
        environment->define("rigidbody_apply_impulse",  std::string("__builtin_rigidbody_apply_impulse__"));
        environment->define("rigidbody_get_x",          std::string("__builtin_rigidbody_get_x__"));
        environment->define("rigidbody_get_y",          std::string("__builtin_rigidbody_get_y__"));
        environment->define("rigidbody_get_vx",         std::string("__builtin_rigidbody_get_vx__"));
        environment->define("rigidbody_get_vy",         std::string("__builtin_rigidbody_get_vy__"));
        environment->define("collider_add_box",         std::string("__builtin_collider_add_box__"));
        environment->define("collider_add_circle",      std::string("__builtin_collider_add_circle__"));
        environment->define("collider_add_convex_hull", std::string("__builtin_collider_add_convex_hull__"));
        environment->define("collision_query_pair",     std::string("__builtin_collision_query_pair__"));
        environment->define("constraint_add_distance",  std::string("__builtin_constraint_add_distance__"));
        environment->define("constraint_add_spring",    std::string("__builtin_constraint_add_spring__"));
        environment->define("constraint_add_hinge",     std::string("__builtin_constraint_add_hinge__"));
        environment->define("constraint_destroy",       std::string("__builtin_constraint_destroy__"));
        environment->define("particles_create",         std::string("__builtin_particles_create__"));
        environment->define("particles_destroy",        std::string("__builtin_particles_destroy__"));
        environment->define("particles_emit",           std::string("__builtin_particles_emit__"));
        environment->define("particles_update",         std::string("__builtin_particles_update__"));
        environment->define("particles_set_color_gradient", std::string("__builtin_particles_set_color_gradient__"));
        environment->define("particles_active_count",   std::string("__builtin_particles_active_count__"));
        environment->define("physics_error",            std::string("__builtin_physics_error__"));
        return;
    }

    // Milestone 13: simulation
    if (mod == "simulation") {
        environment->define("nbody_create",                 std::string("__builtin_nbody_create__"));
        environment->define("nbody_destroy",                std::string("__builtin_nbody_destroy__"));
        environment->define("nbody_add_body",               std::string("__builtin_nbody_add_body__"));
        environment->define("nbody_step",                   std::string("__builtin_nbody_step__"));
        environment->define("nbody_get_position",           std::string("__builtin_nbody_get_position__"));
        environment->define("nbody_get_x",                  std::string("__builtin_nbody_get_x__"));
        environment->define("nbody_get_y",                  std::string("__builtin_nbody_get_y__"));
        environment->define("nbody_total_energy",           std::string("__builtin_nbody_total_energy__"));
        environment->define("fluid_create",                 std::string("__builtin_fluid_create__"));
        environment->define("fluid_destroy",                std::string("__builtin_fluid_destroy__"));
        environment->define("fluid_step",                   std::string("__builtin_fluid_step__"));
        environment->define("fluid_get_particle_position",  std::string("__builtin_fluid_get_particle_position__"));
        environment->define("fluid_get_particle_x",         std::string("__builtin_fluid_get_particle_x__"));
        environment->define("fluid_get_particle_y",         std::string("__builtin_fluid_get_particle_y__"));
        environment->define("fluid_set_viscosity",          std::string("__builtin_fluid_set_viscosity__"));
        environment->define("blackhole_create",             std::string("__builtin_blackhole_create__"));
        environment->define("blackhole_destroy",            std::string("__builtin_blackhole_destroy__"));
        environment->define("blackhole_add_particle",       std::string("__builtin_blackhole_add_particle__"));
        environment->define("blackhole_step",               std::string("__builtin_blackhole_step__"));
        environment->define("blackhole_get_particle_position", std::string("__builtin_blackhole_get_particle_position__"));
        environment->define("blackhole_get_particle_x",     std::string("__builtin_blackhole_get_particle_x__"));
        environment->define("blackhole_get_particle_y",     std::string("__builtin_blackhole_get_particle_y__"));
        environment->define("blackhole_captured_count",     std::string("__builtin_blackhole_captured_count__"));
        environment->define("field_sim_create",             std::string("__builtin_field_sim_create__"));
        environment->define("field_sim_destroy",            std::string("__builtin_field_sim_destroy__"));
        environment->define("field_sim_add_attractor",      std::string("__builtin_field_sim_add_attractor__"));
        environment->define("field_sim_add_repulsor",       std::string("__builtin_field_sim_add_repulsor__"));
        environment->define("field_sim_step",               std::string("__builtin_field_sim_step__"));
        environment->define("field_sim_get_particle_position", std::string("__builtin_field_sim_get_particle_position__"));
        environment->define("field_sim_get_particle_x",     std::string("__builtin_field_sim_get_particle_x__"));
        environment->define("field_sim_get_particle_y",     std::string("__builtin_field_sim_get_particle_y__"));
        return;
    }

    // Milestone 14: OS Development
    if (mod == "os") {
        environment->define("boot_create",                std::string("__builtin_boot_create__"));
        environment->define("boot_destroy",               std::string("__builtin_boot_destroy__"));
        environment->define("boot_load_mbr",              std::string("__builtin_boot_load_mbr__"));
        environment->define("boot_load_stage2",           std::string("__builtin_boot_load_stage2__"));
        environment->define("boot_load_kernel",           std::string("__builtin_boot_load_kernel__"));
        environment->define("boot_status",                std::string("__builtin_boot_status__"));
        environment->define("boot_get_memory_map_count",  std::string("__builtin_boot_get_memory_map_count__"));
        environment->define("boot_get_memory_map_entry",  std::string("__builtin_boot_get_memory_map_entry__"));
        environment->define("boot_enter_protected_mode",  std::string("__builtin_boot_enter_protected_mode__"));
        environment->define("boot_enter_long_mode",       std::string("__builtin_boot_enter_long_mode__"));
        environment->define("boot_mode",                  std::string("__builtin_boot_mode__"));
        environment->define("kernel_create",              std::string("__builtin_os_kernel_create__"));
        environment->define("kernel_destroy",             std::string("__builtin_os_kernel_destroy__"));
        environment->define("kernel_init_idt",            std::string("__builtin_os_kernel_init_idt__"));
        environment->define("kernel_init_gdt",            std::string("__builtin_os_kernel_init_gdt__"));
        environment->define("kernel_init_pic",            std::string("__builtin_os_kernel_init_pic__"));
        environment->define("kernel_init_pit",            std::string("__builtin_os_kernel_init_pit__"));
        environment->define("kernel_register_isr",        std::string("__builtin_os_kernel_register_isr__"));
        environment->define("kernel_isr_count",           std::string("__builtin_os_kernel_isr_count__"));
        environment->define("kernel_isr_name",            std::string("__builtin_os_kernel_isr_name__"));
        environment->define("kernel_trigger_interrupt",   std::string("__builtin_os_kernel_trigger_interrupt__"));
        environment->define("kernel_interrupt_count",     std::string("__builtin_os_kernel_interrupt_count__"));
        environment->define("kernel_vga_write",           std::string("__builtin_os_kernel_vga_write__"));
        environment->define("kernel_vga_read",            std::string("__builtin_os_kernel_vga_read__"));
        environment->define("kernel_vga_clear",           std::string("__builtin_os_kernel_vga_clear__"));
        environment->define("kernel_status",              std::string("__builtin_os_kernel_status__"));
        environment->define("pmm_create",                 std::string("__builtin_pmm_create__"));
        environment->define("pmm_destroy",                std::string("__builtin_pmm_destroy__"));
        environment->define("pmm_mark_free",              std::string("__builtin_pmm_mark_free__"));
        environment->define("pmm_mark_used",              std::string("__builtin_pmm_mark_used__"));
        environment->define("pmm_alloc_page",             std::string("__builtin_pmm_alloc_page__"));
        environment->define("pmm_free_page",              std::string("__builtin_pmm_free_page__"));
        environment->define("pmm_alloc_pages",            std::string("__builtin_pmm_alloc_pages__"));
        environment->define("pmm_free_pages_count",       std::string("__builtin_pmm_free_pages_count__"));
        environment->define("pmm_used_pages_count",       std::string("__builtin_pmm_used_pages_count__"));
        environment->define("pmm_total_pages",            std::string("__builtin_pmm_total_pages__"));
        environment->define("pmm_page_size",              std::string("__builtin_pmm_page_size__"));
        environment->define("vmm_create",                 std::string("__builtin_vmm_create__"));
        environment->define("vmm_destroy",                std::string("__builtin_vmm_destroy__"));
        environment->define("vmm_map",                    std::string("__builtin_vmm_map__"));
        environment->define("vmm_unmap",                  std::string("__builtin_vmm_unmap__"));
        environment->define("vmm_translate",              std::string("__builtin_vmm_translate__"));
        environment->define("vmm_is_mapped",              std::string("__builtin_vmm_is_mapped__"));
        environment->define("vmm_page_fault_count",       std::string("__builtin_vmm_page_fault_count__"));
        environment->define("vmm_dump_table",             std::string("__builtin_vmm_dump_table__"));
        environment->define("heap_create",                std::string("__builtin_heap_create__"));
        environment->define("heap_destroy",               std::string("__builtin_heap_destroy__"));
        environment->define("heap_alloc",                 std::string("__builtin_heap_alloc__"));
        environment->define("heap_free",                  std::string("__builtin_heap_free__"));
        environment->define("heap_used",                  std::string("__builtin_heap_used__"));
        environment->define("heap_free_space",            std::string("__builtin_heap_free_space__"));
        environment->define("heap_block_count",           std::string("__builtin_heap_block_count__"));
        environment->define("scheduler_create",           std::string("__builtin_scheduler_create__"));
        environment->define("scheduler_destroy",          std::string("__builtin_scheduler_destroy__"));
        environment->define("scheduler_add_process",      std::string("__builtin_scheduler_add_process__"));
        environment->define("scheduler_terminate",        std::string("__builtin_scheduler_terminate__"));
        environment->define("scheduler_tick",             std::string("__builtin_scheduler_tick__"));
        environment->define("scheduler_current_pid",      std::string("__builtin_scheduler_current_pid__"));
        environment->define("scheduler_current_name",     std::string("__builtin_scheduler_current_name__"));
        environment->define("scheduler_process_count",    std::string("__builtin_scheduler_process_count__"));
        environment->define("scheduler_process_name",     std::string("__builtin_scheduler_process_name__"));
        environment->define("scheduler_process_state",    std::string("__builtin_scheduler_process_state__"));
        environment->define("scheduler_process_priority", std::string("__builtin_scheduler_process_priority__"));
        environment->define("scheduler_process_ticks",    std::string("__builtin_scheduler_process_ticks__"));
        environment->define("scheduler_block",            std::string("__builtin_scheduler_block__"));
        environment->define("scheduler_unblock",          std::string("__builtin_scheduler_unblock__"));
        environment->define("syscall_table_create",       std::string("__builtin_syscall_table_create__"));
        environment->define("syscall_table_destroy",      std::string("__builtin_syscall_table_destroy__"));
        environment->define("syscall_table_register",     std::string("__builtin_syscall_table_register__"));
        environment->define("syscall_table_invoke",       std::string("__builtin_syscall_table_invoke__"));
        environment->define("syscall_table_count",        std::string("__builtin_syscall_table_count__"));
        environment->define("syscall_table_name",         std::string("__builtin_syscall_table_name__"));
        environment->define("driver_create",              std::string("__builtin_driver_create__"));
        environment->define("driver_destroy",             std::string("__builtin_driver_destroy__"));
        environment->define("driver_write",               std::string("__builtin_driver_write__"));
        environment->define("driver_read",                std::string("__builtin_driver_read__"));
        environment->define("driver_irq_fire",            std::string("__builtin_driver_irq_fire__"));
        environment->define("driver_irq_count",           std::string("__builtin_driver_irq_count__"));
        environment->define("driver_name",                std::string("__builtin_driver_name__"));
        environment->define("driver_type",                std::string("__builtin_driver_type__"));
        environment->define("driver_is_ready",            std::string("__builtin_driver_is_ready__"));
        environment->define("vga_create",                 std::string("__builtin_os_vga_create__"));
        environment->define("vga_destroy",                std::string("__builtin_os_vga_destroy__"));
        environment->define("vga_putchar",                std::string("__builtin_os_vga_putchar__"));
        environment->define("vga_puts",                   std::string("__builtin_os_vga_puts__"));
        environment->define("vga_set_cursor",             std::string("__builtin_os_vga_set_cursor__"));
        environment->define("vga_clear",                  std::string("__builtin_os_vga_clear__"));
        environment->define("vga_cursor_row",             std::string("__builtin_os_vga_cursor_row__"));
        environment->define("vga_cursor_col",             std::string("__builtin_os_vga_cursor_col__"));
        environment->define("vga_get_line",               std::string("__builtin_os_vga_get_line__"));
        environment->define("vga_cols",                   std::string("__builtin_os_vga_cols__"));
        environment->define("vga_rows",                   std::string("__builtin_os_vga_rows__"));
        return;
    }

    // Milestone 15: Advanced Cryptography
    if (mod == "advancedcrypto") {
        // Post-Quantum: Kyber
        environment->define("kyber_keygen",              std::string("__builtin_kyber_keygen__"));
        environment->define("kyber_destroy_keypair",     std::string("__builtin_kyber_destroy_keypair__"));
        environment->define("kyber_public_key",          std::string("__builtin_kyber_public_key__"));
        environment->define("kyber_secret_key",          std::string("__builtin_kyber_secret_key__"));
        environment->define("kyber_security_level",      std::string("__builtin_kyber_security_level__"));
        environment->define("kyber_encapsulate",         std::string("__builtin_kyber_encapsulate__"));
        environment->define("kyber_destroy_encap",       std::string("__builtin_kyber_destroy_encap__"));
        environment->define("kyber_ciphertext",          std::string("__builtin_kyber_ciphertext__"));
        environment->define("kyber_shared_secret",       std::string("__builtin_kyber_shared_secret__"));
        environment->define("kyber_decapsulate",         std::string("__builtin_kyber_decapsulate__"));
        // Post-Quantum: SPHINCS+
        environment->define("sphincs_keygen",            std::string("__builtin_sphincs_keygen__"));
        environment->define("sphincs_destroy_keypair",   std::string("__builtin_sphincs_destroy_keypair__"));
        environment->define("sphincs_public_key",        std::string("__builtin_sphincs_public_key__"));
        environment->define("sphincs_secret_key",        std::string("__builtin_sphincs_secret_key__"));
        environment->define("sphincs_sign",              std::string("__builtin_sphincs_sign__"));
        environment->define("sphincs_verify",            std::string("__builtin_sphincs_verify__"));
        // Post-Quantum: McEliece
        environment->define("mceliece_keygen",           std::string("__builtin_mceliece_keygen__"));
        environment->define("mceliece_destroy_keypair",  std::string("__builtin_mceliece_destroy_keypair__"));
        environment->define("mceliece_public_key",       std::string("__builtin_mceliece_public_key__"));
        environment->define("mceliece_secret_key",       std::string("__builtin_mceliece_secret_key__"));
        environment->define("mceliece_encrypt",          std::string("__builtin_mceliece_encrypt__"));
        environment->define("mceliece_decrypt",          std::string("__builtin_mceliece_decrypt__"));
        // Post-Quantum: Rainbow
        environment->define("rainbow_keygen",            std::string("__builtin_rainbow_keygen__"));
        environment->define("rainbow_destroy_keypair",   std::string("__builtin_rainbow_destroy_keypair__"));
        environment->define("rainbow_public_key",        std::string("__builtin_rainbow_public_key__"));
        environment->define("rainbow_secret_key",        std::string("__builtin_rainbow_secret_key__"));
        environment->define("rainbow_sign",              std::string("__builtin_rainbow_sign__"));
        environment->define("rainbow_verify",            std::string("__builtin_rainbow_verify__"));
        // ZK: SNARK
        environment->define("zksnark_setup",             std::string("__builtin_zksnark_setup__"));
        environment->define("zksnark_destroy_keys",      std::string("__builtin_zksnark_destroy_keys__"));
        environment->define("zksnark_proving_key",       std::string("__builtin_zksnark_proving_key__"));
        environment->define("zksnark_verification_key",  std::string("__builtin_zksnark_verification_key__"));
        environment->define("zksnark_circuit_id",        std::string("__builtin_zksnark_circuit_id__"));
        environment->define("zksnark_prove",             std::string("__builtin_zksnark_prove__"));
        environment->define("zksnark_destroy_proof",     std::string("__builtin_zksnark_destroy_proof__"));
        environment->define("zksnark_proof_hex",         std::string("__builtin_zksnark_proof_hex__"));
        environment->define("zksnark_public_inputs",     std::string("__builtin_zksnark_public_inputs__"));
        environment->define("zksnark_verify",            std::string("__builtin_zksnark_verify__"));
        // ZK: STARK
        environment->define("zkstark_prove",             std::string("__builtin_zkstark_prove__"));
        environment->define("zkstark_destroy_proof",     std::string("__builtin_zkstark_destroy_proof__"));
        environment->define("zkstark_proof_hex",         std::string("__builtin_zkstark_proof_hex__"));
        environment->define("zkstark_trace_commitment",  std::string("__builtin_zkstark_trace_commitment__"));
        environment->define("zkstark_verify",            std::string("__builtin_zkstark_verify__"));
        // ZK: Bulletproofs
        environment->define("bulletproof_prove_range",   std::string("__builtin_bulletproof_prove_range__"));
        environment->define("bulletproof_destroy",       std::string("__builtin_bulletproof_destroy__"));
        environment->define("bulletproof_proof_hex",     std::string("__builtin_bulletproof_proof_hex__"));
        environment->define("bulletproof_commitment_hex",std::string("__builtin_bulletproof_commitment_hex__"));
        environment->define("bulletproof_verify_range",  std::string("__builtin_bulletproof_verify_range__"));
        // ZK: PLONK
        environment->define("plonk_setup",               std::string("__builtin_plonk_setup__"));
        environment->define("plonk_destroy_srs",         std::string("__builtin_plonk_destroy_srs__"));
        environment->define("plonk_srs_hex",             std::string("__builtin_plonk_srs_hex__"));
        environment->define("plonk_srs_id",              std::string("__builtin_plonk_srs_id__"));
        environment->define("plonk_prove",               std::string("__builtin_plonk_prove__"));
        environment->define("plonk_destroy_proof",       std::string("__builtin_plonk_destroy_proof__"));
        environment->define("plonk_proof_hex",           std::string("__builtin_plonk_proof_hex__"));
        environment->define("plonk_verify",              std::string("__builtin_plonk_verify__"));
        // HE: BGV
        environment->define("bgv_create_context",        std::string("__builtin_bgv_create_context__"));
        environment->define("bgv_destroy_context",       std::string("__builtin_bgv_destroy_context__"));
        environment->define("bgv_context_id",            std::string("__builtin_bgv_context_id__"));
        environment->define("bgv_keygen",                std::string("__builtin_bgv_keygen__"));
        environment->define("bgv_destroy_keypair",       std::string("__builtin_bgv_destroy_keypair__"));
        environment->define("bgv_public_key",            std::string("__builtin_bgv_public_key__"));
        environment->define("bgv_secret_key",            std::string("__builtin_bgv_secret_key__"));
        environment->define("bgv_encrypt",               std::string("__builtin_bgv_encrypt__"));
        environment->define("bgv_destroy_ciphertext",    std::string("__builtin_bgv_destroy_ciphertext__"));
        environment->define("bgv_ciphertext_hex",        std::string("__builtin_bgv_ciphertext_hex__"));
        environment->define("bgv_decrypt",               std::string("__builtin_bgv_decrypt__"));
        environment->define("bgv_add",                   std::string("__builtin_bgv_add__"));
        environment->define("bgv_multiply",              std::string("__builtin_bgv_multiply__"));
        // HE: BFV
        environment->define("bfv_create_context",        std::string("__builtin_bfv_create_context__"));
        environment->define("bfv_destroy_context",       std::string("__builtin_bfv_destroy_context__"));
        environment->define("bfv_context_id",            std::string("__builtin_bfv_context_id__"));
        environment->define("bfv_keygen",                std::string("__builtin_bfv_keygen__"));
        environment->define("bfv_destroy_keypair",       std::string("__builtin_bfv_destroy_keypair__"));
        environment->define("bfv_public_key",            std::string("__builtin_bfv_public_key__"));
        environment->define("bfv_secret_key",            std::string("__builtin_bfv_secret_key__"));
        environment->define("bfv_encrypt",               std::string("__builtin_bfv_encrypt__"));
        environment->define("bfv_destroy_ciphertext",    std::string("__builtin_bfv_destroy_ciphertext__"));
        environment->define("bfv_ciphertext_hex",        std::string("__builtin_bfv_ciphertext_hex__"));
        environment->define("bfv_decrypt",               std::string("__builtin_bfv_decrypt__"));
        environment->define("bfv_add",                   std::string("__builtin_bfv_add__"));
        environment->define("bfv_multiply",              std::string("__builtin_bfv_multiply__"));
        // HE: CKKS
        environment->define("ckks_create_context",       std::string("__builtin_ckks_create_context__"));
        environment->define("ckks_destroy_context",      std::string("__builtin_ckks_destroy_context__"));
        environment->define("ckks_context_id",           std::string("__builtin_ckks_context_id__"));
        environment->define("ckks_keygen",               std::string("__builtin_ckks_keygen__"));
        environment->define("ckks_destroy_keypair",      std::string("__builtin_ckks_destroy_keypair__"));
        environment->define("ckks_public_key",           std::string("__builtin_ckks_public_key__"));
        environment->define("ckks_secret_key",           std::string("__builtin_ckks_secret_key__"));
        environment->define("ckks_encrypt",              std::string("__builtin_ckks_encrypt__"));
        environment->define("ckks_destroy_ciphertext",   std::string("__builtin_ckks_destroy_ciphertext__"));
        environment->define("ckks_ciphertext_hex",       std::string("__builtin_ckks_ciphertext_hex__"));
        environment->define("ckks_decrypt",              std::string("__builtin_ckks_decrypt__"));
        environment->define("ckks_add",                  std::string("__builtin_ckks_add__"));
        environment->define("ckks_multiply",             std::string("__builtin_ckks_multiply__"));
        // HE: TFHE
        environment->define("tfhe_create_context",       std::string("__builtin_tfhe_create_context__"));
        environment->define("tfhe_destroy_context",      std::string("__builtin_tfhe_destroy_context__"));
        environment->define("tfhe_context_id",           std::string("__builtin_tfhe_context_id__"));
        environment->define("tfhe_keygen",               std::string("__builtin_tfhe_keygen__"));
        environment->define("tfhe_destroy_keypair",      std::string("__builtin_tfhe_destroy_keypair__"));
        environment->define("tfhe_secret_key",           std::string("__builtin_tfhe_secret_key__"));
        environment->define("tfhe_cloud_key",            std::string("__builtin_tfhe_cloud_key__"));
        environment->define("tfhe_encrypt_bit",          std::string("__builtin_tfhe_encrypt_bit__"));
        environment->define("tfhe_destroy_ciphertext",   std::string("__builtin_tfhe_destroy_ciphertext__"));
        environment->define("tfhe_ciphertext_hex",       std::string("__builtin_tfhe_ciphertext_hex__"));
        environment->define("tfhe_decrypt_bit",          std::string("__builtin_tfhe_decrypt_bit__"));
        environment->define("tfhe_gate_and",             std::string("__builtin_tfhe_gate_and__"));
        environment->define("tfhe_gate_or",              std::string("__builtin_tfhe_gate_or__"));
        environment->define("tfhe_gate_xor",             std::string("__builtin_tfhe_gate_xor__"));
        environment->define("tfhe_gate_not",             std::string("__builtin_tfhe_gate_not__"));
        // Shamir Secret Sharing
        environment->define("shamir_split",              std::string("__builtin_shamir_split__"));
        environment->define("shamir_destroy_shares",     std::string("__builtin_shamir_destroy_shares__"));
        environment->define("shamir_share_count",        std::string("__builtin_shamir_share_count__"));
        environment->define("shamir_share_x",            std::string("__builtin_shamir_share_x__"));
        environment->define("shamir_share_y",            std::string("__builtin_shamir_share_y__"));
        environment->define("shamir_reconstruct",        std::string("__builtin_shamir_reconstruct__"));
        // SMPC
        environment->define("smpc_share",               std::string("__builtin_smpc_share__"));
        environment->define("smpc_destroy_shares",       std::string("__builtin_smpc_destroy_shares__"));
        environment->define("smpc_share_count",          std::string("__builtin_smpc_share_count__"));
        environment->define("smpc_share_hex",            std::string("__builtin_smpc_share_hex__"));
        environment->define("smpc_reconstruct",          std::string("__builtin_smpc_reconstruct__"));
        // Threshold Cryptography
        environment->define("threshold_keygen",          std::string("__builtin_threshold_keygen__"));
        environment->define("threshold_destroy",         std::string("__builtin_threshold_destroy__"));
        environment->define("threshold_public_key",      std::string("__builtin_threshold_public_key__"));
        environment->define("threshold_share_count",     std::string("__builtin_threshold_share_count__"));
        environment->define("threshold_share_hex",       std::string("__builtin_threshold_share_hex__"));
        environment->define("threshold_partial_sign",    std::string("__builtin_threshold_partial_sign__"));
        environment->define("threshold_combine",         std::string("__builtin_threshold_combine__"));
        environment->define("threshold_verify",          std::string("__builtin_threshold_verify__"));
        // Oblivious Transfer
        environment->define("ot_sender_init",            std::string("__builtin_ot_sender_init__"));
        environment->define("ot_destroy_sender",         std::string("__builtin_ot_destroy_sender__"));
        environment->define("ot_sender_state",           std::string("__builtin_ot_sender_state__"));
        environment->define("ot_receiver_init",          std::string("__builtin_ot_receiver_init__"));
        environment->define("ot_destroy_receiver",       std::string("__builtin_ot_destroy_receiver__"));
        environment->define("ot_receiver_query",         std::string("__builtin_ot_receiver_query__"));
        environment->define("ot_sender_respond",         std::string("__builtin_ot_sender_respond__"));
        environment->define("ot_destroy_message",        std::string("__builtin_ot_destroy_message__"));
        environment->define("ot_message_hex",            std::string("__builtin_ot_message_hex__"));
        environment->define("ot_receiver_extract",       std::string("__builtin_ot_receiver_extract__"));
        return;
    }

    // Milestone 16: Mathematical Computing
    if (mod == "mathx") {
        // Number Theory
        environment->define("mathx_sieve",              std::string("__builtin_mathx_sieve__"));
        environment->define("mathx_sieve_destroy",      std::string("__builtin_mathx_sieve_destroy__"));
        environment->define("mathx_sieve_count",        std::string("__builtin_mathx_sieve_count__"));
        environment->define("mathx_sieve_get",          std::string("__builtin_mathx_sieve_get__"));
        environment->define("mathx_is_prime",           std::string("__builtin_mathx_is_prime__"));
        environment->define("mathx_prime_factors",      std::string("__builtin_mathx_prime_factors__"));
        environment->define("mathx_vec_destroy",        std::string("__builtin_mathx_vec_destroy__"));
        environment->define("mathx_vec_count",          std::string("__builtin_mathx_vec_count__"));
        environment->define("mathx_vec_get",            std::string("__builtin_mathx_vec_get__"));
        environment->define("mathx_gcd",                std::string("__builtin_mathx_gcd__"));
        environment->define("mathx_lcm",                std::string("__builtin_mathx_lcm__"));
        environment->define("mathx_mod_pow",            std::string("__builtin_mathx_mod_pow__"));
        environment->define("mathx_mod_inverse",        std::string("__builtin_mathx_mod_inverse__"));
        environment->define("mathx_euler_totient",      std::string("__builtin_mathx_euler_totient__"));
        environment->define("mathx_is_perfect",         std::string("__builtin_mathx_is_perfect__"));
        environment->define("mathx_is_abundant",        std::string("__builtin_mathx_is_abundant__"));
        environment->define("mathx_divisors",           std::string("__builtin_mathx_divisors__"));
        environment->define("mathx_sum_divisors",       std::string("__builtin_mathx_sum_divisors__"));
        environment->define("mathx_nth_prime",          std::string("__builtin_mathx_nth_prime__"));
        environment->define("mathx_collatz_length",     std::string("__builtin_mathx_collatz_length__"));
        environment->define("mathx_digital_root",       std::string("__builtin_mathx_digital_root__"));
        environment->define("mathx_is_palindrome_num",  std::string("__builtin_mathx_is_palindrome_num__"));
        environment->define("mathx_reverse_num",        std::string("__builtin_mathx_reverse_num__"));
        environment->define("mathx_sum_digits",         std::string("__builtin_mathx_sum_digits__"));
        environment->define("mathx_is_pandigital",      std::string("__builtin_mathx_is_pandigital__"));
        // Matrix
        environment->define("mathx_mat_create",         std::string("__builtin_mathx_mat_create__"));
        environment->define("mathx_mat_identity",       std::string("__builtin_mathx_mat_identity__"));
        environment->define("mathx_mat_destroy",        std::string("__builtin_mathx_mat_destroy__"));
        environment->define("mathx_mat_rows",           std::string("__builtin_mathx_mat_rows__"));
        environment->define("mathx_mat_cols",           std::string("__builtin_mathx_mat_cols__"));
        environment->define("mathx_mat_get",            std::string("__builtin_mathx_mat_get__"));
        environment->define("mathx_mat_set",            std::string("__builtin_mathx_mat_set__"));
        environment->define("mathx_mat_add",            std::string("__builtin_mathx_mat_add__"));
        environment->define("mathx_mat_sub",            std::string("__builtin_mathx_mat_sub__"));
        environment->define("mathx_mat_mul",            std::string("__builtin_mathx_mat_mul__"));
        environment->define("mathx_mat_scale",          std::string("__builtin_mathx_mat_scale__"));
        environment->define("mathx_mat_transpose",      std::string("__builtin_mathx_mat_transpose__"));
        environment->define("mathx_mat_det",            std::string("__builtin_mathx_mat_det__"));
        environment->define("mathx_mat_inverse",        std::string("__builtin_mathx_mat_inverse__"));
        environment->define("mathx_mat_solve",          std::string("__builtin_mathx_mat_solve__"));
        environment->define("mathx_mat_trace",          std::string("__builtin_mathx_mat_trace__"));
        environment->define("mathx_mat_to_string",      std::string("__builtin_mathx_mat_to_string__"));
        environment->define("mathx_mat_dominant_eigenvalue",  std::string("__builtin_mathx_mat_dominant_eigenvalue__"));
        environment->define("mathx_mat_dominant_eigenvector", std::string("__builtin_mathx_mat_dominant_eigenvector__"));
        // Symbolic
        environment->define("mathx_sym_num",            std::string("__builtin_mathx_sym_num__"));
        environment->define("mathx_sym_var",            std::string("__builtin_mathx_sym_var__"));
        environment->define("mathx_sym_add",            std::string("__builtin_mathx_sym_add__"));
        environment->define("mathx_sym_sub",            std::string("__builtin_mathx_sym_sub__"));
        environment->define("mathx_sym_mul",            std::string("__builtin_mathx_sym_mul__"));
        environment->define("mathx_sym_div",            std::string("__builtin_mathx_sym_div__"));
        environment->define("mathx_sym_pow",            std::string("__builtin_mathx_sym_pow__"));
        environment->define("mathx_sym_neg",            std::string("__builtin_mathx_sym_neg__"));
        environment->define("mathx_sym_sin",            std::string("__builtin_mathx_sym_sin__"));
        environment->define("mathx_sym_cos",            std::string("__builtin_mathx_sym_cos__"));
        environment->define("mathx_sym_exp",            std::string("__builtin_mathx_sym_exp__"));
        environment->define("mathx_sym_ln",             std::string("__builtin_mathx_sym_ln__"));
        environment->define("mathx_sym_simplify",       std::string("__builtin_mathx_sym_simplify__"));
        environment->define("mathx_sym_diff",           std::string("__builtin_mathx_sym_diff__"));
        environment->define("mathx_sym_eval",           std::string("__builtin_mathx_sym_eval__"));
        environment->define("mathx_sym_to_string",      std::string("__builtin_mathx_sym_to_string__"));
        environment->define("mathx_sym_destroy",        std::string("__builtin_mathx_sym_destroy__"));
        environment->define("mathx_integrate_expr",     std::string("__builtin_mathx_integrate_expr__"));
        environment->define("mathx_find_root",          std::string("__builtin_mathx_find_root__"));
        // Statistics
        environment->define("mathx_dvec_create",        std::string("__builtin_mathx_dvec_create__"));
        environment->define("mathx_dvec_push",          std::string("__builtin_mathx_dvec_push__"));
        environment->define("mathx_dvec_destroy",       std::string("__builtin_mathx_dvec_destroy__"));
        environment->define("mathx_dvec_count",         std::string("__builtin_mathx_dvec_count__"));
        environment->define("mathx_dvec_get",           std::string("__builtin_mathx_dvec_get__"));
        environment->define("mathx_stat_mean",          std::string("__builtin_mathx_stat_mean__"));
        environment->define("mathx_stat_variance",      std::string("__builtin_mathx_stat_variance__"));
        environment->define("mathx_stat_stddev",        std::string("__builtin_mathx_stat_stddev__"));
        environment->define("mathx_stat_median",        std::string("__builtin_mathx_stat_median__"));
        environment->define("mathx_stat_correlation",   std::string("__builtin_mathx_stat_correlation__"));
        environment->define("mathx_stat_linear_regression", std::string("__builtin_mathx_stat_linear_regression__"));
        return;
    }

    // Milestone 17: Polish & Optimization
    if (mod == "polish") {
        // Profiler
        environment->define("polish_profiler_reset",        std::string("__builtin_polish_profiler_reset__"));
        environment->define("polish_profiler_start",        std::string("__builtin_polish_profiler_start__"));
        environment->define("polish_profiler_stop",         std::string("__builtin_polish_profiler_stop__"));
        environment->define("polish_profiler_entry_count",  std::string("__builtin_polish_profiler_entry_count__"));
        environment->define("polish_profiler_entry_name",   std::string("__builtin_polish_profiler_entry_name__"));
        environment->define("polish_profiler_entry_calls",  std::string("__builtin_polish_profiler_entry_calls__"));
        environment->define("polish_profiler_entry_total_ms", std::string("__builtin_polish_profiler_entry_total_ms__"));
        environment->define("polish_profiler_entry_mean_ms",  std::string("__builtin_polish_profiler_entry_mean_ms__"));
        environment->define("polish_profiler_entry_min_ms",   std::string("__builtin_polish_profiler_entry_min_ms__"));
        environment->define("polish_profiler_entry_max_ms",   std::string("__builtin_polish_profiler_entry_max_ms__"));
        environment->define("polish_profiler_report",       std::string("__builtin_polish_profiler_report__"));
        // Benchmark
        environment->define("polish_bench_create",          std::string("__builtin_polish_bench_create__"));
        environment->define("polish_bench_destroy",         std::string("__builtin_polish_bench_destroy__"));
        environment->define("polish_bench_iter_start",      std::string("__builtin_polish_bench_iter_start__"));
        environment->define("polish_bench_iter_stop",       std::string("__builtin_polish_bench_iter_stop__"));
        environment->define("polish_bench_iterations",      std::string("__builtin_polish_bench_iterations__"));
        environment->define("polish_bench_mean_ms",         std::string("__builtin_polish_bench_mean_ms__"));
        environment->define("polish_bench_min_ms",          std::string("__builtin_polish_bench_min_ms__"));
        environment->define("polish_bench_max_ms",          std::string("__builtin_polish_bench_max_ms__"));
        environment->define("polish_bench_ops_per_sec",     std::string("__builtin_polish_bench_ops_per_sec__"));
        environment->define("polish_bench_report",          std::string("__builtin_polish_bench_report__"));
        // Logger
        environment->define("polish_logger_set_level",      std::string("__builtin_polish_logger_set_level__"));
        environment->define("polish_logger_get_level",      std::string("__builtin_polish_logger_get_level__"));
        environment->define("polish_logger_set_file",       std::string("__builtin_polish_logger_set_file__"));
        environment->define("polish_logger_close_file",     std::string("__builtin_polish_logger_close_file__"));
        environment->define("polish_logger_log",            std::string("__builtin_polish_logger_log__"));
        environment->define("polish_logger_enable_timestamps", std::string("__builtin_polish_logger_enable_timestamps__"));
        environment->define("polish_logger_enable_colors",  std::string("__builtin_polish_logger_enable_colors__"));
        environment->define("polish_logger_message_count",  std::string("__builtin_polish_logger_message_count__"));
        environment->define("polish_logger_get_message",    std::string("__builtin_polish_logger_get_message__"));
        environment->define("polish_logger_clear",          std::string("__builtin_polish_logger_clear__"));
        // Stack trace
        environment->define("polish_stacktrace_capture",    std::string("__builtin_polish_stacktrace_capture__"));
        environment->define("polish_stacktrace_destroy",    std::string("__builtin_polish_stacktrace_destroy__"));
        environment->define("polish_stacktrace_depth",      std::string("__builtin_polish_stacktrace_depth__"));
        environment->define("polish_stacktrace_frame_function", std::string("__builtin_polish_stacktrace_frame_function__"));
        environment->define("polish_stacktrace_frame_file", std::string("__builtin_polish_stacktrace_frame_file__"));
        environment->define("polish_stacktrace_frame_line", std::string("__builtin_polish_stacktrace_frame_line__"));
        environment->define("polish_stacktrace_to_string",  std::string("__builtin_polish_stacktrace_to_string__"));
        // Memory tracker
        environment->define("polish_memtrack_reset",        std::string("__builtin_polish_memtrack_reset__"));
        environment->define("polish_memtrack_alloc",        std::string("__builtin_polish_memtrack_alloc__"));
        environment->define("polish_memtrack_free",         std::string("__builtin_polish_memtrack_free__"));
        environment->define("polish_memtrack_current_bytes",std::string("__builtin_polish_memtrack_current_bytes__"));
        environment->define("polish_memtrack_peak_bytes",   std::string("__builtin_polish_memtrack_peak_bytes__"));
        environment->define("polish_memtrack_alloc_count",  std::string("__builtin_polish_memtrack_alloc_count__"));
        environment->define("polish_memtrack_report",       std::string("__builtin_polish_memtrack_report__"));
        return;
    }

    // Milestone 18: Community & Ecosystem
    if (mod == "ecosystem") {
        // Semver
        environment->define("eco_semver_parse",         std::string("__builtin_eco_semver_parse__"));
        environment->define("eco_semver_destroy",       std::string("__builtin_eco_semver_destroy__"));
        environment->define("eco_semver_major",         std::string("__builtin_eco_semver_major__"));
        environment->define("eco_semver_minor",         std::string("__builtin_eco_semver_minor__"));
        environment->define("eco_semver_patch",         std::string("__builtin_eco_semver_patch__"));
        environment->define("eco_semver_pre",           std::string("__builtin_eco_semver_pre__"));
        environment->define("eco_semver_to_string",     std::string("__builtin_eco_semver_to_string__"));
        environment->define("eco_semver_compare",       std::string("__builtin_eco_semver_compare__"));
        environment->define("eco_semver_satisfies",     std::string("__builtin_eco_semver_satisfies__"));
        environment->define("eco_semver_bump_major",    std::string("__builtin_eco_semver_bump_major__"));
        environment->define("eco_semver_bump_minor",    std::string("__builtin_eco_semver_bump_minor__"));
        environment->define("eco_semver_bump_patch",    std::string("__builtin_eco_semver_bump_patch__"));
        environment->define("eco_semver_is_valid",      std::string("__builtin_eco_semver_is_valid__"));
        environment->define("eco_semver_is_prerelease", std::string("__builtin_eco_semver_is_prerelease__"));
        // Manifest
        environment->define("eco_manifest_create",      std::string("__builtin_eco_manifest_create__"));
        environment->define("eco_manifest_destroy",     std::string("__builtin_eco_manifest_destroy__"));
        environment->define("eco_manifest_set_description", std::string("__builtin_eco_manifest_set_description__"));
        environment->define("eco_manifest_set_author",  std::string("__builtin_eco_manifest_set_author__"));
        environment->define("eco_manifest_set_license", std::string("__builtin_eco_manifest_set_license__"));
        environment->define("eco_manifest_add_dep",     std::string("__builtin_eco_manifest_add_dep__"));
        environment->define("eco_manifest_add_keyword", std::string("__builtin_eco_manifest_add_keyword__"));
        environment->define("eco_manifest_name",        std::string("__builtin_eco_manifest_name__"));
        environment->define("eco_manifest_version",     std::string("__builtin_eco_manifest_version__"));
        environment->define("eco_manifest_description", std::string("__builtin_eco_manifest_description__"));
        environment->define("eco_manifest_dep_count",   std::string("__builtin_eco_manifest_dep_count__"));
        environment->define("eco_manifest_dep_name",    std::string("__builtin_eco_manifest_dep_name__"));
        environment->define("eco_manifest_dep_range",   std::string("__builtin_eco_manifest_dep_range__"));
        environment->define("eco_manifest_to_toml",     std::string("__builtin_eco_manifest_to_toml__"));
        // Registry
        environment->define("eco_registry_reset",       std::string("__builtin_eco_registry_reset__"));
        environment->define("eco_registry_publish",     std::string("__builtin_eco_registry_publish__"));
        environment->define("eco_registry_pkg_destroy", std::string("__builtin_eco_registry_pkg_destroy__"));
        environment->define("eco_registry_exists",      std::string("__builtin_eco_registry_exists__"));
        environment->define("eco_registry_get",         std::string("__builtin_eco_registry_get__"));
        environment->define("eco_registry_pkg_name",    std::string("__builtin_eco_registry_pkg_name__"));
        environment->define("eco_registry_pkg_latest",  std::string("__builtin_eco_registry_pkg_latest__"));
        environment->define("eco_registry_pkg_description", std::string("__builtin_eco_registry_pkg_description__"));
        environment->define("eco_registry_pkg_author",  std::string("__builtin_eco_registry_pkg_author__"));
        environment->define("eco_registry_pkg_downloads", std::string("__builtin_eco_registry_pkg_downloads__"));
        environment->define("eco_registry_package_count", std::string("__builtin_eco_registry_package_count__"));
        environment->define("eco_registry_search",      std::string("__builtin_eco_registry_search__"));
        environment->define("eco_registry_list_all",    std::string("__builtin_eco_registry_list_all__"));
        // StrVec
        environment->define("eco_strvec_destroy",       std::string("__builtin_eco_strvec_destroy__"));
        environment->define("eco_strvec_count",         std::string("__builtin_eco_strvec_count__"));
        environment->define("eco_strvec_get",           std::string("__builtin_eco_strvec_get__"));
        // Package manager
        environment->define("eco_pkgmgr_reset",         std::string("__builtin_eco_pkgmgr_reset__"));
        environment->define("eco_pkgmgr_install",       std::string("__builtin_eco_pkgmgr_install__"));
        environment->define("eco_pkgmgr_install_message", std::string("__builtin_eco_pkgmgr_install_message__"));
        environment->define("eco_pkgmgr_install_manifest", std::string("__builtin_eco_pkgmgr_install_manifest__"));
        environment->define("eco_pkgmgr_is_installed",  std::string("__builtin_eco_pkgmgr_is_installed__"));
        environment->define("eco_pkgmgr_installed_version", std::string("__builtin_eco_pkgmgr_installed_version__"));
        environment->define("eco_pkgmgr_list_installed",std::string("__builtin_eco_pkgmgr_list_installed__"));
        environment->define("eco_pkgmgr_uninstall",     std::string("__builtin_eco_pkgmgr_uninstall__"));
        environment->define("eco_pkgmgr_lockfile",      std::string("__builtin_eco_pkgmgr_lockfile__"));
        // Template engine
        environment->define("eco_tmpl_vars_create",     std::string("__builtin_eco_tmpl_vars_create__"));
        environment->define("eco_tmpl_vars_destroy",    std::string("__builtin_eco_tmpl_vars_destroy__"));
        environment->define("eco_tmpl_vars_set",        std::string("__builtin_eco_tmpl_vars_set__"));
        environment->define("eco_tmpl_render",          std::string("__builtin_eco_tmpl_render__"));
        // Formatter
        environment->define("eco_fmt_indent",           std::string("__builtin_eco_fmt_indent__"));
        environment->define("eco_fmt_trim",             std::string("__builtin_eco_fmt_trim__"));
        environment->define("eco_fmt_normalize",        std::string("__builtin_eco_fmt_normalize__"));
        environment->define("eco_fmt_count_lines",      std::string("__builtin_eco_fmt_count_lines__"));
        environment->define("eco_fmt_count_chars",      std::string("__builtin_eco_fmt_count_chars__"));
        return;
    }

    // Milestone 19: Database & Storage
    if (mod == "database") {
        // DB core
        environment->define("db_open",                  std::string("__builtin_db_open__"));
        environment->define("db_close",                 std::string("__builtin_db_close__"));
        environment->define("db_exec",                  std::string("__builtin_db_exec__"));
        environment->define("db_result_destroy",        std::string("__builtin_db_result_destroy__"));
        environment->define("db_result_ok",             std::string("__builtin_db_result_ok__"));
        environment->define("db_result_error",          std::string("__builtin_db_result_error__"));
        environment->define("db_result_row_count",      std::string("__builtin_db_result_row_count__"));
        environment->define("db_result_col_count",      std::string("__builtin_db_result_col_count__"));
        environment->define("db_result_col_name",       std::string("__builtin_db_result_col_name__"));
        environment->define("db_result_get",            std::string("__builtin_db_result_get__"));
        environment->define("db_result_rows_affected",  std::string("__builtin_db_result_rows_affected__"));
        environment->define("db_begin",                 std::string("__builtin_db_begin__"));
        environment->define("db_commit",                std::string("__builtin_db_commit__"));
        environment->define("db_rollback",              std::string("__builtin_db_rollback__"));
        // Query builder
        environment->define("db_qb_select",             std::string("__builtin_db_qb_select__"));
        environment->define("db_qb_insert",             std::string("__builtin_db_qb_insert__"));
        environment->define("db_qb_update",             std::string("__builtin_db_qb_update__"));
        environment->define("db_qb_delete",             std::string("__builtin_db_qb_delete__"));
        environment->define("db_qb_create_table",       std::string("__builtin_db_qb_create_table__"));
        environment->define("db_qb_destroy",            std::string("__builtin_db_qb_destroy__"));
        environment->define("db_qb_cols",               std::string("__builtin_db_qb_cols__"));
        environment->define("db_qb_where",              std::string("__builtin_db_qb_where__"));
        environment->define("db_qb_order",              std::string("__builtin_db_qb_order__"));
        environment->define("db_qb_limit",              std::string("__builtin_db_qb_limit__"));
        environment->define("db_qb_offset",             std::string("__builtin_db_qb_offset__"));
        environment->define("db_qb_set",                std::string("__builtin_db_qb_set__"));
        environment->define("db_qb_value",              std::string("__builtin_db_qb_value__"));
        environment->define("db_qb_add_col",            std::string("__builtin_db_qb_add_col__"));
        environment->define("db_qb_exec",               std::string("__builtin_db_qb_exec__"));
        environment->define("db_qb_sql",                std::string("__builtin_db_qb_sql__"));
        // KV Store
        environment->define("kv_open",                  std::string("__builtin_kv_open__"));
        environment->define("kv_close",                 std::string("__builtin_kv_close__"));
        environment->define("kv_set",                   std::string("__builtin_kv_set__"));
        environment->define("kv_setex",                 std::string("__builtin_kv_setex__"));
        environment->define("kv_get",                   std::string("__builtin_kv_get__"));
        environment->define("kv_exists",                std::string("__builtin_kv_exists__"));
        environment->define("kv_del",                   std::string("__builtin_kv_del__"));
        environment->define("kv_incr",                  std::string("__builtin_kv_incr__"));
        environment->define("kv_decr",                  std::string("__builtin_kv_decr__"));
        environment->define("kv_keys",                  std::string("__builtin_kv_keys__"));
        environment->define("kv_flush",                 std::string("__builtin_kv_flush__"));
        environment->define("kv_count",                 std::string("__builtin_kv_count__"));
        environment->define("kv_lpush",                 std::string("__builtin_kv_lpush__"));
        environment->define("kv_rpush",                 std::string("__builtin_kv_rpush__"));
        environment->define("kv_lpop",                  std::string("__builtin_kv_lpop__"));
        environment->define("kv_rpop",                  std::string("__builtin_kv_rpop__"));
        environment->define("kv_llen",                  std::string("__builtin_kv_llen__"));
        environment->define("kv_lindex",                std::string("__builtin_kv_lindex__"));
        environment->define("kv_hset",                  std::string("__builtin_kv_hset__"));
        environment->define("kv_hget",                  std::string("__builtin_kv_hget__"));
        environment->define("kv_hexists",               std::string("__builtin_kv_hexists__"));
        environment->define("kv_hlen",                  std::string("__builtin_kv_hlen__"));
        environment->define("kv_hkeys",                 std::string("__builtin_kv_hkeys__"));
        environment->define("db_strvec_destroy",        std::string("__builtin_db_strvec_destroy__"));
        environment->define("db_strvec_count",          std::string("__builtin_db_strvec_count__"));
        environment->define("db_strvec_get",            std::string("__builtin_db_strvec_get__"));
        // Document store
        environment->define("doc_open",                 std::string("__builtin_doc_open__"));
        environment->define("doc_close",                std::string("__builtin_doc_close__"));
        environment->define("doc_map_create",           std::string("__builtin_doc_map_create__"));
        environment->define("doc_map_destroy",          std::string("__builtin_doc_map_destroy__"));
        environment->define("doc_map_set",              std::string("__builtin_doc_map_set__"));
        environment->define("doc_map_get",              std::string("__builtin_doc_map_get__"));
        environment->define("doc_map_keys",             std::string("__builtin_doc_map_keys__"));
        environment->define("doc_insert",               std::string("__builtin_doc_insert__"));
        environment->define("doc_find",                 std::string("__builtin_doc_find__"));
        environment->define("doc_find_all",             std::string("__builtin_doc_find_all__"));
        environment->define("doc_update",               std::string("__builtin_doc_update__"));
        environment->define("doc_delete",               std::string("__builtin_doc_delete__"));
        environment->define("doc_count",                std::string("__builtin_doc_count__"));
        environment->define("doc_list_destroy",         std::string("__builtin_doc_list_destroy__"));
        environment->define("doc_list_count",           std::string("__builtin_doc_list_count__"));
        environment->define("doc_list_get_map",         std::string("__builtin_doc_list_get_map__"));
        // ORM
        environment->define("orm_create",               std::string("__builtin_orm_create__"));
        environment->define("orm_destroy",              std::string("__builtin_orm_destroy__"));
        environment->define("orm_fields_create",        std::string("__builtin_orm_fields_create__"));
        environment->define("orm_fields_destroy",       std::string("__builtin_orm_fields_destroy__"));
        environment->define("orm_fields_add",           std::string("__builtin_orm_fields_add__"));
        environment->define("orm_register",             std::string("__builtin_orm_register__"));
        environment->define("orm_migrate",              std::string("__builtin_orm_migrate__"));
        environment->define("orm_find_all",             std::string("__builtin_orm_find_all__"));
        environment->define("orm_find_by",              std::string("__builtin_orm_find_by__"));
        environment->define("orm_insert",               std::string("__builtin_orm_insert__"));
        environment->define("orm_update",               std::string("__builtin_orm_update__"));
        environment->define("orm_delete",               std::string("__builtin_orm_delete__"));
        return;
    }

    if (stmt.is_from_import) {
        for (const auto& name : stmt.imported_names) {
            environment->define(name, Value{});
        }
    } else {
        std::string import_name = stmt.alias.empty() ? stmt.module_name : stmt.alias;
        environment->define(import_name, Value{});
    }
}

void Interpreter::visitTraitDecl(TraitDecl& stmt) {
    // For now, traits are just registered but not stored in the environment
    // In a full implementation, we'd register them in a trait registry
    // and use them for type checking and method dispatch
    
    // Placeholder: just acknowledge the trait exists
    std::cout << "Trait '" << stmt.name << "' declared" << std::endl;
}

void Interpreter::visitImplBlock(ImplBlock& stmt) {
    // Store trait implementation methods
    // Key: (type_name, trait_name) -> map of method_name -> Function
    
    if (stmt.for_type_name.empty()) {
        // Impl block inside a class - we'll handle this later
        std::cout << "Impl block for trait '" << stmt.trait_name << "' (inside class) registered" << std::endl;
        return;
    }
    
    // Standalone impl block: impl Trait for Type
    std::cout << "Impl block for trait '" << stmt.trait_name << "' for type '" << stmt.for_type_name << "' registered" << std::endl;
    
    // Store each method
    auto key = std::make_pair(stmt.for_type_name, stmt.trait_name);
    
    for (auto& method : stmt.methods) {
        auto func = std::make_shared<Function>(
            method->name,
            method->parameters,
            method->return_type,
            &method->body,
            environment
        );
        
        trait_impls[key][method->name] = func;
    }
}

// Channel send statement: ch <- value
void Interpreter::visitChannelSendStmt(ChannelSendStmt& stmt) {
    Value channel_val = evaluateExpr(*stmt.channel);
    
    if (!std::holds_alternative<std::shared_ptr<ChannelValue>>(channel_val)) {
        throw RuntimeError("Can only send to a channel");
    }
    
    auto channel = std::get<std::shared_ptr<ChannelValue>>(channel_val);
    Value value = evaluateExpr(*stmt.value);
    
    try {
        channel->send(value);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(e.what());
    }
}

// Select statement implementation
void Interpreter::visitSelectStmt(SelectStmt& stmt) {
    // Simple implementation: try each case in order
    // In a full implementation, this would use non-blocking operations
    // and wait for the first available channel
    
    // Try each case
    for (auto& select_case : stmt.cases) {
        Value channel_val = evaluateExpr(*select_case.channel);
        
        if (!std::holds_alternative<std::shared_ptr<ChannelValue>>(channel_val)) {
            throw RuntimeError("Select case must use a channel");
        }
        
        auto channel = std::get<std::shared_ptr<ChannelValue>>(channel_val);
        
        // Try to receive (non-blocking)
        if (!channel->empty()) {
            Value received = channel->receive();
            
            // Bind variable if specified
            if (!select_case.variable.empty()) {
                environment->define(select_case.variable, received);
            }
            
            // Execute case body
            for (auto& stmt : select_case.body) {
                stmt->accept(*this);
            }
            
            return;  // Exit after first successful case
        }
    }
    
    // If no case succeeded, execute default case if present
    if (!stmt.default_case.empty()) {
        for (auto& stmt : stmt.default_case) {
            stmt->accept(*this);
        }
    }
}

// Go statement implementation: spawn a real goroutine thread
void Interpreter::visitGoStmt(GoStmt& stmt) {
    // Capture the call expression arguments eagerly (evaluate in current thread)
    // then dispatch to a new thread
    std::string func_name;
    std::vector<Value> arg_values;

    // Evaluate callee name
    if (auto* var = dynamic_cast<VariableExpr*>(stmt.call->callee.get())) {
        func_name = var->name;
    }

    // Evaluate arguments in the calling thread
    for (auto& arg : stmt.call->arguments) {
        arg_values.push_back(evaluateExpr(*arg));
    }

    // Capture what we need for the thread
    auto env_snapshot = environment;
    auto self = this;

    std::lock_guard<std::mutex> lock(goroutine_mutex_);
    goroutine_threads_.emplace_back([self, func_name, arg_values, env_snapshot]() mutable {
        try {
            Value callee = env_snapshot->get(func_name);
            if (std::holds_alternative<std::shared_ptr<Function>>(callee)) {
                auto fn = std::get<std::shared_ptr<Function>>(callee);
                auto fn_env = std::make_shared<Environment>(fn->closure);
                for (size_t i = 0; i < fn->params.size() && i < arg_values.size(); i++) {
                    fn_env->define(fn->params[i], arg_values[i]);
                }
                // Create a lightweight interpreter for this goroutine
                Interpreter goroutine_interp;
                goroutine_interp.environment = fn_env;
                goroutine_interp.interpret(*fn->getBody());
            }
        } catch (...) {
            // Goroutine errors are silently swallowed (like Go)
        }
    });
}

// Execute all pending tasks
void Interpreter::executePendingTasks() {
    while (!pending_tasks.empty()) {
        auto task = std::move(pending_tasks.front());
        pending_tasks.erase(pending_tasks.begin());
        task();
    }
}

// Destructor — join all goroutine threads
Interpreter::~Interpreter() {
    std::lock_guard<std::mutex> lock(goroutine_mutex_);
    for (auto& t : goroutine_threads_) {
        if (t.joinable()) t.join();
    }
}

// Pattern matching implementation

void Interpreter::visitMatchExpr(MatchExpr& expr) {
    // Evaluate the scrutinee (the value being matched)
    Value scrutinee = evaluateExpr(*expr.scrutinee);
    
    // Try each arm in order
    for (auto& arm : expr.arms) {
        // Try each pattern in the arm (patterns are OR'd together)
        bool pattern_matched = false;
        std::map<std::string, Value> bindings;
        
        for (auto& pat : arm.patterns) {
            std::map<std::string, Value> temp_bindings;
            if (matchPattern(*pat, scrutinee, temp_bindings)) {
                pattern_matched = true;
                bindings = std::move(temp_bindings);
                break;  // Found a matching pattern, no need to try others
            }
        }
        
        if (!pattern_matched) {
            continue;  // None of the patterns matched, try next arm
        }
        
        // Check guard if present
        if (arm.guard) {
            // Create temporary environment with pattern bindings
            auto guard_env = std::make_shared<Environment>(environment);
            for (const auto& [name, value] : bindings) {
                guard_env->define(name, value);
            }
            
            // Evaluate guard in temporary environment
            auto saved_env = environment;
            environment = guard_env;
            Value guard_result = evaluateExpr(*arm.guard);
            environment = saved_env;
            
            // If guard fails, try next arm
            if (!isTruthy(guard_result)) {
                continue;
            }
        }
        
        // Pattern matched (and guard passed if present)
        // Create environment with bindings and evaluate body
        auto match_env = std::make_shared<Environment>(environment);
        for (const auto& [name, value] : bindings) {
            match_env->define(name, value);
        }
        
        auto saved_env = environment;
        environment = match_env;
        last_value = evaluateExpr(*arm.body);
        environment = saved_env;
        
        return;
    }
    
    // No pattern matched — check if any arm had a wildcard (exhaustiveness check)
    bool has_wildcard = false;
    for (auto& arm : expr.arms) {
        for (auto& pat : arm.patterns) {
            if (pat->type == Pattern::Type::WILDCARD ||
                pat->type == Pattern::Type::VARIABLE) {
                has_wildcard = true;
                break;
            }
        }
        if (has_wildcard) break;
    }
    if (!has_wildcard) {
        std::cerr << "warning[W001]: non-exhaustive match — add a wildcard arm '_ => ...' to handle all cases\n";
    }
    throw std::runtime_error("Non-exhaustive pattern match");
}

bool Interpreter::matchPattern(Pattern& pattern, const Value& value, 
                                std::map<std::string, Value>& bindings) {
    switch (pattern.type) {
        case Pattern::Type::WILDCARD:
            // Wildcard always matches, no bindings
            return true;
        
        case Pattern::Type::VARIABLE: {
            // Variable pattern always matches and binds the value
            auto& var_pattern = static_cast<VariablePattern&>(pattern);
            bindings[var_pattern.name] = value;
            return true;
        }
        
        case Pattern::Type::LITERAL: {
            // Literal pattern matches if values are equal
            auto& lit_pattern = static_cast<LiteralPattern&>(pattern);
            Value pattern_value = evaluateExpr(*lit_pattern.value);
            return valuesEqual(value, pattern_value);
        }
        
        case Pattern::Type::ARRAY: {
            // Array pattern matches arrays
            auto& arr_pattern = static_cast<ArrayPattern&>(pattern);
            
            if (!std::holds_alternative<std::shared_ptr<ArrayValue>>(value)) {
                return false;
            }
            
            auto arr = std::get<std::shared_ptr<ArrayValue>>(value);
            
            if (arr_pattern.has_rest) {
                // Pattern like [first, ...rest]
                if (arr->elements.size() < arr_pattern.elements.size()) {
                    return false;
                }
                
                // Match fixed elements
                for (size_t i = 0; i < arr_pattern.elements.size(); i++) {
                    if (!matchPattern(*arr_pattern.elements[i], arr->elements[i], bindings)) {
                        return false;
                    }
                }
                
                // Bind rest elements
                auto rest_arr = std::make_shared<ArrayValue>();
                for (size_t i = arr_pattern.elements.size(); i < arr->elements.size(); i++) {
                    rest_arr->elements.push_back(arr->elements[i]);
                }
                bindings[arr_pattern.rest_name] = rest_arr;
                
                return true;
            } else {
                // Exact length match required
                if (arr->elements.size() != arr_pattern.elements.size()) {
                    return false;
                }
                
                // Match each element
                for (size_t i = 0; i < arr_pattern.elements.size(); i++) {
                    if (!matchPattern(*arr_pattern.elements[i], arr->elements[i], bindings)) {
                        return false;
                    }
                }
                
                return true;
            }
        }
        
        case Pattern::Type::OBJECT: {
            // Object pattern matches instances
            auto& obj_pattern = static_cast<ObjectPattern&>(pattern);
            
            if (!std::holds_alternative<std::shared_ptr<Instance>>(value)) {
                return false;
            }
            
            auto instance = std::get<std::shared_ptr<Instance>>(value);
            
            // Match each field
            for (const auto& [field_name, field_pattern] : obj_pattern.fields) {
                auto it = instance->fields.find(field_name);
                if (it == instance->fields.end()) {
                    return false;  // Field doesn't exist
                }
                
                if (!matchPattern(*field_pattern, it->second, bindings)) {
                    return false;
                }
            }
            
            return true;
        }
        
        case Pattern::Type::CONSTRUCTOR: {
            // Constructor pattern matches Option/Result types
            auto& cons_pattern = static_cast<ConstructorPattern&>(pattern);
            
            if (cons_pattern.constructor_name == "Some") {
                // Some(x) pattern
                if (!std::holds_alternative<std::shared_ptr<OptionValue>>(value)) {
                    return false;
                }
                
                auto option = std::get<std::shared_ptr<OptionValue>>(value);
                if (!option->isSome()) {
                    return false;  // This is None, not Some
                }
                
                // Match inner pattern with the wrapped value
                if (cons_pattern.inner_pattern) {
                    return matchPattern(*cons_pattern.inner_pattern, option->value, bindings);
                }
                
                return true;
            } else if (cons_pattern.constructor_name == "None") {
                // None pattern
                if (!std::holds_alternative<std::shared_ptr<OptionValue>>(value)) {
                    return false;
                }
                
                auto option = std::get<std::shared_ptr<OptionValue>>(value);
                return option->isNone();
            } else if (cons_pattern.constructor_name == "Ok") {
                // Ok(x) pattern
                if (!std::holds_alternative<std::shared_ptr<ResultValue>>(value)) {
                    return false;
                }
                
                auto result = std::get<std::shared_ptr<ResultValue>>(value);
                if (!result->isOk()) {
                    return false;  // This is Err, not Ok
                }
                
                // Match inner pattern with the wrapped value
                if (cons_pattern.inner_pattern) {
                    return matchPattern(*cons_pattern.inner_pattern, result->value, bindings);
                }
                
                return true;
            } else if (cons_pattern.constructor_name == "Err") {
                // Err(e) pattern
                if (!std::holds_alternative<std::shared_ptr<ResultValue>>(value)) {
                    return false;
                }
                
                auto result = std::get<std::shared_ptr<ResultValue>>(value);
                if (!result->isErr()) {
                    return false;  // This is Ok, not Err
                }
                
                // Match inner pattern with the error value
                if (cons_pattern.inner_pattern) {
                    return matchPattern(*cons_pattern.inner_pattern, result->value, bindings);
                }
                
                return true;
            }
            
            return false;
        }
        
        default:
            return false;
    }
}

bool Interpreter::valuesEqual(const Value& a, const Value& b) {
    // Check if both values are the same type and equal
    if (a.index() != b.index()) {
        return false;
    }
    
    if (std::holds_alternative<int>(a)) {
        return std::get<int>(a) == std::get<int>(b);
    }
    if (std::holds_alternative<double>(a)) {
        return std::get<double>(a) == std::get<double>(b);
    }
    if (std::holds_alternative<std::string>(a)) {
        return std::get<std::string>(a) == std::get<std::string>(b);
    }
    if (std::holds_alternative<bool>(a)) {
        return std::get<bool>(a) == std::get<bool>(b);
    }
    if (std::holds_alternative<std::nullptr_t>(a)) {
        return true;  // Both are null
    }
    
    // For complex types, use pointer equality
    return false;
}

// Await expression implementation
void Interpreter::visitAwaitExpr(AwaitExpr& expr) {
    // For now, just evaluate the future expression directly
    // In a full implementation, this would:
    // 1. Check if the expression returns a Future
    // 2. Suspend the current task
    // 3. Resume when the future completes
    // 4. Return the future's value
    
    // Simplified implementation: just evaluate the expression
    last_value = evaluateExpr(*expr.future);
    
    // TODO: Implement proper async/await with:
    // - Future type checking
    // - Task suspension/resumption
    // - Event loop integration
}

// Channel expression implementation: chan<T>(capacity)
void Interpreter::visitChannelExpr(ChannelExpr& expr) {
    // Evaluate capacity if provided
    size_t capacity = 0;
    if (expr.capacity) {
        Value cap_val = evaluateExpr(*expr.capacity);
        if (std::holds_alternative<int>(cap_val)) {
            capacity = std::get<int>(cap_val);
        } else {
            throw RuntimeError("Channel capacity must be an integer");
        }
    }
    
    // Create a new channel
    auto channel = std::make_shared<ChannelValue>(capacity);
    last_value = channel;
}

// Channel receive expression: <-ch
void Interpreter::visitChannelReceiveExpr(ChannelReceiveExpr& expr) {
    Value channel_val = evaluateExpr(*expr.channel);
    
    if (!std::holds_alternative<std::shared_ptr<ChannelValue>>(channel_val)) {
        throw RuntimeError("Can only receive from a channel");
    }
    
    auto channel = std::get<std::shared_ptr<ChannelValue>>(channel_val);
    
    try {
        last_value = channel->receive();
    } catch (const std::runtime_error& e) {
        throw RuntimeError(e.what());
    }
}

// Try expression: expr? for error propagation
void Interpreter::visitTryExpr(TryExpr& expr) {
    Value result_val = evaluateExpr(*expr.operand);
    
    // Handle Result types (Ok → unwrap, Err → propagate)
    if (std::holds_alternative<std::shared_ptr<ResultValue>>(result_val)) {
        auto result = std::get<std::shared_ptr<ResultValue>>(result_val);
        if (result->isOk()) {
            last_value = result->value;
        } else {
            throw ReturnException(result_val);
        }
        return;
    }
    
    // Handle Option types (Some → unwrap, None → propagate)
    if (std::holds_alternative<std::shared_ptr<OptionValue>>(result_val)) {
        auto opt = std::get<std::shared_ptr<OptionValue>>(result_val);
        if (opt->isSome()) {
            last_value = opt->value;
        } else {
            // Propagate None by returning early
            throw ReturnException(result_val);
        }
        return;
    }
    
    throw RuntimeError("? operator can only be used with Result or Option types");
}

// Move expression: move expr
void Interpreter::visitMoveExpr(MoveExpr& expr) {
    // For now, just evaluate the operand
    // In a full implementation, this would transfer ownership
    // For now, we just evaluate and return the value
    evaluateExpr(*expr.operand);
}

// Stringify expression: stringify(expr) for macros
void Interpreter::visitStringifyExpr(StringifyExpr& expr) {
    // Convert the expression to a string representation
    // For now, we'll use a simple approach - just return the expression as a string
    // In a full implementation, this would reconstruct the source code from the AST
    
    // Try to evaluate the expression and convert to string
    try {
        Value val = evaluateExpr(*expr.operand);
        last_value = valueToString(val);
    } catch (...) {
        // If evaluation fails, return a placeholder
        last_value = std::string("<expression>");
    }
}

// F-string expression: f"hello {name} you are {age}"
void Interpreter::visitFStringExpr(FStringExpr& expr) {
    std::string result;
    for (auto& segment : expr.segments) {
        if (!segment.is_expr) {
            result += segment.text;
        } else {
            Value val = evaluateExpr(*segment.expr);
            result += valueToString(val);
        }
    }
    last_value = result;
}

// Macro declaration
void Interpreter::visitMacroDecl(MacroDecl& stmt) {
    // For now, macros are stored but not expanded
    // A full implementation would require AST transformation and substitution
    // This is a placeholder that allows macro declarations to be parsed
    
    // Store macro name in environment as a marker
    environment->define(stmt.name, std::string("<macro>"));
}

void Interpreter::visitUnsafeStmt(UnsafeStmt& stmt) {
    // Execute all statements in the unsafe block
    // In the future, this could disable certain safety checks
    for (auto& s : stmt.body) {
        s->accept(*this);
    }
}

void Interpreter::visitExtendDecl(ExtendDecl& stmt) {
    std::string type_name = stmt.type_name;
    
    // Register each method in the extension registry
    for (auto& method_ast : stmt.methods) {
        auto func = std::make_shared<Function>(
            method_ast->name,
            method_ast->parameters,
            method_ast->return_type,
            &method_ast->body,
            environment
        );
        
        // Handle decorators on extension methods
        for (auto& dec : method_ast->decorators) {
            Value decorator_val = environment->get(dec.name);
            if (std::holds_alternative<std::string>(decorator_val)) {
                std::string d = std::get<std::string>(decorator_val);
                if (d == "__builtin_staticmethod__") {
                    func->params.erase(func->params.begin());
                } else if (d == "__builtin_classmethod__") {
                    // classmethod — keep self-like first param
                }
            }
        }
        
        extensions[type_name][method_ast->name] = func;
    }
}

// Trait method lookup

std::shared_ptr<Function> Interpreter::findTraitMethod(const std::string& type_name, const std::string& method_name) {
    // Search through all trait implementations for this type
    for (const auto& [key, methods] : trait_impls) {
        const auto& [impl_type, trait_name] = key;
        
        if (impl_type == type_name) {
            auto it = methods.find(method_name);
            if (it != methods.end()) {
                return it->second;
            }
        }
    }
    
    return nullptr;
}

} // namespace sapphire
