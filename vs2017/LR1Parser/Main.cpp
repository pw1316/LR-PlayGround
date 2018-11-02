#include <stdafx.h>

#include <Grammar.hpp>
#include <Lexer.hpp>
#include <Parser.hpp>

#include <string>

constexpr auto GRAMMAR_FILE = "token.txt";
constexpr auto INPUT_STRING = "(1*(4+5+2)-3)*(6+8)";

int main()
{
    LR::Grammar grammar(GRAMMAR_FILE);
    grammar.Dump();

    auto lexer = LR::Lexer(grammar);
    if (!lexer.SetInput(INPUT_STRING))
    {
        std::cout << "Not valid input string: " << INPUT_STRING << "\n";
        return 1;
    }
    lexer.Dump();

    auto parser = LR::LRParser(grammar);
    parser.Dump();
    parser.BeginParse(lexer.TokenStream());
    while (parser.Step());
    return 0;
}
