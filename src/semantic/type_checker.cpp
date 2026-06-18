#include "type_checker.h"

namespace sapphire {

// Expression visitors

void TypeChecker::visitLiteralExpr(LiteralExpr& expr) {
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

void TypeChecker::visitVariableExpr(VariableExpr& expr) {
    try {
        current_type = env->get(expr.name);
    } catch (const std::exception& e) {
        error("Undefined variable '" + expr.name + "'");
        current_type = std::make_shared<TypeVariable>();
    }
}

void TypeChecker::visitBinaryExpr(BinaryExpr& expr) {
    auto left_type = checkExpr(*expr.left);
    auto right_type = checkExpr(*expr.right);
    
    // Arithmetic operators
    if (expr.op == "+" || expr.op == "-" || expr.op == "*" || 
        expr.op == "/" || expr.op == "%" || expr.op == "**") {
        
        // String concatenation
        if (expr.op == "+" && left_type->isString() && right_type->isString()) {
            current_type = std::make_shared<StringType>();
            return;
        }
        
        // Numeric operations
        if (left_type->isNumeric() && right_type->isNumeric()) {
            // If either is float, result is float
            if (left_type->isFloat() || right_type->isFloat()) {
                current_type = std::make_shared<FloatType>();
            } else {
                current_type = std::make_shared<IntType>();
            }
            return;
        }
        
        error("Type mismatch: cannot apply '" + expr.op + "' to " + 
              left_type->toString() + " and " + right_type->toString());
        current_type = std::make_shared<TypeVariable>();
    }
    // Comparison operators
    else if (expr.op == "==" || expr.op == "!=" || 
             expr.op == "<" || expr.op == "<=" || 
             expr.op == ">" || expr.op == ">=") {
        current_type = std::make_shared<BoolType>();
    }
    // Logical operators
    else if (expr.op == "and" || expr.op == "or") {
        current_type = std::make_shared<BoolType>();
    }
}

void TypeChecker::visitUnaryExpr(UnaryExpr& expr) {
    auto operand_type = checkExpr(*expr.operand);
    
    if (expr.op == "-") {
        if (!operand_type->isNumeric()) {
            error("Type mismatch: cannot apply unary '-' to " + operand_type->toString());
        }
        current_type = operand_type;
    } else if (expr.op == "not") {
        current_type = std::make_shared<BoolType>();
    }
}

void TypeChecker::visitCallExpr(CallExpr& expr) {
    auto callee_type = checkExpr(*expr.callee);
    
    if (!callee_type->isFunction()) {
        error("Type mismatch: cannot call non-function type " + callee_type->toString());
        current_type = std::make_shared<TypeVariable>();
        return;
    }
    
    auto func_type = std::static_pointer_cast<FunctionType>(callee_type);
    
    // Check argument count
    if (expr.arguments.size() != func_type->getParamTypes().size()) {
        error("Argument count mismatch: expected " + 
              std::to_string(func_type->getParamTypes().size()) + 
              ", got " + std::to_string(expr.arguments.size()));
    }
    
    // Check argument types
    for (size_t i = 0; i < expr.arguments.size() && i < func_type->getParamTypes().size(); i++) {
        auto arg_type = checkExpr(*expr.arguments[i]);
        auto param_type = func_type->getParamTypes()[i];
        
        // Allow type variables to match anything (for generic functions)
        if (!param_type->isTypeVar() && !arg_type->equals(*param_type)) {
            error("Type mismatch in argument " + std::to_string(i + 1) + 
                  ": expected " + param_type->toString() + 
                  ", got " + arg_type->toString());
        }
    }
    
    current_type = func_type->getReturnType();
}

void TypeChecker::visitListExpr(ListExpr& expr) {
    if (expr.elements.empty()) {
        // Empty list has type List<T> where T is a type variable
        current_type = std::make_shared<ListType>(std::make_shared<TypeVariable>());
        return;
    }
    
    // Infer element type from first element
    auto elem_type = checkExpr(*expr.elements[0]);
    
    // Check all elements have compatible types
    for (size_t i = 1; i < expr.elements.size(); i++) {
        auto type = checkExpr(*expr.elements[i]);
        if (!type->equals(*elem_type)) {
            error("List elements have inconsistent types: " + 
                  elem_type->toString() + " and " + type->toString());
        }
    }
    
    current_type = std::make_shared<ListType>(elem_type);
}

void TypeChecker::visitIndexExpr(IndexExpr& expr) {
    auto object_type = checkExpr(*expr.object);
    auto index_type = checkExpr(*expr.index);
    
    if (!object_type->isList()) {
        error("Type mismatch: cannot index non-list type " + object_type->toString());
        current_type = std::make_shared<TypeVariable>();
        return;
    }
    
    if (!index_type->isInt()) {
        error("Type mismatch: list index must be int, got " + index_type->toString());
    }
    
    auto list_type = std::static_pointer_cast<ListType>(object_type);
    current_type = list_type->getElementType();
}

void TypeChecker::visitAssignExpr(AssignExpr& expr) {
    // Get the type of the variable being assigned to
    std::shared_ptr<Type> var_type;
    try {
        var_type = env->get(expr.name);
    } catch (const std::exception& e) {
        error("Undefined variable '" + expr.name + "'");
        var_type = std::make_shared<TypeVariable>();
    }
    
    // Check the type of the value being assigned
    auto value_type = checkExpr(*expr.value);
    
    // Check type compatibility
    if (!var_type->isTypeVar() && !value_type->equals(*var_type)) {
        error("Type mismatch: cannot assign " + value_type->toString() + 
              " to variable '" + expr.name + "' of type " + var_type->toString());
    }
    
    current_type = value_type;
}

// Statement visitors

void TypeChecker::visitExprStmt(ExprStmt& stmt) {
    checkExpr(*stmt.expression);
}

void TypeChecker::visitVarDeclStmt(VarDeclStmt& stmt) {
    std::shared_ptr<Type> var_type;
    
    if (stmt.initializer) {
        var_type = checkExpr(*stmt.initializer);
    } else {
        var_type = std::make_shared<TypeVariable>();
    }
    
    // If explicit type is given, check compatibility
    if (!stmt.type.empty()) {
        // Parse type name (simplified)
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
            error("Unknown type: " + stmt.type);
            declared_type = std::make_shared<TypeVariable>();
        }
        
        if (stmt.initializer && !var_type->equals(*declared_type)) {
            error("Type mismatch: variable '" + stmt.name + "' declared as " + 
                  declared_type->toString() + " but initialized with " + 
                  var_type->toString());
        }
        
        var_type = declared_type;
    }
    
    env->define(stmt.name, var_type);
}

void TypeChecker::visitFunctionDecl(FunctionDecl& stmt) {
    // Parse parameter types
    std::vector<std::shared_ptr<Type>> param_types;
    for (const auto& param : stmt.parameters) {
        std::shared_ptr<Type> param_type;
        if (param.type.empty()) {
            param_type = std::make_shared<TypeVariable>();
        } else if (param.type == "int") {
            param_type = std::make_shared<IntType>();
        } else if (param.type == "float") {
            param_type = std::make_shared<FloatType>();
        } else if (param.type == "string") {
            param_type = std::make_shared<StringType>();
        } else if (param.type == "bool") {
            param_type = std::make_shared<BoolType>();
        } else {
            param_type = std::make_shared<TypeVariable>();
        }
        param_types.push_back(param_type);
    }
    
    // Parse return type
    std::shared_ptr<Type> return_type;
    if (stmt.return_type.empty()) {
        return_type = std::make_shared<TypeVariable>();
    } else if (stmt.return_type == "int") {
        return_type = std::make_shared<IntType>();
    } else if (stmt.return_type == "float") {
        return_type = std::make_shared<FloatType>();
    } else if (stmt.return_type == "string") {
        return_type = std::make_shared<StringType>();
    } else if (stmt.return_type == "bool") {
        return_type = std::make_shared<BoolType>();
    } else {
        return_type = std::make_shared<TypeVariable>();
    }
    
    auto func_type = std::make_shared<FunctionType>(param_types, return_type);
    env->define(stmt.name, func_type);
    
    // TODO: Check function body
}

void TypeChecker::visitReturnStmt(ReturnStmt& stmt) {
    if (stmt.value) {
        checkExpr(*stmt.value);
    }
}

void TypeChecker::visitIfStmt(IfStmt& stmt) {
    auto cond_type = checkExpr(*stmt.condition);
    
    // Condition should be boolean (or truthy)
    // For now, we allow any type
    
    for (auto& s : stmt.then_branch) {
        s->accept(*this);
    }
    
    for (auto& s : stmt.else_branch) {
        s->accept(*this);
    }
}

void TypeChecker::visitWhileStmt(WhileStmt& stmt) {
    checkExpr(*stmt.condition);
    
    for (auto& s : stmt.body) {
        s->accept(*this);
    }
}

void TypeChecker::visitForStmt(ForStmt& stmt) {
    auto iterable_type = checkExpr(*stmt.iterable);
    
    // For now, assume iterable is a list or range (int)
    std::shared_ptr<Type> elem_type;
    if (iterable_type->isList()) {
        auto list_type = std::static_pointer_cast<ListType>(iterable_type);
        elem_type = list_type->getElementType();
    } else if (iterable_type->isInt()) {
        // range(n) produces ints
        elem_type = std::make_shared<IntType>();
    } else {
        error("Type mismatch: cannot iterate over " + iterable_type->toString());
        elem_type = std::make_shared<TypeVariable>();
    }
    
    env->define(stmt.variable, elem_type);
    
    for (auto& s : stmt.body) {
        s->accept(*this);
    }
}

void TypeChecker::visitTryStmt(TryStmt& stmt) {
    // Type check try block
    for (auto& s : stmt.try_body) {
        s->accept(*this);
    }
    
    // Type check catch blocks
    for (auto& catch_clause : stmt.catch_clauses) {
        // Bind exception variable as string (exception message)
        if (!catch_clause.variable_name.empty()) {
            env->define(catch_clause.variable_name, std::make_shared<StringType>());
        }
        
        for (auto& s : catch_clause.body) {
            s->accept(*this);
        }
    }
    
    // Type check finally block
    for (auto& s : stmt.finally_body) {
        s->accept(*this);
    }
}

void TypeChecker::visitThrowStmt(ThrowStmt& stmt) {
    // Type check the message expression
    if (stmt.message) {
        auto message_type = checkExpr(*stmt.message);
        // Message should be convertible to string
        // For now, we allow any type
    }
}

void TypeChecker::visitGetExpr(GetExpr& expr) {
    auto object_type = checkExpr(*expr.object);
    // Property access: object.name
    // For now, we'll return a generic type since we don't have class types
    current_type = std::make_shared<TypeVariable>();
}

void TypeChecker::visitSetExpr(SetExpr& expr) {
    auto object_type = checkExpr(*expr.object);
    auto value_type = checkExpr(*expr.value);
    // Property assignment: object.name = value
    // The type of the expression is the type of the value
    current_type = value_type;
}

void TypeChecker::visitIndexAssignExpr(IndexAssignExpr& expr) {
    auto object_type = checkExpr(*expr.object);
    auto index_type = checkExpr(*expr.index);
    auto value_type = checkExpr(*expr.value);
    // Index assignment: object[index] = value
    // The type of the expression is the type of the value
    current_type = value_type;
}

void TypeChecker::visitHashMapExpr(HashMapExpr& expr) {
    // Check all key-value pairs
    for (const auto& pair : expr.pairs) {
        checkExpr(*pair.first);   // Check key expression
        checkExpr(*pair.second);  // Check value expression
    }
    // Hash map literals have a generic hash map type (for now use TypeVariable)
    current_type = std::make_shared<TypeVariable>();
}

void TypeChecker::visitClassDecl(ClassDecl& stmt) {
    // Create a new environment for the class
    auto class_env = std::make_shared<TypeEnvironment>(env);
    
    // Type check class methods in the class environment
    for (auto& method : stmt.methods) {
        // Push class environment
        auto old_env = env;
        env = class_env;
        
        // Type check the method
        method->accept(*this);
        
        // Restore environment
        env = old_env;
    }
    
    // Class declaration creates a class type in the current environment
    // For now, we'll use a generic type
    current_type = std::make_shared<TypeVariable>();
}

} // namespace sapphire
