#include <stdafx.h>

#include <iomanip>
#include <iostream>
#include <queue>

#include <Grammar.hpp>
#include <Lexer.hpp>
#include <Parser.hpp>

int main()
{
    LR::Grammar::Grammar g("token.txt");
    auto q = LR::Lexer::Lexer::Lex(g, "(1*(4+5+2)-3)*(6+8)");
    auto firstSet = LR::Parser::Parser::FirstSet(g);
    auto followSet = LR::Parser::Parser::FollowSet(g, firstSet);

    auto lr0dfa = LR::Parser::Parser::BuildDFALR0(g, firstSet, followSet);
    std::cout << "TODO LR1 Items\n";

    /* Dump */
    std::cout << "Token string:\n  ";
    auto tmpq = q;
    while (!tmpq.empty())
    {
        std::cout << g.GetTokenName(tmpq.front()) << " ";
        tmpq.pop();
    }
    std::cout << "\n";

    std::cout << "Grammar:\n";
    unsigned int idx = 0;
    for (auto& gg : g.G())
    {
        bool isLeft = true;
        std::cout << std::setw(4) << idx++ << " ";
        for (auto token : gg)
        {
            std::cout << g.GetTokenName(token) << " ";
            if (isLeft)
            {
                std::cout << "-> ";
            }
            isLeft = false;
        }
        std::cout << "\n";
    }

    /* SETS */
    std::cout << "First/Follow Set:\n";
    for (unsigned int i = 0; i < static_cast<unsigned int>(g.NumToken()) + 2U; ++i)
    {
        std::cout << "  " << g.GetTokenName(i) << ": {";
        for (auto token : firstSet[i])
        {
            std::cout << g.GetTokenName(token) << ",";
        }
        std::cout << "} {";
        for (auto token : followSet[i])
        {
            std::cout << g.GetTokenName(token) << ",";
        }
        std::cout << "}\n";
    }

    /* LR0 DFA */
    LR::Parser::Parser::DumpDFA(g, lr0dfa);
    return 0;
}
