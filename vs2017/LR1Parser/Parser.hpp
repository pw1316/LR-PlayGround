#pragma once
#include <stdafx.h>

#include <functional>
#include <map>

namespace LR
{
    class Grammar;

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
        LR1Item(Utils::GrammarId gId, Utils::TokenId dPos, const Utils::TokenSet& lh) : item(gId, dPos), lookAhead(lh) {}
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
        void Closure(const Grammar& grammar, const Utils::TokenSetList& firstSet);
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
        void MergeLookAhead();
        std::set<LR1Item> m_closure;
    };
    using StateList = std::vector<LRState>;

    enum class DFA_FLAG
    {
        DFA_LR0,
        DFA_SLR1,
        DFA_LALR1,
        DFA_LR1,
        DFA_HIGHER
    };
    using Edge = std::map<Utils::TokenId, Utils::StateId>;
    using EdgeList = std::vector<Edge>;
    using TransformCallBack = std::function<bool()>;
    using TransformCallBackList = std::vector<TransformCallBack>;
    using TransformTable = std::vector<TransformCallBackList>;

    enum class ElementType
    {
        State,
        Token
    };
    struct Element
    {
        Element() :type(ElementType::State), sId(0U), token(0U, "") {}
        ElementType type;
        Utils::StateId sId;
        Utils::Token token;
    };
    using ParseStack = std::list<Element>;

    class LRParser
    {
    public:
        LRParser(const Grammar& grammar);
        void Dump();
        bool Valid()
        {
            return m_flag < DFA_FLAG::DFA_HIGHER;
        }
        LRParser DeGenerate();
        void BeginParse(const Utils::TokenStream& ts);
        bool Step();
    private:
        std::tuple<Utils::TokenSetList, Utils::TokenSetList> FirstAndFollowSet();
        void m_BuildDFA();
        void m_BuildTransformTable();
        bool m_Shift(Utils::StateId sId);
        bool m_Goto(Utils::StateId sId);
        bool m_Reduce(Utils::GrammarId gId);

        const Grammar& m_grammar;
        DFA_FLAG m_flag;
        std::tuple<Utils::TokenSetList, Utils::TokenSetList> m_FFSet;
        StateList m_states;
        EdgeList m_edges;
        TransformTable m_table;
        Utils::TokenStream m_tokenStream;
        ParseStack m_parseStack;
    };
}
