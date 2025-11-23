#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "token_actions/token.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso in correcto"<<std::endl;
        std::cerr << "El uso correcto es: " << argv[0] << " ocean <input.oc>" << std::endl;
    }

    std::string contents;
    {
        std::stringstream content_stream; //stringstream object
        std::fstream input(argv[1],std::ios::in);//recibo el input del archivo
        content_stream << input.rdbuf();//creo un stream de los datos del archivo
        contents = content_stream.str();// aqui asigno el contenido del archivo a una variable para manipularlo
    }

    std::vector<Token> tokens = tokenize(contents);
    {
        std::fstream file("out.asm",std::ios::out);
        file << tokens_to_asm(tokens);
    }

    system("nasm -f macho64 out.asm -o out.o");
    system("clang -Wl,-platform_version,macos,11.0.0,11.0.0 out.o -o out");




    return EXIT_SUCCESS;
}

