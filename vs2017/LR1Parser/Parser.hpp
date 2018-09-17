#pragma once
#include <stdafx.h>

#include <fstream>
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
        LR0Item() : grammarId(0U), dotPos(1U) {}
        LR0Item(unsigned int gId, unsigned int dPos) : grammarId(gId), dotPos(dPos) {}
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
        void Closure(const Grammar::Grammar& grammar)
        {
            std::queue<LR0Item> BFS;
            for (auto& item : m_closure)
            {
                BFS.push(item);
            }
            m_closure.clear();
            std::vector<int> visited(grammar.NonTerminalTokenNames().size() + 1, 0);
            while (!BFS.empty())
            {
                auto item = BFS.front();
                BFS.pop();
                if (item.dotPos == 1U)
                {
                    visited[grammar.G()[item.grammarId][0] - grammar.TerminalTokenNames().size()] = 1;
                }
                if (item.dotPos < grammar.G()[item.grammarId].size() && grammar.IsNonTerminalToken(grammar.G()[item.grammarId][item.dotPos]) && !visited[grammar.G()[item.grammarId][item.dotPos] - grammar.TerminalTokenNames().size()])
                {
                    for (size_t i = 0; i < grammar.G().size(); ++i)
                    {
                        if (grammar.G()[i][0] == grammar.G()[item.grammarId][item.dotPos])
                        {
                            LR0Item nitem(static_cast<unsigned int>(i), 1U);
                            BFS.push(nitem);
                        }
                    }
                }
                m_closure.insert(item);
            }
        }
        void AddAndClosure(const Grammar::Grammar& grammar, const LR0Item& item)
        {
            Add(grammar, item);
            Closure(grammar);
        }
        const std::set<LR0Item>& Items() const
        {
            return m_closure;
        }
        bool operator==(const LR0State& rhs) const
        {
            return m_closure == rhs.m_closure;
        }
        bool operator<(const LR0State& rhs) const
        {
            return m_closure < rhs.m_closure;
        }
        bool operator>(const LR0State& rhs) const
        {
            return m_closure > rhs.m_closure;
        }
    private:
        std::set<LR0Item> m_closure;
    };

    template<class T>
    struct LRDFA
    {
        std::vector<T> states;
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

        static LRDFA<LR0State> BuildDFALR0(const Grammar::Grammar& grammar, const TokenSetList& firstSet, const TokenSetList& followSet)
        {
            LRDFA<LR0State> dfa;
            LR0State state;
            for (size_t gId = 0; gId < grammar.G().size(); ++gId)
            {
                if (grammar.G()[gId][0] == grammar.START())
                {
                    LR0Item item(static_cast<unsigned int>(gId), 1U);
                    state.Add(grammar, item);
                }
            }
            assert(!state.Empty());
            state.Closure(grammar);

            std::queue<LR0State> BFS;
            BFS.push(std::move(state));
            std::map<LR0State, unsigned int> visited;
            visited[BFS.front()] = 0U;
            while (!BFS.empty())
            {
                state = BFS.front();
                BFS.pop();
                dfa.states.push_back(state);
                dfa.edges.emplace_back();
                std::set<unsigned int> possibleEdges;
                for (auto& item : state.Items())
                {
                    if (item.dotPos < grammar.G()[item.grammarId].size() && !(grammar.HasEpsilon() && grammar.G()[item.grammarId][item.dotPos] == grammar.EPSILON()))
                    {
                        possibleEdges.insert(grammar.G()[item.grammarId][item.dotPos]);
                    }
                }
                for (auto edge : possibleEdges)
                {
                    LR0State nstate;
                    for (auto& item : state.Items())
                    {
                        if (item.dotPos < grammar.G()[item.grammarId].size() && grammar.G()[item.grammarId][item.dotPos] == edge)
                        {
                            LR0Item nitem(static_cast<unsigned int>(item.grammarId), item.dotPos + 1);
                            nstate.Add(grammar, nitem);
                        }
                    }
                    nstate.Closure(grammar);
                    if (visited.find(nstate) != visited.end())
                    {
                        dfa.edges[dfa.states.size() - 1][edge] = visited[nstate];
                    }
                    else
                    {
                        BFS.push(nstate);
                        visited[nstate] = static_cast<unsigned int>(dfa.states.size() + BFS.size() - 1U);
                        dfa.edges[dfa.states.size() - 1][edge] = static_cast<unsigned int>(dfa.states.size() + BFS.size() - 1U);
                    }
                }
                // TODO BFS for edge
            }
            return dfa;
        }

        static void DumpDFA(const Grammar::Grammar& grammar, const LRDFA<LR0State>& dfa)
        {
            std::ofstream ofs("graph.md");
            ofs << "```mermaid\n";
            ofs << "graph LR\n";
            unsigned int idx = 0U;
            for (auto& state : dfa.states)
            {
                ofs << idx << "(\"";
                for (auto& item : state.Items())
                {
                    bool isLeft = true;
                    for (size_t tId = 0; tId < grammar.G()[item.grammarId].size(); ++tId)
                    {
                        auto token = grammar.G()[item.grammarId][tId];
                        auto tokenType = grammar.GetTokenType(token);
                        if (item.dotPos == static_cast<unsigned int>(tId))
                        {
                            ofs << ". ";
                        }
                        switch (tokenType)
                        {
                        case LR::Grammar::TokenType::TT_START:
                            ofs << "! ";
                            break;
                        case LR::Grammar::TokenType::TT_EPSILON:
                            ofs << "@ ";
                            break;
                        case LR::Grammar::TokenType::TT_TOKEN_SYMB:
                            ofs << grammar.NonTerminalTokenNames()[token - grammar.TerminalTokenNames().size()] << " ";
                            break;
                        case LR::Grammar::TokenType::TT_TOKEN_TERM:
                            ofs << grammar.TerminalTokenNames()[token] << " ";
                            break;
                        default:
                            break;
                        }
                        if (isLeft)
                        {
                            ofs << "-> ";
                        }
                        isLeft = false;
                    }
                    if (item.dotPos == static_cast<unsigned int>(grammar.G()[item.grammarId].size()))
                    {
                        ofs << ". ";
                    }
                    ofs << "<br />";
                }
                ofs << "\")\n";
                ++idx;
            }
            for (size_t sId = 0; sId < dfa.edges.size(); ++sId)
            {
                for (const auto& edge : dfa.edges[sId])
                {
                    if (grammar.IsTerminalToken(edge.first))
                    {
                        ofs << sId << "--" << grammar.TerminalTokenNames()[edge.first] << "-->" << edge.second << "\n";
                    }
                    else
                    {
                        ofs << sId << "--" << grammar.NonTerminalTokenNames()[edge.first - static_cast<unsigned int>(grammar.TerminalTokenNames().size())] << "-->" << edge.second << "\n";
                    }
                }
            }
            ofs << "```\n";
        }
    private:
    };
}
