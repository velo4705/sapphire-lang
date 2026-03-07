#include "stmt.h"

namespace sapphire {

void ExprStmt::accept(StmtVisitor& visitor) {
    visitor.visitExprStmt(*this);
}

void VarDeclStmt::accept(StmtVisitor& visitor) {
    visitor.visitVarDeclStmt(*this);
}

void FunctionDecl::accept(StmtVisitor& visitor) {
    visitor.visitFunctionDecl(*this);
}

void ReturnStmt::accept(StmtVisitor& visitor) {
    visitor.visitReturnStmt(*this);
}

void IfStmt::accept(StmtVisitor& visitor) {
    visitor.visitIfStmt(*this);
}

void WhileStmt::accept(StmtVisitor& visitor) {
    visitor.visitWhileStmt(*this);
}

void ForStmt::accept(StmtVisitor& visitor) {
    visitor.visitForStmt(*this);
}

void TryStmt::accept(StmtVisitor& visitor) {
    visitor.visitTryStmt(*this);
}

void ThrowStmt::accept(StmtVisitor& visitor) {
    visitor.visitThrowStmt(*this);
}

void ClassDecl::accept(StmtVisitor& visitor) {
    visitor.visitClassDecl(*this);
}

void ImportStmt::accept(StmtVisitor& visitor) {
    visitor.visitImportStmt(*this);
}

void TraitDecl::accept(StmtVisitor& visitor) {
    visitor.visitTraitDecl(*this);
}

void ImplBlock::accept(StmtVisitor& visitor) {
    visitor.visitImplBlock(*this);
}

void ChannelSendStmt::accept(StmtVisitor& visitor) {
    visitor.visitChannelSendStmt(*this);
}

void SelectStmt::accept(StmtVisitor& visitor) {
    visitor.visitSelectStmt(*this);
}

void GoStmt::accept(StmtVisitor& visitor) {
    visitor.visitGoStmt(*this);
}

void MacroDecl::accept(StmtVisitor& visitor) {
    visitor.visitMacroDecl(*this);
}

} // namespace sapphire
