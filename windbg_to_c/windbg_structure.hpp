#pragma once

#include "windbg_field.hpp"

class windbg_structure
{
public:
    windbg_structure(const std::string& text);

    static bool is_header(const std::string& line);
    static std::string get_field_offset(const std::string& line);

    template<typename Iter>
    static bool is_union_or_bitfield(Iter it)
    {
        try {
            return get_field_offset(*it) == get_field_offset(*(it + 1));
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
    std::string _name;
    std::vector<windbg_field> _fields;
};
