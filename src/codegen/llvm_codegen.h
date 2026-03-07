#ifndef SAPPHIRE_LLVM_CODEGEN_H
#define SAPPHIRE_LLVM_CODEGEN_H

#include <memory>
#include <string>
#include <map>
#include <vector>

// Forward declarations for LLVM types
namespace llvm {
    class Value;
    class Function;
}

namespace sapphire {

// Forward declarations
class Stmt;
class Expr;
class BinaryExpr;
class UnaryExpr;
class LiteralExpr;
class VariableExpr;
class CallExpr;
class MatchExpr;
class VarDeclStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class FunctionDecl;
class ReturnStmt;

/**
 * LLVM Code Generator
 * 
 * Transforms Sapphire AST into LLVM IR, which is then compiled to native machine code.
 */
class LLVMCodeGen {
public:
    LLVMCodeGen(const std::string& module_name);
    ~LLVMCodeGen();

    // Main entry point: generate code for a list of statements
    void generate(const std::vector<std::unique_ptr<Stmt>>& statements);
    
    // Output methods
    void printIR() const;                          // Print LLVM IR to stdout
    void writeIR(const std::string& filename);     // Write IR to file
    void writeObject(const std::string& filename); // Write object file
    void writeExecutable(const std::string& filename); // Write executable
    
    // Optimization
    void optimize(int level = 2); // 0=none, 1=basic, 2=default, 3=aggressive
    
private:
    // Expression generation
    llvm::Value* generateExpr(Expr* expr);
    llvm::Value* generateBinaryExpr(BinaryExpr* expr);
    llvm::Value* generateLogicalExpr(BinaryExpr* expr);
    llvm::Value* generateUnaryExpr(UnaryExpr* expr);
    llvm::Value* generateLiteralExpr(LiteralExpr* expr);
    llvm::Value* generateVariableExpr(VariableExpr* expr);
    llvm::Value* generateCallExpr(CallExpr* expr);
    llvm::Value* generateMatchExpr(MatchExpr* expr);
    
    // Statement generation
    void generateStmt(Stmt* stmt);
    void generateVarDeclStmt(VarDeclStmt* stmt);
    void generateIfStmt(IfStmt* stmt);
    void generateWhileStmt(WhileStmt* stmt);
    void generateForStmt(ForStmt* stmt);
    void generateFunctionDecl(FunctionDecl* stmt);
    void generateReturnStmt(ReturnStmt* stmt);
    
    // Built-in functions
    llvm::Value* generatePrintCall(CallExpr* expr);
    llvm::Function* getPrintfFunction();
    
    // Module name
    std::string module_name_;
    
    // Opaque pointer to LLVM implementation details
    // This avoids exposing LLVM headers in our public interface
    void* impl_;
    
    // Current function being generated (opaque)
    void* current_function_;
};

} // namespace sapphire

#endif // SAPPHIRE_LLVM_CODEGEN_H
