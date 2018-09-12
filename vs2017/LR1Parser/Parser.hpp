#pragma once
#include <set>
#include <vector>

#include <Grammar.hpp>

namespace LR::Parser
{
    class Parser
    {
    public:
        static std::vector<std::set<unsigned int>> FirstSet(const Grammar::Grammar& grammar)
        {
            /* All Tokens & START & TERMINAL */
            const auto setSize = static_cast<unsigned int>(grammar.TerminalTokenNames().size() + grammar.NonTerminalTokenNames().size() + 2);
            std::vector<std::set<unsigned int>> ret(setSize);
            isDone.clear();
            isDone.resize(setSize);

            for (auto token = 0U; token < setSize; ++token)
            {
                FirstSet(grammar, ret, token);
            }
            return ret;
        }

        static void FirstSet(const Grammar::Grammar& grammar, std::vector<std::set<unsigned int>>& sets, const unsigned int token)
        {
            isDone[token] = 1;
            std::set<unsigned int> set;
            /* terminals */
            if (grammar.IsTerminalToken(token) || grammar.IsTerminal(token))
            {
                set.insert(token);
                sets[token] = std::move(set);
                return;
            }

            /* non terminals */
            for (auto& g : grammar.G())
            {
                if (g[0] == token)
                {

                }
            }
        }
    private:
        static std::vector<int> isDone;
    };
}
