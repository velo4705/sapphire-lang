#ifndef SAPPHIRE_LEXER_H
#define SAPPHIRE_LEXER_H

#include "token.h"
#include <vector>
#include <string>

namespace sapphire {

class Lexer {
private:
    std::string source;
    size_t current;
    int line;
    int column;
    std::vector<Token> tokens;
    std::vector<int> indent_stack;
    
    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;
    bool match(char expected);
    void skipWhitespaceAndComments();
    void handleIndentation();
    void scanToken();
    
    Token makeToken(TokenType type, const std::string& lexeme);
    void scanNumber();
    void scanString(char quote);
    void scanFString(char quote);
    void scanIdentifier();

public:
    explicit Lexer(const std::string& src);
    std::vector<Token> tokenize();
};

} // namespace sapphire

#endif // SAPPHIRE_LEXER_H
