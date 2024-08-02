#include "tokenizer.h"
#include <iostream>
#include <format>

int main(int argc, const char* argv[]) {
    auto input = "let a = 1 + b * 2;";


    Tokenizer tokenizer(input);


    while (true) {
        auto token = tokenizer.next();

        // std::cout << "->" <<  token.text << std::endl;

        if (token.kind == TokenKind::Eof) {
            break;
        }
        std::cout << std::format("[{0}]\t{1}", int(token.kind), token.text) << std::endl;
    }
}