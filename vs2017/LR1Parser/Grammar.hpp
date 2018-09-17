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
    using StateId = unsigned int;
    using TokenId = unsigned int;
    struct ElementHeader
    {
        ElementType type;
    };
    struct ElementState
    {
        ElementHeader header;
        StateId stateId;
    };
    struct ElementToken
    {
        ElementHeader header;
        TokenId tokenId;
    };
    union Element
    {
        ElementHeader header;
        ElementState state;
        ElementToken token;
    };
    using TokenNameList = std::vector<std::string>;
    using Production = std::vector<TokenId>;
    using ProductionList = std::vector<Production>;

    class Grammar
    {
    public:
        explicit Grammar(const std::string& fname);
        TokenType GetTokenType(TokenId token) const
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
            if (token == 0U)
            {
                return TokenType::TT_START;
            }
            if (token == 1U)
            {
                return TokenType::TT_TERMINAL;
            }
            if (m_HasEmpty && token == 2U)
            {
                return TokenType::TT_EPSILON;
            }
            return TokenType::TT_NONE;
        }
        bool IsNone(TokenId token) const
        {
            return GetTokenType(token) == TokenType::TT_NONE;
        }
        bool IsTerminalToken(TokenId token) const
        {
            return GetTokenType(token) == TokenType::TT_TOKEN_TERM;
        }
        bool IsNonTerminalToken(TokenId token) const
        {
            return GetTokenType(token) == TokenType::TT_TOKEN_SYMB;
        }
        bool IsStart(TokenId token) const
        {
            return GetTokenType(token) == TokenType::TT_START;
        }
        bool IsTerminal(TokenId token) const
        {
            return GetTokenType(token) == TokenType::TT_TERMINAL;
        }
        bool IsEpsilon(TokenId token) const
        {
            return GetTokenType(token) == TokenType::TT_EPSILON;
        }

        bool HasEpsilon() const
        {
            return m_HasEmpty;
        }
        TokenId START() const
        {
            return NumToken();
        }
        TokenId TERMINAL() const
        {
            return NumToken() + 1U;
        }
        TokenId EPSILON() const
        {
            /*
                To check whether token is epsilon, use:
                if(HasEpsilon() && token == EPSILON()) { ... } as condition
            */
            assert(HasEpsilon());
            return NumToken() + 2U;
        }

        const TokenNameList& TerminalTokenValues() const
        {
            return m_TerminalTokenValues;
        }
        const ProductionList& G() const
        {
            return m_G;
        }
        const std::string& GetTokenName(TokenId token) const
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
        const TokenId NumTerminalToken() const
        {
            return static_cast<TokenId>(m_TerminalTokenNames.size());
        }
        const TokenId NumNonTerminalToken() const
        {
            return static_cast<TokenId>(m_NonTerminalTokenNames.size());
        }
        const TokenId NumToken() const
        {
            return NumTerminalToken() + NumNonTerminalToken();
        }
    private:
        const std::string m_NONE = "";
        const std::string m_START = "!";
        const std::string m_TERMINAL = "$";
        const std::string m_EPSILON = "@";
        bool m_HasEmpty;
        TokenNameList m_TerminalTokenNames;
        TokenNameList m_TerminalTokenValues;
        TokenNameList m_NonTerminalTokenNames;
        ProductionList m_G;
    };
}
