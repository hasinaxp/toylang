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
        auto program = codegen.gen_program(ast, ctx);
        std::cout << "> Generated code" << std::endl;
        std::cout << "program bytecode size: " << program.bytecode.size() << std::endl;
        std::cout << "bytecode: " << std::endl;
        std::string asm_code = program.asm_str(true);
        utils::write_string_to_file("asm_code.s", asm_code);
        // compile using gcc
        std::string cmd = "gcc -no-pie -o out asm_code.s";
        system(cmd.c_str());

    
    } catch (utils::error_t & e) {
        std::cerr << e.what() << std::endl;
    }
    
    

    return 0;
}