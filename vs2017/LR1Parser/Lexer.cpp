#include <stdafx.h>
#include "Lexer.hpp"
#include <Grammar.hpp>

#include <regex>

namespace LR
{
    bool Lexer::SetInput(const std::string& input)
    {
        Utils::TokenStream nts;
        auto iter = input.begin();
        while (iter != input.end())
        {
            bool isFailed = true;
            for (Utils::TokenId tId = 0U; tId < m_g.NumTerminalToken(); ++tId)
            {
                std::smatch ms;
                if (std::regex_search(iter, input.end(), ms, std::regex(m_g.TerminalTokenValues()[tId]), std::regex_constants::match_continuous))
                {
                    nts.emplace_back(tId, ms[0].str());
                    iter += ms[0].length();
                    isFailed = false;
                    break;
                }
            }
            /* Failed */
            if (isFailed)
            {
                return false;
            }
        }
        m_ts = std::move(nts);
        return true;
    }
    void Lexer::Dump()
    {
        std::cout << "[INPUT] ";
        for (auto token : m_ts)
        {
            std::cout << token.value << " ";
        }
        std::cout << "\n";
    }
}
