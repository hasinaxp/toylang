#pragma once
#include <vector>
#include <unordered_map>
#include <string_view>
#include <stdint.h>
#include "parsing.hpp"



struct symbol_table_t
{
    std::unordered_map<std::string_view, size_t> symbols = {};
    symbol_table_t * parent = nullptr;
};


typedef std::vector<uint8_t> code_block_t;

struct program_code_t
{
    symbol_table_t global_symbols;
    std::vector<code_block_t> code_blocks;
};

struct code_generator_t
{
    program_code_t parse_program(const ast_t & ast)
    {

    };
};
