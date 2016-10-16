#pragma once

#include <string>
#include <map>

#include "helpers.hpp"
#include "windbg_field.hpp"

//
// Known WinDbg Data Types
//
const std::map<std::string, std::string> known_types = {
    {"void", "VOID"},
    {"Void", "VOID"},
    {"Char", "CHAR"},
    {"Int2B", "SHORT"},
    {"Int4B", "LONG"},
    {"Int8B", "LONGLONG"},
    {"UChar", "UCHAR"},
    {"Uint2B", "USHORT"},
    {"Uint4B", "ULONG"},
    {"Uint8B", "ULONGLONG"}
};

enum field_types
{
    regular_field,
    array_field,
    bitfield,
};

class windbg_structure
{
public:
    windbg_structure(const std::string& text);

    static bool is_header(const std::string& line);

    template<typename Iter>
    static bool is_union_or_bitfield(Iter it)
    {
        try {
            return parse_field_offset(*it) == parse_field_offset(*(it + 1));
        } catch(const std::out_of_range&) { //end iterator throws when you try to increment past it
            return false;
        }
    }

    const std::string& get_name() const
    {
        return _name;
    }

    std::string as_string(int tabcount = 0) const;

private:
    static std::string parse_field_name(const std::string& line)
    {
        auto name_start = line.find_first_of(' ') + 1;
        auto name_end = line.find_first_of(':');
        auto temp = line.substr(name_start, name_end - name_start);
        return trim_spaces(temp);
    }

    static uint32_t parse_field_offset(const std::string& line)
    {
        auto temp = line.substr(3, line.find_first_of(' ') - 3);
        return strtoul(std::data(temp), nullptr, 16);
    }
    
    static std::unique_ptr<windbg_field> parse_field(const std::string& line);
private:
    std::string _name;
    std::vector<std::unique_ptr<windbg_field>> _fields;
};
