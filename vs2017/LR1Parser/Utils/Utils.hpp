#pragma once
//#include <iosfwd>

namespace LR::Utils
{
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
}
