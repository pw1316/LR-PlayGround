#pragma once
#include <stdafx.h>

#include <queue>
#include <regex>
#include <string>

#include <Grammar.hpp>

namespace LR::Lexer
{
    class Lexer
    {
    public:
        using TokenStream = std::queue<unsigned int>;

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
                        ret.push(tId);
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
    private:
    };
}
