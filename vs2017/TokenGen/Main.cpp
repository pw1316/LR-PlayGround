#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>

constexpr int TOKEN_IO_ERROR = 1;

static int indent = 0;

inline bool IsWhiteSpace(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == -1;
}

inline void AbsorbWhiteSpace(std::istream& is)
{
    while (true)
    {
        auto c = is.peek();
        if (IsWhiteSpace(c) && c != EOF)
        {
            is.get();
            continue;
        }
        break;
    }
}

static void BeginScope(std::ostream& os)
{
    os << std::setw(indent + 2) << "{\n";
    indent += 4;
}

static void EndScope(std::ostream& os)
{
    indent -= 4;
    os << std::setw(indent + 2) << "}\n";
}

static void BeginStructure(std::ostream& os)
{
    os << std::setw(indent + 2) << "{\n";
    indent += 4;
}

static void EndStructure(std::ostream& os)
{
    indent -= 4;
    os << std::setw(indent + 3) << "};\n";
}

static void BeginCustomLine(std::ostream& os, const std::string& name)
{
    for (int i = 0; i < indent; ++i)
    {
        os << " ";
    }
    os << name << "\n";
}

static void BeginNamespace(std::ostream& os, const std::string& name)
{
    os << std::setw(indent + 10) << "namespace " << name << "\n";
    BeginScope(os);
}

static void EndNamespace(std::ostream& os)
{
    EndScope(os);
}

int main(int argc, char* argv[])
{
    std::string fname = argc > 1 ? argv[1] : "token.txt";
    std::ifstream fin(fname);
    if (!fin.is_open())
    {
        return TOKEN_IO_ERROR;
    }

    std::set<std::string> terminals;
    std::map<std::string, std::string> terminal2regex;
    std::set<std::string> nonterminals;
    std::vector<std::vector<std::string>> grammar;

    int lineNo = 0;
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
            std::stringstream ss;
            std::string token;
            std::string regex;
            ss << line;
            ss >> token;
            AbsorbWhiteSpace(ss);
            std::getline(ss, regex);
            if (terminals.find(token) != terminals.end())
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
            std::stringstream ss;
            std::string token;
            std::vector<std::string> g;
            ss << line;
            bool isLeft = true;
            while (ss >> token)
            {
                if (isLeft && (terminals.find(token) != terminals.end() || token == "@"))
                {
                    std::cout << std::setw(6) << lineNo << "[WARN] The symbol on the left side is a terminal(or epsilon). Ignore this grammar\n";
                }
                else if (token == "@")
                {
                    hasEmpty = true;
                    g.push_back("");
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

    std::ofstream fout("Token.hpp");
    fout << "#pragma once\n";

    BeginNamespace(fout, "LR::Token");

    BeginCustomLine(fout, "enum class SymbolType");
    BeginStructure(fout);
    BeginCustomLine(fout, "NONE,");
    if (hasEmpty)
    {
        BeginCustomLine(fout, "EPSILON,");
    }
    for (auto terminal : terminals)
    {
        std::string s("TERM_");
        BeginCustomLine(fout, s + terminal + ",");
    }
    for (auto nonterminal : nonterminals)
    {
        std::string s("SYMB_");
        BeginCustomLine(fout, s + nonterminal + ",");
    }
    BeginCustomLine(fout, "START,");
    BeginCustomLine(fout, "TERMINAL");
    EndStructure(fout);

    BeginCustomLine(fout, "enum class ElementType");
    BeginStructure(fout);
    BeginCustomLine(fout, "State,");
    BeginCustomLine(fout, "Symbol");
    EndStructure(fout);

    BeginCustomLine(fout, "struct ElementHeader");
    BeginStructure(fout);
    BeginCustomLine(fout, "ElementType type;");
    EndStructure(fout);

    BeginCustomLine(fout, "struct ElementState");
    BeginStructure(fout);
    BeginCustomLine(fout, "ElementHeader header;");
    BeginCustomLine(fout, "int id;");
    EndStructure(fout);

    BeginCustomLine(fout, "struct ElementSymbol");
    BeginStructure(fout);
    BeginCustomLine(fout, "ElementHeader header;");
    BeginCustomLine(fout, "SymbolType type;");
    BeginCustomLine(fout, "int value;");
    EndStructure(fout);

    BeginCustomLine(fout, "union Element");
    BeginStructure(fout);
    BeginCustomLine(fout, "ElementHeader header;");
    BeginCustomLine(fout, "ElementState state;");
    BeginCustomLine(fout, "ElementSymbol symbol;");
    EndStructure(fout);

    EndNamespace(fout);
    fout.close();
}
