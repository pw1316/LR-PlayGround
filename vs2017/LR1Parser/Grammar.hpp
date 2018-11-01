#pragma once
#include <stdafx.h>

namespace LR::Grammar
{
    class Grammar
    {
    public:
        explicit Grammar(const std::string& fname);

        const Utils::TokenId NumTerminalToken() const
        {
            return static_cast<Utils::TokenId>(m_TerminalTokenNames.size());
        }
        const Utils::TokenId NumNonTerminalToken() const
        {
            return static_cast<Utils::TokenId>(m_NonTerminalTokenNames.size());
        }
        const Utils::TokenId NumToken() const
        {
            return NumTerminalToken() + NumNonTerminalToken();
        }

        Utils::TokenType GetTokenType(Utils::TokenId token) const;
        const std::string& GetTokenName(Utils::TokenId token) const;
        bool IsNone(Utils::TokenId token) const
        {
            return GetTokenType(token) == Utils::TokenType::TT_NONE;
        }
        bool IsTerminalToken(Utils::TokenId token) const
        {
            return GetTokenType(token) == Utils::TokenType::TT_TOKEN_TERM;
        }
        bool IsNonTerminalToken(Utils::TokenId token) const
        {
            return GetTokenType(token) == Utils::TokenType::TT_TOKEN_SYMB;
        }
        bool IsStart(Utils::TokenId token) const
        {
            return GetTokenType(token) == Utils::TokenType::TT_START;
        }
        bool IsTerminal(Utils::TokenId token) const
        {
            return GetTokenType(token) == Utils::TokenType::TT_TERMINAL;
        }
        bool IsEpsilon(Utils::TokenId token) const
        {
            return GetTokenType(token) == Utils::TokenType::TT_EPSILON;
        }

        bool HasEpsilon() const
        {
            return m_HasEmpty;
        }
        Utils::TokenId START() const
        {
            return NumToken();
        }
        Utils::TokenId TERMINAL() const
        {
            return NumToken() + 1U;
        }
        Utils::TokenId EPSILON() const
        {
            /*
                To check whether token is epsilon, use:
                if(HasEpsilon() && token == EPSILON()) { ... } as condition
            */
            assert(HasEpsilon());
            return NumToken() + 2U;
        }

        const Utils::TokenNameList& TerminalTokenValues() const
        {
            return m_TerminalTokenValues;
        }
        const Utils::ProductionList& G() const
        {
            return m_G;
        }

        void DumpGrammar() const;
    private:
        const std::string m_NONE = "";
        const std::string m_START = "!";
        const std::string m_TERMINAL = "$";
        const std::string m_EPSILON = "@";
        bool m_HasEmpty;
        Utils::TokenNameList m_TerminalTokenNames;
        Utils::TokenNameList m_TerminalTokenValues;
        Utils::TokenNameList m_NonTerminalTokenNames;
        Utils::ProductionList m_G;
    };
}
