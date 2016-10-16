#include "windbg_structure.hpp"
#include "helpers.hpp"
#include <algorithm>
#include <sstream>

windbg_structure::windbg_structure(const std::string& text)
{
    auto lines = split_string(text, "\n");

    std::transform(lines.begin(), lines.end(), lines.begin(), [](std::string& str) {
        return trim_spaces(str);
    });

    //
    // For each line...
    //
    for(auto it = lines.begin(); it != lines.end(); ++it) {
        if(it->empty()) continue;

        //
        // A line can be either:
        //  - Header
        //  - Union
        //  - Bitfield
        //  - Regular type
        //
        if(is_header(*it)) {
            _name = it->substr(it->find('!') + 1);
            if(_name[0] == '_')
                _name = _name.substr(1);
        } else if(is_union_or_bitfield(it)) {
            std::vector<std::unique_ptr<windbg_field>> union_fields;
            do {
                union_fields.emplace_back(parse_field(*it));
                ++it;
            } while(is_union_or_bitfield(it));

            union_fields.emplace_back(parse_field(*it));
            ++it;
            
            auto bitfield_count = std::count_if(
                union_fields.begin(),
                union_fields.end(),
                [](const std::unique_ptr<windbg_field>& field) { return field->is_bitfield(); });
            
            if(bitfield_count != union_fields.size() && bitfield_count != 0) { //Its a union of a bitfield + non-bitfield
                std::unique_ptr<windbg_union>         field = std::make_unique<windbg_union>(parse_field_offset(*it));
                std::unique_ptr<windbg_bitfield_pack> pack = std::make_unique<windbg_bitfield_pack>(parse_field_offset(*it));

                for(auto& f : union_fields) {
                    if(f->is_bitfield())
                        pack->add_bitfield_member(std::move(f));
                    else
                        field->add_union_member(std::move(f));
                }

                field->add_union_member(std::move(pack));
                _fields.emplace_back(std::move(field));
            } else if(bitfield_count == 0) { //It just a union
                std::unique_ptr<windbg_union> field = std::make_unique<windbg_union>(parse_field_offset(*it));
                for(auto& f : union_fields) {
                    field->add_union_member(std::move(f));
                }
                _fields.emplace_back(std::move(field));
            } else { //It's just a bitfield
                std::unique_ptr<windbg_bitfield_pack> pack = std::make_unique<windbg_bitfield_pack>(parse_field_offset(*it));
                for(auto& f : union_fields) {
                    pack->add_bitfield_member(std::move(f));
                }
                _fields.emplace_back(std::move(pack));
            }
        } else {
            _fields.emplace_back(parse_field(*it));
        }
    }
}

bool windbg_structure::is_header(const std::string& line)
{
    return line.find('!') != std::string::npos;
}

std::unique_ptr<windbg_field> windbg_structure::parse_field(const std::string& line)
{
    using namespace std::string_literals;

    auto pointer_count = 0;
    auto is_array = false;
    auto array_len = 0;

    auto offset_string = line.substr(3, line.find_first_of(' ') - 3);
    auto offset = strtoul(std::data(offset_string), nullptr, 16);

    auto name_start = line.find_first_of(' ') + 1;
    auto name_end = line.find_first_of(' ', name_start);
    auto name_string = line.substr(name_start, name_end - name_start);

    auto type_start = line.find_first_of(':') + 1;
    auto type_string = line.substr(type_start);

    type_string = trim_spaces(type_string);
    name_string = trim_spaces(name_string);

    //
    // Check if this is a bitfield. 
    // We return early if it is
    //
    if(type_string.find("Pos") == 0) {
        auto separator = type_string.find(',');
        auto pos = type_string.substr(4, separator - 4);
        auto len = type_string.substr(separator + 2);
        auto bitfield_pos = std::strtoul(std::data(pos), nullptr, 10);
        auto bitfield_len = std::strtoul(std::data(len), nullptr, 10);
        auto type = "UCHAR"s;

        if(bitfield_len > 32)
            type = "ULONGLONG";
        else if(bitfield_len > 16)
            type = "ULONG";
        else if(bitfield_len > 8)
            type = "USHORT";

        return std::make_unique<windbg_bitfield>(name_string, type, offset, bitfield_pos, bitfield_len);
    }

    if(type_string[0] == '[') {
        is_array = true;
        auto array_end = type_string.find(']');
        auto subscript = type_string.substr(1, array_end - 1);
        array_len = std::strtoul(std::data(subscript), nullptr, 10);

        type_string = type_string.substr(array_end + 2);

    }

    while(type_string.find("Ptr64") != std::string::npos) {
        pointer_count++;
        type_string = type_string.substr(6);
    }

    type_string = trim_spaces(type_string);

    auto it = known_types.find(type_string);
    if(it != known_types.end()) {
        type_string = it->second;
    } else {
        if(type_string[0] == '_')
            type_string = type_string.substr(1);
    }

    if(pointer_count > 1) {
        type_string = "P"s + type_string;
        while(--pointer_count) {
            type_string += "*";
        }
    } else if(pointer_count == 1) {
        type_string = "P"s + type_string;
    }

    if(is_array)
        return std::make_unique<windbg_array>(name_string, type_string, offset, array_len);
    else
        return std::make_unique<windbg_simple>(name_string, type_string, offset);
}


std::string windbg_structure::as_string(int tabcount/* = 0*/) const
{
    std::stringstream out;
    out << std::string(tabcount * 4, ' ') << "typedef struct _" << _name << "\n{\n";
    for(auto& field : _fields) {
        out << field->as_string(tabcount + 1) << "\n";
    }
    out << std::string(tabcount * 4, ' ') << "} " << _name << ", *P" << _name << ";" << std::endl;
    return out.str();
}