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


    
    
    } catch (utils::error_t & e) {
        std::cerr << e.what() << std::endl;
    }
    
    

    return 0;
}