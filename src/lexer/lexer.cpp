#include "lexer.h"
#include "keywords.h"
#include <cctype>
#include <iostream>

namespace sapphire {

Lexer::Lexer(const std::string& src) 
    : source(src), current(0), line(1), column(1), indent_stack({0}) {}

std::vector<Token> Lexer::tokenize() {
    bool at_line_start = true;
    
    while (!isAtEnd()) {
        // Handle indentation at the start of a line
        if (at_line_start) {
            handleIndentation();
            at_line_start = false;
        }
        
        skipWhitespaceAndComments();
        if (isAtEnd()) break;
        
        // Check if we just consumed a newline
        if (peek() == '\n') {
            advance();
            tokens.push_back(makeToken(TokenType::NEWLINE, "\\n"));
            at_line_start = true;
            continue;
        }
        
        scanToken();
    }
    
    // Emit remaining dedents
    while (indent_stack.size() > 1) {
        indent_stack.pop_back();
        tokens.push_back(makeToken(TokenType::DEDENT, ""));
    }
    
    tokens.push_back(makeToken(TokenType::END_OF_FILE, ""));
    return tokens;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

char Lexer::advance() {
    char c = source[current++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    advance();
    return true;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '#') {
            // Skip comment until end of line
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

void Lexer::handleIndentation() {
    int spaces = 0;
    
    // Count leading spaces/tabs
    while (!isAtEnd() && (peek() == ' ' || peek() == '\t')) {
        if (peek() == ' ') {
            spaces++;
        } else {
            spaces += 4; // Tab = 4 spaces
        }
        advance();
    }
    
    // Skip empty lines and comment-only lines
    if (isAtEnd() || peek() == '\n' || peek() == '#') {
        // Skip the rest of the line if it's a comment
        if (peek() == '#') {
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        }
        return;
    }
    
    int current_indent = indent_stack.back();
    
    if (spaces > current_indent) {
        // Increase indentation
        indent_stack.push_back(spaces);
        tokens.push_back(makeToken(TokenType::INDENT, ""));
    } else if (spaces < current_indent) {
        // Decrease indentation (may be multiple levels)
        while (!indent_stack.empty() && indent_stack.back() > spaces) {
            indent_stack.pop_back();
            tokens.push_back(makeToken(TokenType::DEDENT, ""));
        }
        
        // Check for indentation error
        if (indent_stack.empty() || indent_stack.back() != spaces) {
            std::cerr << "Indentation error at line " << line << std::endl;
        }
    }
    // If spaces == current_indent, no change in indentation
}

void Lexer::scanToken() {
    if (isAtEnd()) return;
    
    size_t start = current;
    char c = advance();
    
    switch (c) {
        case '(': tokens.push_back(makeToken(TokenType::LPAREN, "(")); break;
        case ')': tokens.push_back(makeToken(TokenType::RPAREN, ")")); break;
        case '{': tokens.push_back(makeToken(TokenType::LBRACE, "{")); break;
        case '}': tokens.push_back(makeToken(TokenType::RBRACE, "}")); break;
        case '[': tokens.push_back(makeToken(TokenType::LBRACKET, "[")); break;
        case ']': tokens.push_back(makeToken(TokenType::RBRACKET, "]")); break;
        case ',': tokens.push_back(makeToken(TokenType::COMMA, ",")); break;
        case '.':
            if (match('.')) {
                if (match('.')) {
                    tokens.push_back(makeToken(TokenType::TRIPLE_DOT, "..."));
                } else {
                    tokens.push_back(makeToken(TokenType::DOUBLE_DOT, ".."));
                }
            } else {
                tokens.push_back(makeToken(TokenType::DOT, "."));
            }
            break;
        case '?': tokens.push_back(makeToken(TokenType::QUESTION, "?")); break;
        case '@': tokens.push_back(makeToken(TokenType::AT, "@")); break;
        case ':': tokens.push_back(makeToken(TokenType::COLON, ":")); break;
        case '|': tokens.push_back(makeToken(TokenType::PIPE, "|")); break;
        
        case '+':
            if (match('=')) {
                tokens.push_back(makeToken(TokenType::PLUS_EQUAL, "+="));
            } else {
                tokens.push_back(makeToken(TokenType::PLUS, "+"));
            }
            break;
        
        case '-':
            if (match('=')) {
                tokens.push_back(makeToken(TokenType::MINUS_EQUAL, "-="));
            } else if (match('>')) {
                tokens.push_back(makeToken(TokenType::ARROW, "->"));
            } else {
                tokens.push_back(makeToken(TokenType::MINUS, "-"));
            }
            break;
        
        case '*':
            if (match('*')) {
                tokens.push_back(makeToken(TokenType::POWER, "**"));
            } else {
                tokens.push_back(makeToken(TokenType::STAR, "*"));
            }
            break;
        
        case '/': tokens.push_back(makeToken(TokenType::SLASH, "/")); break;
        case '%': tokens.push_back(makeToken(TokenType::PERCENT, "%")); break;
        
        case '=':
            if (match('=')) {
                tokens.push_back(makeToken(TokenType::EQUAL_EQUAL, "=="));
            } else if (match('>')) {
                tokens.push_back(makeToken(TokenType::ARROW, "=>"));
            } else {
                tokens.push_back(makeToken(TokenType::EQUAL, "="));
            }
            break;
        
        case '!':
            if (match('=')) {
                tokens.push_back(makeToken(TokenType::NOT_EQUAL, "!="));
            } else {
                tokens.push_back(makeToken(TokenType::INVALID, "!"));
            }
            break;
        
        case '<':
            if (match('=')) {
                tokens.push_back(makeToken(TokenType::LESS_EQUAL, "<="));
            } else if (match('-')) {
                tokens.push_back(makeToken(TokenType::CHANNEL_SEND, "<-"));
            } else {
                tokens.push_back(makeToken(TokenType::LESS, "<"));
            }
            break;
        
        case '>':
            if (match('=')) {
                tokens.push_back(makeToken(TokenType::GREATER_EQUAL, ">="));
            } else {
                tokens.push_back(makeToken(TokenType::GREATER, ">"));
            }
            break;
        
        case '"':
        case '\'':
            scanString(c);
            break;
        
        default:
            if (std::isdigit(c)) {
                current = start;
                scanNumber();
            } else if (std::isalpha(c) || c == '_') {
                current = start;
                scanIdentifier();
            } else {
                tokens.push_back(makeToken(TokenType::INVALID, std::string(1, c)));
            }
            break;
    }
}

Token Lexer::makeToken(TokenType type, const std::string& lexeme) {
    return Token(type, lexeme, line, column);
}

void Lexer::scanNumber() {
    size_t start = current;
    bool is_float = false;
    
    while (std::isdigit(peek())) {
        advance();
    }
    
    // Check for decimal point
    if (peek() == '.' && std::isdigit(peekNext())) {
        is_float = true;
        advance(); // consume '.'
        
        while (std::isdigit(peek())) {
            advance();
        }
    }
    
    // Check for scientific notation
    if (peek() == 'e' || peek() == 'E') {
        is_float = true;
        advance();
        
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        
        while (std::isdigit(peek())) {
            advance();
        }
    }
    
    std::string lexeme = source.substr(start, current - start);
    TokenType type = is_float ? TokenType::FLOAT : TokenType::INTEGER;
    tokens.push_back(makeToken(type, lexeme));
}

void Lexer::scanString(char quote) {
    size_t start = current;
    std::string value;
    
    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\\') {
            advance();
            if (!isAtEnd()) {
                char escaped = advance();
                switch (escaped) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    case '\'': value += '\''; break;
                    default: value += escaped; break;
                }
            }
        } else {
            value += advance();
        }
    }
    
    if (isAtEnd()) {
        tokens.push_back(makeToken(TokenType::INVALID, "Unterminated string"));
        return;
    }
    
    advance(); // closing quote
    tokens.push_back(makeToken(TokenType::STRING, value));
}

void Lexer::scanIdentifier() {
    size_t start = current;
    
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    
    std::string lexeme = source.substr(start, current - start);
    
    // Check if it's a keyword
    auto it = KEYWORDS.find(lexeme);
    if (it != KEYWORDS.end()) {
        tokens.push_back(makeToken(it->second, lexeme));
    } else {
        tokens.push_back(makeToken(TokenType::IDENTIFIER, lexeme));
    }
}

} // namespace sapphire
