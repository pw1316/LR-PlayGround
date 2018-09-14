#pragma once
#include <stdafx.h>

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

            /* Terminal's first set only contains the terminal itself */
            for (unsigned int token = 0; token < setSize; ++token)
            {
                if (grammar.IsTerminalToken(token) || grammar.IsTerminal(token))
                {
                    ret[token].insert(token);
                }
            }
            /* Loop until first sets no longer change */
            bool isDirty = true;
            while (isDirty)
            {
                isDirty = false;
                for (auto& g : grammar.G())
                {
                    assert(g.size() > 1U);
                    size_t idx = 1;
                    while (idx < g.size())
                    {
                        size_t oldSize = ret[g[0]].size();
                        if (grammar.HasEpsilon() && g[idx] == grammar.EPSILON())
                        {
                            ret[g[0]].insert(grammar.EPSILON());
                            isDirty |= oldSize != ret[g[0]].size();
                            break;
                        }
                        else
                        {
                            ret[g[0]].insert(ret[g[idx]].begin(), ret[g[idx]].end());
                            isDirty |= oldSize != ret[g[0]].size();
                            if (grammar.HasEpsilon() && ret[g[idx]].find(grammar.EPSILON()) != ret[g[idx]].end())
                            {
                                ++idx;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
            }
            return ret;
        }

        static std::vector<std::set<unsigned int>> FollowSet(const Grammar::Grammar& grammar, const std::vector<std::set<unsigned int>>& firstSet)
        {
            /* All Tokens & START & TERMINAL */
            const auto setSize = static_cast<unsigned int>(grammar.TerminalTokenNames().size() + grammar.NonTerminalTokenNames().size() + 2);
            std::vector<std::set<unsigned int>> ret(setSize);

            /* First, $ is in START's Follow Set */
            ret[grammar.START()].insert(grammar.TERMINAL());


            /* Loop until Follow Sets no longer change */
            bool isDirty = true;
            while (isDirty)
            {
                isDirty = false;
                for (auto& g : grammar.G())
                {
                    assert(g.size() > 1U);
                    size_t idx = g.size() - 1;
                    while (idx > 0)
                    {
                        if (grammar.IsNonTerminalToken(g[idx]))
                        {
                            size_t oldSize = ret[g[idx]].size();
                            ret[g[idx]].insert(ret[g[0]].begin(), ret[g[0]].end());
                            isDirty |= oldSize != ret[g[idx]].size();
                        }
                        if (grammar.HasEpsilon() && g[idx] == grammar.EPSILON())
                        {
                            break;
                        }
                        else if (grammar.HasEpsilon() && firstSet[g[idx]].find(grammar.EPSILON()) != firstSet[g[idx]].end())
                        {
                            --idx;
                        }
                        else
                        {
                            break;
                        }
                    }
                    for (idx = 1; idx < g.size() - 1; ++idx)
                    {
                        if (grammar.IsNonTerminalToken(g[idx]))
                        {
                            size_t oldSize = ret[g[idx]].size();
                            ret[g[idx]].insert(firstSet[g[idx + 1]].begin(), firstSet[g[idx + 1]].end());
                            if (grammar.HasEpsilon())
                            {
                                ret[g[idx]].erase(grammar.EPSILON());
                            }
                            isDirty |= oldSize != ret[g[idx]].size();
                        }
                    }
                }
            }
            return ret;
        }
    private:
    };
}
