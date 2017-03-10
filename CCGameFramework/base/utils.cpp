#include "stdafx.h"
#include "utils.h"

template<typename Out>
void split(const std::string &s, char delim, Out result)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        *(result++) = item;
    }
}

namespace std
{
    std::vector<std::string> split(const std::string &s, char delim)
    {
        std::vector<std::string> elems;
        ::split(s, delim, std::back_inserter(elems));
        return elems;
    }
}