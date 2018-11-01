#include <stdafx.h>

#include <Lexer.hpp>
#include <Grammar.hpp>
#include <Parser.hpp>

#include <string>

int main()
{
    LR::Grammar::Grammar grammar("token.txt");
    grammar.DumpGrammar();

    auto tokenStream = LR::Lexer::Lexer::Lex(grammar, "(1*(4+5+2)-3)*(6+8)");
    LR::Lexer::Lexer::DumpTokenStream(grammar, tokenStream);

    auto ffSet = LR::Parser::LRParser::FirstAndFollowSet(grammar);
    auto[firstSet, followSet] = ffSet;

    //auto lr0dfa = LR::Parser::Parser::BuildDFALR0(grammar, firstSet, followSet);
    auto dfa = LR::Parser::LRParser(grammar);

    /* SETS */
    std::cout << "First/Follow Set:\n";
    for (LR::Utils::TokenId i = 0U; i < static_cast<LR::Utils::TokenId>(grammar.NumToken()) + 2U; ++i)
    {
        std::cout << "  " << grammar.GetTokenName(i) << ": {";
        for (auto token : firstSet[i])
        {
            std::cout << grammar.GetTokenName(token) << ",";
        }
        std::cout << "} {";
        for (auto token : followSet[i])
        {
            std::cout << grammar.GetTokenName(token) << ",";
        }
        std::cout << "}\n";
    }

    dfa.Dump(grammar);

    //LR::Parser::Parser parser(grammar);
    dfa.BeginParse(grammar, tokenStream);
    while (dfa.Step(grammar));
    return 0;
}
