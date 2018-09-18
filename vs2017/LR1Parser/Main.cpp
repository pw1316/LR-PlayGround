#include <stdafx.h>

#include <queue>

#include <Grammar.hpp>
#include <Lexer.hpp>
#include <Parser.hpp>

int main()
{
    LR::Grammar::Grammar grammar("token.txt");
    grammar.DumpGrammar();

    auto tokenStream = LR::Lexer::Lexer::Lex(grammar, "(1*(4+5+2)-3)*(6+8)");
    LR::Lexer::Lexer::DumpTokenStream(grammar, tokenStream);

    auto firstSet = LR::Parser::Parser::FirstSet(grammar);
    auto followSet = LR::Parser::Parser::FollowSet(grammar, firstSet);

    auto lr0dfa = LR::Parser::Parser::BuildDFALR0(grammar, firstSet, followSet);
    auto lr1dfa = LR::Parser::Parser::BuildDFALR1(grammar, firstSet, followSet);

    /* SETS */
    std::cout << "First/Follow Set:\n";
    for (unsigned int i = 0; i < static_cast<unsigned int>(grammar.NumToken()) + 2U; ++i)
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

    /* LR0 DFA */
    LR::Parser::Parser::DumpDFA(grammar, lr0dfa);
    LR::Parser::Parser::DumpDFA(grammar, lr1dfa);

    LR::Parser::Parser parser(grammar);
    parser.BeginParse(grammar, tokenStream);
    while (parser.Step(grammar));
    return 0;
}
