#include <iostream>
#include <fstream>
#include <string>
#include "lexing.hpp"
#include "parsing.hpp"
#include "codegen.hpp"
#include "utils.hpp"





int main(int argc, char **argv) {
    if(argc == 1) {
        std::cerr << "No input file provided" << std::endl;
        return 1;
    }

    std::string src = utils::read_file_as_string(argv[1]);

    lexer_t lexer;
    std::cout << "> Lexing..." << std::endl;
    try {
        lexer.init(src.c_str(), src.size());
        while (lexer.has_token())
        {
            auto token = lexer.next_token();
            std::cout << "token: " << token.value
                        << " line: " << lexer.line << std::endl;
        }
        std::cout << "> Lexed" << std::endl;
    } catch (utils::error_t & e) {
        std::cerr << e.what() << std::endl;
    }
  
    parser_t parser;
    std::cout << "> Parsing..." << std::endl;
    try {
        ast_t ast = parser.parse(src.c_str(), src.size());
        std::cout << "> Parsed AST" << std::endl;
        std::cout << "ast node size: " << sizeof(ast) << std::endl;
        parser.print(ast);

        std::cout << "> Generating code..." << std::endl;
        code_generator_t codegen;
        var_context_t ctx;
        auto program = codegen.gen_program(ast);
        std::cout << "> Generated code" << std::endl;
        std::cout << "program bytecode size: " << program.bytecode.size() << std::endl;
        std::cout << "bytecode: " << std::endl;
        std::string asm_code = program.asm_str(true);
        utils::write_string_to_file("asm_code.s", asm_code);
        // compile using gcc
        std::string cmd = "gcc -no-pie -o out asm_code.s";
        system(cmd.c_str());
        std::cout << "> Compiled code" << std::endl;
    
    } catch (utils::error_t & e) {
        std::cerr << e.what() << std::endl;
    }
    
    

    return 0;
}