#pragma once
#include <stdafx.h>

#include <functional>
#include <map>

namespace LR::Grammar
{
    class Grammar;
}

namespace LR::Parser
{
    struct LR0Item
    {
        LR0Item() : grammarId(0U), dotPos(1U) {}
        LR0Item(Utils::GrammarId gId, Utils::TokenId dPos) : grammarId(gId), dotPos(dPos) {}
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
        Utils::GrammarId grammarId;
        Utils::TokenId dotPos;
    };

    struct LR1Item
    {
        LR1Item() {}
        LR1Item(Utils::GrammarId gId, Utils::TokenId dPos, const std::initializer_list<Utils::TokenId>& lh) : item(gId, dPos), lookAhead(lh) {}
        LR1Item(Utils::GrammarId gId, Utils::TokenId dPos, const std::set<Utils::TokenId>& lh) : item(gId, dPos), lookAhead(lh) {}
        bool operator==(const LR1Item& rhs) const
        {
            return item == rhs.item && lookAhead == rhs.lookAhead;
        }
        bool operator<(const LR1Item& rhs) const
        {
            if (item < rhs.item)
            {
                return true;
            }
            if (item > rhs.item)
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
        LR0Item item;
        Utils::TokenSet lookAhead;
    };

    class LRState
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
        void Closure(const Grammar::Grammar& grammar, const Utils::TokenSetList& firstSet);
        void MergeLookAhead();
        const std::set<LR1Item>& Items() const
        {
            return m_closure;
        }
        bool operator==(const LRState& rhs) const
        {
            return m_closure == rhs.m_closure;
        }
        bool operator<(const LRState& rhs) const
        {
            return m_closure < rhs.m_closure;
        }
        bool operator>(const LRState& rhs) const
        {
            return m_closure > rhs.m_closure;
        }
    private:
        std::set<LR1Item> m_closure;
    };
    using StateList = std::vector<LRState>;

    enum class DFA_FLAG
    {
        DFA_LR0,
        DFA_SLR1,
        DFA_LALR1,
        DFA_LR1,
    };
    using Edge = std::map<Utils::TokenId, Utils::StateId>;
    using EdgeList = std::vector<Edge>;
    using TransformCallBack = std::function<bool(const Grammar::Grammar&)>;
    using TransformCallBackList = std::vector<TransformCallBack>;
    using TransformTable = std::vector<TransformCallBackList>;

    enum class ElementType
    {
        State,
        Token
    };
    struct ElementHeader
    {
        ElementType type;
    };
    struct ElementState
    {
        ElementHeader header;
        Utils::StateId stateId;
    };
    struct ElementToken
    {
        ElementHeader header;
        Utils::TokenId tokenId;
    };
    union Element
    {
        ElementHeader header;
        ElementState state;
        ElementToken token;
    };
    using ParseStack = std::list<Element>;

    class LRParser
    {
    public:
        static std::tuple<Utils::TokenSetList, Utils::TokenSetList> FirstAndFollowSet(const Grammar::Grammar& grammar);
        LRParser(const Grammar::Grammar& grammar);
        void Dump(const Grammar::Grammar& grammar);
        LRParser DeGenerate(const Grammar::Grammar& grammar);
        void BeginParse(const Grammar::Grammar& grammar, const Utils::TokenStream& ts);
        bool Step(const Grammar::Grammar& grammar);
    private:
        void m_BuildTransformTable(const Grammar::Grammar& grammar);
        bool m_Shift(const Grammar::Grammar& grammar, Utils::StateId sId);
        bool m_Goto(const Grammar::Grammar& grammar, Utils::StateId sId);
        bool m_Reduce(const Grammar::Grammar& grammar, Utils::GrammarId gId);

        DFA_FLAG m_flag;
        std::tuple<Utils::TokenSetList, Utils::TokenSetList> m_FFSet;
        StateList m_states;
        EdgeList m_edges;
        TransformTable m_table;
        Utils::TokenStream m_tokenStream;
        ParseStack m_parseStack;
    };
}
