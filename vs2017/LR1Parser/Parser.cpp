#include <stdafx.h>
#include "Parser.hpp"
#include <Grammar.hpp>

#include <fstream>
#include <queue>
#include <string>

namespace LR
{
    void LRState::Closure(const Grammar& grammar, const Utils::TokenSetList& firstSet)
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
    void LRState::MergeLookAhead()
    {
        std::set<LR1Item> nclosure;
        for (auto&& item : m_closure)
        {
            bool found = false;
            for (auto&& nitem : nclosure)
            {
                if (nitem.item == item.item)
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

    LRParser::LRParser(const Grammar& grammar) :m_grammar(grammar), m_flag(DFA_FLAG::DFA_LR1), m_FFSet(FirstAndFollowSet())
    {
        m_BuildDFA();
        m_BuildTransformTable();
    }
    void LRParser::Dump()
    {
        auto[firstSet, followSet] = m_FFSet;
        std::cout << "First/Follow Set:\n";
        for (LR::Utils::TokenId i = 0U; i < static_cast<LR::Utils::TokenId>(m_grammar.NumToken()) + 2U; ++i)
        {
            std::cout << "  " << m_grammar.GetTokenName(i) << ": {";
            for (auto token : firstSet[i])
            {
                std::cout << m_grammar.GetTokenName(token) << ",";
            }
            std::cout << "} {";
            for (auto token : followSet[i])
            {
                std::cout << m_grammar.GetTokenName(token) << ",";
            }
            std::cout << "}\n";
        }
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
                for (size_t tId = 0; tId < m_grammar.G()[item.item.grammarId].size(); ++tId)
                {
                    auto token = m_grammar.G()[item.item.grammarId][tId];
                    if (item.item.dotPos == static_cast<Utils::TokenId>(tId))
                    {
                        ofs << ". ";
                    }
                    ofs << m_grammar.GetTokenName(token) << " ";
                    if (isLeft)
                    {
                        ofs << "-> ";
                    }
                    isLeft = false;
                }
                if (item.item.dotPos == static_cast<Utils::TokenId>(m_grammar.G()[item.item.grammarId].size()))
                {
                    ofs << ". ";
                }
                ofs << "{";
                for (auto lah : item.lookAhead)
                {
                    ofs << m_grammar.GetTokenName(lah) << ",";
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
                ofs << sId << "--\"" << m_grammar.GetTokenName(edge.first) << "\"-->" << edge.second << "\n";
            }
        }
        ofs << "```\n";
    }
    LRParser LRParser::DeGenerate()
    {
        return *this;
    }
    void LRParser::BeginParse(const Utils::TokenStream& ts)
    {
        m_parseStack.clear();
        Element ele;
        ele.type = ElementType::State;
        ele.sId = 0U;
        m_parseStack.push_back(ele);

        m_tokenStream = ts;
        if (m_tokenStream.back().id != m_grammar.TERMINAL())
        {
            auto tId = m_grammar.TERMINAL();
            m_tokenStream.emplace_back(tId, m_grammar.GetTokenName(tId));
        }
    }
    bool LRParser::Step()
    {
        if (m_tokenStream.empty())
        {
            return false;
        }
        if (m_tokenStream.front().id == m_grammar.START())
        {
            std::cout << "[PARSE] DONE.\n";
            return false;
        }
        auto token = m_tokenStream.front();
        m_table[m_parseStack.back().sId][token.id]();
        return true;
    }
    std::tuple<Utils::TokenSetList, Utils::TokenSetList> LRParser::FirstAndFollowSet()
    {
        /* All Tokens & START & TERMINAL */
        const auto setSize = m_grammar.NumToken() + 2U;
        Utils::TokenSetList firstSet(setSize);
        Utils::TokenSetList followSet(setSize);

#pragma region FirstSet
        /* Terminal's first set only contains the terminal itself */
        for (Utils::TokenId token = 0U; token < setSize; ++token)
        {
            if (m_grammar.IsTerminalToken(token) || m_grammar.IsTerminal(token))
            {
                firstSet[token].insert(token);
            }
        }
        /* Loop until first sets no longer change */
        bool isDirty = true;
        while (isDirty)
        {
            isDirty = false;
            for (auto&& production : m_grammar.G())
            {
                assert(production.size() > 1U);
                size_t idx = 1U;
                size_t oldSize = firstSet[production[0]].size();
                bool hasEpsilon = true;
                while (idx < production.size())
                {
                    if (m_grammar.IsEpsilon(production[idx]))
                    {
                        /* Epsilon must be "E -> epsilon" */
                        assert(production.size() == 2U);
                        break;
                    }
                    bool keepGoing = false;
                    for (auto&& token : firstSet[production[idx]])
                    {
                        if (!m_grammar.IsEpsilon(token))
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
                    firstSet[production[0]].insert(m_grammar.EPSILON());
                }
                isDirty |= oldSize != firstSet[production[0]].size();
            }
        }
#pragma endregion

#pragma region FollowSet
        /* First, $ is in START's Follow Set */
        followSet[m_grammar.START()].insert(m_grammar.TERMINAL());

        /* Loop until Follow Sets no longer change */
        isDirty = true;
        while (isDirty)
        {
            isDirty = false;
            for (auto&& production : m_grammar.G())
            {
                assert(production.size() > 1U);
                size_t idx = production.size() - 1U;
                while (idx > 0U)
                {
                    if (!m_grammar.IsNonTerminalToken(production[idx]))
                    {
                        break;
                    }
                    assert(m_grammar.IsNonTerminalToken(production[0]) || m_grammar.IsStart(production[0]));
                    size_t oldSize = followSet[production[idx]].size();
                    followSet[production[idx]].insert(followSet[production[0]].begin(), followSet[production[0]].end());
                    isDirty |= oldSize != followSet[production[idx]].size();
                    if (m_grammar.HasEpsilon() && firstSet[production[idx]].find(m_grammar.EPSILON()) != firstSet[production[idx]].end())
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
                    if (m_grammar.IsNonTerminalToken(production[idx]))
                    {
                        size_t oldSize = followSet[production[idx]].size();
                        for (auto&& token : firstSet[production[idx + 1]])
                        {
                            if (!m_grammar.IsEpsilon(token))
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
    void LRParser::m_BuildDFA()
    {
        m_states.clear();
        m_edges.clear();
        auto[firstSet, followSet] = m_FFSet;
        LRState initState;
        for (Utils::GrammarId gId = 0U; gId < static_cast<Utils::GrammarId>(m_grammar.G().size()); ++gId)
        {
            if (m_grammar.IsStart(m_grammar.G()[gId][0]))
            {
                LR1Item item(gId, 1U, { m_grammar.TERMINAL() });
                initState.Add(item);
            }
        }
        assert(!initState.Empty());
        initState.Closure(m_grammar, firstSet);

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
                if (curItem.item.dotPos < m_grammar.G()[curItem.item.grammarId].size() && !m_grammar.IsEpsilon(m_grammar.G()[curItem.item.grammarId][curItem.item.dotPos]))
                {
                    possibleEdges.insert(m_grammar.G()[curItem.item.grammarId][curItem.item.dotPos]);
                }
            }

            for (auto&& edge : possibleEdges)
            {
                LRState nextState;
                for (auto&& curItem : m_states[curStateId].Items())
                {
                    if (curItem.item.dotPos < m_grammar.G()[curItem.item.grammarId].size() && m_grammar.G()[curItem.item.grammarId][curItem.item.dotPos] == edge)
                    {
                        LR1Item nextItem(curItem.item.grammarId, curItem.item.dotPos + 1, curItem.lookAhead);
                        nextState.Add(nextItem);
                    }
                }
                nextState.Closure(m_grammar, firstSet);
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
    }
    void LRParser::m_BuildTransformTable()
    {
        m_table = TransformTable(m_states.size(), TransformCallBackList(m_grammar.NumToken() + 2U, nullptr));
        for (Utils::StateId sId = 0U; sId < static_cast<Utils::StateId>(m_states.size()); ++sId)
        {
            for (auto& edge : m_edges[sId])
            {
                if (m_grammar.IsTerminalToken(edge.first) || m_grammar.IsTerminal(edge.first))
                {
                    m_table[sId][edge.first] = std::bind(&LRParser::m_Shift, this, edge.second);
                }
                else if (m_grammar.IsNonTerminalToken(edge.first) || m_grammar.IsStart(edge.first))
                {
                    m_table[sId][edge.first] = std::bind(&LRParser::m_Goto, this, edge.second);
                }
            }
            for (auto& item : m_states[sId].Items())
            {
                if (item.item.dotPos == m_grammar.G()[item.item.grammarId].size())
                {
                    for (auto token : item.lookAhead)
                    {
                        if (m_table[sId][token] != nullptr)
                        {
                            std::cout << "[ERR] Not LR(1) language!!!\n";
                            m_table.clear();
                            return;
                        }
                        m_table[sId][token] = std::bind(&LRParser::m_Reduce, this, item.item.grammarId);
                    }
                }
            }
        }
    }
    bool LRParser::m_Shift(Utils::StateId sId)
    {
        std::cout << "[PARSE] SHIFT " << m_tokenStream.front().value << " to state " << sId << "\n";
        auto token = m_tokenStream.front();
        m_tokenStream.pop_front();
        Element ele;
        ele.type = ElementType::Token;
        ele.token = token;
        m_parseStack.push_back(ele);

        ele.type = ElementType::State;
        ele.sId = sId;
        m_parseStack.push_back(ele);
        return true;
    }
    bool LRParser::m_Goto(Utils::StateId sId)
    {
        std::cout << "[PARSE] GOTO state " << sId << "\n";
        auto token = m_tokenStream.front();
        m_tokenStream.pop_front();
        Element ele;
        ele.type = ElementType::Token;
        ele.token = token;
        m_parseStack.push_back(ele);

        ele.type = ElementType::State;
        ele.sId = sId;
        m_parseStack.push_back(ele);
        return true;
    }
    bool LRParser::m_Reduce(Utils::GrammarId gId)
    {
        std::cout << "[PARSE] REDUCE ";
        {
            bool isLeft = true;
            for (auto token : m_grammar.G()[gId])
            {
                std::cout << m_grammar.GetTokenName(token) << " ";
                if (isLeft)
                {
                    std::cout << "-> ";
                }
                isLeft = false;
            }
            std::cout << "\n";
        }
        auto &g = m_grammar.G()[gId];
        for (size_t i = 1; i < g.size(); ++i)
        {
            m_parseStack.pop_back();
            m_parseStack.pop_back();
        }

        m_tokenStream.emplace_front(g[0], m_grammar.GetTokenName(g[0]));
        return true;
    }
}
