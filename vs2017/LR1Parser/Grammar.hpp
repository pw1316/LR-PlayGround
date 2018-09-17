#pragma once
#include <stdafx.h>

#include <string>
#include <vector>

namespace LR::Grammar
{
    enum class ElementType
    {
        State,
        Token
    };
    enum class TokenType
    {
        TT_NONE,
        TT_TOKEN_TERM,
        TT_TOKEN_SYMB,
        TT_START,
        TT_TERMINAL,
        TT_EPSILON
    };
    struct ElementHeader
    {
        ElementType type;
    };
    struct ElementState
    {
        ElementHeader header;
        unsigned int stateId;
    };
    struct ElementToken
    {
        ElementHeader header;
        unsigned int tokenId;
    };
    union Element
    {
        ElementHeader header;
        ElementState state;
        ElementToken token;
    };

    class Grammar
    {
    public:
        using NameList = std::vector<std::string>;
        using Production = std::vector<unsigned int>;
        using ProductionList = std::vector<Production>;

        explicit Grammar(const std::string& fname);
        TokenType GetTokenType(unsigned int token) const
        {
            if (token < NumTerminalToken())
            {
                return TokenType::TT_TOKEN_TERM;
            }
            if (token < NumToken())
            {
                return TokenType::TT_TOKEN_SYMB;
            }
            token -= NumToken();
            if (token == 0)
            {
                return TokenType::TT_START;
            }
            if (token == 1)
            {
                return TokenType::TT_TERMINAL;
            }
            if (m_HasEmpty && token == 2)
            {
                return TokenType::TT_EPSILON;
            }
            return TokenType::TT_NONE;
        }
        bool IsNone(unsigned int token) const
        {
            return GetTokenType(token) == TokenType::TT_NONE;
        }
        bool IsTerminalToken(unsigned int token) const
        {
            return GetTokenType(token) == TokenType::TT_TOKEN_TERM;
        }
        bool IsNonTerminalToken(unsigned int token) const
        {
            return GetTokenType(token) == TokenType::TT_TOKEN_SYMB;
        }
        bool IsStart(unsigned int token) const
        {
            return GetTokenType(token) == TokenType::TT_START;
        }
        bool IsTerminal(unsigned int token) const
        {
            return GetTokenType(token) == TokenType::TT_TERMINAL;
        }
        bool IsEpsilon(unsigned int token) const
        {
            return GetTokenType(token) == TokenType::TT_EPSILON;
        }

        bool HasEpsilon() const
        {
            return m_HasEmpty;
        }
        unsigned int START() const
        {
            return NumToken();
        }
        unsigned int TERMINAL() const
        {
            return NumToken() + 1U;
        }
        unsigned int EPSILON() const
        {
            /*
                To check whether token is epsilon, use:
                if(HasEpsilon() && token == EPSILON()) { ... } as condition
            */
            assert(HasEpsilon());
            return NumToken() + 2U;
        }

        const NameList& TerminalTokenValues() const
        {
            return m_TerminalTokenValues;
        }
        const ProductionList& G() const
        {
            return m_G;
        }
        const std::string& GetTokenName(unsigned int token) const
        {
            if (IsTerminalToken(token))
            {
                return m_TerminalTokenNames[token];
            }
            if (IsNonTerminalToken(token))
            {
                return m_NonTerminalTokenNames[token - NumTerminalToken()];
            }
            if (IsStart(token))
            {
                return m_START;
            }
            if (IsTerminal(token))
            {
                return m_TERMINAL;
            }
            if (IsEpsilon(token))
            {
                return m_EPSILON;
            }
            return m_NONE;
        }
        const unsigned int NumTerminalToken() const
        {
            return static_cast<unsigned int>(m_TerminalTokenNames.size());
        }
        const unsigned int NumNonTerminalToken() const
        {
            return static_cast<unsigned int>(m_NonTerminalTokenNames.size());
        }
        const unsigned int NumToken() const
        {
            return NumTerminalToken() + NumNonTerminalToken();
        }
    private:
        const std::string m_NONE = "";
        const std::string m_START = "!";
        const std::string m_TERMINAL = "$";
        const std::string m_EPSILON = "@";
        bool m_HasEmpty;
        NameList m_TerminalTokenNames;
        NameList m_TerminalTokenValues;
        NameList m_NonTerminalTokenNames;
        ProductionList m_G;
    };
}
