#pragma once
#include <vector>
#include <unordered_map>
#include <string_view>
#include <stdint.h>
#include <sstream>
#include "parsing.hpp"


enum VariableType
{
    VAR_UNKNOWN = 0,
    VAR_INT,
    VAR_FLOAT,
    VAR_STRING,
    VAR_BOOL,
    VAR_VOID,
    VARIABLE_TYPE
};

enum BytecodeOp
{
    BC_HALT = 0,
    BC_PUSH_INT,
    BC_ADD_INT_INT,
    BC_SUB_INT_INT,
};

struct variable_t
{
    size_t type = 0;
    size_t offset = 0;
    size_t size = 4;
};


inline int64_t change_endian(int64_t value) {
    return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24);
}


struct program_data_t
{
    std::unordered_map<std::string_view, variable_t> vars;
    std::vector<uint8_t> bytecode;


    std::string asm_str(bool entry_point = false)
    {
        std::stringstream ss;

        int i = 0;
        while(i < bytecode.size()) {
            auto opcode = bytecode[i];
            if(opcode == BC_PUSH_INT) {
                ss << "  movabs rax, " << *(int64_t*)&bytecode[i + 1] << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 9;
                
            } else if (opcode == BC_ADD_INT_INT) {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  add rax, rbx" << std::endl;
                ss << "  push rax" << std::endl << std::endl;;
                i += 1;
            } else if (opcode == BC_SUB_INT_INT) {
                ss << "  pop rbx" << std::endl;
                ss << "  pop rax" << std::endl;
                ss << "  sub rax, rbx" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            } else {
                throw utils::error_t(0, "Unknown opcode: " + std::to_string(opcode));
                break;
            }
        }
        std::string asm_code = ss.str();
        if(entry_point) {
            ss.str("");
            ss.clear();
            ss << ".intel_syntax noprefix" << std::endl;
            ss << ".section .text" << std::endl;
            ss << "  .globl main" << std::endl;
            ss << "main:" << std::endl;
            ss << asm_code << std::endl;
            ss << "  movabs rax, 60" << std::endl;
            ss << "  pop rdi" << std::endl;
            ss << "  syscall" << std::endl;
            return ss.str();
        }
        return asm_code;

    }
};


inline void check_ast_type(const ast_t &ast, size_t type) {
    if (ast.type != type) {
        std::string msg = "Unexpected AST node type: ";
        msg += std::to_string(ast.type);
        msg += ", expected: ";
        msg += std::to_string(type);
        throw utils::error_t(ast.line, msg);
    }
}


struct var_context_t {
    std::unordered_map<std::string_view, variable_t> vars;
    var_context_t * parent = nullptr;
    variable_t * get_var(std::string_view name) {
        auto it = vars.find(name);
        if(it != vars.end()) {
            return &(it->second);
        }
        if(parent) {
            return parent->get_var(name);
        }
        return nullptr;
    }

    void set_var(std::string_view name, size_t type, size_t &stack_size, size_t size = 8) {
        variable_t var;
        var.type = type;
        var.offset = stack_size;
        var.size = size;
        vars[name] = var;
        stack_size += var.size;
    }
};

struct code_generator_t
{
    program_data_t gen_program(const ast_t &ast, var_context_t & ctx) {
        check_ast_type(ast, AST_PROGRAM);
        program_data_t data;
        size_t stack_size = 0;
        for(auto & child : ast.children) {
            generate_code_stmt(child, data, stack_size, ctx);
        }
        return data;
    }

    void generate_code_stmt(const ast_t &ast, program_data_t &data, size_t &stack_size,  var_context_t & ctx) {
        check_ast_type(ast, AST_EXPR_STMT);
        auto & expr = ast.children[0];
        generate_code_expr(expr, data, stack_size, ctx);
    }

    const ASTType & generate_code_expr(const ast_t &ast, program_data_t &data, size_t &stack_size, var_context_t & ctx) {
        switch(ast.type) {
            case AST_INT: {
                generate_code_int(ast, data, stack_size);
                break;
            }
            case AST_ID: {
                auto var = ctx.get_var(ast.value);
                if(!var) {
                    throw utils::error_t(ast.line, "Undefined variable: " + std::string(ast.value));
                }
                data.bytecode.push_back(BC_PUSH_INT);
                int64_t value = var->offset;
                stack_size += sizeof(int64_t);
                data.bytecode.insert(data.bytecode.end(), reinterpret_cast<uint8_t*>(&value),
                                                    reinterpret_cast<uint8_t*>(&value) + sizeof(value));
                break;
            }
            case AST_BINARY_OP: {
                generate_code_binary_op(ast, data, stack_size, ctx);
                break;
            }
            case AST_ASSIGN: {
                generate_code_assign(ast, data, stack_size, ctx);
                break;
            }
            default:
                throw utils::error_t(ast.line, "Unexpected AST node type in statement: " + std::to_string(ast.type));
        }

        return ast.type;   
    }

    void generate_code_int(const ast_t &ast, program_data_t &data, size_t &stack_size) {
        std::cout << "generate_code_int" << std::endl;
        data.bytecode.push_back(BC_PUSH_INT);
        int64_t value = std::stoll(std::string(ast.value));
        stack_size += sizeof(int64_t);
        data.bytecode.insert(data.bytecode.end(), reinterpret_cast<uint8_t*>(&value),
                                                reinterpret_cast<uint8_t*>(&value) + sizeof(value));
    }


    void generate_code_binary_op(const ast_t &ast, program_data_t &data, size_t &stack_size, var_context_t & ctx) {
        std::cout << "generate_code_binary_op" << std::endl;
        check_ast_type(ast, AST_BINARY_OP);
        auto & t1 = generate_code_expr(ast.children[0], data, stack_size, ctx);
        auto & t2 = generate_code_expr(ast.children[1], data, stack_size, ctx);
        switch(ast.value[0]) {
            case '+':
                if(t1 == AST_INT || t2 == AST_INT) {
                    data.bytecode.push_back(BC_ADD_INT_INT);
                }
                break;
            case '-':
                if(t1 == AST_INT || t2 == AST_INT) {
                    data.bytecode.push_back(BC_SUB_INT_INT);
                }
                break;
            default:
                throw utils::error_t(ast.line, std::string("Unknown binary operator: ") + std::string(ast.value));
        }
    }

    void generate_code_assign(const ast_t &ast, program_data_t &data, size_t &stack_size, var_context_t & ctx) {
        std::cout << "generate_code_assign" << std::endl;
        check_ast_type(ast, AST_ASSIGN);
        auto & lhs = ast.children[0];
        auto & rhs = generate_code_expr(ast.children[1], data, stack_size, ctx);
        if(lhs.type != AST_ID) {
            throw utils::error_t(ast.line, "Invalid assignment target");
        }
        auto var = ctx.get_var(ast.children[0].value);
        if(!var) {
            auto type = get_expression_type(ast.children[1], ctx);
            ctx.set_var(ast.children[0].value, type, stack_size);


        }
    }

    size_t get_expression_type(const ast_t & exp, var_context_t & ctx) {
        if(exp.type == AST_INT) {
            return VAR_INT;
        }
        if(exp.type == AST_ID) {
            return ctx.get_var(exp.value)->type;
        }
        if(exp.type == AST_BINARY_OP) {
            return get_expression_type(exp.children[0], ctx);
        }
        throw utils::error_t(exp.line, "cannot determine expression type");
    }
};