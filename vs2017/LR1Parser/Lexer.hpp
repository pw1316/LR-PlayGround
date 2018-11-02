#pragma once
#include <stdafx.h>

namespace LR
{
    class Grammar;

    class Lexer
    {
    public:
        Lexer(const Grammar& g) :m_g(g) {}
        bool SetInput(const std::string& input);
        const Utils::TokenStream& TokenStream() const
        {
            return m_ts;
        }
        void Dump();
    private:
        const Grammar& m_g;
        Utils::TokenStream m_ts;
    };
}
