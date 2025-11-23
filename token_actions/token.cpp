//
// Created by Rousbel Villar on 11/22/25.
//

#include "token.h"
#include <optional>
#include <string>
#include <iostream>
#include <sstream>
/*
 * En esta funcion traducimos el contenido de los archivos a tokens con sus valores
 */
std::vector<Token> tokenize(const std::string& str) {
    std::vector<Token> tokens;

    std::string buffer;

    for (int i=0;i<str.length();i++) {//iteramos el archivo
        char c = str.at(i);
        if (std::isalpha(c)) {//separamos los caracteres numericos de las letras
            buffer.push_back(c);
            i++;
            while (i < str.length() && std::isalnum(str.at(i))) {
                buffer.push_back(str.at(i));
                i++;
            }
            i--;
            if (buffer == "return") {
                // si el buffer es igual a return ya tenemos un token tipo return
                tokens.push_back({.type = TokenType::_return});
                buffer.clear();
                continue;
            }else {
                std::cerr<< "Error: Identificador desconocido\n"<<std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (std::isdigit(c)) {//Si es numero aparte del tipo de token almacenamos el valor
            buffer.push_back(c);
            i++;
            while ( i < str.length() && std::isdigit(str.at(i))) {
                buffer.push_back(str.at(i));
                i++;
            }
            i--;
            tokens.push_back({.type = TokenType::int_lit,.value = buffer});
            buffer.clear();
        }else if (c == ';') {// Punto y coma indica otro token
            tokens.push_back({.type = TokenType::semi});
        }else if (std::isspace(c)) {
            continue;
        }else {
            std::cerr << "Error: caracter invalido"<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return tokens;
}
/*
 * En esta funcion convertimos los tokens a lenguage ensamblador asm
 */
std::string tokens_to_asm(const std::vector<Token>& tokens) {
    std::stringstream output;
    output << "global _main\n\n";
    output << "section .text\n";
    output << "_main:\n";

    for (int i=0;i<tokens.size();i++) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::_return) {
            if (i+1 < tokens.size() && tokens.at(i+1).type == TokenType::int_lit) {
                if (i + 2 < tokens.size() && tokens.at(i+2).type == TokenType::semi) {
                    output << "    mov rax, 0x2000001\n";
                    output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall\n";
                }
            }
        }
    }
    return output.str();
}