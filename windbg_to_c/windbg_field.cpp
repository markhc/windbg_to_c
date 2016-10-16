#include "windbg_field.hpp"
#include "helpers.hpp"

#include <map>
#include <sstream>

//
// Known WinDbg Data Types
//
const std::map<std::string, std::string> known_types = {
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

windbg_field::windbg_field()
{
}

windbg_field::windbg_field(const std::string& line)
{
    parse_field_name(line);
    parse_field_offset(line);
    parse_field_type(line);
}
void windbg_field::append_union_member(const windbg_field& field)
{
    _is_union = true;
    _union_fields.emplace_back(field);
}
void windbg_field::append_bitfield_member(const windbg_field& field)
{
    _is_bitfield_pack = true;
    _bitfield_pack.emplace_back(field);
}

std::string windbg_field::as_string(int tabcount /*= 0*/) const
{
    auto padding = std::string((tabcount + 1) * 4, ' ');
    std::stringstream out;
    if(is_union()) {
        out << padding << "union\n"
            << padding << "{\n";
        for(auto& union_field : get_union_fields()) {
            out << union_field.as_string(tabcount + 1) << "\n";
        }
        out << padding << "};";
    } else if(is_bitfield_pack()) {
        out << padding << "struct\n"
            << padding << "{\n";
        for(auto& bitfield : get_bitpack_fields()) {
            out << bitfield.as_string(tabcount + 1) << "\n";
        }
        out << padding << "};";
    } else if(is_bitfield()) {
        out << padding << _type << " " << _name << " : " << _bitfield_len << ";";
    } else if(is_array()) {
        out << padding << _type << " " << _name << "[" << _array_len << "];";
    } else {
        out << padding << _type << " " << _name << ";";
    }
    return out.str();
}

void windbg_field::parse_field_name(const std::string& line)
{
    auto name_start = line.find_first_of(' ') + 1;
    auto name_end = line.find_first_of(':');
    auto temp = line.substr(name_start, name_end - name_start);
    _name = trim_spaces(temp);
}

void windbg_field::parse_field_offset(const std::string& line)
{
    _offset = line.substr(3, line.find_first_of(' ') - 3);
}

void windbg_field::parse_field_type(const std::string& line)
{
    using namespace std::string_literals;

    auto pointer_count = 0;
    auto type_start  = line.find_first_of(':') + 1;
    auto type_string = line.substr(type_start);
    type_string      = trim_spaces(type_string);

    //
    // Check if this is a bitfield. 
    // We return early if it is
    //
    if(type_string.find("Pos") == 0) {
        _is_bitfield = true;
        _type = "UCHAR";
        auto separator = type_string.find(',');
        auto pos = type_string.substr(4, separator - 4);
        auto len = type_string.substr(separator + 2);
        _bitfield_pos = std::strtoul(std::data(pos), nullptr, 10);
        _bitfield_len = std::strtoul(std::data(len), nullptr, 10);
        return;
    }

    //
    // Get pointer count. We can have stuff like "Ptr64 Ptr64 Ptr64 Void"
    // which will translate to PVOID**
    //
    while(type_string.find("Ptr64") != std::string::npos) {
        pointer_count++;
        type_string = type_string.substr(6);
    }

    //
    // Check if it's an array and get the size if it is
    //
    if(type_string[0] == '[') {
        auto array_end = type_string.find(']');
        auto subscript = type_string.substr(1, array_end - 1);
        _array_len = std::strtoul(std::data(subscript), nullptr, 10);
        _is_array = true;

        type_string = type_string.substr(array_end + 2);
    }

    type_string = trim_spaces(type_string);

    //
    // Translate the type name
    //
    auto it = known_types.find(trim_spaces(type_string));
    if(it != known_types.end()) {
        //
        // If it is a known type, use it.
        //
        type_string = it->second;
    } else {
        //
        // Otherwise just drop the leading underscore
        //
        if(type_string[0] == '_')
            type_string = type_string.substr(1);
    }

    //
    // Append the pointer stuff
    //
    if(pointer_count > 1) {
        type_string = "P"s + type_string;
        while(--pointer_count) {
            type_string += "*";
        }
    } else if(pointer_count == 1) {
        type_string = "P"s + type_string;
    }

    //
    // Set the type name
    //
    _type = type_string;
}