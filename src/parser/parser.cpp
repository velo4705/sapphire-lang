#include "parser.h"
#include <iostream>

namespace sapphire {

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    
    while (!isAtEnd()) {
        try {
            skipNewlines();
            if (!isAtEnd()) {
                statements.push_back(declaration());
            }
        } catch (const ParseError& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            synchronize();
        }
    }
    
    return statements;
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return;
    }
    throw error(peek(), message);
}

Token Parser::consumeToken(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    throw error(peek(), message);
}

void Parser::skipNewlines() {
    while (match({TokenType::NEWLINE})) {}
}

ParseError Parser::error(const Token& token, const std::string& message) {
    std::string error_msg = "Line " + std::to_string(token.line) + ": " + message;
    return ParseError(error_msg);
}

void Parser::synchronize() {
    advance();
    
    while (!isAtEnd()) {
        if (previous().type == TokenType::NEWLINE) return;
        
        switch (peek().type) {
            case TokenType::FN:
            case TokenType::MACRO:
            case TokenType::LET:
            case TokenType::CONST:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        
        advance();
    }
}

// Statement parsing

std::unique_ptr<Stmt> Parser::declaration() {
    // Parse decorators first
    std::vector<Decorator> decorators = parseDecorators();
    
    if (match({TokenType::LET, TokenType::CONST})) {
        if (!decorators.empty()) {
            throw error(previous(), "Decorators cannot be applied to variable declarations");
        }
        return varDeclaration();
    }
    if (match({TokenType::FN, TokenType::ASYNC})) {
        return functionDeclaration(std::move(decorators));
    }
    if (match({TokenType::MACRO})) {
        if (!decorators.empty()) {
            throw error(previous(), "Decorators cannot be applied to macro declarations");
        }
        return macroDeclaration();
    }
    if (match({TokenType::CLASS})) {
        return classDeclaration(std::move(decorators));
    }
    if (match({TokenType::TRAIT})) {
        if (!decorators.empty()) {
            throw error(previous(), "Decorators cannot be applied to trait declarations");
        }
        return traitDeclaration();
    }
    if (match({TokenType::IMPL})) {
        if (!decorators.empty()) {
            throw error(previous(), "Decorators cannot be applied to impl blocks");
        }
        return implBlock();
    }
    if (match({TokenType::IMPORT, TokenType::FROM})) {
        if (!decorators.empty()) {
            throw error(previous(), "Decorators cannot be applied to import statements");
        }
        return importDeclaration();
    }
    
    if (!decorators.empty()) {
        throw error(peek(), "Decorators can only be applied to functions and classes");
    }
    
    return statement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    bool is_const = previous().type == TokenType::CONST;
    
    Token name = consumeToken(TokenType::IDENTIFIER, "Expected variable name");
    
    std::string type_name;
    if (match({TokenType::COLON})) {
        Token type = consumeToken(TokenType::IDENTIFIER, "Expected type name");
        type_name = type.lexeme;
    }
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }
    
    skipNewlines();
    
    return std::make_unique<VarDeclStmt>(name.lexeme, type_name, 
                                         std::move(initializer), is_const);
}

std::unique_ptr<Stmt> Parser::functionDeclaration(std::vector<Decorator> decorators) {
    bool is_async = previous().type == TokenType::ASYNC;
    if (is_async) {
        consume(TokenType::FN, "Expected 'fn' after 'async'");
    }
    
    Token name = consumeToken(TokenType::IDENTIFIER, "Expected function name");
    
    consume(TokenType::LPAREN, "Expected '(' after function name");
    
    std::vector<std::pair<std::string, std::string>> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            Token param_name = consumeToken(TokenType::IDENTIFIER, "Expected parameter name");
            std::string param_type;
            
            if (match({TokenType::COLON})) {
                Token type = consumeToken(TokenType::IDENTIFIER, "Expected parameter type");
                param_type = type.lexeme;
            }
            
            parameters.push_back({param_name.lexeme, param_type});
        } while (match({TokenType::COMMA}));
    }
    
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    
    std::string return_type;
    if (match({TokenType::ARROW})) {
        Token type = consumeToken(TokenType::IDENTIFIER, "Expected return type");
        return_type = type.lexeme;
    }
    
    consume(TokenType::COLON, "Expected ':' before function body");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indented block");
    
    auto body = block();
    
    return std::make_unique<FunctionDecl>(std::move(decorators), name.lexeme, std::move(parameters),
                                          return_type, std::move(body), is_async);
}

std::unique_ptr<Stmt> Parser::macroDeclaration() {
    // macro name(params):
    Token name = consumeToken(TokenType::IDENTIFIER, "Expected macro name");
    
    consume(TokenType::LPAREN, "Expected '(' after macro name");
    
    std::vector<std::string> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            Token param_name = consumeToken(TokenType::IDENTIFIER, "Expected parameter name");
            parameters.push_back(param_name.lexeme);
        } while (match({TokenType::COMMA}));
    }
    
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    consume(TokenType::COLON, "Expected ':' before macro body");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indented block");
    
    auto body = block();
    
    return std::make_unique<MacroDecl>(name.lexeme, std::move(parameters), std::move(body));
}

std::unique_ptr<Stmt> Parser::classDeclaration(std::vector<Decorator> decorators) {
    // class Name[:]
    Token name = consumeToken(TokenType::IDENTIFIER, "Expected class name");
    
    std::string superclass_name;
    if (match({TokenType::LPAREN})) {
        Token base = consumeToken(TokenType::IDENTIFIER, "Expected base class name");
        superclass_name = base.lexeme;
        consume(TokenType::RPAREN, "Expected ')' after base class name");
    }
    
    consume(TokenType::COLON, "Expected ':' after class header");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indented block for class body");
    
    // Parse class body as a block and collect methods
    auto body_statements = block();
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    
    for (auto& stmt : body_statements) {
        if (auto* fn = dynamic_cast<FunctionDecl*>(stmt.get())) {
            // Transfer ownership into methods vector
            std::unique_ptr<FunctionDecl> method(fn);
            stmt.release();
            methods.push_back(std::move(method));
        }
        // Non-function statements in class body are ignored for now
    }
    
    return std::make_unique<ClassDecl>(std::move(decorators), name.lexeme, superclass_name, std::move(methods));
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::FOR})) return forStatement();
    if (match({TokenType::TRY})) return tryStatement();
    if (match({TokenType::THROW})) return throwStatement();
    if (match({TokenType::SELECT})) return selectStatement();
    if (match({TokenType::GO})) return goStatement();
    
    return exprStatement();
}

std::unique_ptr<Stmt> Parser::exprStatement() {
    auto expr = expression();
    
    // Check for channel send: ch <- value
    if (match({TokenType::CHANNEL_SEND})) {
        auto value = expression();
        skipNewlines();
        return std::make_unique<ChannelSendStmt>(std::move(expr), std::move(value));
    }
    
    skipNewlines();
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    std::unique_ptr<Expr> value = nullptr;
    
    if (!check(TokenType::NEWLINE) && !isAtEnd()) {
        value = expression();
    }
    
    skipNewlines();
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    auto condition = expression();
    consume(TokenType::COLON, "Expected ':' after if condition");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indented block");
    
    auto then_branch = block();
    
    std::vector<std::unique_ptr<Stmt>> else_branch;
    if (match({TokenType::ELSE})) {
        consume(TokenType::COLON, "Expected ':' after else");
        skipNewlines();
        consume(TokenType::INDENT, "Expected indented block");
        else_branch = block();
    }
    
    return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch),
                                    std::move(else_branch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    auto condition = expression();
    consume(TokenType::COLON, "Expected ':' after while condition");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indented block");
    
    auto body = block();
    
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::forStatement() {
    Token var = consumeToken(TokenType::IDENTIFIER, "Expected variable name");
    consume(TokenType::IN, "Expected 'in' in for loop");
    auto iterable = expression();
    consume(TokenType::COLON, "Expected ':' after for clause");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indented block");
    
    auto body = block();
    
    return std::make_unique<ForStmt>(var.lexeme, std::move(iterable), std::move(body));
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;
    
    while (!check(TokenType::DEDENT) && !isAtEnd()) {
        skipNewlines();
        if (!check(TokenType::DEDENT) && !isAtEnd()) {
            statements.push_back(declaration());
        }
    }
    
    consume(TokenType::DEDENT, "Expected dedent");
    return statements;
}

// Expression parsing

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    auto expr = logicalOr();
    
    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        auto value = assignment();
        
        // Check if left side is a variable or property
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(var->name, std::move(value));
        } else if (auto* get = dynamic_cast<GetExpr*>(expr.get())) {
            // Transform "object.name = value" into a SetExpr
            auto object = std::move(get->object);
            std::string name = get->name;
            return std::make_unique<SetExpr>(std::move(object), name, std::move(value));
        }
        
        throw error(equals, "Invalid assignment target");
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::logicalOr() {
    auto expr = logicalAnd();
    
    while (match({TokenType::OR})) {
        std::string op = previous().lexeme;
        auto right = logicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
    auto expr = equality();
    
    while (match({TokenType::AND})) {
        std::string op = previous().lexeme;
        auto right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();
    
    while (match({TokenType::EQUAL_EQUAL, TokenType::NOT_EQUAL})) {
        std::string op = previous().lexeme;
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();
    
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL,
                  TokenType::LESS, TokenType::LESS_EQUAL})) {
        std::string op = previous().lexeme;
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();
    
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        std::string op = previous().lexeme;
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = unary();
    
    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        std::string op = previous().lexeme;
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    // Channel receive: <-ch
    if (match({TokenType::CHANNEL_SEND})) {
        auto channel = unary();
        return std::make_unique<ChannelReceiveExpr>(std::move(channel));
    }
    
    if (match({TokenType::NOT, TokenType::MINUS})) {
        std::string op = previous().lexeme;
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    
    return power();
}

std::unique_ptr<Expr> Parser::power() {
    auto expr = call();
    
    if (match({TokenType::POWER})) {
        auto right = power();
        return std::make_unique<BinaryExpr>(std::move(expr), "**", std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::call() {
    auto expr = primary();
    
    while (true) {
        if (match({TokenType::LPAREN})) {
            std::vector<std::unique_ptr<Expr>> arguments;
            
            if (!check(TokenType::RPAREN)) {
                do {
                    arguments.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            
            consume(TokenType::RPAREN, "Expected ')' after arguments");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(arguments));
        } else if (match({TokenType::LBRACKET})) {
            auto index = expression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else if (match({TokenType::DOT})) {
            Token name = consumeToken(TokenType::IDENTIFIER, "Expected property name after '.'");
            expr = std::make_unique<GetExpr>(std::move(expr), name.lexeme);
        } else if (match({TokenType::QUESTION})) {
            // ? operator for error propagation
            expr = std::make_unique<TryExpr>(std::move(expr));
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    // Await expression
    if (match({TokenType::AWAIT})) {
        auto future = unary();  // Parse the expression to await
        return std::make_unique<AwaitExpr>(std::move(future));
    }
    
    // Channel creation: chan<T>(capacity) or chan<T>()
    if (match({TokenType::CHAN})) {
        consume(TokenType::LESS, "Expected '<' after 'chan'");
        Token type_token = consumeToken(TokenType::IDENTIFIER, "Expected type name");
        std::string element_type = type_token.lexeme;
        consume(TokenType::GREATER, "Expected '>' after type name");
        consume(TokenType::LPAREN, "Expected '(' after channel type");
        
        std::unique_ptr<Expr> capacity = nullptr;
        if (!check(TokenType::RPAREN)) {
            capacity = expression();
        }
        
        consume(TokenType::RPAREN, "Expected ')' after channel capacity");
        return std::make_unique<ChannelExpr>(element_type, std::move(capacity));
    }
    
    // Match expression
    if (match({TokenType::MATCH})) {
        return matchExpression();
    }
    
    if (match({TokenType::TRUE})) {
        return std::make_unique<LiteralExpr>(LiteralExpr::Type::BOOLEAN, "true");
    }
    if (match({TokenType::FALSE})) {
        return std::make_unique<LiteralExpr>(LiteralExpr::Type::BOOLEAN, "false");
    }
    if (match({TokenType::NONE})) {
        return std::make_unique<LiteralExpr>(LiteralExpr::Type::NONE, "none");
    }
    
    if (match({TokenType::INTEGER})) {
        return std::make_unique<LiteralExpr>(LiteralExpr::Type::INTEGER, previous().lexeme);
    }
    if (match({TokenType::FLOAT})) {
        return std::make_unique<LiteralExpr>(LiteralExpr::Type::FLOAT, previous().lexeme);
    }
    if (match({TokenType::STRING})) {
        return std::make_unique<LiteralExpr>(LiteralExpr::Type::STRING, previous().lexeme);
    }
    
    if (match({TokenType::IDENTIFIER})) {
        std::string name = previous().lexeme;
        
        // Check for stringify() macro function
        if (name == "stringify" && check(TokenType::LPAREN)) {
            advance();  // consume '('
            auto expr = expression();
            consume(TokenType::RPAREN, "Expected ')' after stringify argument");
            return std::make_unique<StringifyExpr>(std::move(expr));
        }
        
        return std::make_unique<VariableExpr>(name);
    }
    
    if (match({TokenType::LBRACKET})) {
        std::vector<std::unique_ptr<Expr>> elements;
        
        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        
        consume(TokenType::RBRACKET, "Expected ']' after list elements");
        return std::make_unique<ListExpr>(std::move(elements));
    }
    
    if (match({TokenType::LPAREN})) {
        auto expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw error(peek(), "Expected expression");
}

std::unique_ptr<Stmt> Parser::tryStatement() {
    // try:
    consume(TokenType::COLON, "Expected ':' after 'try'");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indentation after 'try:'");
    
    // Parse try body
    std::vector<std::unique_ptr<Stmt>> try_body;
    while (!check(TokenType::DEDENT) && !isAtEnd()) {
        try_body.push_back(declaration());
        skipNewlines();
    }
    consume(TokenType::DEDENT, "Expected dedent after try block");
    
    // Parse catch clauses
    std::vector<CatchClause> catch_clauses;
    while (match({TokenType::CATCH})) {
        std::string exception_type;
        std::string variable_name;
        
        // Check for exception type and variable binding
        if (check(TokenType::IDENTIFIER)) {
            Token first = advance();
            
            if (match({TokenType::AS})) {
                // catch ExceptionType as e:
                exception_type = first.lexeme;
                Token var_token = consumeToken(TokenType::IDENTIFIER, "Expected variable name after 'as'");
                variable_name = var_token.lexeme;
            } else {
                // catch e:
                variable_name = first.lexeme;
            }
        }
        
        consume(TokenType::COLON, "Expected ':' after catch clause");
        skipNewlines();
        consume(TokenType::INDENT, "Expected indentation after catch:");
        
        std::vector<std::unique_ptr<Stmt>> catch_body;
        while (!check(TokenType::DEDENT) && !isAtEnd()) {
            catch_body.push_back(declaration());
            skipNewlines();
        }
        consume(TokenType::DEDENT, "Expected dedent after catch block");
        
        catch_clauses.emplace_back(exception_type, variable_name, std::move(catch_body));
    }
    
    // Parse finally clause
    std::vector<std::unique_ptr<Stmt>> finally_body;
    if (match({TokenType::FINALLY})) {
        consume(TokenType::COLON, "Expected ':' after 'finally'");
        skipNewlines();
        consume(TokenType::INDENT, "Expected indentation after finally:");
        
        while (!check(TokenType::DEDENT) && !isAtEnd()) {
            finally_body.push_back(declaration());
            skipNewlines();
        }
        consume(TokenType::DEDENT, "Expected dedent after finally block");
    }
    
    // Validate: must have at least one catch or finally
    if (catch_clauses.empty() && finally_body.empty()) {
        throw error(previous(), "Try statement must have at least one catch or finally clause");
    }
    
    return std::make_unique<TryStmt>(std::move(try_body),
                                     std::move(catch_clauses),
                                     std::move(finally_body));
}

std::unique_ptr<Stmt> Parser::throwStatement() {
    std::string exception_type = "RuntimeError";  // Default
    std::unique_ptr<Expr> message;
    
    // Check if exception type is specified: throw ValueError("message")
    if (check(TokenType::IDENTIFIER) && peek().lexeme[0] >= 'A' && peek().lexeme[0] <= 'Z') {
        // Looks like an exception type (starts with capital letter)
        Token type_token = advance();
        
        if (match({TokenType::LPAREN})) {
            exception_type = type_token.lexeme;
            message = expression();
            consume(TokenType::RPAREN, "Expected ')' after exception message");
        } else {
            // Not a function call, treat as regular expression
            current--;  // Back up
            message = expression();
        }
    } else {
        // throw "message"
        message = expression();
    }
    
    skipNewlines();
    return std::make_unique<ThrowStmt>(exception_type, std::move(message));
}

std::unique_ptr<Stmt> Parser::selectStatement() {
    // select:
    //     case x = <-ch1:
    //         body
    //     case y = <-ch2:
    //         body
    //     default:
    //         body
    
    consume(TokenType::COLON, "Expected ':' after 'select'");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indentation after 'select:'");
    
    std::vector<SelectCase> cases;
    std::vector<std::unique_ptr<Stmt>> default_case;
    
    while (!check(TokenType::DEDENT) && !isAtEnd()) {
        if (match({TokenType::CASE})) {
            // Parse: case var = <-ch:
            std::string variable;
            std::unique_ptr<Expr> channel;
            
            // Check if there's a variable binding
            if (check(TokenType::IDENTIFIER)) {
                Token var_token = advance();
                variable = var_token.lexeme;
                consume(TokenType::EQUAL, "Expected '=' after variable in select case");
            }
            
            // Expect channel receive: <-ch
            consume(TokenType::CHANNEL_SEND, "Expected '<-' in select case");
            channel = primary();  // Parse the channel expression
            
            consume(TokenType::COLON, "Expected ':' after select case");
            skipNewlines();
            consume(TokenType::INDENT, "Expected indentation after case:");
            
            std::vector<std::unique_ptr<Stmt>> case_body;
            while (!check(TokenType::DEDENT) && !isAtEnd()) {
                case_body.push_back(declaration());
                skipNewlines();
            }
            consume(TokenType::DEDENT, "Expected dedent after case body");
            
            cases.emplace_back(std::move(channel), variable, std::move(case_body));
            
        } else if (match({TokenType::DEFAULT})) {
            // Parse: default:
            consume(TokenType::COLON, "Expected ':' after 'default'");
            skipNewlines();
            consume(TokenType::INDENT, "Expected indentation after default:");
            
            while (!check(TokenType::DEDENT) && !isAtEnd()) {
                default_case.push_back(declaration());
                skipNewlines();
            }
            consume(TokenType::DEDENT, "Expected dedent after default body");
            
        } else {
            throw error(peek(), "Expected 'case' or 'default' in select statement");
        }
        
        skipNewlines();
    }
    
    consume(TokenType::DEDENT, "Expected dedent after select block");
    
    return std::make_unique<SelectStmt>(std::move(cases), std::move(default_case));
}

std::unique_ptr<Stmt> Parser::goStatement() {
    // go function_call()
    // Parse the function call expression
    auto expr = expression();
    
    // Ensure it's a call expression
    CallExpr* call_expr = dynamic_cast<CallExpr*>(expr.get());
    if (!call_expr) {
        throw error(previous(), "Expected function call after 'go'");
    }
    
    // Transfer ownership to GoStmt
    std::unique_ptr<CallExpr> call(static_cast<CallExpr*>(expr.release()));
    
    skipNewlines();
    return std::make_unique<GoStmt>(std::move(call));
}

std::unique_ptr<Stmt> Parser::importDeclaration() {
    // import module
    // import module as alias
    // from module import name1, name2
    // from module import name1 as alias1, name2
    
    // Check if FROM was already matched (previous token is FROM)
    bool is_from_import = previous().type == TokenType::FROM;
    
    if (is_from_import) {
        // from module import name1, name2
        Token module_token = consumeToken(TokenType::IDENTIFIER, "Expected module name after 'from'");
        std::string module_name = module_token.lexeme;
        
        consume(TokenType::IMPORT, "Expected 'import' after module name");
        
        std::vector<std::string> imported_names;
        
        // Parse imported names
        do {
            Token name_token = consumeToken(TokenType::IDENTIFIER, "Expected identifier in import list");
            std::string name = name_token.lexeme;
            
            // Check for alias
            if (match({TokenType::AS})) {
                Token alias_token = consumeToken(TokenType::IDENTIFIER, "Expected alias name after 'as'");
                name = alias_token.lexeme; // For now, just use the alias as the name
            }
            
            imported_names.push_back(name);
        } while (match({TokenType::COMMA}));
        
        skipNewlines();
        return std::unique_ptr<ImportStmt>(new ImportStmt(module_name, imported_names));
    } else {
        // import module
        // import module as alias
        Token module_token = consumeToken(TokenType::IDENTIFIER, "Expected module name after 'import'");
        std::string module_name = module_token.lexeme;
        std::string alias;
        
        // Check for alias
        if (match({TokenType::AS})) {
            Token alias_token = consumeToken(TokenType::IDENTIFIER, "Expected alias name after 'as'");
            alias = alias_token.lexeme;
        }
        
        skipNewlines();
        return std::unique_ptr<ImportStmt>(new ImportStmt(module_name, alias));
    }
}

// Pattern matching implementation

std::unique_ptr<Expr> Parser::matchExpression() {
    // match <scrutinee>:
    auto scrutinee = expression();
    
    skipNewlines();
    consume(TokenType::COLON, "Expected ':' after match scrutinee");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indentation after match:");
    
    // Parse match arms
    std::vector<MatchArm> arms;
    
    while (!check(TokenType::DEDENT) && !isAtEnd()) {
        // Parse pattern(s) - support multiple patterns with |
        std::vector<std::unique_ptr<Pattern>> patterns;
        patterns.push_back(pattern());
        
        // Check for additional patterns separated by |
        while (match({TokenType::PIPE})) {
            patterns.push_back(pattern());
        }
        
        // Optional guard: if <condition>
        std::unique_ptr<Expr> guard = nullptr;
        if (match({TokenType::IF})) {
            guard = expression();
        }
        
        // Expect =>
        consume(TokenType::ARROW, "Expected '=>' after pattern");
        
        // Parse body expression
        auto body = expression();
        
        // Create match arm with all patterns
        arms.emplace_back(std::move(patterns), std::move(guard), std::move(body));
        
        skipNewlines();
    }
    
    consume(TokenType::DEDENT, "Expected dedent after match arms");
    
    return std::make_unique<MatchExpr>(std::move(scrutinee), std::move(arms));
}

std::unique_ptr<Pattern> Parser::pattern() {
    // Wildcard pattern: _
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "_") {
        advance();  // consume the _
        return std::make_unique<WildcardPattern>();
    }
    
    // Array pattern: [...]
    if (check(TokenType::LBRACKET)) {
        return arrayPattern();
    }
    
    // Object pattern: {...}
    if (check(TokenType::LBRACE)) {
        return objectPattern();
    }
    
    // Literal pattern: 0, "hello", true, false, none
    if (match({TokenType::INTEGER, TokenType::FLOAT, TokenType::STRING, 
               TokenType::TRUE, TokenType::FALSE, TokenType::NONE})) {
        Token lit = previous();
        std::unique_ptr<Expr> value;
        
        switch (lit.type) {
            case TokenType::INTEGER:
                value = std::make_unique<LiteralExpr>(LiteralExpr::Type::INTEGER, lit.lexeme);
                break;
            case TokenType::FLOAT:
                value = std::make_unique<LiteralExpr>(LiteralExpr::Type::FLOAT, lit.lexeme);
                break;
            case TokenType::STRING:
                value = std::make_unique<LiteralExpr>(LiteralExpr::Type::STRING, lit.lexeme);
                break;
            case TokenType::TRUE:
                value = std::make_unique<LiteralExpr>(LiteralExpr::Type::BOOLEAN, "true");
                break;
            case TokenType::FALSE:
                value = std::make_unique<LiteralExpr>(LiteralExpr::Type::BOOLEAN, "false");
                break;
            case TokenType::NONE:
                value = std::make_unique<LiteralExpr>(LiteralExpr::Type::NONE, "none");
                break;
            default:
                break;
        }
        
        return std::make_unique<LiteralPattern>(std::move(value));
    }
    
    // Constructor pattern or Variable pattern: Some(x), None, Ok(x), Err(e), or just x
    if (match({TokenType::IDENTIFIER})) {
        std::string name = previous().lexeme;
        
        // Check if this is a constructor pattern (Some, None, Ok, Err)
        if (name == "Some" || name == "None" || name == "Ok" || name == "Err") {
            // Check for parentheses
            if (check(TokenType::LPAREN)) {
                advance();  // consume (
                
                if (name == "None") {
                    // None() - no inner pattern
                    consume(TokenType::RPAREN, "Expected ')' after None");
                    return std::make_unique<ConstructorPattern>(name, nullptr);
                } else {
                    // Some(x), Ok(x), Err(e) - has inner pattern
                    auto inner = pattern();
                    consume(TokenType::RPAREN, "Expected ')' after pattern");
                    return std::make_unique<ConstructorPattern>(name, std::move(inner));
                }
            } else if (name == "None") {
                // None without parentheses
                return std::make_unique<ConstructorPattern>(name, nullptr);
            }
        }
        
        // Regular variable pattern
        return std::make_unique<VariablePattern>(name);
    }
    
    throw error(peek(), "Expected pattern");
}

std::unique_ptr<Pattern> Parser::arrayPattern() {
    consume(TokenType::LBRACKET, "Expected '['");
    
    std::vector<std::unique_ptr<Pattern>> elements;
    bool has_rest = false;
    std::string rest_name;
    
    if (!check(TokenType::RBRACKET)) {
        do {
            skipNewlines();
            
            // Check for rest pattern: ...rest
            if (match({TokenType::TRIPLE_DOT})) {
                has_rest = true;
                Token rest_token = consumeToken(TokenType::IDENTIFIER, "Expected identifier after '...'");
                rest_name = rest_token.lexeme;
                break;  // Rest must be last
            }
            
            elements.push_back(pattern());
            skipNewlines();
        } while (match({TokenType::COMMA}));
    }
    
    consume(TokenType::RBRACKET, "Expected ']' after array pattern");
    
    return std::make_unique<ArrayPattern>(std::move(elements), has_rest, rest_name);
}

std::unique_ptr<Pattern> Parser::objectPattern() {
    consume(TokenType::LBRACE, "Expected '{'");
    
    std::vector<std::pair<std::string, std::unique_ptr<Pattern>>> fields;
    
    if (!check(TokenType::RBRACE)) {
        do {
            skipNewlines();
            
            Token field_name = consumeToken(TokenType::IDENTIFIER, "Expected field name");
            std::string name = field_name.lexeme;
            
            std::unique_ptr<Pattern> field_pattern;
            
            // Check for explicit pattern: {name: pattern}
            if (match({TokenType::COLON})) {
                field_pattern = pattern();
            } else {
                // Shorthand: {name} is equivalent to {name: name}
                field_pattern = std::make_unique<VariablePattern>(name);
            }
            
            fields.emplace_back(name, std::move(field_pattern));
            skipNewlines();
        } while (match({TokenType::COMMA}));
    }
    
    consume(TokenType::RBRACE, "Expected '}' after object pattern");
    
    return std::make_unique<ObjectPattern>(std::move(fields));
}

// Trait declaration parsing

std::unique_ptr<Stmt> Parser::traitDeclaration() {
    // trait <name>:
    Token name_token = consumeToken(TokenType::IDENTIFIER, "Expected trait name");
    std::string name = name_token.lexeme;
    
    consume(TokenType::COLON, "Expected ':' after trait name");
    
    // Optional super traits on same line: trait Foo: Bar + Baz
    std::vector<std::string> super_traits;
    if (check(TokenType::IDENTIFIER)) {
        do {
            Token super_trait = consumeToken(TokenType::IDENTIFIER, "Expected trait name");
            super_traits.push_back(super_trait.lexeme);
        } while (match({TokenType::PLUS}));
    }
    
    skipNewlines();
    consume(TokenType::INDENT, "Expected indentation after trait:");
    
    // Parse trait methods (just signatures, no bodies)
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    
    while (!check(TokenType::DEDENT) && !isAtEnd()) {
        if (check(TokenType::FN)) {
            advance();  // Consume FN
            
            // Parse method signature
            Token method_name = consumeToken(TokenType::IDENTIFIER, "Expected method name");
            
            consume(TokenType::LPAREN, "Expected '(' after method name");
            
            // Parse parameters
            std::vector<std::pair<std::string, std::string>> parameters;
            if (!check(TokenType::RPAREN)) {
                do {
                    Token param_name = consumeToken(TokenType::IDENTIFIER, "Expected parameter name");
                    std::string param_type;
                    
                    if (match({TokenType::COLON})) {
                        Token type_token = consumeToken(TokenType::IDENTIFIER, "Expected parameter type");
                        param_type = type_token.lexeme;
                    }
                    
                    parameters.emplace_back(param_name.lexeme, param_type);
                } while (match({TokenType::COMMA}));
            }
            
            consume(TokenType::RPAREN, "Expected ')' after parameters");
            
            // Optional return type
            std::string return_type;
            if (match({TokenType::ARROW})) {
                Token ret_type = consumeToken(TokenType::IDENTIFIER, "Expected return type");
                return_type = ret_type.lexeme;
            }
            
            // Trait methods don't have bodies
            skipNewlines();
            
            // Create function declaration with empty body
            std::vector<std::unique_ptr<Stmt>> empty_body;
            std::vector<Decorator> empty_decorators;
            auto method = std::make_unique<FunctionDecl>(
                std::move(empty_decorators),
                method_name.lexeme,
                parameters,
                return_type,
                std::move(empty_body)
            );
            
            methods.push_back(std::move(method));
        } else {
            skipNewlines();
            if (!check(TokenType::DEDENT) && !isAtEnd()) {
                advance();  // Skip unexpected tokens
            }
        }
    }
    
    consume(TokenType::DEDENT, "Expected dedent after trait body");
    
    return std::make_unique<TraitDecl>(name, super_traits, std::move(methods));
}

// Impl block parsing

std::unique_ptr<Stmt> Parser::implBlock() {
    // impl <TraitName> for <TypeName>:
    // OR
    // Inside a class: impl <TraitName>:
    
    Token trait_token = consumeToken(TokenType::IDENTIFIER, "Expected trait name after 'impl'");
    std::string trait_name = trait_token.lexeme;
    
    std::string for_type_name;
    
    // Check for standalone impl block: impl Trait for Type
    if (match({TokenType::FOR})) {
        Token type_token = consumeToken(TokenType::IDENTIFIER, "Expected type name after 'for'");
        for_type_name = type_token.lexeme;
    }
    // Otherwise it's inside a class, type will be determined by context
    
    consume(TokenType::COLON, "Expected ':' after impl declaration");
    skipNewlines();
    consume(TokenType::INDENT, "Expected indentation after impl:");
    
    // Parse method implementations
    std::vector<std::unique_ptr<FunctionDecl>> methods;
    
    while (!check(TokenType::DEDENT) && !isAtEnd()) {
        if (check(TokenType::FN)) {
            advance();  // Consume FN
            
            // Parse full method implementation
            Token method_name = consumeToken(TokenType::IDENTIFIER, "Expected method name");
            
            consume(TokenType::LPAREN, "Expected '(' after method name");
            
            // Parse parameters
            std::vector<std::pair<std::string, std::string>> parameters;
            if (!check(TokenType::RPAREN)) {
                do {
                    Token param_name = consumeToken(TokenType::IDENTIFIER, "Expected parameter name");
                    std::string param_type;
                    
                    if (match({TokenType::COLON})) {
                        Token type_token = consumeToken(TokenType::IDENTIFIER, "Expected parameter type");
                        param_type = type_token.lexeme;
                    }
                    
                    parameters.emplace_back(param_name.lexeme, param_type);
                } while (match({TokenType::COMMA}));
            }
            
            consume(TokenType::RPAREN, "Expected ')' after parameters");
            
            // Optional return type
            std::string return_type;
            if (match({TokenType::ARROW})) {
                Token ret_type = consumeToken(TokenType::IDENTIFIER, "Expected return type");
                return_type = ret_type.lexeme;
            }
            
            // Parse method body
            consume(TokenType::COLON, "Expected ':' before method body");
            skipNewlines();
            consume(TokenType::INDENT, "Expected indentation for method body");
            
            std::vector<std::unique_ptr<Stmt>> body = block();
            std::vector<Decorator> empty_decorators;
            
            auto method = std::make_unique<FunctionDecl>(
                std::move(empty_decorators),
                method_name.lexeme,
                parameters,
                return_type,
                std::move(body)
            );
            
            methods.push_back(std::move(method));
        } else {
            skipNewlines();
            if (!check(TokenType::DEDENT) && !isAtEnd()) {
                advance();  // Skip unexpected tokens
            }
        }
    }
    
    consume(TokenType::DEDENT, "Expected dedent after impl block");
    
    return std::make_unique<ImplBlock>(trait_name, for_type_name, std::move(methods));
}

// Parse decorators (@decorator or @decorator(args))
std::vector<Decorator> Parser::parseDecorators() {
    std::vector<Decorator> decorators;
    
    while (match({TokenType::AT})) {
        Token name = consumeToken(TokenType::IDENTIFIER, "Expected decorator name after '@'");
        
        std::vector<std::unique_ptr<Expr>> arguments;
        
        // Check for decorator with arguments: @decorator(args)
        if (match({TokenType::LPAREN})) {
            if (!check(TokenType::RPAREN)) {
                do {
                    arguments.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')' after decorator arguments");
        }
        
        decorators.emplace_back(name.lexeme, std::move(arguments));
        skipNewlines();
    }
    
    return decorators;
}

} // namespace sapphire
