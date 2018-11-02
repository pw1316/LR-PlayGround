#include <stdafx.h>

#include <Grammar.hpp>
#include <Lexer.hpp>
#include <Parser.hpp>

#include <string>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "USAGE: LR1Parser <grammar-file> <input-string>\n";
        return 1;
    }
    LR::Grammar grammar(argv[1]);
    grammar.Dump();

    auto lexer = LR::Lexer(grammar);
    auto parser = LR::LRParser(grammar);
    if (!lexer.SetInput(argv[2]))
    {
        std::cout << "Not valid input string: " << argv[2] << "\n";
        return 2;
    }
    if (!parser.Valid())
    {
        std::cout << "Not LR(1) language!!!\n";
        return 3;
    }
    lexer.Dump();
    parser.Dump();
    parser.BeginParse(lexer.TokenStream());
    while (parser.Step());
    return 0;
}
