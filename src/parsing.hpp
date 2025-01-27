#pragma once
#include <vector>
#include <string>
#include <string_view>
#include "lexing.hpp"

enum ASTType {
    AST_INVALID = 0,
    AST_INT,
    AST_FLOAT,
    AST_STR,
    AST_ID,
    AST_OP,
    AST_UNARY_OP,
    AST_BINARY_OP,
    AST_ASSIGN,
    AST_VAR_DECL,
    AST_VAR_ASSIGN,
    AST_FUNC_DECL,
    AST_FUNC_CALL,
    AST_BLOCK,
    AST_IF,
    AST_WHILE,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_EXPR_STMT,
    AST_STMT_LIST,
    AST_FUNC_PARAMS,
    AST_VAR_LIST,
    AST_ARG_LIST,
    AST_EXPR_LIST,
    AST_PROGRAM,
    AST_DOT_ACCESS,
    AST_BRACKET_ACCESS,
};

constexpr const char * ASTTypeNames[] = {
    "Invalid",
    "Int",
    "Float",
    "Str",
    "Id",
    "Op",
    "UnaryOp",
    "BinaryOp",
    "Assign",
    "VarDecl",
    "VarAssign",
    "FuncDecl",
    "FuncCall",
    "Block",
    "If",
    "While",
    "Return",
    "Break",
    "Continue",
    "ExprStmt",
    "StmtList",
    "FuncParams",
    "VarList",
    "ArgList",
    "ExprList",
    "Program",
    "DotAccess",
    "BracketAccess",
};


struct ast_t {
    ASTType type;
    std::string_view value;
    std::vector<ast_t> children;
};






struct parser_t {
    lexer_t lexer;

    ast_t parse(const char * code, size_t length) {
        lexer.init(code, length);
        size_t avg_token_count = length >> 2;
        std::vector<lex_token_t> tokens;
        std::vector<size_t> line_nos;
        tokens.reserve(avg_token_count);
        line_nos.reserve(avg_token_count);
        while(lexer.has_token()) {
            lex_token_t token = lexer.next_token();
            tokens.push_back(token);
            line_nos.push_back(lexer.line);
        }


        return parse_program(tokens, line_nos);
    }

    ast_t parse_program(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos) {
        ast_t program = {AST_PROGRAM, ""};
        size_t i = 0;
        while(i < tokens.size()) {
            ast_t stmt = parse_stmt(tokens, line_nos, i);
            print(stmt);
            program.children.push_back(stmt);
        }
        return program;
    }

    ast_t parse_term(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        lex_token_t& token = tokens[i];
        if(token.type == LEX_TOKEN_INT) {
            i++;
            return {AST_INT, token.value};
        }
        if(token.type == LEX_TOKEN_FLOAT) {
            i++;
            return {AST_FLOAT, token.value};
        }
        if(token.type == LEX_TOKEN_STR) {
            i++;
            return {AST_STR, token.value};
        }
        if(token.type == LEX_TOKEN_ID) {
            i++;
            return {AST_ID, token.value};
        }
        if(token.type == LEX_TOKEN_OP && token.value == "(") {
            i++;
            ast_t expr = parse_expr(tokens, line_nos, i);
            if(tokens[i].type != LEX_TOKEN_OP || tokens[i].value != ")") {
                printf("Expected ')', got %.*s\n", (int)tokens[i].value.size(), tokens[i].value.data());
                exit(1);
            }
            i++;
            return expr;
        }
        printf("Error::%zu:: Unexpected token %.*s\n", line_nos[i], (int)token.value.size(), token.value.data());
        exit(1);
    }

    ast_t parse_binary_op(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i, ast_t (parser_t::*next_level)(std::vector<lex_token_t> &, std::vector<size_t> &, size_t &), const std::vector<std::string> & ops) {
        ast_t lhs = (this->*next_level)(tokens, line_nos, i);
        while(i < tokens.size()) {
            lex_token_t token = tokens[i];
            if(token.type != LEX_TOKEN_OP || std::find(ops.begin(), ops.end(), token.value) == ops.end()) break;
            i++;
            ast_t rhs = (this->*next_level)(tokens, line_nos, i);
            lhs = {AST_BINARY_OP, token.value, {lhs, rhs}};
        }
        return lhs;
    }

    ast_t parse_func_call(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        ast_t func = parse_term(tokens, line_nos, i);
        if(i < tokens.size() && tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "(") {
            if (func.type != AST_ID) {
                printf("Error::%zu:: Expected identifier, got %s\n", line_nos[i], ASTTypeNames[func.type]);
                exit(1);
            }
            i++;
            if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == ")") {
                i++;
                return {AST_FUNC_CALL, "", {func, {AST_ARG_LIST, ""}}};
            }
            ast_t arglist = parse_arglist(tokens, line_nos, i);
            if(tokens[i].type != LEX_TOKEN_OP || tokens[i].value != ")") {
                printf("Expected ')', got %.*s\n", (int)tokens[i].value.size(), tokens[i].value.data());
                exit(1);
            }
            i++;
            return {AST_FUNC_CALL, "", {func, arglist}};
        }
        return func;
    }

    ast_t parse_member_access(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        ast_t term = parse_func_call(tokens, line_nos, i);
        while(i < tokens.size()) {
            if(i < tokens.size() && tokens[i].type == LEX_TOKEN_OP && tokens[i].value == ".") {
                i++;
                if(i >= tokens.size() || tokens[i].type != LEX_TOKEN_ID) {
                    printf("Error::%zu:: Expected identifier, got %.*s\n", line_nos[i], (int)tokens[i].value.size(), tokens[i].value.data());
                    exit(1);
                }
                ast_t member = parse_func_call(tokens, line_nos, i);
                term = {AST_DOT_ACCESS, "", {term, member}};
                i++;
            } else if(i < tokens.size() && tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "[") {
                i++;
                ast_t index = parse_expr(tokens, line_nos, i);
                if(i >= tokens.size() || tokens[i].type != LEX_TOKEN_OP || tokens[i].value != "]") {
                    printf("Error::%zu:: Expected ']', got %.*s\n", line_nos[i], (int)tokens[i].value.size(), tokens[i].value.data());
                    exit(1);
                }
                term = {AST_BRACKET_ACCESS, "", {term, index}};
                i++;
                if(i < tokens.size() && tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "(") {
                    i++;
                    if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == ")") {
                        i++;
                        term = {AST_FUNC_CALL, "", {term, {AST_ARG_LIST, ""}}};
                    } else {
                        ast_t arglist = parse_arglist(tokens, line_nos, i);
                        if(tokens[i].type != LEX_TOKEN_OP || tokens[i].value != ")") {
                            printf("Error::%zu:: Expected ')', got %.*s\n", line_nos[i], (int)tokens[i].value.size(), tokens[i].value.data());
                            exit(1);
                        }
                        i++;
                        term = {AST_FUNC_CALL, "", {term, arglist}};
                    }
                } 
            } else {
                break;
            }
        }
        return term;
    }



    ast_t parse_unary(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        lex_token_t token = tokens[i];
        if(token.type == LEX_TOKEN_OP && (token.value == "~" || token.value == "-" || token.value == "!")) {
            i++;
            ast_t term = parse_member_access(tokens, line_nos, i);
            return {AST_UNARY_OP, token.value, {term}};
        }
        return parse_member_access(tokens, line_nos, i);
    }

    ast_t parse_factor(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_unary, {"*", "/", "%"});
    }

    ast_t parse_arithmatic(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_factor, {"+", "-"});
    }

    ast_t parse_bitwise_shift(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_arithmatic, {"<<", ">>"});
    }

    ast_t parse_compare(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_bitwise_shift, {"<", ">", "<=", ">="});
    }

    ast_t parse_equality(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_compare, {"==", "!="});
    }

    ast_t parse_bitwise_and(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_equality, {"&"});
    }

    ast_t parse_bitwise_xor(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_bitwise_and, {"^"});
    }

    ast_t parse_bitwise_or(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_bitwise_xor, {"|"});
    }

    ast_t parse_logical_and(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_bitwise_or, {"&&"});
    }

    ast_t parse_logical_or(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_binary_op(tokens, line_nos, i, &parser_t::parse_logical_and, {"||"});
    }

    ast_t parse_trinary(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        ast_t cond = parse_logical_or(tokens, line_nos, i);
        if(i < tokens.size() && tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "?") {
            i++;
            ast_t true_expr = parse_expr(tokens, line_nos, i);
            if(tokens[i].type != LEX_TOKEN_OP || tokens[i].value != ":") {
                printf("Error::%zu:: Expected ':', got %.*s\n", line_nos[i], (int)tokens[i].value.size(), tokens[i].value.data());
                exit(1);
            }
            i++;
            ast_t false_expr = parse_expr(tokens, line_nos, i);
            return {AST_OP, "?", {cond, true_expr, false_expr}};
        }
        return cond;
    }

    ast_t parse_assignment(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        ast_t lhs = parse_trinary(tokens, line_nos, i);
        
        if(i < tokens.size() && tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "=") {
            if(lhs.type == AST_BINARY_OP || lhs.type == AST_UNARY_OP) {
                printf("Error::%zu:: Expected identifier, got %s\n", line_nos[i], ASTTypeNames[lhs.type]);
                exit(1);
            }
            i++;
            ast_t rhs = parse_assignment(tokens, line_nos, i);
            return {AST_ASSIGN, "=", {lhs, rhs}};
        }
        return lhs;
    }

   ast_t parse_expr(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        return parse_assignment(tokens, line_nos, i);
    }

    ast_t parse_arglist(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        ast_t arglist = {AST_ARG_LIST, ""};
        while(i < tokens.size()) {
            ast_t expr = parse_expr(tokens, line_nos, i);
            arglist.children.push_back(expr);
            if(i < tokens.size() && tokens[i].type == LEX_TOKEN_OP && tokens[i].value == ",") i++;
            else break;
        }
        return arglist;
    }

    ast_t parse_block(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        if(tokens[i].type != LEX_TOKEN_OP || tokens[i].value != "{") {
            printf("Error::%zu:: Expected '{', got %.*s\n", line_nos[i], (int)tokens[i].value.size(), tokens[i].value.data());
            exit(1);
        }
        i++;
        ast_t block = {AST_BLOCK, ""};
        while(i < tokens.size() && !(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "}")) {
            ast_t stmt = parse_stmt(tokens, line_nos, i);
            block.children.push_back(stmt);
        }
        if(i >= tokens.size() || tokens[i].type != LEX_TOKEN_OP || tokens[i].value != "}") {
            printf("Error::%zu:: Expected '}', got %.*s\n", line_nos[i], (int)tokens[i].value.size(), tokens[i].value.data());
            exit(1);
        }
        i++;
        return block;
    }


    ast_t parse_stmt(std::vector<lex_token_t> & tokens, std::vector<size_t> & line_nos, size_t & i) {
        while(tokens[i].type == LEX_TOKEN_NEWLINE) i++;
        if(tokens[i].type == LEX_TOKEN_ID) {
            if(tokens[i].value == "return") {
                i++;
                ast_t expr = parse_expr(tokens, line_nos, i);
                if(tokens[i].type == LEX_TOKEN_NEWLINE) i++;
                else if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == ";") i++;
                return {AST_RETURN, "", {expr}};
            }
            if(tokens[i].value == "break") {
                i++;
                if(tokens[i].type == LEX_TOKEN_NEWLINE) i++;
                else if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == ";") i++;
                return {AST_BREAK, ""};
            }
            if(tokens[i].value == "continue") {
                i++;
                if(tokens[i].type == LEX_TOKEN_NEWLINE) i++;
                else if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == ";") i++;
                return {AST_CONTINUE, ""};
            }
            if(tokens[i].value == "if") {
                i++;
                ast_t cond = parse_expr(tokens, line_nos, i);
                ast_t if_statement = {AST_IF, "", {cond}};
                if(tokens[i].type == LEX_TOKEN_NEWLINE) i++;
                if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "{") {
                    if_statement.children.push_back(parse_block(tokens, line_nos, i));
                } else {
                    if_statement.children.push_back(parse_stmt(tokens, line_nos, i));
                }
                while (i < tokens.size() && tokens[i].type == LEX_TOKEN_NEWLINE) i++;
                if(tokens[i].type == LEX_TOKEN_ID && tokens[i].value == "else") {
                    i++;
                    if(tokens[i].type == LEX_TOKEN_NEWLINE) i++;
                    if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "{") {
                        if_statement.children.push_back(parse_block(tokens, line_nos, i));
                    } else {
                        if_statement.children.push_back(parse_stmt(tokens, line_nos, i));
                    }
                }
                return if_statement; 
            }
            if(tokens[i].value == "while") {
                i++;
                ast_t cond = parse_expr(tokens, line_nos, i);
                ast_t while_statement = {AST_WHILE, "", {cond}};
                if(tokens[i].type == LEX_TOKEN_NEWLINE) i++;
                if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "{") {
                    while_statement.children.push_back(parse_block(tokens, line_nos, i));
                } else {
                    while_statement.children.push_back(parse_stmt(tokens, line_nos, i));
                }
                return while_statement;
            }
            ast_t expr = parse_expr(tokens, line_nos, i);
            if(tokens[i].type == LEX_TOKEN_NEWLINE) i++;
            else if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == ";") i++;
            return {AST_EXPR_STMT, "", {expr}};
        }
        if(tokens[i].type == LEX_TOKEN_OP && tokens[i].value == "{") {
            return parse_block(tokens, line_nos, i);
        }

        printf("Error::%zu:: Expected statement, got %.*s\n", line_nos[i], (int)tokens[i].value.size(), tokens[i].value.data());
        exit(1);
        return {AST_EXPR_STMT, ""};
      
    }


    


    void print(const ast_t & ast) {
        static int indent = 0;
        for(int i = 0; i < indent; i++) printf(" .");
        printf("%s: ", ASTTypeNames[ast.type]);
        printf("%.*s", (int)ast.value.size(), ast.value.data());
        if(ast.children.size() > 0) {
            printf(" {\n");
            indent++;
            for(const ast_t & child : ast.children) print(child);
            indent--;
            for(int i = 0; i < indent; i++) printf("  ");
            printf("}\n");
        } else {
            printf("\n");
        }
    }

    






};

