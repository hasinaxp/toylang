#include <iostream>
#include <fstream>
#include <string>
#include "lexing.hpp"
#include "parsing.hpp"

std::string read_file_as_string(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::cout << content << std::endl;
    file.close();
    return content;
}




int main(int argc, char **argv) {
    if(argc == 1) {
        std::cerr << "No input file provided" << std::endl;
        return 1;
    }

    std::string src = read_file_as_string(argv[1]);
  
    parser_t parser;
    std::cout << "Parsing..." << std::endl;
    ast_t ast = parser.parse(src.c_str(), src.size());
    std::cout << "Parsed AST" << std::endl;
    parser.print(ast);

    std::cout << "ast node size: " << sizeof(ast) << std::endl; 
    

    return 0;
}