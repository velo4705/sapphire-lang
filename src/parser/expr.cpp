#include "expr.h"

namespace sapphire {

void LiteralExpr::accept(ExprVisitor& visitor) {
    visitor.visitLiteralExpr(*this);
}

void VariableExpr::accept(ExprVisitor& visitor) {
    visitor.visitVariableExpr(*this);
}

void AssignExpr::accept(ExprVisitor& visitor) {
    visitor.visitAssignExpr(*this);
}

void BinaryExpr::accept(ExprVisitor& visitor) {
    visitor.visitBinaryExpr(*this);
}

void UnaryExpr::accept(ExprVisitor& visitor) {
    visitor.visitUnaryExpr(*this);
}

void CallExpr::accept(ExprVisitor& visitor) {
    visitor.visitCallExpr(*this);
}

void ListExpr::accept(ExprVisitor& visitor) {
    visitor.visitListExpr(*this);
}

void IndexExpr::accept(ExprVisitor& visitor) {
    visitor.visitIndexExpr(*this);
}

void GetExpr::accept(ExprVisitor& visitor) {
    visitor.visitGetExpr(*this);
}

void SetExpr::accept(ExprVisitor& visitor) {
    visitor.visitSetExpr(*this);
}

void MatchExpr::accept(ExprVisitor& visitor) {
    visitor.visitMatchExpr(*this);
}

void AwaitExpr::accept(ExprVisitor& visitor) {
    visitor.visitAwaitExpr(*this);
}

void ChannelExpr::accept(ExprVisitor& visitor) {
    visitor.visitChannelExpr(*this);
}

void ChannelReceiveExpr::accept(ExprVisitor& visitor) {
    visitor.visitChannelReceiveExpr(*this);
}

void TryExpr::accept(ExprVisitor& visitor) {
    visitor.visitTryExpr(*this);
}

void StringifyExpr::accept(ExprVisitor& visitor) {
    visitor.visitStringifyExpr(*this);
}

} // namespace sapphire
