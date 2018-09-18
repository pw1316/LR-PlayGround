#pragma once
#include <stdafx.h>

#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include <Grammar.hpp>
#include <Lexer.hpp>

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
        LR1Item(Grammar::GrammarId gId, Grammar::TokenId dPos, const std::set<Grammar::TokenId>& lh) : lr0(gId, dPos), lookAhead(lh) {}
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
                        lookAhead.insert(item.lookAhead.begin(), item.lookAhead.end());
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
        LR0State Core(const Grammar::Grammar& grammar) const
        {
            LR0State ret;
            for (auto& item : m_closure)
            {
                ret.Add(item.lr0);
            }
            ret.Closure(grammar);
            return ret;
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
    private:
        using TransformCB = std::function<bool(const Grammar::Grammar&)>;
        using TransformTable = std::vector<std::vector<TransformCB>>;
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
                        BFS.push(nstate);
                        auto nStateId = static_cast<Grammar::StateId>(dfa.states.size() + BFS.size() - 1U);
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
            LR1State initState;
            for (size_t gId = 0; gId < grammar.G().size(); ++gId)
            {
                if (grammar.IsStart(grammar.G()[gId][0]))
                {
                    LR1Item item(static_cast<Grammar::GrammarId>(gId), 1U, { grammar.TERMINAL() });
                    initState.Add(item);
                }
            }
            assert(!initState.Empty());
            initState.Closure(grammar, firstSet);

            dfa.states.push_back(initState);
            dfa.edges.emplace_back();
            std::map<LR0State, Grammar::StateId> visited;
            visited[dfa.states[0U].Core(grammar)] = 0U;
            std::queue<Grammar::StateId> BFS;
            BFS.push(0U);
            while (!BFS.empty())
            {
                auto curStateId = BFS.front();
                BFS.pop();
                std::set<Grammar::TokenId> possibleEdges;
                for (auto& curItem : dfa.states[curStateId].Items())
                {
                    if (curItem.lr0.dotPos < grammar.G()[curItem.lr0.grammarId].size() && !grammar.IsEpsilon(grammar.G()[curItem.lr0.grammarId][curItem.lr0.dotPos]))
                    {
                        possibleEdges.insert(grammar.G()[curItem.lr0.grammarId][curItem.lr0.dotPos]);
                    }
                }
                for (auto edge : possibleEdges)
                {
                    LR1State nextState;
                    for (auto& curItem : dfa.states[curStateId].Items())
                    {
                        if (curItem.lr0.dotPos < grammar.G()[curItem.lr0.grammarId].size() && grammar.G()[curItem.lr0.grammarId][curItem.lr0.dotPos] == edge)
                        {
                            LR1Item nextItem(curItem.lr0.grammarId, curItem.lr0.dotPos + 1, curItem.lookAhead);
                            nextState.Add(nextItem);
                        }
                    }
                    nextState.Closure(grammar, firstSet);
                    if (visited.find(nextState.Core(grammar)) != visited.end())
                    {
                        Grammar::StateId existStateId = visited[nextState.Core(grammar)];
                        auto backup = dfa.states[existStateId];
                        for (auto& nextItem : nextState.Items())
                        {
                            dfa.states[existStateId].Add(nextItem);
                        }
                        dfa.states[existStateId].Closure(grammar, firstSet);
                        if (!(backup == dfa.states[existStateId]))
                        {
                            BFS.push(existStateId);
                        }
                        dfa.edges[curStateId][edge] = existStateId;
                    }
                    else
                    {
                        auto nStateId = static_cast<Grammar::StateId>(dfa.states.size());
                        dfa.states.push_back(nextState);
                        dfa.edges.emplace_back();
                        BFS.push(nStateId);
                        visited[nextState.Core(grammar)] = nStateId;
                        dfa.edges[curStateId][edge] = nStateId;
                    }
                }
            }
            return dfa;
        }

        static std::vector<std::vector<TransformCB>> BuildTransformTable(const Grammar::Grammar& grammar, const LRDFA<LR1State>& dfa, Parser& parser)
        {
            std::vector<std::vector<TransformCB>> ret(dfa.states.size(), std::vector<TransformCB>(grammar.NumToken() + 2U, nullptr));
            for (Grammar::StateId sId = 0U; sId < static_cast<Grammar::StateId>(dfa.states.size()); ++sId)
            {
                for (auto& edge : dfa.edges[sId])
                {
                    if (grammar.IsTerminalToken(edge.first) || grammar.IsTerminal(edge.first))
                    {
                        ret[sId][edge.first] = std::bind(&Parser::Shift, &parser, std::placeholders::_1, edge.first, edge.second);
                    }
                    else if (grammar.IsNonTerminalToken(edge.first) || grammar.IsStart(edge.first))
                    {
                        ret[sId][edge.first] = std::bind(&Parser::Goto, &parser, std::placeholders::_1, edge.first, edge.second);
                    }
                }
                for (auto& item : dfa.states[sId].Items())
                {
                    if (item.lr0.dotPos == grammar.G()[item.lr0.grammarId].size())
                    {
                        for (auto token : item.lookAhead)
                        {
                            if (ret[sId][token] != nullptr)
                            {
                                std::cout << "[ERR] Not LR(1) language!!!\n";
                                return std::vector<std::vector<TransformCB>>();
                            }
                            ret[sId][token] = std::bind(&Parser::Reduce, &parser, std::placeholders::_1, token, item.lr0.grammarId);
                        }
                    }
                }
            }
            return ret;
        }

        static bool IsLR0(const Grammar::Grammar& grammar, const LRDFA<LR0State>& dfa)
        {
            for (Grammar::StateId sId = 0; sId < static_cast<Grammar::StateId>(dfa.states.size()); ++sId)
            {
                for (auto iter1 = dfa.states[sId].Items().begin(); iter1 != dfa.states[sId].Items().end(); ++iter1)
                {
                    for (auto iter2 = iter1; iter2 != dfa.states[sId].Items().end(); ++iter2)
                    {
                        if (iter1 != iter2)
                        {
                            if (iter1->dotPos == grammar.G()[iter1->grammarId].size()\
                                || iter2->dotPos == grammar.G()[iter2->grammarId].size())
                            {
                                std::cout << "[ERR LR0] Conflict at state " << sId << "\n";
                                return false;
                            }
                        }
                    }
                }
            }
            return true;
        }

        static void DumpDFA(const Grammar::Grammar& grammar, const LRDFA<LR0State>& dfa)
        {
            std::ofstream ofs("graph_lr0.md");
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

        static void DumpDFA(const Grammar::Grammar& grammar, const LRDFA<LR1State>& dfa)
        {
            std::ofstream ofs("graph_lr1.md");
            ofs << "```mermaid\n";
            ofs << "graph LR\n";
            Grammar::StateId idx = 0U;
            for (auto& state : dfa.states)
            {
                ofs << idx << "(\"";
                for (auto& item : state.Items())
                {
                    bool isLeft = true;
                    for (size_t tId = 0; tId < grammar.G()[item.lr0.grammarId].size(); ++tId)
                    {
                        auto token = grammar.G()[item.lr0.grammarId][tId];
                        if (item.lr0.dotPos == static_cast<Grammar::TokenId>(tId))
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
                    if (item.lr0.dotPos == static_cast<Grammar::TokenId>(grammar.G()[item.lr0.grammarId].size()))
                    {
                        ofs << ". ";
                    }
                    ofs << "{";
                    for (auto lah : item.lookAhead)
                    {
                        ofs << grammar.GetTokenName(lah) << ",";
                    }
                    ofs << "}";
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

        Parser(const Grammar::Grammar& grammar)
        {
            bool res = true;
            m_firstSet = FirstSet(grammar);
            m_followSet = FollowSet(grammar, m_firstSet);

            m_LR0DFA = BuildDFALR0(grammar, m_firstSet, m_followSet);
            m_LR1DFA = BuildDFALR1(grammar, m_firstSet, m_followSet);
            m_table = BuildTransformTable(grammar, m_LR1DFA, *this);
        }
        void BeginParse(const Grammar::Grammar& grammar, const Lexer::TokenStream& ts)
        {
            m_parseStack.clear();
            Grammar::Element ele;
            ele.state.header.type = Grammar::ElementType::State;
            ele.state.stateId = 0U;
            m_parseStack.push_back(ele);

            m_tokenStream = ts;
            if (m_tokenStream.back() != grammar.TERMINAL())
            {
                m_tokenStream.push_back(grammar.TERMINAL());
            }
        }
        bool Step(const Grammar::Grammar& grammar)
        {
            if (m_tokenStream.empty())
            {
                return false;
            }
            if (m_tokenStream.front() == grammar.START())
            {
                std::cout << "[PARSE] DONE.\n";
                return false;
            }
            auto token = m_tokenStream.front();
            m_table[m_parseStack.back().state.stateId][token](grammar);
            return true;
        }
    private:
        bool Shift(const Grammar::Grammar& grammar, Grammar::TokenId token, Grammar::StateId state)
        {
            std::cout << "[PARSE] SHIFT " << grammar.GetTokenName(token) << " to " << state << "\n";
            if (m_tokenStream.empty())
            {
                return false;
            }
            auto tstoken = m_tokenStream.front();
            assert(tstoken == token);
            m_tokenStream.pop_front();
            Grammar::Element ele;
            ele.token.header.type = Grammar::ElementType::Token;
            ele.token.tokenId = token;
            m_parseStack.push_back(ele);

            ele.state.header.type = Grammar::ElementType::State;
            ele.state.stateId = state;
            m_parseStack.push_back(ele);
            return true;
        }

        bool Goto(const Grammar::Grammar& grammar, Grammar::TokenId token, Grammar::StateId state)
        {
            std::cout << "[PARSE] GOTO " << state << "\n";
            if (m_tokenStream.empty())
            {
                return false;
            }
            auto tstoken = m_tokenStream.front();
            assert(tstoken == token);
            m_tokenStream.pop_front();
            Grammar::Element ele;
            ele.token.header.type = Grammar::ElementType::Token;
            ele.token.tokenId = token;
            m_parseStack.push_back(ele);

            ele.state.header.type = Grammar::ElementType::State;
            ele.state.stateId = state;
            m_parseStack.push_back(ele);
            return true;
        }

        bool Reduce(const Grammar::Grammar& grammar, Grammar::TokenId token, Grammar::GrammarId gId)
        {
            std::cout << "[PARSE] REDUCE ";
            {
                bool isLeft = true;
                for (auto token : grammar.G()[gId])
                {
                    std::cout << grammar.GetTokenName(token) << " ";
                    if (isLeft)
                    {
                        std::cout << "-> ";
                    }
                    isLeft = false;
                }
                std::cout << "\n";
            }
            auto &g = grammar.G()[gId];
            for (size_t i = 1; i < g.size(); ++i)
            {
                m_parseStack.pop_back();
                m_parseStack.pop_back();
            }

            m_tokenStream.push_front(g[0]);
            return true;
        }

        Grammar::TokenSetList m_firstSet;
        Grammar::TokenSetList m_followSet;
        LRDFA<LR0State> m_LR0DFA;
        LRDFA<LR1State> m_LR1DFA;
        TransformTable m_table;
        Grammar::ParseStack m_parseStack;
        Lexer::TokenStream m_tokenStream;
    };
}
