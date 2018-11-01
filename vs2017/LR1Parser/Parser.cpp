#include <stdafx.h>
#include "Parser.hpp"
#include <Grammar.hpp>

#include <fstream>
#include <queue>
#include <string>

namespace LR::Parser
{
    void LRState::Closure(const Grammar::Grammar& grammar, const Utils::TokenSetList& firstSet)
    {
        std::queue<LR1Item> BFS;
        for (auto&& item : m_closure)
        {
            BFS.push(item);
        }
        m_closure.clear();
        std::vector<std::set<Utils::TokenId>> visited(grammar.NumToken() + 2);
        while (!BFS.empty())
        {
            auto item = BFS.front();
            BFS.pop();
            if (item.item.dotPos == 1U)
            {
                visited[grammar.G()[item.item.grammarId][0]].insert(item.lookAhead.begin(), item.lookAhead.end());
            }
            /* Token after dot is a Non-Terminal */
            if (item.item.dotPos < grammar.G()[item.item.grammarId].size() && grammar.IsNonTerminalToken(grammar.G()[item.item.grammarId][item.item.dotPos]))
            {
                Utils::TokenSet possibleLookAhead;
                Utils::TokenId pos = item.item.dotPos + 1;
                while (pos < grammar.G()[item.item.grammarId].size())
                {
                    assert(!grammar.IsEpsilon(grammar.G()[item.item.grammarId][pos]));
                    for (auto&& token : firstSet[grammar.G()[item.item.grammarId][pos]])
                    {
                        possibleLookAhead.insert(token);
                    }
                    if (grammar.HasEpsilon() && firstSet[grammar.G()[item.item.grammarId][pos]].find(grammar.EPSILON()) != firstSet[grammar.G()[item.item.grammarId][pos]].end())
                    {
                        ++pos;
                    }
                    else
                    {
                        break;
                    }
                }
                if (pos == grammar.G()[item.item.grammarId].size())
                {
                    possibleLookAhead.insert(item.lookAhead.begin(), item.lookAhead.end());
                }
                for (auto lah : possibleLookAhead)
                {
                    if (visited[grammar.G()[item.item.grammarId][item.item.dotPos]].find(lah) == visited[grammar.G()[item.item.grammarId][item.item.dotPos]].end())
                    {
                        for (size_t i = 0; i < grammar.G().size(); ++i)
                        {
                            if (grammar.G()[i][0] == grammar.G()[item.item.grammarId][item.item.dotPos])
                            {
                                LR1Item nitem(static_cast<Utils::GrammarId>(i), 1U, { lah });
                                BFS.push(nitem);
                            }
                        }
                    }
                }
            }
            m_closure.insert(item);
        }
    }

    std::tuple<Utils::TokenSetList, Utils::TokenSetList> LRParser::FirstAndFollowSet(const Grammar::Grammar& grammar)
    {
        /* All Tokens & START & TERMINAL */
        const auto setSize = grammar.NumToken() + 2U;
        Utils::TokenSetList firstSet(setSize);
        Utils::TokenSetList followSet(setSize);

#pragma region FirstSet
        /* Terminal's first set only contains the terminal itself */
        for (Utils::TokenId token = 0U; token < setSize; ++token)
        {
            if (grammar.IsTerminalToken(token) || grammar.IsTerminal(token))
            {
                firstSet[token].insert(token);
            }
        }
        /* Loop until first sets no longer change */
        bool isDirty = true;
        while (isDirty)
        {
            isDirty = false;
            for (auto&& production : grammar.G())
            {
                assert(production.size() > 1U);
                size_t idx = 1U;
                size_t oldSize = firstSet[production[0]].size();
                bool hasEpsilon = true;
                while (idx < production.size())
                {
                    if (grammar.IsEpsilon(production[idx]))
                    {
                        /* Epsilon must be "E -> epsilon" */
                        assert(production.size() == 2U);
                        break;
                    }
                    bool keepGoing = false;
                    for (auto&& token : firstSet[production[idx]])
                    {
                        if (!grammar.IsEpsilon(token))
                        {
                            firstSet[production[0]].insert(token);
                        }
                        else
                        {
                            keepGoing = true;
                        }
                    }
                    /* Keep on if epsilon in the first set */
                    if (keepGoing)
                    {
                        ++idx;
                    }
                    /* Otherwise, stop right here */
                    else
                    {
                        hasEpsilon = false;
                        break;
                    }
                }
                if (hasEpsilon)
                {
                    firstSet[production[0]].insert(grammar.EPSILON());
                }
                isDirty |= oldSize != firstSet[production[0]].size();
            }
        }
#pragma endregion

#pragma region FollowSet
        /* First, $ is in START's Follow Set */
        followSet[grammar.START()].insert(grammar.TERMINAL());

        /* Loop until Follow Sets no longer change */
        isDirty = true;
        while (isDirty)
        {
            isDirty = false;
            for (auto&& production : grammar.G())
            {
                assert(production.size() > 1U);
                size_t idx = production.size() - 1U;
                while (idx > 0U)
                {
                    if (!grammar.IsNonTerminalToken(production[idx]))
                    {
                        break;
                    }
                    assert(grammar.IsNonTerminalToken(production[0]) || grammar.IsStart(production[0]));
                    size_t oldSize = followSet[production[idx]].size();
                    followSet[production[idx]].insert(followSet[production[0]].begin(), followSet[production[0]].end());
                    isDirty |= oldSize != followSet[production[idx]].size();
                    if (grammar.HasEpsilon() && firstSet[production[idx]].find(grammar.EPSILON()) != firstSet[production[idx]].end())
                    {
                        --idx;
                    }
                    else
                    {
                        break;
                    }
                }
                for (idx = 1; idx < production.size() - 1U; ++idx)
                {
                    if (grammar.IsNonTerminalToken(production[idx]))
                    {
                        size_t oldSize = followSet[production[idx]].size();
                        for (auto&& token : firstSet[production[idx + 1]])
                        {
                            if (!grammar.IsEpsilon(token))
                            {
                                followSet[production[idx]].insert(token);
                            }
                        }
                        isDirty |= oldSize != followSet[production[idx]].size();
                    }
                }
            }
        }
#pragma endregion
        return std::make_tuple(firstSet, followSet);
    }
    LRParser::LRParser(const Grammar::Grammar& grammar):m_flag(DFA_FLAG::DFA_LR1), m_FFSet(FirstAndFollowSet(grammar))
    {
        auto[firstSet, followSet] = m_FFSet;
        LRState initState;
        for (Utils::GrammarId gId = 0U; gId < static_cast<Utils::GrammarId>(grammar.G().size()); ++gId)
        {
            if (grammar.IsStart(grammar.G()[gId][0]))
            {
                LR1Item item(gId, 1U, { grammar.TERMINAL() });
                initState.Add(item);
            }
        }
        assert(!initState.Empty());
        initState.Closure(grammar, firstSet);

        m_states.push_back(initState);
        m_edges.emplace_back();
        std::map<LRState, Utils::StateId> visited;
        visited[initState] = 0U;
        std::queue<Utils::StateId> BFS;
        BFS.push(0U);
        while (!BFS.empty())
        {
            auto curStateId = BFS.front();
            BFS.pop();
            std::set<Utils::TokenId> possibleEdges;
            for (auto&& curItem : m_states[curStateId].Items())
            {
                if (curItem.item.dotPos < grammar.G()[curItem.item.grammarId].size() && !grammar.IsEpsilon(grammar.G()[curItem.item.grammarId][curItem.item.dotPos]))
                {
                    possibleEdges.insert(grammar.G()[curItem.item.grammarId][curItem.item.dotPos]);
                }
            }

            for (auto&& edge : possibleEdges)
            {
                LRState nextState;
                for (auto&& curItem : m_states[curStateId].Items())
                {
                    if (curItem.item.dotPos < grammar.G()[curItem.item.grammarId].size() && grammar.G()[curItem.item.grammarId][curItem.item.dotPos] == edge)
                    {
                        LR1Item nextItem(curItem.item.grammarId, curItem.item.dotPos + 1, curItem.lookAhead);
                        nextState.Add(nextItem);
                    }
                }
                nextState.Closure(grammar, firstSet);
                if (visited.find(nextState) != visited.end())
                {
                    m_edges[curStateId][edge] = visited[nextState];
                }
                else
                {
                    auto nStateId = static_cast<Utils::StateId>(m_states.size());
                    m_states.push_back(nextState);
                    m_edges.emplace_back();
                    BFS.push(nStateId);
                    visited[nextState] = nStateId;
                    m_edges[curStateId][edge] = nStateId;
                }
            }
        }
        m_BuildTransformTable(grammar);
    }
    void LRParser::Dump(const Grammar::Grammar & grammar)
    {
        std::ofstream ofs("graph.md");
        ofs << "```mermaid\n";
        ofs << "graph LR\n";
        Utils::StateId idx = 0U;
        for (auto&& state : m_states)
        {
            ofs << idx << "(\"";
            for (auto& item : state.Items())
            {
                bool isLeft = true;
                for (size_t tId = 0; tId < grammar.G()[item.item.grammarId].size(); ++tId)
                {
                    auto token = grammar.G()[item.item.grammarId][tId];
                    if (item.item.dotPos == static_cast<Utils::TokenId>(tId))
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
                if (item.item.dotPos == static_cast<Utils::TokenId>(grammar.G()[item.item.grammarId].size()))
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
        for (size_t sId = 0; sId < m_edges.size(); ++sId)
        {
            for (const auto& edge : m_edges[sId])
            {
                ofs << sId << "--" << grammar.GetTokenName(edge.first) << "-->" << edge.second << "\n";
            }
        }
        ofs << "```\n";
    }
    void LRParser::BeginParse(const Grammar::Grammar& grammar, const Utils::TokenStream& ts)
    {
        m_parseStack.clear();
        Element ele;
        ele.state.header.type = ElementType::State;
        ele.state.stateId = 0U;
        m_parseStack.push_back(ele);

        m_tokenStream = ts;
        if (m_tokenStream.back() != grammar.TERMINAL())
        {
            m_tokenStream.push_back(grammar.TERMINAL());
        }
    }
    bool LRParser::Step(const Grammar::Grammar& grammar)
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
    void LRParser::m_BuildTransformTable(const Grammar::Grammar& grammar)
    {
        m_table = TransformTable(m_states.size(), TransformCallBackList(grammar.NumToken() + 2U, nullptr));
        for (Utils::StateId sId = 0U; sId < static_cast<Utils::StateId>(m_states.size()); ++sId)
        {
            for (auto& edge : m_edges[sId])
            {
                if (grammar.IsTerminalToken(edge.first) || grammar.IsTerminal(edge.first))
                {
                    m_table[sId][edge.first] = std::bind(&LRParser::m_Shift, this, std::placeholders::_1, edge.second);
                }
                else if (grammar.IsNonTerminalToken(edge.first) || grammar.IsStart(edge.first))
                {
                    m_table[sId][edge.first] = std::bind(&LRParser::m_Goto, this, std::placeholders::_1, edge.second);
                }
            }
            for (auto& item : m_states[sId].Items())
            {
                if (item.item.dotPos == grammar.G()[item.item.grammarId].size())
                {
                    for (auto token : item.lookAhead)
                    {
                        if (m_table[sId][token] != nullptr)
                        {
                            std::cout << "[ERR] Not LR(1) language!!!\n";
                            m_table.clear();
                            return;
                        }
                        m_table[sId][token] = std::bind(&LRParser::m_Reduce, this, std::placeholders::_1, item.item.grammarId);
                    }
                }
            }
        }
    }
    bool LRParser::m_Shift(const Grammar::Grammar& grammar, Utils::StateId sId)
    {
        std::cout << "[PARSE] SHIFT " << grammar.GetTokenName(m_tokenStream.front()) << " to " << sId << "\n";
        auto token = m_tokenStream.front();
        m_tokenStream.pop_front();
        Element ele;
        ele.token.header.type = ElementType::Token;
        ele.token.tokenId = token;
        m_parseStack.push_back(ele);

        ele.state.header.type = ElementType::State;
        ele.state.stateId = sId;
        m_parseStack.push_back(ele);
        return true;
    }
    bool LRParser::m_Goto(const Grammar::Grammar& grammar, Utils::StateId sId)
    {
        std::cout << "[PARSE] GOTO " << sId << "\n";
        auto token = m_tokenStream.front();
        m_tokenStream.pop_front();
        Element ele;
        ele.token.header.type = ElementType::Token;
        ele.token.tokenId = token;
        m_parseStack.push_back(ele);

        ele.state.header.type = ElementType::State;
        ele.state.stateId = sId;
        m_parseStack.push_back(ele);
        return true;
    }
    bool LRParser::m_Reduce(const Grammar::Grammar& grammar, Utils::GrammarId gId)
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
}
