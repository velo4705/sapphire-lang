#ifndef SAPPHIRE_STMT_H
#define SAPPHIRE_STMT_H

#include "expr.h"
#include <memory>
#include <string>
#include <vector>

namespace sapphire {

// Forward declarations
class StmtVisitor;

// Decorator for functions and classes
class Decorator {
public:
    std::string name;
    std::vector<std::unique_ptr<Expr>> arguments;
    
    Decorator(const std::string& n, std::vector<std::unique_ptr<Expr>> args)
        : name(n), arguments(std::move(args)) {}
    
    // Move constructor
    Decorator(Decorator&&) = default;
    Decorator& operator=(Decorator&&) = default;
};

// Base statement class
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

// Expression statement
class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    
    explicit ExprStmt(std::unique_ptr<Expr> expr)
        : expression(std::move(expr)) {}
    void accept(StmtVisitor& visitor) override;
};

// Variable declaration
class VarDeclStmt : public Stmt {
public:
    std::string name;
    std::string type;
    std::unique_ptr<Expr> initializer;
    bool is_const;
    
    VarDeclStmt(const std::string& n, const std::string& t, 
                std::unique_ptr<Expr> init, bool is_const = false)
        : name(n), type(t), initializer(std::move(init)), is_const(is_const) {}
    void accept(StmtVisitor& visitor) override;
};

// Function declaration
class FunctionDecl : public Stmt {
public:
    std::vector<Decorator> decorators;
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters;
    std::string return_type;
    std::vector<std::unique_ptr<Stmt>> body;
    bool is_async;
    
    FunctionDecl(std::vector<Decorator> decs,
                 const std::string& n,
                 std::vector<std::pair<std::string, std::string>> params,
                 const std::string& ret_type,
                 std::vector<std::unique_ptr<Stmt>> b,
                 bool async = false)
        : decorators(std::move(decs)), name(n), parameters(std::move(params)), return_type(ret_type),
          body(std::move(b)), is_async(async) {}
    void accept(StmtVisitor& visitor) override;
};

// Return statement

class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;
    
    explicit ReturnStmt(std::unique_ptr<Expr> v = nullptr)
        : value(std::move(v)) {}
    void accept(StmtVisitor& visitor) override;
};

// If statement
class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> then_branch;
    std::vector<std::unique_ptr<Stmt>> else_branch;
    
    IfStmt(std::unique_ptr<Expr> cond,
           std::vector<std::unique_ptr<Stmt>> then_b,
           std::vector<std::unique_ptr<Stmt>> else_b = {})
        : condition(std::move(cond)), then_branch(std::move(then_b)),
          else_branch(std::move(else_b)) {}
    void accept(StmtVisitor& visitor) override;
};

// While statement
class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
    
    WhileStmt(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt>> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void accept(StmtVisitor& visitor) override;
};

// For statement
class ForStmt : public Stmt {
public:
    std::string variable;
    std::unique_ptr<Expr> iterable;
    std::vector<std::unique_ptr<Stmt>> body;
    
    ForStmt(const std::string& var, std::unique_ptr<Expr> iter,
            std::vector<std::unique_ptr<Stmt>> b)
        : variable(var), iterable(std::move(iter)), body(std::move(b)) {}
    void accept(StmtVisitor& visitor) override;
};

// Catch clause for try statement
class CatchClause {
public:
    std::string exception_type;  // Empty string means catch all
    std::string variable_name;   // Variable to bind exception to
    std::vector<std::unique_ptr<Stmt>> body;
    
    CatchClause(const std::string& type, const std::string& var,
                std::vector<std::unique_ptr<Stmt>> b)
        : exception_type(type), variable_name(var), body(std::move(b)) {}
};

// Try statement
class TryStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> try_body;
    std::vector<CatchClause> catch_clauses;
    std::vector<std::unique_ptr<Stmt>> finally_body;
    
    TryStmt(std::vector<std::unique_ptr<Stmt>> try_b,
            std::vector<CatchClause> catches,
            std::vector<std::unique_ptr<Stmt>> finally_b)
        : try_body(std::move(try_b)),
          catch_clauses(std::move(catches)),
          finally_body(std::move(finally_b)) {}
    void accept(StmtVisitor& visitor) override;
};

// Throw statement
class ThrowStmt : public Stmt {
public:
    std::string exception_type;
    std::unique_ptr<Expr> message;
    
    ThrowStmt(const std::string& type, std::unique_ptr<Expr> msg)
        : exception_type(type), message(std::move(msg)) {}
    void accept(StmtVisitor& visitor) override;
};

// Class declaration
class ClassDecl : public Stmt {
public:
    std::vector<Decorator> decorators;
    std::string name;
    std::string superclass_name; // Empty if no base class
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    
    ClassDecl(std::vector<Decorator> decs,
              const std::string& n,
              const std::string& super,
              std::vector<std::unique_ptr<FunctionDecl>> m)
        : decorators(std::move(decs)), name(n), superclass_name(super), methods(std::move(m)) {}
    void accept(StmtVisitor& visitor) override;
};

// Import statement
class ImportStmt : public Stmt {
public:
    std::string module_name;
    std::string alias; // Empty if no alias
    std::vector<std::string> imported_names; // Empty for "import module"
    bool is_from_import; // True for "from module import name"
    
    // Constructor for simple import: import module
    ImportStmt(const std::string& module, const std::string& alias = "")
        : module_name(module), alias(alias), is_from_import(false) {}
    
    // Constructor for from import: from module import name1, name2
    ImportStmt(const std::string& module, const std::vector<std::string>& names)
        : module_name(module), imported_names(names), is_from_import(true) {}
    
    void accept(StmtVisitor& visitor) override;
};

// Trait declaration
class TraitDecl : public Stmt {
public:
    std::string name;
    std::vector<std::string> super_traits;
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    
    TraitDecl(const std::string& n,
              std::vector<std::string> supers,
              std::vector<std::unique_ptr<FunctionDecl>> m)
        : name(n), super_traits(std::move(supers)), methods(std::move(m)) {}
    
    void accept(StmtVisitor& visitor) override;
};

// Impl block (trait implementation)
class ImplBlock : public Stmt {
public:
    std::string trait_name;
    std::string for_type_name;
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    
    ImplBlock(const std::string& trait,
              const std::string& for_type,
              std::vector<std::unique_ptr<FunctionDecl>> m)
        : trait_name(trait), for_type_name(for_type), methods(std::move(m)) {}
    
    void accept(StmtVisitor& visitor) override;
};

// Channel send statement: ch <- value
class ChannelSendStmt : public Stmt {
public:
    std::unique_ptr<Expr> channel;
    std::unique_ptr<Expr> value;
    
    ChannelSendStmt(std::unique_ptr<Expr> ch, std::unique_ptr<Expr> val)
        : channel(std::move(ch)), value(std::move(val)) {}
    
    void accept(StmtVisitor& visitor) override;
};

// Select case for select statement
class SelectCase {
public:
    std::unique_ptr<Expr> channel;  // Channel to receive from
    std::string variable;            // Variable to bind received value (empty if no binding)
    std::vector<std::unique_ptr<Stmt>> body;
    
    SelectCase(std::unique_ptr<Expr> ch, const std::string& var, 
               std::vector<std::unique_ptr<Stmt>> b)
        : channel(std::move(ch)), variable(var), body(std::move(b)) {}
    
    // Move constructor
    SelectCase(SelectCase&&) = default;
    SelectCase& operator=(SelectCase&&) = default;
};

// Select statement: select { case x = <-ch: ... }
class SelectStmt : public Stmt {
public:
    std::vector<SelectCase> cases;
    std::vector<std::unique_ptr<Stmt>> default_case;
    
    SelectStmt(std::vector<SelectCase> c, std::vector<std::unique_ptr<Stmt>> def)
        : cases(std::move(c)), default_case(std::move(def)) {}
    
    void accept(StmtVisitor& visitor) override;
};

// Go statement: go function_call()
class GoStmt : public Stmt {
public:
    std::unique_ptr<CallExpr> call;
    
    explicit GoStmt(std::unique_ptr<CallExpr> c)
        : call(std::move(c)) {}
    
    void accept(StmtVisitor& visitor) override;
};

// Macro declaration: macro name(params): body
class MacroDecl : public Stmt {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::unique_ptr<Stmt>> body;
    
    MacroDecl(const std::string& n, std::vector<std::string> params, 
              std::vector<std::unique_ptr<Stmt>> b)
        : name(n), parameters(std::move(params)), body(std::move(b)) {}
    
    void accept(StmtVisitor& visitor) override;
};

// Visitor interface
class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visitExprStmt(ExprStmt& stmt) = 0;
    virtual void visitVarDeclStmt(VarDeclStmt& stmt) = 0;
    virtual void visitFunctionDecl(FunctionDecl& stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt& stmt) = 0;
    virtual void visitIfStmt(IfStmt& stmt) = 0;
    virtual void visitWhileStmt(WhileStmt& stmt) = 0;
    virtual void visitForStmt(ForStmt& stmt) = 0;
    virtual void visitTryStmt(TryStmt& stmt) = 0;
    virtual void visitThrowStmt(ThrowStmt& stmt) = 0;
    virtual void visitClassDecl(ClassDecl& stmt) = 0;
    virtual void visitImportStmt(ImportStmt& stmt) = 0;
    virtual void visitTraitDecl(TraitDecl& stmt) = 0;
    virtual void visitImplBlock(ImplBlock& stmt) = 0;
    virtual void visitChannelSendStmt(ChannelSendStmt& stmt) = 0;
    virtual void visitSelectStmt(SelectStmt& stmt) = 0;
    virtual void visitGoStmt(GoStmt& stmt) = 0;
    virtual void visitMacroDecl(MacroDecl& stmt) = 0;
};

} // namespace sapphire

#endif // SAPPHIRE_STMT_H
