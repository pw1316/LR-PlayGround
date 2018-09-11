#pragma once
#include <queue>
#include <regex>
#include <string>

#include <iostream>

#include <Token.hpp>

namespace LR::Lexer
{
    class Lexer
    {
    public:
        static std::queue<Token::ElementSymbol> Lex(const std::string input)
        {
            std::queue<Token::ElementSymbol> ret;
            std::vector<std::regex> token_reg(Token::TOKEN_VALUE.size());
            for (size_t i = 0; i < token_reg.size(); ++i)
            {
                token_reg[i] = Token::TOKEN_VALUE[i];
            }

            auto iter = input.begin();
            Token::ElementSymbol symbolCache;
            symbolCache.header.type = Token::ElementType::Symbol;
            symbolCache.type = Token::SymbolType::NONE;
            while (iter != input.end())
            {
                size_t i = 0;
                for (i = 0; i < token_reg.size(); ++i)
                {
                    std::smatch ms;
                    if (std::regex_search(iter, input.end(), ms, token_reg[i], std::regex_constants::match_continuous))
                    {
                        symbolCache.header.type = Token::ElementType::Symbol;
                        symbolCache.type = static_cast<Token::SymbolType>(i + 1);
                        PushSymbol(ret, symbolCache);
                        iter += ms[0].length();
                        break;
                    }
                }
                /* Failed */
                if (i == token_reg.size())
                {
                    return std::queue<Token::ElementSymbol>();
                }
            }
            return ret;
        }
    private:
        static void PushSymbol(std::queue<Token::ElementSymbol> &q, Token::ElementSymbol &symbol)
        {
            if (symbol.type == Token::SymbolType::NONE)
            {
                return;
            }
            q.push(symbol);
            symbol.header.type = Token::ElementType::Symbol;
            symbol.type = Token::SymbolType::NONE;
        }
    };
}