//
// Created by Rousbel Villar on 11/22/25.
//
#include <optional>
#include <string>
#include <vector>

#ifndef MINI_COMPILADOR_TOKEN_H
#define MINI_COMPILADOR_TOKEN_H


//En esta clase haremos todas las operaciones de los tokens
enum class TokenType{
    _return,
    int_lit,
    semi
};

struct Token {
    TokenType type;
    std::optional<std::string> value{};
};

std::vector<Token> tokenize(const std::string& str);
std::string tokens_to_asm(const std::vector<Token>& tokens);

#endif //MINI_COMPILADOR_TOKEN_H
