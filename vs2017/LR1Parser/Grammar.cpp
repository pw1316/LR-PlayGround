#include "Grammar.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <Utils/Utils.hpp>

LR::Grammar::Grammar::Grammar(const std::string& fname)
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
                if (isLeft && (terminals.find(token) != terminals.end() || token == "@"))
                {
                    std::cout << std::setw(6) << lineNo << "[WARN] The symbol on the left side is a terminal(or epsilon). Ignore this grammar\n";
                    break;
                }
                else if (token == "@")
                {
                    hasEmpty = true;
                    g.push_back(token);
                }
                else
                {
                    if ((terminals.find(token) == terminals.end()) && (nonterminals.find(token) == nonterminals.end()))
                    {
                        nonterminals.insert(token);
                    }
                    g.push_back(token);
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

    m_HasEmpty = hasEmpty;

    std::set<unsigned int> leftOnlySymbol;
    std::map<std::string, unsigned int> auxT2I;
    unsigned int auxI = 0;
    for (auto terminal : terminals)
    {
        m_TerminalTokenNames.push_back(terminal);
        m_TerminalTokenValues.push_back(terminal2regex[terminal]);
        auxT2I[terminal] = auxI++;
    }
    for (auto nonterminal : nonterminals)
    {
        m_NonTerminalTokenNames.push_back(nonterminal);
        leftOnlySymbol.insert(auxI);
        auxT2I[nonterminal] = auxI++;
    }

    for (auto& g : grammar)
    {
        std::vector<unsigned int> gg;
        bool isLeft = true;
        for (auto& smb : g)
        {
            unsigned int token;
            if (auxT2I.find(smb) != auxT2I.end())
            {
                token = auxT2I[smb];
            }
            else if (smb == "@")
            {
                /*
                    auxI     = START
                    auxI + 1 = TERMINAL
                    auxI + 2 = EPSILON
                */
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
    for (auto &leftOnly : leftOnlySymbol)
    {
        std::vector<unsigned int> gg{ auxI, leftOnly };
        m_G.push_back(std::move(gg));
    }
}
