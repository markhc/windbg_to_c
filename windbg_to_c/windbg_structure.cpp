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

    for(auto it = lines.begin(); it != lines.end(); ++it) {
        if(it->empty()) continue;
        if(is_header(*it)) {
            _name = it->substr(it->find('!') + 1);
            if(_name[0] == '_')
                _name = _name.substr(1);
        } else if(is_union_or_bitfield(it)) {
            std::vector<windbg_field> union_fields;
            do {
                union_fields.emplace_back(*it);
                ++it;
            } while(is_union_or_bitfield(it));

            union_fields.emplace_back(*it);
            ++it;

            auto bitfield_count = std::count_if(
                union_fields.begin(),
                union_fields.end(),
                [](const windbg_field& field) { return field.is_bitfield(); });
            if(bitfield_count != union_fields.size()) { //Its a union of a bitfield + non-bitfield
                windbg_field field;
                windbg_field bitfield;

                for(auto& f : union_fields) {
                    if(f.is_bitfield())
                        bitfield.append_bitfield_member(f);
                    else
                        field.append_union_member(f);
                }

                field.append_union_member(bitfield);
                _fields.emplace_back(std::move(field));
            } else if(bitfield_count == 0) { //It just a union
                windbg_field field;
                for(auto& f : union_fields) {
                    field.append_union_member(f);
                }
                _fields.emplace_back(std::move(field));
            } else { //It's just a bitfield
                windbg_field field;
                for(auto& f : union_fields) {
                    field.append_bitfield_member(f);
                }
                _fields.emplace_back(std::move(field));
            }
        } else {
            _fields.emplace_back(*it);
        }
    }
}

bool windbg_structure::is_header(const std::string& line)
{
    return line.find('!') != std::string::npos;
}

std::string windbg_structure::get_field_offset(const std::string& line)
{
    return line.substr(3, line.find_first_of(' ') - 3);
}
std::string windbg_structure::as_string(int tabcount/* = 0*/) const
{
    std::stringstream out;
    out << std::string(tabcount * 4, ' ') << "typedef struct _" << _name << "\n{\n";
    for(auto& field : _fields) {
        out << field.as_string(tabcount) << "\n";
    }
    out << std::string(tabcount * 4, ' ') << "} " << _name << ", *P" << _name << ";" << std::endl;
    return out.str();
}