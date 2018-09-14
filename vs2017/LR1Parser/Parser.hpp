#pragma once
#include <stdafx.h>

#include <list>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include <Grammar.hpp>

namespace LR::Parser
{
    struct LR0Item
    {
        unsigned int grammarId;
        unsigned int dotPos;
        bool operator==(const LR0Item& rhs) const
        {
            return grammarId == rhs.grammarId && dotPos == rhs.dotPos;
        }
        bool operator<(const LR0Item& rhs) const
        {
            if (grammarId < rhs.grammarId)
            {
                return true;
            }
            if (grammarId > rhs.grammarId)
            {
                return false;
            }
            if (dotPos < rhs.dotPos)
            {
                return true;
            }
            return false;
        }
        bool operator>(const LR0Item& rhs) const
        {
            return !(*this < rhs || *this == rhs);
        }
    };

    struct LR1Item
    {
        LR0Item lr0;
        std::set<unsigned int> lookAhead;
        bool operator<(const LR1Item& rhs) const
        {
            if (lr0 < rhs.lr0)
            {
                return true;
            }
            if (lr0 > rhs.lr0)
            {
                return false;
            }
            if (lookAhead < rhs.lookAhead)
            {
                return true;
            }
            return false;
        }
    };

    class LR0State
    {
    public:
        void Clear()
        {
            m_closure.clear();
        }
        bool Empty()
        {
            return m_closure.empty();
        }
        void Add(const Grammar::Grammar& grammar, const LR0Item& item)
        {
            m_closure.insert(item);
        }
        void Closure()
        {
            std::queue<LR0Item> BFS;

        }
        void AddAndClosure(const Grammar::Grammar& grammar, const LR0Item& item)
        {
            Add(grammar, item);
            Closure();
        }
    private:
        std::set<LR0Item> m_closure;
    };

    struct LR0DFA
    {
        std::vector<LR0State> states;
        std::vector<std::map<unsigned int, unsigned int>> edges;
    };

    class Parser
    {
    public:
        using TokenSet = std::set<unsigned int>;
        using TokenSetList = std::vector<TokenSet>;

        static TokenSetList FirstSet(const Grammar::Grammar& grammar)
        {
            /* All Tokens & START & TERMINAL */
            const auto setSize = static_cast<unsigned int>(grammar.TerminalTokenNames().size() + grammar.NonTerminalTokenNames().size() + 2);
            TokenSetList ret(setSize);

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

        static TokenSetList FollowSet(const Grammar::Grammar& grammar, const TokenSetList& firstSet)
        {
            /* All Tokens & START & TERMINAL */
            const auto setSize = static_cast<unsigned int>(grammar.TerminalTokenNames().size() + grammar.NonTerminalTokenNames().size() + 2);
            TokenSetList ret(setSize);

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

        static LR0DFA BuildDFALR0(const Grammar::Grammar& grammar, const TokenSetList& firstSet, const TokenSetList& followSet)
        {
            LR0DFA dfa;
            LR0State state;
            for (size_t gId = 0; gId < grammar.G().size(); ++gId)
            {
                if (grammar.G()[gId][0] == grammar.START())
                {
                    LR0Item item;
                    item.grammarId = static_cast<unsigned int>(gId);
                    item.dotPos = 0;
                    state.Add(grammar, item);
                }
            }
            assert(!state.Empty());
            state.Closure();

            std::queue<LR0State> BFS;
            BFS.push(std::move(state));
            while (!BFS.empty())
            {
                state = BFS.front();
                BFS.pop();
                // TODO BFS for edge
            }
        }
    private:
    };
}
