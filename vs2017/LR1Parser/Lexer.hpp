#pragma once
#include <stdafx.h>

namespace LR::Grammar
{
    class Grammar;
}

namespace LR::Lexer
{
    class Lexer
    {
    public:
        static Utils::TokenStream Lex(const Grammar::Grammar& g, const std::string& input);
        static void DumpTokenStream(const Grammar::Grammar& g, const Utils::TokenStream& ts);
    };
}
