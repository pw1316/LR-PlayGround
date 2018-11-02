#include <stdafx.h>
#include "Grammar.hpp"
#include <Utils/Utils.hpp>

#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

namespace LR
{
    Grammar::Grammar(const std::string& fname)
    {
        std::ifstream fin(fname);
        if (!fin.is_open())
        {
            return;
        }

        std::set<std::string> terminals;
        std::map<std::string, std::string> terminal2regex;
        std::set<std::string> nonterminals;
        std::vector<std::vector<std::string>> grammar;

        int lineNo = 1;
        std::stringstream ss;
        std::string line;
        bool isToken = true;
        bool hasEmpty = false;
        while (std::getline(fin, line))
        {
            /* Comment */
            if (line.empty() || line[0] == '#')
            {
                // Nothing
            }
            else if (line == "%token%")
            {
                isToken = true;
            }
            else if (line == "%grammar%")
            {
                isToken = false;
            }
            /* Token */
            else if (isToken)
            {
                ss = std::stringstream();
                std::string token;
                std::string regex;
                ss << line;
                ss >> token;
                Utils::AbsorbWhiteSpace(ss);
                std::getline(ss, regex);
                if (regex.empty())
                {
                    std::cout << std::setw(6) << lineNo << "[WARN] Terminal cannot be empty. Ignore this line\n";
                }
                else if (terminals.find(token) != terminals.end())
                {
                    std::cout << std::setw(6) << lineNo << "[WARN] Duplication found. Ignore this line\n";
                }
                else
                {
                    terminals.insert(token);
                    terminal2regex[token] = regex;
                }
            }
            /* Grammar */
            else
            {
                ss = std::stringstream();
                std::string token;
                std::vector<std::string> g;
                ss << line;
                bool isLeft = true;
                while (ss >> token)
                {
                    if (isLeft && (terminals.find(token) != terminals.end() || token == m_EPSILON))
                    {
                        std::cout << std::setw(6) << lineNo << "[WARN] The symbol on the left side is a terminal(or epsilon). Ignore this grammar\n";
                        break;
                    }
                    else if (token == m_EPSILON)
                    {
                        if (g.size() == 1)
                        {
                            g.push_back(token);
                        }
                    }
                    else
                    {
                        if ((terminals.find(token) == terminals.end()) && (nonterminals.find(token) == nonterminals.end()))
                        {
                            nonterminals.insert(token);
                        }
                        if (!g.empty() && g.back() == m_EPSILON)
                        {
                            g.back() = token;
                        }
                        else
                        {
                            g.push_back(token);
                        }
                    }
                    isLeft = false;
                }
                if (!g.empty())
                {
                    grammar.push_back(std::move(g));
                }
            }
            ++lineNo;
        }

        Utils::TokenSet leftOnlySymbol;
        std::map<std::string, Utils::TokenId> auxT2I;
        Utils::TokenId auxI = 0;
        for (auto&& terminal : terminals)
        {
            m_TerminalTokenNames.push_back(terminal);
            m_TerminalTokenValues.push_back(terminal2regex[terminal]);
            auxT2I[terminal] = auxI++;
        }
        for (auto&& nonterminal : nonterminals)
        {
            m_NonTerminalTokenNames.push_back(nonterminal);
            leftOnlySymbol.insert(auxI);
            auxT2I[nonterminal] = auxI++;
        }

        for (auto&& g : grammar)
        {
            Utils::Production gg;
            bool isLeft = true;
            for (auto& smb : g)
            {
                Utils::TokenId token;
                if (auxT2I.find(smb) != auxT2I.end())
                {
                    token = auxT2I[smb];
                }
                else if (smb == m_EPSILON)
                {
                    /*
                        auxI     = START
                        auxI + 1 = TERMINAL
                        auxI + 2 = EPSILON
                    */
                    hasEmpty = true;
                    token = auxI + 2;
                }
                /* Should Never Happen Due To Previous Code */
                else
                {
                    assert(false);
                }
                gg.push_back(token);
                if (!isLeft && leftOnlySymbol.find(token) != leftOnlySymbol.end())
                {
                    leftOnlySymbol.erase(token);
                }
                isLeft = false;
            }
            m_G.push_back(std::move(gg));
        }
        /* No Leftmost, select the LHS of first rule */
        if (leftOnlySymbol.empty())
        {
            Utils::Production gg{ auxI, m_G[0][0] };
            m_G.push_back(std::move(gg));
        }
        /* Has Leftmost, select the leftmost */
        else
        {
            for (auto&& leftOnly : leftOnlySymbol)
            {
                Utils::Production gg{ auxI, leftOnly };
                m_G.push_back(std::move(gg));
            }
        }
        m_HasEmpty = hasEmpty;
    }

    Utils::TokenType Grammar::GetTokenType(Utils::TokenId token) const
    {
        if (token < NumTerminalToken())
        {
            return Utils::TokenType::TT_TOKEN_TERM;
        }
        if (token < NumToken())
        {
            return Utils::TokenType::TT_TOKEN_SYMB;
        }
        token -= NumToken();
        token += 3U;// Non-Token starts at 3
        if ((m_HasEmpty && token > 5U) || (!m_HasEmpty && token > 4U))
        {
            return Utils::TokenType::TT_NONE;
        }
        return static_cast<Utils::TokenType>(token);
    }
    const std::string& Grammar::GetTokenName(Utils::TokenId token) const
    {
        if (token < NumTerminalToken())
        {
            return m_TerminalTokenNames[token];
        }
        if (token < NumToken())
        {
            return m_NonTerminalTokenNames[token - NumTerminalToken()];
        }
        token -= NumToken();
        if (token == 0U)
        {
            return m_START;
        }
        if (token == 1U)
        {
            return m_TERMINAL;
        }
        if (m_HasEmpty && token == 2U)
        {
            return m_EPSILON;
        }
        return m_NONE;
    }

    void Grammar::Dump() const
    {
        Utils::GrammarId idx = 0U;
        for (auto&& production : m_G)
        {
            bool isLeft = true;
            std::cout << "[GRAMMAR] " << std::setw(2) << idx++ << " ";
            for (auto token : production)
            {
                std::cout << GetTokenName(token) << " ";
                if (isLeft)
                {
                    std::cout << "-> ";
                }
                isLeft = false;
            }
            std::cout << "\n";
        }
    }
}
