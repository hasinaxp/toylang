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

static const char *VariableTypeNames[] = {
    "unknown",
    "int",
    "float",
    "string",
    "bool",
    "void",
    "variable"};

enum FunctionType
{
    FUNC_UNKNOWN = 0,
    FUNC_SISCALL,
    FUNCTION_TYPE
};

enum BytecodeOp
{
    BC_HALT = 0,
    BC_PUSH_INT,
    BC_SET_INT,
    BC_COPY_INT,
    BC_ADD_INT_INT,
    BC_SUB_INT_INT,
    BC_MUL_INT_INT,
    BC_DIV_INT_INT,
    BC_MOD_INT_INT,
    BC_AND,
    BC_OR,
    BC_XOR,
    BC_SHL,
    BC_SHR,

    BC_SYSCALL,
};

struct variable_t
{
    size_t type = 0;
    size_t offset = 0;
    size_t size = 4;
};

inline int64_t change_endian(int64_t value)
{
    return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

enum SystemCallType
{
    BC_SYS_EXIT = 0,
    BC_SYS_WRITE_INT,
};

struct program_data_t
{
    std::unordered_map<std::string_view, variable_t> vars;
    std::vector<uint8_t> bytecode;

    void init_basic_syscalls()
    {
    }

    void push_int(int64_t value, bool ce = true)
    {
        if (ce)
            value = change_endian(value);
        bytecode.insert(bytecode.end(), reinterpret_cast<uint8_t *>(&value),
                        reinterpret_cast<uint8_t *>(&value) + sizeof(value));
    }

    std::string asm_str(bool entry_point = false)
    {
        std::stringstream ss;
        size_t syscalls = 0;

        int i = 0;
        while (i < bytecode.size())
        {
            auto opcode = bytecode[i];
            if (opcode == BC_PUSH_INT)
            {
                ss << "  movabs rax, " << *(int64_t *)&bytecode[i + 1] << std::endl;
                ss << "  push rax" << std::endl
                   << std::endl;
                i += 9;
            }
            else if (opcode == BC_ADD_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  add rax, rbx" << std::endl;
                ss << "  push rax" << std::endl
                   << std::endl;
                ;
                i += 1;
            }
            else if (opcode == BC_SUB_INT_INT)
            {
                ss << "  pop rbx" << std::endl;
                ss << "  pop rax" << std::endl;
                ss << "  sub rax, rbx" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            }
            else if (opcode == BC_MUL_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  imul rax, rbx" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            }
            else if (opcode == BC_DIV_INT_INT)
            {
                ss << "  pop rbx" << std::endl;
                ss << "  pop rax" << std::endl;
                ss << "  cqo" << std::endl;
                ss << "  idiv rbx" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            }
            else if (opcode == BC_SET_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  mov [rsp + " << *(int64_t *)&bytecode[i + 1] << "], rax" << std::endl;
                i += 9;
            }
            else if (opcode == BC_COPY_INT)
            {
                ss << "  mov rax, [rsp + " << *(int64_t *)&bytecode[i + 1] << "]" << std::endl;
                ss << "  push rax" << std::endl;
                i += 9;
            }
            else if (opcode == BC_AND)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  and rax, rbx" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            }
            else if (opcode == BC_OR)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  or rax, rbx" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            }
            else if (opcode == BC_XOR)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  xor rax, rbx" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            }
            else if (opcode == BC_SHL)
            {
                ss << "  pop rcx" << std::endl;
                ss << "  pop rax" << std::endl;
                ss << "  shl rax, cl" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            }
            else if (opcode == BC_SHR)
            {
                ss << "  pop rcx" << std::endl;
                ss << "  pop rax" << std::endl;
                ss << "  shr rax, cl" << std::endl;
                ss << "  push rax" << std::endl;
                i += 1;
            }
            else if (opcode == BC_HALT)
            {
                ss << "  movabs rax, 60" << std::endl;
                ss << "  xor rdi, rdi" << std::endl;
                ss << "  syscall" << std::endl;
                i += 1;
            }
            else if (opcode == BC_SYSCALL)
            {
                auto syscall = *(int64_t *)&bytecode[i + 1];
                if (syscall == BC_SYS_EXIT)
                {
                    ss << "  movabs rax, 60" << std::endl;
                    ss << "  pop rdi" << std::endl;
                    ss << "  syscall" << std::endl;
                }
                else if (syscall == BC_SYS_WRITE_INT)
                {
                    std::cout << "sys_call_write_int" << std::endl;
                    sys_call_write_int(ss, syscalls);
                }
                i += 9;
            }
            else
            {
                throw utils::error_t(0, "Unknown opcode: " + std::to_string(opcode));
                break;
            }
        }
        std::string asm_code = ss.str();
        if (entry_point)
        {
            ss.str("");
            ss.clear();
            ss << ".intel_syntax noprefix" << std::endl;
            ss << ".section .bss" << std::endl;
            ss << "numbuf: .skip 20" << std::endl;
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

    void sys_call_write_int(std::stringstream & ss, size_t &i)
    {
        ss << "  pop rdi" << std::endl;
        ss << "  mov rsi, offset numbuf + 19" << std::endl;
        ss << "  mov byte ptr [rsi], 0xA" << std::endl;
        ss << "  dec rsi" << std::endl;

        ss << "itoa_loop" << i << ":" << std::endl;
        ss << "  mov rax, rdi" << std::endl;
        ss << "  mov rdx, 0" << std::endl;
        ss << "  mov rcx, 10" << std::endl;
        ss << "  div rdi" << std::endl;
        ss << "  add dl, '0'" << std::endl;
        ss << "  mov [rsi], dl" << std::endl;
        ss << "  dec rsi" << std::endl;
        ss << "  test rax, rax" << std::endl;
        ss << "  jnz itoa_loop" << std::endl;

        ss << "  inc rsi" << std::endl;

        ss << "  mov rax, 1" << std::endl;
        ss << "  mov rdi, 1" << std::endl;
        ss << "  lea rdx, [rsi + numbuf]" << std::endl;
        ss << "  syscall" << std::endl;
    }

};

inline void check_ast_type(const ast_t &ast, size_t type)
{
    if (ast.type != type)
    {
        std::string msg = "Unexpected AST node type: ";
        msg += std::to_string(ast.type);
        msg += ", expected: ";
        msg += std::to_string(type);
        throw utils::error_t(ast.line, msg);
    }
}

struct var_context_t
{
    size_t stack_size = 0;
    std::vector<variable_t> vars;
    std::vector<size_t> scopes;
    std::unordered_map<std::string_view, size_t> var_map;

    void add_var(std::string_view name, size_t type, size_t size)
    {
        variable_t var;
        var.type = type;
        var.offset = stack_size;
        var.size = size;
        vars.push_back(var);
        var_map[name] = vars.size() - 1;
    }

    variable_t *get_var(std::string_view name)
    {
        auto it = var_map.find(name);
        if (it == var_map.end())
        {
            return nullptr;
        }
        return &vars[it->second];
    }

    void push_scope()
    {
        scopes.push_back(vars.size());
    }
    void pop_scope()
    {
        size_t scope_start = scopes.back();
        scopes.pop_back();
        while (vars.size() > scope_start)
        {
            stack_size -= vars.back().size;
            vars.pop_back();
        }
    }
};

struct code_generator_t
{
    program_data_t gen_program(const ast_t &ast)
    {
        check_ast_type(ast, AST_PROGRAM);
        program_data_t data;
        var_context_t ctx;
        for (auto &child : ast.children)
        {
            generate_code_stmt(child, data, ctx);
        }
        for (auto &v : ctx.vars)
        {
            std::cout << "var: " << v.type << " offset: " << v.offset << std::endl;
        }
        return data;
    }

    void generate_code_stmt(const ast_t &ast, program_data_t &data, var_context_t &ctx)
    {
        if (ast.type == AST_EXPR_STMT)
        {
            auto &expr = ast.children[0];
            generate_code_expr(expr, data, ctx);
        }
        else if (ast.type == AST_BLOCK)
        {
            ctx.push_scope();
            for (auto &child : ast.children)
            {
                generate_code_stmt(child, data, ctx);
            }
            ctx.pop_scope();
        }
        else
        {
            throw utils::error_t(ast.line, "Unexpected AST node type in statement: " + std::to_string(ast.type));
        }
    }

    ASTType generate_code_expr(const ast_t &ast, program_data_t &data, var_context_t &ctx)
    {
        auto type_to_return = ast.type;
        switch (ast.type)
        {
        case AST_INT:
        {
            generate_code_int(ast, data, ctx.stack_size);
            break;
        }
        case AST_ID:
        {
            std::cout << "generate_code_id: " << ast.value << std::endl;
            auto var = ctx.get_var(ast.value);
            if (!var)
            {
                throw utils::error_t(ast.line, "Undefined variable: " + std::string(ast.value));
            }
            if (var->type == VAR_INT)
            {
                std::cout << "var type: " << var->type << std::endl;
                data.bytecode.push_back(BC_COPY_INT);
                data.push_int(ctx.stack_size - var->offset, false);
                ctx.stack_size += sizeof(int64_t);
                type_to_return = AST_INT;
            }
            break;
        }
        case AST_BINARY_OP:
        {
            parser_t p;
            p.print(ast);
            type_to_return = generate_code_binary_op(ast, data, ctx);
            break;
        }
        case AST_ASSIGN:
        {
            generate_code_assign(ast, data, ctx);
            break;
        }
        case AST_FUNC_CALL:
        {
            auto name = ast.children[0].value;
            if (name == "write")
            {
                data.bytecode.push_back(BC_SYSCALL);
                data.push_int(BC_SYS_WRITE_INT, false);
                generate_code_expr(ast.children[1].children[0], data, ctx);
                return AST_INT;
            }
        }
        break;
        default:
            throw utils::error_t(ast.line, "Unexpected AST node type in statement: " + std::to_string(ast.type));
        }

        return type_to_return;
    }

    void generate_code_int(const ast_t &ast, program_data_t &data, size_t &stack_size)
    {
        std::cout << "generate_code_int" << std::endl;
        data.bytecode.push_back(BC_PUSH_INT);
        int64_t value = std::stoll(std::string(ast.value));
        stack_size += sizeof(int64_t);
        data.bytecode.insert(data.bytecode.end(), reinterpret_cast<uint8_t *>(&value),
                             reinterpret_cast<uint8_t *>(&value) + sizeof(value));
    }

    ASTType generate_code_binary_op(const ast_t &ast, program_data_t &data, var_context_t &ctx)
    {
        check_ast_type(ast, AST_BINARY_OP);
        auto t1 = generate_code_expr(ast.children[0], data, ctx);
        auto t2 = generate_code_expr(ast.children[1], data, ctx);

        static const char *arithmatic_ops[] = {"+", "-", "*", "/"};
        static BytecodeOp arithmatic_ops_int[] = {BC_ADD_INT_INT, BC_SUB_INT_INT, BC_MUL_INT_INT, BC_DIV_INT_INT};

        for (size_t i = 0; i < 4; i++)
        {

            if (ast.value == arithmatic_ops[i] && t1 == AST_INT && t2 == AST_INT)
            {
                data.bytecode.push_back(arithmatic_ops_int[i]);
                ctx.stack_size -= sizeof(int64_t);
                return AST_INT;
            }
        }

        static const char *logical_ops[] = {"&&", "||", "^^", "&", "|", "^", "<<", ">>"};
        static BytecodeOp logical_ops_code[] = {BC_AND, BC_OR, BC_XOR, BC_AND, BC_OR, BC_XOR, BC_SHL, BC_SHR};

        for (size_t i = 0; i < 8; i++)
        {
            if (ast.value == logical_ops[i])
            {
                data.bytecode.push_back(logical_ops_code[i]);
                ctx.stack_size -= sizeof(int64_t);
                return AST_INT;
            }
        }

        throw utils::error_t(ast.line, std::string("Unknown binary operator: ") + std::string(ast.value));
        std::cout << "generate_code_binary_op" << std::endl;
    }

    void generate_code_assign(const ast_t &ast, program_data_t &data, var_context_t &ctx)
    {
        check_ast_type(ast, AST_ASSIGN);
        auto &lhs = ast.children[0];
        if (lhs.type != AST_ID)
        {
            throw utils::error_t(ast.line, "Invalid assignment target");
        }
        auto rhs = get_expression_type(ast.children[1], ctx);
        generate_code_expr(ast.children[1], data, ctx);
        std::cout << "rhs type: " << ASTTypeNames[rhs] << std::endl;
        auto var = ctx.get_var(lhs.value);
        if (!var)
        {
            if (rhs == AST_INT)
            {
                ctx.add_var(lhs.value, rhs, sizeof(int64_t));
                std::cout << "stack size: " << ctx.stack_size << std::endl;
            }
        }
        else
        {
            if (var->type != rhs)
            {
                throw utils::error_t(ast.line, "Type mismatch in assignment");
            }
            data.bytecode.push_back(BC_SET_INT);
            data.push_int(var->offset, false);
        }
        std::cout << "generate_code_assign" << std::endl;
    }

    size_t get_expression_type(const ast_t &exp, var_context_t &ctx)
    {
        if (exp.type == AST_INT)
        {
            return VAR_INT;
        }
        if (exp.type == AST_ID)
        {
            return ctx.get_var(exp.value)->type;
        }
        if (exp.type == AST_BINARY_OP)
        {
            return get_expression_type(exp.children[0], ctx);
        }
        throw utils::error_t(exp.line, "cannot determine expression type");
    }
};