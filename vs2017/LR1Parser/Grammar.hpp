#pragma once
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
        explicit Grammar(const std::string& fname);
        TokenType GetTokenType(unsigned int token)
        {
            const auto TN = static_cast<unsigned int>(m_TerminalTokenNames.size());
            const auto NTN = static_cast<unsigned int>(m_NonTerminalTokenNames.size());
            if (token < TN)
            {
                return TokenType::TT_TOKEN_TERM;
            }
            if (token < TN + NTN)
            {
                return TokenType::TT_TOKEN_SYMB;
            }
            token -= TN + NTN;
            if (token == 0)
            {
                return TokenType::TT_START;
            }
            if (token == 1)
            {
                return TokenType::TT_TERMINAL;
            }
            if (token == 2 && m_HasEmpty)
            {
                return TokenType::TT_EPSILON;
            }
            return TokenType::TT_NONE;
        }
        bool IsNone(unsigned int token)
        {
            return GetTokenType(token) == TokenType::TT_NONE;
        }
        bool IsTerminalToken(unsigned int token)
        {
            return GetTokenType(token) == TokenType::TT_TOKEN_TERM;
        }
        bool IsNonTerminalToken(unsigned int token)
        {
            return GetTokenType(token) == TokenType::TT_TOKEN_SYMB;
        }
        bool IsStart(unsigned int token)
        {
            return GetTokenType(token) == TokenType::TT_START;
        }
        bool IsTerminal(unsigned int token)
        {
            return GetTokenType(token) == TokenType::TT_TERMINAL;
        }
        bool IsEpsilon(unsigned int token)
        {
            return GetTokenType(token) == TokenType::TT_EPSILON;
        }
        const std::vector<std::string>& TerminalTokenNames() const
        {
            return m_TerminalTokenNames;
        }
        const std::vector<std::string>& TerminalTokenValues() const
        {
            return m_TerminalTokenValues;
        }
        const std::vector<std::string>& NonTerminalTokenNames() const
        {
            return m_NonTerminalTokenNames;
        }
        const std::vector<std::vector<unsigned int>>& G() const
        {
            return m_G;
        }
    private:
        bool m_HasEmpty;
        std::vector<std::string> m_TerminalTokenNames;
        std::vector<std::string> m_TerminalTokenValues;
        std::vector<std::string> m_NonTerminalTokenNames;
        std::vector<std::vector<unsigned int>> m_G;
    };
}
