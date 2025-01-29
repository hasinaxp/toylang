#pragma once
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include "parsing.hpp"
#include <stdio.h>

enum Instructions {
    INS_ASSIGN,
    INS_ASSIGN_COPY,
    INS_ADD,
    INS_SUB,
    INS_MUL,
    INS_DIV,
    INS_MOD,
    INS_PUSH,
    INS_POP,
    INS_INT,
    INS_FLOAT,
    INS_STR,
    INS_ID,
};

enum DataTypes {
    DT_INT,
    DT_FLOAT,
    DT_STR,
    DT_ID,
};


struct code_block_t {
    std::vector<uint64_t> code;
    std::unordered_map<std::string_view, size_t> locals;

    size_t get_id_var(const std::string_view & sv) {
        if(locals.count(sv) == 0) {
            locals[sv] = locals.size();
        }
        return locals[sv];
    }

    void push_int(const std::string_view & value) {
        int64_t v = std::stoll(std::string(value));
        code.push_back(INS_PUSH);
        code.push_back(DataTypes::DT_INT);
        code.push_back(0);
        *((int64_t*)&(code.back())) = v;
    }

    void push_float(const std::string_view & value) {
        double v = std::stod(std::string(value));
        code.push_back(INS_PUSH);
        code.push_back(DataTypes::DT_FLOAT);
        code.push_back(0);
        *((double*)&(code.back())) = v;
    }

    void push_str(const std::string_view & value) {
        code.push_back(INS_PUSH);
        code.push_back(DataTypes::DT_STR);
        code.push_back(0);
        code.push_back(value.size());
        for(size_t i = 0; i < value.size(); i++) {
            code.push_back(value[i]);
        }
    }

    void push_id(const std::string_view & value) {
        code.push_back(INS_PUSH);
        code.push_back(DataTypes::DT_ID);
        code.push_back(get_id_var(value));
    }

    void print()
    {
        printf("var table:\n");
        for(auto & [key, value] : locals) {
            printf("%.*s: %zu\n", (int)key.size(), key.data(), value);
        }
        printf("Code block:\n");
        for(size_t i = 0; i < code.size(); i++) {
            if(i % 8 == 0) {
                printf("\n");
            }
            printf("%zu ", code[i]);
        }
        printf("\n");
    }

};



struct code_generator_t {


    
    code_block_t generate_code(ast_t & ast) {
        code_block_t block;
        if(ast.type == AST_EXPR_STMT) {
            generate_expr_code(block, ast.children[0]);
        }
        else if (ast.type == AST_BLOCK) {
            for(auto & expr_stmt : ast.children) {
                condition_assert(expr_stmt.type == AST_EXPR_STMT, "Expected expression statement", expr_stmt.line);
                generate_expr_code(block, expr_stmt);
            }
        }
        else if (ast.type == AST_PROGRAM) {
            for(auto & stmt : ast.children) {
                auto block2 = generate_code(stmt);
                for(size_t i = 0; i < block2.code.size(); i++) {
                    block.code.push_back(block2.code[i]);
                }
            }
        }
        else {
            condition_assert(false, "Unknown AST type", ast.line);
        }
        return block;
    }

    code_block_t generate_expr_code(code_block_t & code, const ast_t & ast)
    {
        if(ast.type == AST_EXPR_STMT) {
            generate_expr_code(code, ast.children[0]);
        }
        if(ast.type == AST_ASSIGN) {
            condition_assert(ast.children[0].type == AST_ID, "Exected a variable", ast.line);
            generate_expr_code(code, ast.children[1]);
            code.code.push_back(INS_ASSIGN);
            code.push_id(ast.children[0].value);
        }
        if(ast.type == AST_INT) {
            code.push_int(ast.value);
        }
        if (ast.type == AST_FLOAT) {
            code.push_float(ast.value);
        }
        if (ast.type == AST_STR) {
            code.push_str(ast.value);
        }
        if (ast.type == AST_ID) {
            code.push_id(ast.value);
        }
        if (ast.type == AST_BINARY_OP) {
            generate_expr_code(code, ast.children[0]);
            generate_expr_code(code, ast.children[1]);
            if(ast.value == "+") {
                code.code.push_back(INS_ADD);
            }
            if(ast.value == "-") {
                code.code.push_back(INS_SUB);
            }
            if(ast.value == "*") {
                code.code.push_back(INS_MUL);
            }
            if(ast.value == "/") {
                code.code.push_back(INS_DIV);
            }
            if(ast.value == "%") {
                code.code.push_back(INS_MOD);
            }
            else {
                condition_assert(false, "Unknown binary operator", ast.line);
            }
        }
    }


    void condition_assert(bool condition, const char * message, size_t line_no) {
    if(!condition) {
        printf("Error::%zu: %s\n", line_no, message);
        exit(1);
    }
}

};