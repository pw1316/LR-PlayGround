#include <iostream>
#include <queue>

#include <Grammar.hpp>
#include <Lexer.hpp>
//#include <Parser.hpp>

int main()
{
    LR::Grammar::Grammar g("token.txt");
    auto q = LR::Lexer::Lexer::Lex(g, "(1*(4+5+2)-3)*(6+8)");
    //auto firstSet = LR::Parser::Parser::FirstSet();
    std::cout << "TODO First Set\n";
}
