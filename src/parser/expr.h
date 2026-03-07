#ifndef SAPPHIRE_EXPR_H
#define SAPPHIRE_EXPR_H

#include <memory>
#include <string>
#include <vector>

namespace sapphire {

// Forward declarations
class ExprVisitor;

// Base expression class
class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& visitor) = 0;
    virtual std::unique_ptr<Expr> clone() const { return nullptr; }  // Override in subclasses
};

// Literal expression
class LiteralExpr : public Expr {
public:
    enum class Type { INTEGER, FLOAT, STRING, BOOLEAN, NONE };
    Type type;
    std::string value;
    
    LiteralExpr(Type t, const std::string& v) : type(t), value(v) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<LiteralExpr>(type, value);
    }
};

// Variable expression
class VariableExpr : public Expr {
public:
    std::string name;
    
    explicit VariableExpr(const std::string& n) : name(n) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<VariableExpr>(name);
    }
};

// Assignment expression
class AssignExpr : public Expr {
public:
    std::string name;
    std::unique_ptr<Expr> value;
    
    AssignExpr(const std::string& n, std::unique_ptr<Expr> v)
        : name(n), value(std::move(v)) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<AssignExpr>(name, value ? value->clone() : nullptr);
    }
};

// Binary expression
class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
    
    BinaryExpr(std::unique_ptr<Expr> l, const std::string& o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<BinaryExpr>(
            left ? left->clone() : nullptr,
            op,
            right ? right->clone() : nullptr
        );
    }
};

// Unary expression
class UnaryExpr : public Expr {
public:
    std::string op;
    std::unique_ptr<Expr> operand;
    
    UnaryExpr(const std::string& o, std::unique_ptr<Expr> operand)
        : op(o), operand(std::move(operand)) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<UnaryExpr>(op, operand ? operand->clone() : nullptr);
    }
};

// Call expression
class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    
    CallExpr(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> cloned_args;
        for (const auto& arg : arguments) {
            cloned_args.push_back(arg ? arg->clone() : nullptr);
        }
        return std::make_unique<CallExpr>(
            callee ? callee->clone() : nullptr,
            std::move(cloned_args)
        );
    }
};

// List expression
class ListExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    
    explicit ListExpr(std::vector<std::unique_ptr<Expr>> elems)
        : elements(std::move(elems)) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> cloned_elems;
        for (const auto& elem : elements) {
            cloned_elems.push_back(elem ? elem->clone() : nullptr);
        }
        return std::make_unique<ListExpr>(std::move(cloned_elems));
    }
};

// Index expression
class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;
    
    IndexExpr(std::unique_ptr<Expr> obj, std::unique_ptr<Expr> idx)
        : object(std::move(obj)), index(std::move(idx)) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<IndexExpr>(
            object ? object->clone() : nullptr,
            index ? index->clone() : nullptr
        );
    }
};

// Property access expression: object.name
class GetExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::string name;
    
    GetExpr(std::unique_ptr<Expr> obj, const std::string& n)
        : object(std::move(obj)), name(n) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<GetExpr>(
            object ? object->clone() : nullptr,
            name
        );
    }
};

// Property assignment expression: object.name = value
class SetExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::string name;
    std::unique_ptr<Expr> value;
    
    SetExpr(std::unique_ptr<Expr> obj, const std::string& n, std::unique_ptr<Expr> v)
        : object(std::move(obj)), name(n), value(std::move(v)) {}
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<SetExpr>(
            object ? object->clone() : nullptr,
            name,
            value ? value->clone() : nullptr
        );
    }
};

// Pattern base class for pattern matching
class Pattern {
public:
    enum class Type {
        LITERAL,      // 0, "hello", true
        VARIABLE,     // x, name
        WILDCARD,     // _
        ARRAY,        // [a, b, c]
        OBJECT,       // {x, y}
        REST,         // ...rest
        CONSTRUCTOR   // Some(x), None, Ok(x), Err(e)
    };
    
    Type type;
    virtual ~Pattern() = default;
    
    explicit Pattern(Type t) : type(t) {}
    
    // Clone method for pattern duplication
    virtual std::unique_ptr<Pattern> clone() const = 0;
};

// Literal pattern: 0, "hello", true
class LiteralPattern : public Pattern {
public:
    std::unique_ptr<Expr> value;
    
    explicit LiteralPattern(std::unique_ptr<Expr> v)
        : Pattern(Type::LITERAL), value(std::move(v)) {}
    
    std::unique_ptr<Pattern> clone() const override {
        return std::make_unique<LiteralPattern>(value ? value->clone() : nullptr);
    }
};

// Variable pattern: x, name (binds value to variable)
class VariablePattern : public Pattern {
public:
    std::string name;
    
    explicit VariablePattern(const std::string& n)
        : Pattern(Type::VARIABLE), name(n) {}
    
    std::unique_ptr<Pattern> clone() const override {
        return std::make_unique<VariablePattern>(name);
    }
};

// Wildcard pattern: _ (matches anything, doesn't bind)
class WildcardPattern : public Pattern {
public:
    WildcardPattern() : Pattern(Type::WILDCARD) {}
    
    std::unique_ptr<Pattern> clone() const override {
        return std::make_unique<WildcardPattern>();
    }
};

// Array pattern: [a, b, c] or [first, ...rest]
class ArrayPattern : public Pattern {
public:
    std::vector<std::unique_ptr<Pattern>> elements;
    bool has_rest;
    std::string rest_name; // Name for ...rest pattern
    
    ArrayPattern(std::vector<std::unique_ptr<Pattern>> elems, bool rest = false, const std::string& rest_n = "")
        : Pattern(Type::ARRAY), elements(std::move(elems)), has_rest(rest), rest_name(rest_n) {}
    
    std::unique_ptr<Pattern> clone() const override {
        std::vector<std::unique_ptr<Pattern>> cloned_elems;
        for (const auto& elem : elements) {
            cloned_elems.push_back(elem ? elem->clone() : nullptr);
        }
        return std::make_unique<ArrayPattern>(std::move(cloned_elems), has_rest, rest_name);
    }
};

// Object pattern: {x, y} or {name: n, age: a}
class ObjectPattern : public Pattern {
public:
    std::vector<std::pair<std::string, std::unique_ptr<Pattern>>> fields;
    
    explicit ObjectPattern(std::vector<std::pair<std::string, std::unique_ptr<Pattern>>> f)
        : Pattern(Type::OBJECT), fields(std::move(f)) {}
    
    std::unique_ptr<Pattern> clone() const override {
        std::vector<std::pair<std::string, std::unique_ptr<Pattern>>> cloned_fields;
        for (const auto& [key, pattern] : fields) {
            cloned_fields.emplace_back(key, pattern ? pattern->clone() : nullptr);
        }
        return std::make_unique<ObjectPattern>(std::move(cloned_fields));
    }
};

// Constructor pattern: Some(x), None, Ok(x), Err(e)
class ConstructorPattern : public Pattern {
public:
    std::string constructor_name;  // "Some", "None", "Ok", "Err"
    std::unique_ptr<Pattern> inner_pattern;  // Pattern for the wrapped value (null for None)
    
    ConstructorPattern(const std::string& name, std::unique_ptr<Pattern> inner = nullptr)
        : Pattern(Type::CONSTRUCTOR), constructor_name(name), inner_pattern(std::move(inner)) {}
    
    std::unique_ptr<Pattern> clone() const override {
        return std::make_unique<ConstructorPattern>(
            constructor_name,
            inner_pattern ? inner_pattern->clone() : nullptr
        );
    }
};

// Match arm: pattern (with optional guard) => body
// Supports multiple patterns: 0 | 1 | 2 => "low"
class MatchArm {
public:
    std::vector<std::unique_ptr<Pattern>> patterns;  // Multiple patterns separated by |
    std::unique_ptr<Expr> guard;  // Optional: if condition
    std::unique_ptr<Expr> body;
    
    MatchArm(std::vector<std::unique_ptr<Pattern>> p, std::unique_ptr<Expr> g, std::unique_ptr<Expr> b)
        : patterns(std::move(p)), guard(std::move(g)), body(std::move(b)) {}
    
    // Explicitly defaulted move constructor
    MatchArm(MatchArm&&) = default;
    MatchArm& operator=(MatchArm&&) = default;
};

// Match expression
class MatchExpr : public Expr {
public:
    std::unique_ptr<Expr> scrutinee;  // The value being matched
    std::vector<MatchArm> arms;
    
    MatchExpr(std::unique_ptr<Expr> s, std::vector<MatchArm> a)
        : scrutinee(std::move(s)), arms(std::move(a)) {}
    
    void accept(ExprVisitor& visitor) override;
};

// Await expression: await future_expr
class AwaitExpr : public Expr {
public:
    std::unique_ptr<Expr> future;
    
    explicit AwaitExpr(std::unique_ptr<Expr> f)
        : future(std::move(f)) {}
    
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        // Await expressions can't be easily cloned
        return nullptr;
    }
};

// Channel creation expression: chan<T>(capacity)
class ChannelExpr : public Expr {
public:
    std::string element_type;  // Type of elements in channel
    std::unique_ptr<Expr> capacity;  // Optional capacity for buffered channels
    
    ChannelExpr(const std::string& type, std::unique_ptr<Expr> cap = nullptr)
        : element_type(type), capacity(std::move(cap)) {}
    
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<ChannelExpr>(
            element_type,
            capacity ? capacity->clone() : nullptr
        );
    }
};

// Channel receive expression: <-ch
class ChannelReceiveExpr : public Expr {
public:
    std::unique_ptr<Expr> channel;
    
    explicit ChannelReceiveExpr(std::unique_ptr<Expr> ch)
        : channel(std::move(ch)) {}
    
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<ChannelReceiveExpr>(
            channel ? channel->clone() : nullptr
        );
    }
};

// Try expression: expr? for error propagation
class TryExpr : public Expr {
public:
    std::unique_ptr<Expr> operand;
    
    explicit TryExpr(std::unique_ptr<Expr> op)
        : operand(std::move(op)) {}
    
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<TryExpr>(
            operand ? operand->clone() : nullptr
        );
    }
};

// Stringify expression: stringify(expr) for macros
class StringifyExpr : public Expr {
public:
    std::unique_ptr<Expr> operand;
    
    explicit StringifyExpr(std::unique_ptr<Expr> op)
        : operand(std::move(op)) {}
    
    void accept(ExprVisitor& visitor) override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<StringifyExpr>(
            operand ? operand->clone() : nullptr
        );
    }
};


// Visitor interface
class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visitLiteralExpr(LiteralExpr& expr) = 0;
    virtual void visitVariableExpr(VariableExpr& expr) = 0;
    virtual void visitAssignExpr(AssignExpr& expr) = 0;
    virtual void visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual void visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual void visitCallExpr(CallExpr& expr) = 0;
    virtual void visitListExpr(ListExpr& expr) = 0;
    virtual void visitIndexExpr(IndexExpr& expr) = 0;
    virtual void visitGetExpr(GetExpr& expr) = 0;
    virtual void visitSetExpr(SetExpr& expr) = 0;
    virtual void visitMatchExpr(MatchExpr& expr) = 0;
    virtual void visitAwaitExpr(AwaitExpr& expr) = 0;
    virtual void visitChannelExpr(ChannelExpr& expr) = 0;
    virtual void visitChannelReceiveExpr(ChannelReceiveExpr& expr) = 0;
    virtual void visitTryExpr(TryExpr& expr) = 0;
    virtual void visitStringifyExpr(StringifyExpr& expr) = 0;
};

} // namespace sapphire

#endif // SAPPHIRE_EXPR_H
