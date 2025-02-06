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
    BC_SHRINK_STACK,
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
    BC_EQ_INT_INT,
    BC_NE_INT_INT,
    BC_GT_INT_INT,
    BC_LT_INT_INT,
    BC_GE_INT_INT,
    BC_LE_INT_INT,
    BC_IF,
    BC_ELSE,
    BC_TEST_FALSE_LABEL,
    BC_TEST_END_END_LABEL,
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

const char * int_to_str_asm_code = R"(

int_to_str:
    push rbx
    push rdi
    push rsi


    mov rax, rdi                    # Get input integer from RDI
    lea rdi, [numbuf + 20]          # Point to the end of numbuf
    mov BYTE PTR [rdi], 0           # Null-terminate string
    dec rdi                         # Move pointer back for storing digits

    test rax, rax
    jnz .convert_loop_int_to_str    
    mov BYTE PTR [rdi], '0'         # Special case for zero
    jmp .done_int_to_str

.convert_loop_int_to_str:
    xor rdx, rdx                    # Clear RDX for division
    mov rbx, 10                     # Divisor
    div rbx                         # RAX /= 10, remainder in RDX

    add dl, '0'                     # Convert remainder to ASCII
    mov BYTE PTR [rdi], dl          # Store ASCII digit
    dec rdi                         # Move backward

    test rax, rax
    jnz .convert_loop_int_to_str

.done_int_to_str:
    inc rdi                         # Adjust pointer to start of the number
    mov rax, rdi                    # Return pointer to the converted string

    pop rsi
    pop rdi
    pop rbx
    ret

)";

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
        
        bool include_int_to_str_code = false;



        int i = 0;
        while (i < bytecode.size())
        {
            auto opcode = bytecode[i];
            if (opcode == BC_PUSH_INT)
            {

                std::cout << "push_int" << std::endl;
                ss << " # push_int" << std::endl;
                ss << "  movabs rax, " << *(int64_t *)&bytecode[i + 1] << std::endl;
                ss << "  push rax" << std::endl
                   << std::endl;
                i += 9;
            }
            else if (opcode == BC_SHRINK_STACK)
            {
                std::cout << "shrink_stack" << std::endl;
                size_t size = *(int64_t *)&bytecode[i + 1];
                ss << " # shrink_stack" << std::endl;
                ss << "  add rsp, " << size << std::endl
                   << std::endl;
                i += 9;
            }
            else if (opcode == BC_ADD_INT_INT)
            {
                std::cout << "add_int_int" << std::endl;
                ss << " # add_int_int" << std::endl;
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
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_MUL_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  imul rax, rbx" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_DIV_INT_INT)
            {
                ss << "  pop rbx" << std::endl;
                ss << "  pop rax" << std::endl;
                ss << "  cqo" << std::endl;
                ss << "  idiv rbx" << std::endl;
                ss << "  push rax" << std::endl<< std::endl;
                i += 1;
            }
            else if (opcode == BC_SET_INT)
            {
                std::cout << "set_int" << std::endl;
                ss << "  # set_int" << std::endl;
                ss << "  mov rax, [rsp]" << std::endl;
                ss << "  mov [rsp + " << *(int64_t *)&bytecode[i + 1] << "], rax" << std::endl << std::endl;
                i += 9;
            }
            else if (opcode == BC_COPY_INT)
            {
                std::cout << "copy_int" << std::endl;
                ss << "  # copy_int" << std::endl;
                ss << "  mov rax, [rsp + " << *(int64_t *)&bytecode[i + 1] << "]" << std::endl;
                ss << "  push rax" << std::endl  << std::endl;
                i += 9;
            }
            else if (opcode == BC_AND)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  and rax, rbx" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_OR)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  or rax, rbx" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_XOR)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  xor rax, rbx" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_SHL)
            {
                ss << "  pop rcx" << std::endl;
                ss << "  pop rax" << std::endl;
                ss << "  shl rax, cl" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_SHR)
            {
                ss << "  pop rcx" << std::endl;
                ss << "  pop rax" << std::endl;
                ss << "  shr rax, cl" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_EQ_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  cmp rax, rbx" << std::endl;
                ss << "  sete al" << std::endl;
                ss << "  movzx rax, al" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_NE_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  cmp rax, rbx" << std::endl;
                ss << "  setne al" << std::endl;
                ss << "  movzx rax, al" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_GT_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  cmp rax, rbx" << std::endl;
                ss << "  setg al" << std::endl;
                ss << "  movzx rax, al" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_LT_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  cmp rax, rbx" << std::endl;
                ss << "  setl al" << std::endl;
                ss << "  movzx rax, al" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_GE_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  cmp rax, rbx" << std::endl;
                ss << "  setge al" << std::endl;
                ss << "  movzx rax, al" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_LE_INT_INT)
            {
                ss << "  pop rax" << std::endl;
                ss << "  pop rbx" << std::endl;
                ss << "  cmp rax, rbx" << std::endl;
                ss << "  setle al" << std::endl;
                ss << "  movzx rax, al" << std::endl;
                ss << "  push rax" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_IF)
            {
                size_t id = *(int64_t *)&bytecode[i + 1];
                ss << "  pop rax" << std::endl;
                ss << "  test rax, rax" << std::endl;
                ss << "  jz .if_end" << id << std::endl;
                i += 9;
            }
            else if (opcode == BC_ELSE)
            {
                size_t id = *(int64_t *)&bytecode[i + 1];
                ss << "  pop rax" << std::endl;
                ss << "  test rax, rax" << std::endl;
                ss << "  jmp .if_end" << id << std::endl;
                ss << ".if_false" << id << ":" << std::endl;
                i += 9;
            }
            else if (opcode == BC_TEST_FALSE_LABEL)
            {
                size_t id = *(int64_t *)&bytecode[i + 1];
                ss << ".if_false" << id << ":" << std::endl;
                i += 9;
            }
            else if (opcode == BC_TEST_END_END_LABEL)
            {
                size_t id = *(int64_t *)&bytecode[i + 1];
                ss << ".if_end" << id << ":" << std::endl;
                i += 9;
            }
            else if (opcode == BC_HALT)
            {
                ss << "  movabs rax, 60" << std::endl;
                ss << "  xor rdi, rdi" << std::endl;
                ss << "  syscall" << std::endl << std::endl;
                i += 1;
            }
            else if (opcode == BC_SYSCALL)
            {
                auto syscall = bytecode[i + 1];
                if (syscall == BC_SYS_EXIT)
                {
                    ss << "  movabs rax, 60" << std::endl;
                    ss << "  pop rdi" << std::endl;
                    ss << "  syscall" << std::endl;
                }
                else if (syscall == BC_SYS_WRITE_INT)
                {
                    include_int_to_str_code = true;
                    std::cout << "sys_call_write_int" << std::endl;
                    //push top of stack to stack
                    ss << "  mov rax, [rsp]" << std::endl;
                    ss << "  push rax" << std::endl;
                    ss << "  pop rdi" << std::endl;
                    ss << "  call int_to_str" << std::endl;
                    // ss << "  sub rsp, 8" << std::endl;
                    ss << "  mov rax, 1" << std::endl;
                    ss << "  mov rdi, 1" << std::endl;
                    ss << "  lea rsi, [numbuf]" << std::endl;
                    ss << "  mov rdx, 20" << std::endl;
                    ss << "  syscall" << std::endl;
                    // ss << "  pop rax" << std::endl;
                    //clear numbuf 20 bytes to 0
                    ss << "  lea rdi, [numbuf]" << std::endl;
                    ss << "  mov rcx, 20" << std::endl;
                    ss << "  mov al, 0" << std::endl;
                    ss << "  rep stosb" << std::endl << std::endl;



                }
                i += 2;
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
            ss << "numbuf:\n .skip 64" << std::endl;
            ss << "tempbuf:\n .skip 2024"<< std::endl;
            ss << ".section .data" << std::endl;
            ss << "  msg: .asciz  \"Hello, World!\"" << std::endl;
            ss << std::endl;
            ss << ".section .text" << std::endl;
            ss << "  .globl main" << std::endl;
            ss << "main:" << std::endl;
            ss << asm_code << std::endl;
            ss << "  movabs rax, 60" << std::endl;
            ss << "  pop rdi" << std::endl;
            ss << "  syscall" << std::endl;
            if (include_int_to_str_code)
            {
                ss << int_to_str_asm_code << std::endl;
            }
            return ss.str();
        }
        return asm_code;
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
    size_t codition_count = 0;

    void add_var(std::string_view name, size_t type, size_t size)
    {
        variable_t var;
        var.type = type;
        var.offset = stack_size;
        var.size = size;
        vars.push_back(var);
        var_map[name] = vars.size() - 1;
    }

    size_t create_condition_id()
    {
        return codition_count++;
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
        scopes.push_back(stack_size);
    }
    size_t pop_scope()
    {
        size_t offset = stack_size - scopes.back();
        stack_size = scopes.back();
        scopes.pop_back();
        return offset;       
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
            size_t offset = ctx.pop_scope();
            if (offset > 0)
            {
                data.bytecode.push_back(BC_SHRINK_STACK);
                data.push_int(offset, false);
            }
        } else if (ast.type == AST_IF) {
            generate_code_if(ast, data, ctx);
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
            auto var = ctx.get_var(ast.value);
            if (!var)
            {
                throw utils::error_t(ast.line, "Undefined variable: " + std::string(ast.value));
            }
            if (var->type == VAR_INT)
            {
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
                generate_code_expr(ast.children[1].children[0], data, ctx);
                data.bytecode.push_back(BC_SYSCALL);
                data.bytecode.push_back(BC_SYS_WRITE_INT);
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

        static const char *comparison_ops[] = {"==", "!=", ">", "<", ">=", "<="};
        static BytecodeOp comparison_ops_code[] = {BC_EQ_INT_INT, BC_NE_INT_INT, BC_GT_INT_INT, BC_LT_INT_INT, BC_GE_INT_INT, BC_LE_INT_INT};

        for (size_t i = 0; i < 6; i++)
        {
            if (ast.value == comparison_ops[i])
            {
                data.bytecode.push_back(comparison_ops_code[i]);
                ctx.stack_size -= sizeof(int64_t);
                return AST_INT;
            }
        }

        throw utils::error_t(ast.line, std::string("Unknown binary operator: ") + std::string(ast.value));
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
        auto var = ctx.get_var(lhs.value);
        if (!var)
        {
            if (rhs == AST_INT)
            {
                ctx.add_var(lhs.value, rhs, sizeof(int64_t));
            }
        }
        else
        {
            if (var->type != rhs)
            {
                throw utils::error_t(ast.line, "Type mismatch in assignment");
            }
            data.bytecode.push_back(BC_SET_INT);
            data.push_int(ctx.stack_size - var->offset, false);
        }
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

    size_t generate_code_if(const ast_t &ast, program_data_t &data, var_context_t &ctx)
    {
        size_t id = ctx.create_condition_id();
        check_ast_type(ast, AST_IF);
        auto &cond = ast.children[0];
        auto &stmt = ast.children[1];
        generate_code_expr(cond, data, ctx);

        if(ast.children.size() == 3) {
            data.bytecode.push_back(BC_IF);
            data.push_int(id, false);
            ctx.stack_size -= sizeof(int64_t);
            generate_code_stmt(stmt, data, ctx);
            data.bytecode.push_back(BC_TEST_FALSE_LABEL);
            auto &else_stmt = ast.children[2];
            generate_code_stmt(else_stmt, data, ctx);
            data.bytecode.push_back(BC_TEST_END_END_LABEL);
            data.push_int(id, false);
        }
        else {
            data.bytecode.push_back(BC_IF);
            data.push_int(id, false);
            ctx.stack_size -= sizeof(int64_t);
            generate_code_stmt(stmt, data, ctx);
            data.bytecode.push_back(BC_TEST_END_END_LABEL);
            data.push_int(id, false);
        }




        // data.bytecode.push_back(BC_IF);
        // data.push_int(id, false);
        // ctx.stack_size -= sizeof(int64_t);
        // if(stmt.type == AST_BLOCK) {
        //     generate_code_stmt(stmt, data, ctx);
        //     data.bytecode.push_back(BC_TEST_FALSE_LABEL);
        //     data.push_int(id, false);
        // }
        // if(ast.children.size() == 3) {
        //     bool is_else_if = ast.children[2].type == AST_IF;
        //     if(is_else_if) {
        //         data.bytecode.push_back(BC_ELSE);
        //         data.push_int(id, false);
        //         generate_code_if(ast.children[2], data, ctx);
        //     } else {
        //         auto &else_stmt = ast.children[2];
        //         generate_code_stmt(else_stmt, data, ctx);
        //         data.bytecode.push_back(BC_TEST_END_END_LABEL);
        //         data.push_int(id, false);
        //     }
        // }

        return 0;
    }
};