#pragma once

#include <string>
#include <vector>

inline std::vector<std::string> split_string(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while((pos = str.find(delimiter, prev)) != std::string::npos) {
        strings.push_back(str.substr(prev, pos - prev));
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}

inline std::string trim_spaces(std::string str)
{
    auto first_non_space = str.find_first_not_of(" \n");
    if(first_non_space != std::string::npos)
        str = str.substr(first_non_space);
    auto last_non_space = str.find_last_not_of(" \n");
    if(last_non_space != std::string::npos)
        str = str.substr(0, last_non_space + 1);
    return str;
}