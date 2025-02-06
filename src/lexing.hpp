#pragma once
#include <stdint.h>
#include <string_view>
#include <string.h>
#include <ctype.h>


enum LexTokenType {
LEX_TOKEN_EOF = 0,
LEX_TOKEN_ID,
LEX_TOKEN_INT ,
LEX_TOKEN_FLOAT,
LEX_TOKEN_HEX ,
LEX_TOKEN_STR ,
LEX_TOKEN_OP,
LEX_TOKEN_NEWLINE
};


struct lex_token_t {
    LexTokenType type; 
    std::string_view value;
    lex_token_t(LexTokenType type, std::string_view value) : type(type), value(value) {}
};

const char * lex_operators = "+-*/%&|:?^~<>=,;(){}[].!";
const char * lex_operators_single = "~,;(){}[]";


struct lexer_t {
    char * cur = nullptr;
    size_t line = 1;
    size_t len = 0;

    void init(const char * src, size_t length) { cur = (char *)src; line = 1; len = length; }


    void skip_whitespace() { while (*cur == ' ' || *cur == '\t') cur++; }
    void skip_newline() { while (*cur == '\n') { cur++; line++; } }
    bool is_operator(char c) { return strchr(lex_operators, c) != nullptr; }

    bool has_token() { return *cur != 0; }


    lex_token_t next_token() {
        if (*cur == 0) return lex_token_t(LEX_TOKEN_EOF, "");
        skip_whitespace();
        
        if(*cur == '\n') {
            skip_newline();
            return lex_token_t(LEX_TOKEN_NEWLINE, "");
        }
        if(*cur == '#') {
            while(*cur != '\n' && *cur != 0) cur++;
            return next_token();
        }
        if(is_operator(*cur)) return read_operator();
        if(isdigit(*cur)) return read_number();
        if(*cur == '"') return read_string();
        if(isalpha(*cur) || *cur == '_') return read_id();
        return lex_token_t(LEX_TOKEN_EOF, "");
    }

    lex_token_t peek_token() {
        char * start = cur;
        lex_token_t token = next_token();
        cur = start;
        return token;
    }

    lex_token_t read_operator()
    {
        char * start = cur;
        if (strchr(lex_operators_single, *cur)) {
            cur++;
            return lex_token_t(LEX_TOKEN_OP, std::string_view(start, 1));
        }
        while (*cur != '\0' && is_operator(*cur)) cur++;
        return lex_token_t(LEX_TOKEN_OP, std::string_view(start, cur - start));
    }

    lex_token_t read_number()
    {
        char * start = cur;
        if (*cur == '0' && (cur[1] == 'x' || cur[1] == 'X')) {
            cur += 2;
            while (isxdigit(*cur)) cur++;
            return lex_token_t(LEX_TOKEN_HEX, std::string_view(start, cur - start));
        }
        while (isdigit(*cur)) cur++;
        if (*cur == '.' || *cur == 'e' || *cur == 'E') {
            cur++;
            while (isdigit(*cur)) cur++;
            return lex_token_t(LEX_TOKEN_FLOAT, std::string_view(start, cur - start));
        }
        return lex_token_t(LEX_TOKEN_INT, std::string_view(start, cur - start));
    }

    lex_token_t read_string()
    {
        char * start = cur;
        cur++;
        while (*cur && *cur != '"') {
            if (*cur == '\\') cur++;
            cur++;
        }
        cur++;
        return lex_token_t(LEX_TOKEN_STR, std::string_view(start, cur - start));
    }

    lex_token_t read_id()
    {
        char * start = cur;
        while (isalnum(*cur) || *cur == '_') cur++;
        return lex_token_t(LEX_TOKEN_ID, std::string_view(start, cur - start));
    }

};

