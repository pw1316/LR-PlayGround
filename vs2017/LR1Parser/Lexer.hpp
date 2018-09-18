#pragma once
#include <stdafx.h>

#include <list>
#include <regex>
#include <string>

#include <Grammar.hpp>

namespace LR::Lexer
{
    using TokenStream = std::list<unsigned int>;

    class Lexer
    {
    public:
        static TokenStream Lex(const Grammar::Grammar& g, const std::string input)
        {
            TokenStream ret;
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
                    return TokenStream();
                }
            }
            return ret;
        }
        static void DumpTokenStream(const Grammar::Grammar& g, const TokenStream& ts)
        {
            std::cout << "[INPUT] ";
            for (auto token : ts)
            {
                std::cout << g.GetTokenName(token) << " ";
            }
            std::cout << "\n";
        }
    private:
    };
}
