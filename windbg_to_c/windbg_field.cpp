#include "windbg_field.hpp"
#include "helpers.hpp"

#include <map>
#include <string>
#include <sstream>

windbg_field::windbg_field(std::string name, std::string type, uint32_t offset)
    : _name(std::move(name)), _type(std::move(type)), _offset(offset)
{

}
windbg_field::~windbg_field()
{
}

windbg_simple::windbg_simple(std::string name, std::string type, uint32_t offset)
    : windbg_field(std::move(name), std::move(type), offset)
{

}

std::string windbg_simple::as_string(int tabcount /*= 0*/) const
{
    return std::string(tabcount * 4, ' ') + _type + " " + _name + ";";
}

windbg_array::windbg_array(std::string name, std::string type, uint32_t offset, uint32_t len)
    : windbg_field(std::move(name), std::move(type), offset), _length(len)
{

}

std::string windbg_array::as_string(int tabcount /*= 0*/) const
{
    return std::string(tabcount * 4, ' ') + _type + " " + _name + "[" + std::to_string(_length) + "];";
}

windbg_union::windbg_union(uint32_t offset)
    : windbg_field("UNNAMED_UNION", "UNION", offset), _members()
{

}

std::string windbg_union::as_string(int tabcount /*= 0*/) const
{
    std::stringstream ss;
    ss << std::string(tabcount * 4, ' ') << "union {\n";
    for(auto& f : _members) {
        ss << f->as_string(tabcount + 1) << '\n';
    }
    ss << std::string(tabcount * 4, ' ') << "};";
    return ss.str();
}

windbg_bitfield::windbg_bitfield(std::string name, std::string type, uint32_t offset, uint32_t pos, uint32_t len)
    : windbg_field(std::move(name), std::move(type), offset), _pos(pos), _len(len)
{

}

std::string windbg_bitfield::as_string(int tabcount /*= 0*/) const
{
    return std::string(tabcount * 4, ' ') + _type + " " + _name + " : " + std::to_string(_len) + ";";
}

windbg_bitfield_pack::windbg_bitfield_pack(uint32_t offset)
    : windbg_field("UNNAMED_PACK", "PACK", offset), _members()
{

}

std::string windbg_bitfield_pack::as_string(int tabcount /*= 0*/) const
{
    std::stringstream ss;
    ss << std::string(tabcount * 4, ' ') << "struct {\n";
    for(auto& f : _members) {
        ss << f->as_string(tabcount + 1) << '\n';
    }
    ss << std::string(tabcount * 4, ' ') << "};";
    return ss.str();
}