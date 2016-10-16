#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class windbg_field
{
public:
    windbg_field(std::string name, std::string type, uint32_t offset);
    virtual ~windbg_field();

    virtual bool is_array() const = 0;
    virtual bool is_union() const = 0;
    virtual bool is_bitfield() const = 0;
    virtual bool is_bitfield_pack() const = 0;
    virtual std::string as_string(int tabcount = 0) const = 0;

    const std::string&  get_name() const { return _name; }
    const std::string&  get_type() const { return _type; }
    uint32_t            get_offset() const { return _offset; }

protected:
    std::string _name;
    std::string _type;
    uint32_t    _offset;
};

class windbg_simple
    : public windbg_field
{
public:
    windbg_simple(std::string name, std::string type, uint32_t offset);

    bool is_array() const override { return false; }
    bool is_union() const override { return false; }
    bool is_bitfield() const override { return false; }
    bool is_bitfield_pack() const override { return false; }

    std::string as_string(int tabcount = 0) const override;
};

class windbg_array
    : public windbg_field
{
public:
    windbg_array(std::string name, std::string type, uint32_t offset, uint32_t len);

    bool is_array() const override { return true; }
    bool is_union() const override { return false; }
    bool is_bitfield() const override { return false; }
    bool is_bitfield_pack() const override { return false; }

    std::string as_string(int tabcount = 0) const override;

    uint32_t get_array_len() const { return _length; }

private:
    uint32_t _length;
};

class windbg_union
    : public windbg_field
{
public:
    windbg_union(uint32_t offset);

    bool is_array() const override { return false; }
    bool is_union() const override { return true; }
    bool is_bitfield() const override { return false; }
    bool is_bitfield_pack() const override { return false; }

    std::string as_string(int tabcount = 0) const override;

    void add_union_member(const std::shared_ptr<windbg_field>& m)
    {
        _members.push_back(m);
    }
private:
    std::vector<std::shared_ptr<windbg_field>> _members;
};

class windbg_bitfield
    : public windbg_field
{
public:
    windbg_bitfield(std::string name, std::string type, uint32_t offset, uint32_t pos, uint32_t len);

    bool is_array() const override { return false; }
    bool is_union() const override { return false; }
    bool is_bitfield() const override { return true; }
    bool is_bitfield_pack() const override { return false; }

    std::string as_string(int tabcount = 0) const override;

    uint32_t get_bitfield_pos() const { return _pos; }
    uint32_t get_bitfield_len() const { return _len; }

private:
    uint32_t _pos;
    uint32_t _len;
};

class windbg_bitfield_pack
    : public windbg_field
{
public:
    windbg_bitfield_pack(uint32_t offset);

    bool is_array() const override { return false; }
    bool is_union() const override { return false; }
    bool is_bitfield() const override { return false; }
    bool is_bitfield_pack() const override { return true; }

    std::string as_string(int tabcount = 0) const override;

    void add_bitfield_member(const std::shared_ptr<windbg_field>& m)
    {
        _members.emplace_back(m);
    }
private:
    std::vector<std::shared_ptr<windbg_field>> _members;
};

