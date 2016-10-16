#pragma once

#include <string>
#include <vector>

class windbg_field
{
public:
    windbg_field();
    windbg_field(const std::string& line);

    bool is_array() const { return _is_array; }
    bool is_bitfield() const { return _is_bitfield; }
    bool is_union() const { return _is_union; }
    bool is_bitfield_pack() const { return _is_bitfield_pack; }

    const std::string& get_name() const { return _name; }
    const std::string& get_offset() const { return _offset; }
    const std::string& get_type() const { return _type; }

    uint32_t get_bitfield_pos() const { return _bitfield_pos; }
    uint32_t get_bitfield_len() const { return _bitfield_len; }
    uint32_t get_array_len() const { return _array_len; }

    const std::vector<windbg_field>& get_union_fields() const { return _union_fields; }
    const std::vector<windbg_field>& get_bitpack_fields() const { return _bitfield_pack; }

    void append_union_member(const windbg_field& field);
    void append_bitfield_member(const windbg_field& field);
    std::string as_string(int tabcount = 0) const;

private:
    void parse_field_name(const std::string& line);
    void parse_field_offset(const std::string& line);
    void parse_field_type(const std::string& line);

    std::string _name;
    std::string _offset;
    std::string _type;

    bool     _is_array = false;
    uint32_t _array_len = 0;

    bool     _is_bitfield = false;
    uint32_t _bitfield_pos = 0;
    uint32_t _bitfield_len = 0;

    bool _is_union = false;
    std::vector<windbg_field> _union_fields;

    bool _is_bitfield_pack = false;
    std::vector<windbg_field> _bitfield_pack;
};