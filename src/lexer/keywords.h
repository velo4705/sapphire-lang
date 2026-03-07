#ifndef SAPPHIRE_KEYWORDS_H
#define SAPPHIRE_KEYWORDS_H

#include "token.h"
#include <map>
#include <string>

namespace sapphire {

static const std::map<std::string, TokenType> KEYWORDS = {
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"fn", TokenType::FN},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"in", TokenType::IN},
    {"match", TokenType::MATCH},
    {"class", TokenType::CLASS},
    {"import", TokenType::IMPORT},
    {"from", TokenType::FROM},
    {"as", TokenType::AS},
    {"true", TokenType::TRUE},
    {"True", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"False", TokenType::FALSE},
    {"none", TokenType::NONE},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"pass", TokenType::PASS},
    {"async", TokenType::ASYNC},
    {"await", TokenType::AWAIT},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"finally", TokenType::FINALLY},
    {"throw", TokenType::THROW},
    {"trait", TokenType::TRAIT},
    {"impl", TokenType::IMPL},
    {"Self", TokenType::SELF},
    {"where", TokenType::WHERE},
    {"dyn", TokenType::DYN},
    {"chan", TokenType::CHAN},
    {"go", TokenType::GO},
    {"select", TokenType::SELECT},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT},
    {"macro", TokenType::MACRO}
};

} // namespace sapphire

#endif // SAPPHIRE_KEYWORDS_H
