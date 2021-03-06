#pragma once
#include <list>
#include <set>
#include <string>
#include <vector>

namespace LR::Utils
{
    /* Token */
    using TokenId = unsigned int;
    enum class TokenType
    {
        TT_NONE,
        TT_TOKEN_TERM,
        TT_TOKEN_SYMB,
        TT_START,
        TT_TERMINAL,
        TT_EPSILON,
        TT_TOTAL,
    };
    struct Token
    {
        Token(TokenId tId, const std::string& tValue) : id(tId), value(tValue) {}
        TokenId id;
        std::string value;
    };
    using TokenNameList = std::vector<std::string>;
    using TokenStream = std::list<Token>;
    using TokenSet = std::set<TokenId>;
    using TokenSetList = std::vector<TokenSet>;

    using StateId = unsigned int;

    /* Grammar */
    using GrammarId = unsigned int;
    using Production = std::vector<TokenId>;
    using ProductionList = std::vector<Production>;
}
