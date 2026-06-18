#ifndef SAPPHIRE_INTERPRETER_H
#define SAPPHIRE_INTERPRETER_H

#include "../parser/stmt.h"
#include "../parser/expr.h"
#include "../error/exception.h"
#include <map>
#include <set>
#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <queue>
#include <stdexcept>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

#include "../../runtime/refcount.h"

namespace sapphire {

// Forward declarations
class Environment;
class Interpreter;
class Function;
class Class;
class Instance;
class BoundMethod;
struct ArrayValue;
struct HashMapValue;
class HashMapMethod;

// Function object to store function definitions
class Function {
public:
    std::string name;
    std::vector<Param> parameters;
    std::vector<std::string> params;  // Just parameter names for REPL display
    std::string return_type;
    std::vector<std::unique_ptr<Stmt>>* body;  // Pointer to AST body (for non-decorated functions)
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> owned_body;  // Owned copy (for decorated functions)
    std::shared_ptr<Environment> closure;
    bool is_async;
    
    // Constructor for regular functions (points to AST)
    Function(const std::string& n, 
             const std::vector<Param>& params_with_ownership,
             const std::string& ret_type,
             std::vector<std::unique_ptr<Stmt>>* b,
             std::shared_ptr<Environment> c,
             bool async = false)
        : name(n), parameters(params_with_ownership), return_type(ret_type), body(b), 
          closure(c), is_async(async) {
        // Extract just the parameter names
        for (const auto& p : params_with_ownership) {
            params.push_back(p.name);
        }
    }
    
    // Get the body to execute (either owned or pointed-to)
    std::vector<std::unique_ptr<Stmt>>* getBody() {
        return owned_body ? owned_body.get() : body;
    }
};

// Class object representing a user-defined class
class Class {
public:
    std::string name;
    std::shared_ptr<Class> superclass;
    std::map<std::string, std::shared_ptr<Function>> methods;
    std::set<std::string> static_methods;  // Methods marked with @staticmethod
    std::set<std::string> class_methods;   // Methods marked with @classmethod
    bool is_singleton;
    bool is_dataclass;  // Marked with @dataclass
    
    Class(const std::string& n,
          std::shared_ptr<Class> super,
          const std::map<std::string, std::shared_ptr<Function>>& m,
          bool singleton = false,
          bool dataclass = false)
        : name(n), superclass(std::move(super)), methods(m), 
          is_singleton(singleton), is_dataclass(dataclass) {}
    
    std::shared_ptr<Function> findMethod(const std::string& method_name) const;
};

// Forward declare Value for use in Instance
class ChannelValue;
class ChannelMethod;
class WaitGroupValue;
class WaitGroupMethod;
class StaticMethod;
class OptionValue;
class OptionMethod;
class ResultValue;
class ResultMethod;
class ArrayMethod;
class StringMethod;
class RcValue;
class RcMethod;
class ArcValue;
class ArcMethod;
class WeakValue;
class WeakMethod;
class ExtensionMethod;

using Value = std::variant<int, double, std::string, bool, std::nullptr_t, 
                           std::shared_ptr<ArrayValue>,
                           std::shared_ptr<HashMapValue>,
                           std::shared_ptr<Function>,
                           std::shared_ptr<Class>,
                           std::shared_ptr<Instance>,
                           std::shared_ptr<BoundMethod>,
                           std::shared_ptr<StaticMethod>,
                           std::shared_ptr<ChannelValue>,
                           std::shared_ptr<ChannelMethod>,
                           std::shared_ptr<WaitGroupValue>,
                           std::shared_ptr<WaitGroupMethod>,
                           std::shared_ptr<OptionValue>,
                           std::shared_ptr<OptionMethod>,
                           std::shared_ptr<ResultValue>,
                           std::shared_ptr<ResultMethod>,
                           std::shared_ptr<ArrayMethod>,
                           std::shared_ptr<StringMethod>,
                           std::shared_ptr<HashMapMethod>,
                           std::shared_ptr<RcValue>,
                           std::shared_ptr<RcMethod>,
                           std::shared_ptr<ArcValue>,
                           std::shared_ptr<ArcMethod>,
                           std::shared_ptr<WeakValue>,
                           std::shared_ptr<WeakMethod>,
                           std::shared_ptr<ExtensionMethod>>;

// Array type (defined after Value)
struct ArrayValue {
    std::vector<Value> elements;
};

// Hash map type
struct HashMapValue {
    std::map<std::string, Value> pairs;
    
    // Helper methods
    bool has(const std::string& key) const {
        return pairs.find(key) != pairs.end();
    }
    
    Value get(const std::string& key) const {
        auto it = pairs.find(key);
        return it != pairs.end() ? it->second : Value(nullptr);
    }
    
    void set(const std::string& key, const Value& value) {
        pairs[key] = value;
    }
    
    bool remove(const std::string& key) {
        return pairs.erase(key) > 0;
    }
    
    size_t size() const {
        return pairs.size();
    }
    
    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        for (const auto& pair : pairs) {
            result.push_back(pair.first);
        }
        return result;
    }
};

// Channel method wrapper (like BoundMethod but for channels)
class ChannelMethod {
public:
    std::shared_ptr<ChannelValue> channel;
    std::string method_name;
    
    ChannelMethod(std::shared_ptr<ChannelValue> ch, const std::string& name)
        : channel(std::move(ch)), method_name(name) {}
};

// WaitGroup method wrapper
class WaitGroupMethod {
public:
    std::shared_ptr<WaitGroupValue> waitgroup;
    std::string method_name;
    
    WaitGroupMethod(std::shared_ptr<WaitGroupValue> wg, const std::string& name)
        : waitgroup(std::move(wg)), method_name(name) {}
};

// Channel wrapper for runtime channels
class ChannelValue {
public:
    // We'll use a simple queue for now, can be enhanced later
    std::queue<Value> buffer;
    size_t capacity;
    bool closed;
    
    ChannelValue(size_t cap = 0) : capacity(cap), closed(false) {}
    
    void send(const Value& value) {
        if (closed) {
            throw std::runtime_error("Cannot send on closed channel");
        }
        if (capacity > 0 && buffer.size() >= capacity) {
            throw std::runtime_error("Channel buffer full");
        }
        buffer.push(value);
    }
    
    Value receive() {
        if (buffer.empty()) {
            if (closed) {
                throw std::runtime_error("Cannot receive from closed empty channel");
            }
            throw std::runtime_error("Channel is empty");
        }
        Value val = buffer.front();
        buffer.pop();
        return val;
    }
    
    void close() {
        closed = true;
    }
    
    bool is_closed() const {
        return closed;
    }
    
    bool empty() const {
        return buffer.empty();
    }
};

// WaitGroup for goroutine synchronization
class WaitGroupValue {
public:
    int counter;
    
    WaitGroupValue() : counter(0) {}
    
    void add(int delta) {
        counter += delta;
    }
    
    void done() {
        counter--;
        if (counter < 0) {
            throw std::runtime_error("WaitGroup counter cannot be negative");
        }
    }
    
    void wait() {
        // In a full implementation, this would block until counter reaches 0
        // For now, it's a no-op since we execute synchronously
        if (counter != 0) {
            throw std::runtime_error("WaitGroup wait() called with non-zero counter");
        }
    }
    
    int count() const {
        return counter;
    }
};

// Option type for handling nullable values
class OptionValue {
public:
    bool is_some;
    Value value;  // Only valid if is_some
    
    OptionValue() : is_some(false), value(nullptr) {}
    explicit OptionValue(const Value& v) : is_some(true), value(v) {}
    
    bool isSome() const { return is_some; }
    bool isNone() const { return !is_some; }
    
    Value unwrap() const {
        if (!is_some) {
            throw std::runtime_error("Called unwrap() on None");
        }
        return value;
    }
    
    Value unwrapOr(const Value& default_val) const {
        return is_some ? value : default_val;
    }
};

// Option method wrapper (like BoundMethod but for Option)
class OptionMethod {
public:
    std::shared_ptr<OptionValue> option;
    std::string method_name;
    
    OptionMethod(std::shared_ptr<OptionValue> opt, const std::string& name)
        : option(std::move(opt)), method_name(name) {}
};

// Result type for error handling
class ResultValue {
public:
    bool is_ok;
    Value value;  // Ok value or Err value
    
    ResultValue(bool ok, const Value& v) : is_ok(ok), value(v) {}
    
    bool isOk() const { return is_ok; }
    bool isErr() const { return !is_ok; }
    
    Value unwrap() const {
        if (!is_ok) {
            // Just throw a simple error without trying to stringify
            throw std::runtime_error("Called unwrap() on Err");
        }
        return value;
    }
    
    Value unwrapOr(const Value& default_val) const {
        return is_ok ? value : default_val;
    }
    
    Value unwrapErr() const {
        if (is_ok) {
            throw std::runtime_error("Called unwrap_err() on Ok");
        }
        return value;
    }
};

// Result method wrapper
class ResultMethod {
public:
    std::shared_ptr<ResultValue> result;
    std::string method_name;
    
    ResultMethod(std::shared_ptr<ResultValue> res, const std::string& name)
        : result(std::move(res)), method_name(name) {}
};

// Array method wrapper (for array methods like push, pop)
class ArrayMethod {
public:
    std::shared_ptr<ArrayValue> array;
    std::string method_name;
    
    ArrayMethod(std::shared_ptr<ArrayValue> arr, const std::string& name)
        : array(std::move(arr)), method_name(name) {}
};

// String method wrapper (for string methods like split, upper)
class StringMethod {
public:
    std::string string_value;
    std::string method_name;
    
    StringMethod(const std::string& str, const std::string& name)
        : string_value(str), method_name(name) {}
};

// Hash map method wrapper (for hash map methods like get, set, keys)
class HashMapMethod {
public:
    std::shared_ptr<HashMapValue> hashmap;
    std::string method_name;
    
    HashMapMethod(std::shared_ptr<HashMapValue> hm, const std::string& name)
        : hashmap(std::move(hm)), method_name(name) {}
};

// Rc reference-counted pointer (wraps runtime::Rc<Value>)
class RcValue {
public:
    runtime::Rc<Value> inner;

    explicit RcValue(const Value& v) : inner(new Value(v)) {}
    explicit RcValue(runtime::Rc<Value> rc) : inner(std::move(rc)) {}

    uint32_t use_count() const { return inner.use_count(); }
    Value& borrow() { return *inner; }
};

// Rc method wrapper
class RcMethod {
public:
    std::shared_ptr<RcValue> rc;
    std::string method_name;

    RcMethod(std::shared_ptr<RcValue> r, const std::string& name)
        : rc(std::move(r)), method_name(name) {}
};

// Arc atomic reference-counted pointer (wraps runtime::Arc<Value>)
class ArcValue {
public:
    runtime::Arc<Value> inner;

    explicit ArcValue(const Value& v) : inner(new Value(v)) {}
    explicit ArcValue(runtime::Arc<Value> arc) : inner(std::move(arc)) {}

    uint32_t use_count() const { return inner.use_count(); }
    Value& borrow() { return *inner; }
};

// Arc method wrapper
class ArcMethod {
public:
    std::shared_ptr<ArcValue> arc;
    std::string method_name;

    ArcMethod(std::shared_ptr<ArcValue> a, const std::string& name)
        : arc(std::move(a)), method_name(name) {}
};

// Weak reference (wraps runtime::Weak<Value>)
class WeakValue {
public:
    runtime::Weak<Value> inner;

    explicit WeakValue(const std::shared_ptr<RcValue>& r) : inner(r->inner) {}
    explicit WeakValue(const std::shared_ptr<ArcValue>& a) : inner(a->inner) {}

    bool expired() const { return inner.expired(); }
    uint32_t use_count() const { return inner.use_count(); }
    runtime::Rc<Value> upgrade_rc() { return inner.upgrade_rc(); }
    runtime::Arc<Value> upgrade_arc() { return inner.upgrade_arc(); }
};

// Weak method wrapper
class WeakMethod {
public:
    std::shared_ptr<WeakValue> weak;
    std::string method_name;

    WeakMethod(std::shared_ptr<WeakValue> w, const std::string& name)
        : weak(std::move(w)), method_name(name) {}
};

// Extension method wrapper (for extend keyword)
class ExtensionMethod {
public:
    Value object;  // The value the method is called on
    std::shared_ptr<Function> method;
    
    ExtensionMethod(const Value& obj, std::shared_ptr<Function> m)
        : object(obj), method(std::move(m)) {}
};

// Instance of a class
class Instance {
public:
    std::shared_ptr<Class> klass;
    std::map<std::string, Value> fields;
    
    explicit Instance(std::shared_ptr<Class> k) : klass(std::move(k)) {}
};

// Bound method (instance + function)
class BoundMethod {
public:
    std::shared_ptr<Instance> instance;
    std::shared_ptr<Function> method;
    
    BoundMethod(std::shared_ptr<Instance> inst, std::shared_ptr<Function> m)
        : instance(std::move(inst)), method(std::move(m)) {}
};

// Static method (class + function, no instance)
class StaticMethod {
public:
    std::shared_ptr<Class> klass;
    std::shared_ptr<Function> method;
    bool is_classmethod;  // True for @classmethod, false for @staticmethod
    
    StaticMethod(std::shared_ptr<Class> cls, std::shared_ptr<Function> m, bool classmethod = false)
        : klass(std::move(cls)), method(std::move(m)), is_classmethod(classmethod) {}
};

class Environment {
private:
    std::map<std::string, Value> values;
    std::map<std::string, Ownership> ownerships;
    std::shared_ptr<Environment> enclosing;

public:
    Environment() : enclosing(nullptr) {}
    explicit Environment(std::shared_ptr<Environment> enc) : enclosing(enc) {}
    
    void define(const std::string& name, const Value& value, Ownership o = Ownership::NONE);
    Value get(const std::string& name);
    void assign(const std::string& name, const Value& value);
    Ownership getOwnership(const std::string& name);
    
    // REPL support: get all variables in current scope
    std::map<std::string, Value> getAllVariables() const {
        return values;
    }
};

class ReturnException : public std::exception {
public:
    Value value;
    explicit ReturnException(const Value& v) : value(v) {}
};

class BreakException : public std::exception {};

class ContinueException : public std::exception {};

class Interpreter : public ExprVisitor, public StmtVisitor {
private:
    std::shared_ptr<Environment> environment;
    Value last_value;
    
    // Trait registry: maps (type_name, trait_name) -> methods
    std::map<std::pair<std::string, std::string>, std::map<std::string, std::shared_ptr<Function>>> trait_impls;
    
    // Extension registry: maps type_name -> (method_name -> function)
    std::map<std::string, std::map<std::string, std::shared_ptr<Function>>> extensions;
    
    // Macro registry: maps macro_name -> MacroDecl
    std::map<std::string, std::shared_ptr<MacroDecl>> macros;
    
    // Task queue for goroutines
    std::vector<std::function<void()>> pending_tasks;
    std::mutex goroutine_mutex_;
    std::vector<std::thread> goroutine_threads_;
    
    // Cache for @cache decorator: maps (function_name, args_string) -> result
    std::map<std::pair<std::string, std::string>, Value> function_cache;
    
    // Stack trace tracking
    std::vector<StackFrame> call_stack;
    
    bool isTruthy(const Value& value);
    Value evaluateExpr(Expr& expr);
    void throwException(const std::string& type, const std::string& message);
    
    // Trait helpers
    std::shared_ptr<Function> findTraitMethod(const std::string& type_name, const std::string& method_name);
    
    // Task execution
    void executePendingTasks();

public:
    Interpreter();
    ~Interpreter();
    void interpret(std::vector<std::unique_ptr<Stmt>>& statements);
    
    // REPL support methods
    Value getLastValue() const { return last_value; }
    std::shared_ptr<Environment> getEnvironment() const { return environment; }
    std::string valueToString(const Value& value);  // Make public for REPL
    
    // Expression visitors
    void visitLiteralExpr(LiteralExpr& expr) override;
    void visitVariableExpr(VariableExpr& expr) override;
    void visitAssignExpr(AssignExpr& expr) override;
    void visitBinaryExpr(BinaryExpr& expr) override;
    void visitUnaryExpr(UnaryExpr& expr) override;
    void visitCallExpr(CallExpr& expr) override;
    void visitListExpr(ListExpr& expr) override;
    void visitIndexExpr(IndexExpr& expr) override;
    void visitGetExpr(GetExpr& expr) override;
    void visitSetExpr(SetExpr& expr) override;
    void visitIndexAssignExpr(IndexAssignExpr& expr) override;
    void visitHashMapExpr(HashMapExpr& expr) override;
    void visitMatchExpr(MatchExpr& expr) override;
    void visitAwaitExpr(AwaitExpr& expr) override;
    void visitChannelExpr(ChannelExpr& expr) override;
    void visitChannelReceiveExpr(ChannelReceiveExpr& expr) override;
    void visitTryExpr(TryExpr& expr) override;
    void visitStringifyExpr(StringifyExpr& expr) override;
    void visitMoveExpr(MoveExpr& expr) override;
    void visitFStringExpr(FStringExpr& expr) override;
    
    // Pattern matching helpers
    bool matchPattern(Pattern& pattern, const Value& value, std::map<std::string, Value>& bindings);
    bool valuesEqual(const Value& a, const Value& b);
    
    // Statement visitors
    void visitExprStmt(ExprStmt& stmt) override;
    void visitVarDeclStmt(VarDeclStmt& stmt) override;
    void visitFunctionDecl(FunctionDecl& stmt) override;
    void visitReturnStmt(ReturnStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitWhileStmt(WhileStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;
    void visitBreakStmt(BreakStmt& stmt) override;
    void visitContinueStmt(ContinueStmt& stmt) override;
    void visitUnsafeStmt(UnsafeStmt& stmt) override;
    void visitTryStmt(TryStmt& stmt) override;
    void visitThrowStmt(ThrowStmt& stmt) override;
    void visitClassDecl(ClassDecl& stmt) override;
    void visitImportStmt(ImportStmt& stmt) override;
    void visitTraitDecl(TraitDecl& stmt) override;
    void visitImplBlock(ImplBlock& stmt) override;
    void visitChannelSendStmt(ChannelSendStmt& stmt) override;
    void visitSelectStmt(SelectStmt& stmt) override;
    void visitGoStmt(GoStmt& stmt) override;
    void visitMacroDecl(MacroDecl& stmt) override;
    void visitExtendDecl(ExtendDecl& stmt) override;
};

} // namespace sapphire

#endif // SAPPHIRE_INTERPRETER_H
