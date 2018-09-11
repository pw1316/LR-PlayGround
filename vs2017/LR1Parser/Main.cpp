#include <iostream>
#include <queue>

#include <Lexer.hpp>

int main()
{
    auto q = LR::Lexer::Lexer::Lex("(1*(4+5+2)-3)*(6+8)");
    std::cout << "TODO First Set\n";
}
