#include <stdafx.h>
#include "Lexer.hpp"
#include <Grammar.hpp>

#include <regex>

namespace LR::Lexer
{
    Utils::TokenStream LR::Lexer::Lexer::Lex(const Grammar::Grammar& g, const std::string& input)
    {
        Utils::TokenStream ret;
        auto iter = input.begin();
        while (iter != input.end())
        {
            bool isFailed = true;
            for (unsigned int tId = 0; tId < g.NumTerminalToken(); ++tId)
            {
                std::smatch ms;
                if (std::regex_search(iter, input.end(), ms, std::regex(g.TerminalTokenValues()[tId]), std::regex_constants::match_continuous))
                {
                    ret.push_back(tId);
                    iter += ms[0].length();
                    isFailed = false;
                    break;
                }
            }
            /* Failed */
            if (isFailed)
            {
                return Utils::TokenStream();
            }
        }
        return ret;
    }
    void Lexer::DumpTokenStream(const Grammar::Grammar& g, const Utils::TokenStream& ts)
    {
        std::cout << "[INPUT] ";
        for (auto token : ts)
        {
            std::cout << g.GetTokenName(token) << " ";
        }
        std::cout << "\n";
    }
}
