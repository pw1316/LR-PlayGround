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
        LR0Item() : grammarId(0U), dotPos(1U) {}
        LR0Item(Grammar::GrammarId gId, Grammar::TokenId dPos) : grammarId(gId), dotPos(dPos) {}
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
        Grammar::GrammarId grammarId;
        Grammar::TokenId dotPos;
    };

    struct LR1Item
    {
        LR1Item() {}
        LR1Item(Grammar::GrammarId gId, Grammar::TokenId dPos, const std::initializer_list<Grammar::TokenId>& lh) : lr0(gId, dPos), lookAhead(lh) {}
        bool operator==(const LR1Item& rhs) const
        {
            return lr0 == rhs.lr0 && lookAhead == rhs.lookAhead;
        }
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
        bool operator>(const LR1Item& rhs) const
        {
            return !(*this < rhs || *this == rhs);
        }
        LR0Item lr0;
        Grammar::TokenSet lookAhead;
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
        void Add(const LR0Item& item)
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
            std::vector<int> visited(grammar.NumToken() + 2, 0);
            while (!BFS.empty())
            {
                auto item = BFS.front();
                BFS.pop();
                if (item.dotPos == 1U)
                {
                    visited[grammar.G()[item.grammarId][0]] = 1;
                }
                if (item.dotPos < grammar.G()[item.grammarId].size() && grammar.IsNonTerminalToken(grammar.G()[item.grammarId][item.dotPos]) && !visited[grammar.G()[item.grammarId][item.dotPos]])
                {
                    for (size_t i = 0; i < grammar.G().size(); ++i)
                    {
                        if (grammar.G()[i][0] == grammar.G()[item.grammarId][item.dotPos])
                        {
                            LR0Item nitem(static_cast<Grammar::GrammarId>(i), 1U);
                            BFS.push(nitem);
                        }
                    }
                }
                m_closure.insert(item);
            }
        }
        void AddAndClosure(const Grammar::Grammar& grammar, const LR0Item& item)
        {
            Add(item);
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


    class LR1State
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
        void Add(const LR1Item& item)
        {
            m_closure.insert(item);
        }
        void Closure(const Grammar::Grammar& grammar, const Grammar::TokenSetList& firstSet)
        {
            std::queue<LR1Item> BFS;
            for (auto& item : m_closure)
            {
                BFS.push(item);
            }
            m_closure.clear();
            std::vector<std::set<Grammar::TokenId>> visited(grammar.NumToken() + 2);
            while (!BFS.empty())
            {
                auto item = BFS.front();
                BFS.pop();
                if (item.lr0.dotPos == 1U)
                {
                    visited[grammar.G()[item.lr0.grammarId][0]].insert(item.lookAhead.begin(), item.lookAhead.end());
                }
                if (item.lr0.dotPos < grammar.G()[item.lr0.grammarId].size() && grammar.IsNonTerminalToken(grammar.G()[item.lr0.grammarId][item.lr0.dotPos]))
                {
                    Grammar::TokenSet lookAhead;
                    Grammar::TokenId pos = item.lr0.dotPos + 1;
                    while (pos < grammar.G()[item.lr0.grammarId].size())
                    {
                        assert(!grammar.IsEpsilon(grammar.G()[item.lr0.grammarId][pos]));
                        lookAhead.insert(firstSet[grammar.G()[item.lr0.grammarId][pos]].begin(), firstSet[grammar.G()[item.lr0.grammarId][pos]].end());
                        if (grammar.HasEpsilon() && firstSet[grammar.G()[item.lr0.grammarId][pos]].find(grammar.EPSILON()) != firstSet[grammar.G()[item.lr0.grammarId][pos]].end())
                        {
                            ++pos;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (pos == grammar.G()[item.lr0.grammarId].size())
                    {
                        lookAhead.insert(grammar.TERMINAL());
                    }
                    for (auto lah : lookAhead)
                    {
                        if (visited[grammar.G()[item.lr0.grammarId][item.lr0.dotPos]].find(lah) == visited[grammar.G()[item.lr0.grammarId][item.lr0.dotPos]].end())
                        {
                            for (size_t i = 0; i < grammar.G().size(); ++i)
                            {
                                if (grammar.G()[i][0] == grammar.G()[item.lr0.grammarId][item.lr0.dotPos])
                                {
                                    LR1Item nitem(static_cast<Grammar::GrammarId>(i), 1U, { lah });
                                    BFS.push(nitem);
                                }
                            }
                        }
                    }
                }
                m_closure.insert(item);
            }
            /* Merge LookAheads */
            std::set<LR1Item> nclosure;
            for (auto& item : m_closure)
            {
                bool found = false;
                for (auto& nitem : nclosure)
                {
                    if (nitem.lr0 == item.lr0)
                    {
                        auto nnitem = nitem;
                        nnitem.lookAhead.insert(item.lookAhead.begin(), item.lookAhead.end());
                        nclosure.erase(nitem);
                        nclosure.insert(nnitem);
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    nclosure.insert(item);
                }
            }
            m_closure = std::move(nclosure);
        }
        void AddAndClosure(const Grammar::Grammar& grammar, const LR1Item& item, const Grammar::TokenSetList& firstSet)
        {
            Add(item);
            Closure(grammar, firstSet);
        }
        const std::set<LR1Item>& Items() const
        {
            return m_closure;
        }
        bool operator==(const LR1State& rhs) const
        {
            return m_closure == rhs.m_closure;
        }
        bool operator<(const LR1State& rhs) const
        {
            return m_closure < rhs.m_closure;
        }
        bool operator>(const LR1State& rhs) const
        {
            return m_closure > rhs.m_closure;
        }
    private:
        std::set<LR1Item> m_closure;
    };

    template<class T>
    struct LRDFA
    {
        using StateList = std::vector<T>;
        using Edge = std::map<Grammar::TokenId, Grammar::StateId>;
        using EdgeList = std::vector<Edge>;
        StateList states;
        EdgeList edges;
    };

    class Parser
    {
    public:
        static Grammar::TokenSetList FirstSet(const Grammar::Grammar& grammar)
        {
            /* All Tokens & START & TERMINAL */
            const auto setSize = grammar.NumToken() + 2U;
            Grammar::TokenSetList ret(setSize);

            /* Terminal's first set only contains the terminal itself */
            for (Grammar::TokenId token = 0U; token < setSize; ++token)
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
                    size_t idx = 1U;
                    while (idx < g.size())
                    {
                        size_t oldSize = ret[g[0]].size();
                        if (grammar.IsEpsilon(g[idx]))
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

        static Grammar::TokenSetList FollowSet(const Grammar::Grammar& grammar, const Grammar::TokenSetList& firstSet)
        {
            /* All Tokens & START & TERMINAL */
            const auto setSize = grammar.NumToken() + 2U;
            Grammar::TokenSetList ret(setSize);

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
                    size_t idx = g.size() - 1U;
                    while (idx > 0U)
                    {
                        if (grammar.IsNonTerminalToken(g[idx]))
                        {
                            size_t oldSize = ret[g[idx]].size();
                            ret[g[idx]].insert(ret[g[0]].begin(), ret[g[0]].end());
                            isDirty |= oldSize != ret[g[idx]].size();
                        }
                        if (grammar.IsEpsilon(g[idx]))
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
                    for (idx = 1; idx < g.size() - 1U; ++idx)
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

        static LRDFA<LR0State> BuildDFALR0(const Grammar::Grammar& grammar, const Grammar::TokenSetList& firstSet, const Grammar::TokenSetList& followSet)
        {
            LRDFA<LR0State> dfa;
            LR0State state;
            for (size_t gId = 0; gId < grammar.G().size(); ++gId)
            {
                if (grammar.IsStart(grammar.G()[gId][0]))
                {
                    LR0Item item(static_cast<Grammar::GrammarId>(gId), 1U);
                    state.Add(item);
                }
            }
            assert(!state.Empty());
            state.Closure(grammar);

            std::queue<LR0State> BFS;
            BFS.push(std::move(state));
            std::map<LR0State, Grammar::StateId> visited;
            visited[BFS.front()] = 0U;
            while (!BFS.empty())
            {
                state = BFS.front();
                BFS.pop();
                dfa.states.push_back(state);
                dfa.edges.emplace_back();
                std::set<Grammar::TokenId> possibleEdges;
                for (auto& item : state.Items())
                {
                    if (item.dotPos < grammar.G()[item.grammarId].size() && !grammar.IsEpsilon(grammar.G()[item.grammarId][item.dotPos]))
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
                            LR0Item nitem(item.grammarId, item.dotPos + 1);
                            nstate.Add(nitem);
                        }
                    }
                    nstate.Closure(grammar);
                    if (visited.find(nstate) != visited.end())
                    {
                        dfa.edges[dfa.states.size() - 1U][edge] = visited[nstate];
                    }
                    else
                    {
                        auto nStateId = static_cast<Grammar::StateId>(dfa.states.size() + BFS.size() - 1U);
                        BFS.push(nstate);
                        visited[nstate] = nStateId;
                        dfa.edges[dfa.states.size() - 1][edge] = nStateId;
                    }
                }
            }
            return dfa;
        }

        static LRDFA<LR1State> BuildDFALR1(const Grammar::Grammar& grammar, const Grammar::TokenSetList& firstSet, const Grammar::TokenSetList& followSet)
        {
            LRDFA<LR1State> dfa;
            LR1State state;
            for (size_t gId = 0; gId < grammar.G().size(); ++gId)
            {
                if (grammar.IsStart(grammar.G()[gId][0]))
                {
                    LR1Item item(static_cast<Grammar::GrammarId>(gId), 1U, { grammar.TERMINAL() });
                    state.Add(item);
                }
            }
            assert(!state.Empty());
            state.Closure(grammar, firstSet);

            return dfa;
        }

        static void DumpDFA(const Grammar::Grammar& grammar, const LRDFA<LR0State>& dfa)
        {
            std::ofstream ofs("graph.md");
            ofs << "```mermaid\n";
            ofs << "graph LR\n";
            Grammar::StateId idx = 0U;
            for (auto& state : dfa.states)
            {
                ofs << idx << "(\"";
                for (auto& item : state.Items())
                {
                    bool isLeft = true;
                    for (size_t tId = 0; tId < grammar.G()[item.grammarId].size(); ++tId)
                    {
                        auto token = grammar.G()[item.grammarId][tId];
                        if (item.dotPos == static_cast<Grammar::TokenId>(tId))
                        {
                            ofs << ". ";
                        }
                        ofs << grammar.GetTokenName(token) << " ";
                        if (isLeft)
                        {
                            ofs << "-> ";
                        }
                        isLeft = false;
                    }
                    if (item.dotPos == static_cast<Grammar::TokenId>(grammar.G()[item.grammarId].size()))
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
                    ofs << sId << "--" << grammar.GetTokenName(edge.first) << "-->" << edge.second << "\n";
                }
            }
            ofs << "```\n";
        }
    private:
    };
}
