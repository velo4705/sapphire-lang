#include "type_inference.h"

namespace sapphire {

// Expression visitors

void TypeInference::visitLiteralExpr(LiteralExpr& expr) {
    switch (expr.type) {
        case LiteralExpr::Type::INTEGER:
            current_type = std::make_shared<IntType>();
            break;
        case LiteralExpr::Type::FLOAT:
            current_type = std::make_shared<FloatType>();
            break;
        case LiteralExpr::Type::STRING:
            current_type = std::make_shared<StringType>();
            break;
        case LiteralExpr::Type::BOOLEAN:
            current_type = std::make_shared<BoolType>();
            break;
        case LiteralExpr::Type::NONE:
            current_type = std::make_shared<NoneType>();
            break;
    }
}

void TypeInference::visitVariableExpr(VariableExpr& expr) {
    if (env->has(expr.name)) {
        current_type = env->get(expr.name);
    } else {
        // Create a fresh type variable for undefined variables
        current_type = freshTypeVar();
        env->define(expr.name, current_type);
    }
}

void TypeInference::visitBinaryExpr(BinaryExpr& expr) {
    auto left_type = inferExpr(*expr.left);
    auto right_type = inferExpr(*expr.right);
    
    // Arithmetic operators
    if (expr.op == "+" || expr.op == "-" || expr.op == "*" || 
        expr.op == "/" || expr.op == "%" || expr.op == "**") {
        
        // String concatenation
        if (expr.op == "+") {
            // Could be string + string or number + number
            auto result_type = freshTypeVar();
            
            // Try string concatenation
            constraints.add(left_type, std::make_shared<StringType>(), 
                          "binary + left operand");
            constraints.add(right_type, std::make_shared<StringType>(), 
                          "binary + right operand");
            constraints.add(result_type, std::make_shared<StringType>(), 
                          "binary + result");
            
            current_type = result_type;
            return;
        }
        
        // Numeric operations
        auto result_type = freshTypeVar();
        
        // Both operands should be numeric
        constraints.add(left_type, result_type, "binary " + expr.op + " left");
        constraints.add(right_type, result_type, "binary " + expr.op + " right");
        
        current_type = result_type;
    }
    // Comparison operators
    else if (expr.op == "==" || expr.op == "!=" || 
             expr.op == "<" || expr.op == "<=" || 
             expr.op == ">" || expr.op == ">=") {
        // Operands should have the same type
        constraints.add(left_type, right_type, "comparison " + expr.op);
        current_type = std::make_shared<BoolType>();
    }
    // Logical operators
    else if (expr.op == "and" || expr.op == "or") {
        // Operands should be boolean
        constraints.add(left_type, std::make_shared<BoolType>(), 
                      "logical " + expr.op + " left");
        constraints.add(right_type, std::make_shared<BoolType>(), 
                      "logical " + expr.op + " right");
        current_type = std::make_shared<BoolType>();
    }
}

void TypeInference::visitUnaryExpr(UnaryExpr& expr) {
    auto operand_type = inferExpr(*expr.operand);
    
    if (expr.op == "-") {
        // Operand should be numeric
        auto result_type = freshTypeVar();
        constraints.add(operand_type, result_type, "unary -");
        current_type = result_type;
    } else if (expr.op == "not") {
        // Operand should be boolean
        constraints.add(operand_type, std::make_shared<BoolType>(), "unary not");
        current_type = std::make_shared<BoolType>();
    }
}

void TypeInference::visitCallExpr(CallExpr& expr) {
    auto callee_type = inferExpr(*expr.callee);
    
    // Infer argument types
    std::vector<std::shared_ptr<Type>> arg_types;
    for (auto& arg : expr.arguments) {
        arg_types.push_back(inferExpr(*arg));
    }
    
    // Create a fresh type variable for the return type
    auto return_type = freshTypeVar();
    
    // Callee should be a function type
    auto expected_func_type = std::make_shared<FunctionType>(arg_types, return_type);
    constraints.add(callee_type, expected_func_type, "function call");
    
    current_type = return_type;
}

void TypeInference::visitListExpr(ListExpr& expr) {
    if (expr.elements.empty()) {
        // Empty list has type List<T> where T is a fresh type variable
        current_type = std::make_shared<ListType>(freshTypeVar());
        return;
    }
    
    // Infer type of first element
    auto elem_type = inferExpr(*expr.elements[0]);
    
    // All elements should have the same type
    for (size_t i = 1; i < expr.elements.size(); i++) {
        auto type = inferExpr(*expr.elements[i]);
        constraints.add(elem_type, type, "list element " + std::to_string(i));
    }
    
    current_type = std::make_shared<ListType>(elem_type);
}

void TypeInference::visitIndexExpr(IndexExpr& expr) {
    auto object_type = inferExpr(*expr.object);
    auto index_type = inferExpr(*expr.index);
    
    // Index should be int
    constraints.add(index_type, std::make_shared<IntType>(), "list index");
    
    // Object should be a list
    auto elem_type = freshTypeVar();
    constraints.add(object_type, std::make_shared<ListType>(elem_type), "indexing");
    
    current_type = elem_type;
}

// Statement visitors

void TypeInference::visitExprStmt(ExprStmt& stmt) {
    inferExpr(*stmt.expression);
}

void TypeInference::visitVarDeclStmt(VarDeclStmt& stmt) {
    std::shared_ptr<Type> var_type;
    
    if (stmt.initializer) {
        var_type = inferExpr(*stmt.initializer);
    } else {
        var_type = freshTypeVar();
    }
    
    // If explicit type is given, add constraint
    if (!stmt.type.empty()) {
        std::shared_ptr<Type> declared_type;
        if (stmt.type == "int") {
            declared_type = std::make_shared<IntType>();
        } else if (stmt.type == "float") {
            declared_type = std::make_shared<FloatType>();
        } else if (stmt.type == "string") {
            declared_type = std::make_shared<StringType>();
        } else if (stmt.type == "bool") {
            declared_type = std::make_shared<BoolType>();
        } else {
            declared_type = freshTypeVar();
        }
        
        constraints.add(var_type, declared_type, 
                      "variable declaration " + stmt.name);
        var_type = declared_type;
    }
    
    env->define(stmt.name, var_type);
}

void TypeInference::visitFunctionDecl(FunctionDecl& stmt) {
    // Parse parameter types
    std::vector<std::shared_ptr<Type>> param_types;
    for (const auto& param : stmt.parameters) {
        std::shared_ptr<Type> param_type;
        if (param.type.empty()) {
            param_type = freshTypeVar();
        } else if (param.type == "int") {
            param_type = std::make_shared<IntType>();
        } else if (param.type == "float") {
            param_type = std::make_shared<FloatType>();
        } else if (param.type == "string") {
            param_type = std::make_shared<StringType>();
        } else if (param.type == "bool") {
            param_type = std::make_shared<BoolType>();
        } else {
            param_type = freshTypeVar();
        }
        param_types.push_back(param_type);
    }
    
    // Parse return type
    std::shared_ptr<Type> return_type;
    if (stmt.return_type.empty()) {
        return_type = freshTypeVar();
    } else if (stmt.return_type == "int") {
        return_type = std::make_shared<IntType>();
    } else if (stmt.return_type == "float") {
        return_type = std::make_shared<FloatType>();
    } else if (stmt.return_type == "string") {
        return_type = std::make_shared<StringType>();
    } else if (stmt.return_type == "bool") {
        return_type = std::make_shared<BoolType>();
    } else {
        return_type = freshTypeVar();
    }
    
    auto func_type = std::make_shared<FunctionType>(param_types, return_type);
    env->define(stmt.name, func_type);
    
    // TODO: Infer function body types
}

void TypeInference::visitReturnStmt(ReturnStmt& stmt) {
    if (stmt.value) {
        inferExpr(*stmt.value);
    }
}

void TypeInference::visitIfStmt(IfStmt& stmt) {
    auto cond_type = inferExpr(*stmt.condition);
    
    // Condition should be boolean (or truthy)
    // For now, we allow any type
    
    for (auto& s : stmt.then_branch) {
        s->accept(*this);
    }
    
    for (auto& s : stmt.else_branch) {
        s->accept(*this);
    }
}

void TypeInference::visitWhileStmt(WhileStmt& stmt) {
    inferExpr(*stmt.condition);
    
    for (auto& s : stmt.body) {
        s->accept(*this);
    }
}

void TypeInference::visitForStmt(ForStmt& stmt) {
    auto iterable_type = inferExpr(*stmt.iterable);
    
    // Create a fresh type variable for the element type
    auto elem_type = freshTypeVar();
    
    // Iterable should be a list or int (for range)
    // For now, assume it's a list
    constraints.add(iterable_type, std::make_shared<ListType>(elem_type), 
                  "for loop iterable");
    
    env->define(stmt.variable, elem_type);
    
    for (auto& s : stmt.body) {
        s->accept(*this);
    }
}

} // namespace sapphire
