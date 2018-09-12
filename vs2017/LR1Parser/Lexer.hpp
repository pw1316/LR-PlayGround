#pragma once
#include <queue>
#include <regex>
#include <string>

#include <iostream>

#include <Grammar.hpp>

namespace LR::Lexer
{
    class Lexer
    {
    public:
        static std::queue<Grammar::ElementToken> Lex(const Grammar::Grammar& g, const std::string input)
        {
            std::queue<Grammar::ElementToken> ret;
            auto iter = input.begin();
            while (iter != input.end())
            {
                size_t i = 0;
                for (i = 0; i < g.TerminalTokenValues().size(); ++i)
                {
                    std::smatch ms;
                    if (std::regex_search(iter, input.end(), ms, std::regex(g.TerminalTokenValues()[i]), std::regex_constants::match_continuous))
                    {
                        Grammar::ElementToken token;
                        token.header.type = Grammar::ElementType::Token;
                        token.tokenId = static_cast<unsigned int>(i);
                        ret.push(token);
                        iter += ms[0].length();
                        break;
                    }
                }
                /* Failed */
                if (i == g.TerminalTokenValues().size())
                {
                    return std::queue<Grammar::ElementToken>();
                }
            }
            return ret;
        }
    private:
    };
}
