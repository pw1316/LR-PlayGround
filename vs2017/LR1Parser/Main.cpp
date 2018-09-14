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
    std::cout << "TODO LR Items\n";

    /* Dump */
    std::cout << "Token string:\n  ";
    auto tmpq = q;
    while (!tmpq.empty())
    {
        std::cout << g.TerminalTokenNames()[tmpq.front()] << " ";
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
            auto tokenType = g.GetTokenType(token);
            switch (tokenType)
            {
            case LR::Grammar::TokenType::TT_START:
                std::cout << "! ";
                break;
            case LR::Grammar::TokenType::TT_EPSILON:
                std::cout << "@ ";
                break;
            case LR::Grammar::TokenType::TT_TOKEN_SYMB:
                std::cout << g.NonTerminalTokenNames()[token - g.TerminalTokenNames().size()] << " ";
                break;
            case LR::Grammar::TokenType::TT_TOKEN_TERM:
                std::cout << g.TerminalTokenNames()[token] << " ";
                break;
            default:
                break;
            }
            if (isLeft)
            {
                std::cout << "-> ";
            }
            isLeft = false;
        }
        std::cout << "\n";
    }

    /* SETS */
    std::cout << "Grammar:\n";
    for (size_t i = 0; i < g.NonTerminalTokenNames().size() + 1; ++i)
    {
        if (static_cast<unsigned int>(i + g.TerminalTokenNames().size()) == g.START())
        {
            std::cout << "  !: {";
        }
        else
        {
            std::cout << "  " << g.NonTerminalTokenNames()[i] << ": {";
        }
        for (auto token : firstSet[i + g.TerminalTokenNames().size()])
        {
            if (g.HasEpsilon() && token == g.EPSILON())
            {
                std::cout << "@,";
            }
            else
            {
                std::cout << g.TerminalTokenNames()[token] << ",";
            }
        }
        std::cout << "} {";
        for (auto token : followSet[i + g.TerminalTokenNames().size()])
        {
            if (token == g.TERMINAL())
            {
                std::cout << "$,";
            }
            else
            {
                std::cout << g.TerminalTokenNames()[token] << ",";
            }
        }
        std::cout << "}\n";
    }
    return 0;
}
