#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <string_view>

namespace utils
{
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

    void write_string_to_file(const std::string &path, const std::string &content) {
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << path << std::endl;
            return;
        }
        file << content;
        file.close();
    }

    class error_t : public std::runtime_error
    {
    public:
        error_t(size_t line, std::string msg) : std::runtime_error(format_message(line, msg)) {}

        std::string format_message(size_t line, std::string msg) {
            return "Error:: line " + std::to_string(line) + ": " + msg;
        }
    };

    void unexpected_token(size_t line, std::string token, std::string expected = "") {
        std::string msg = "Unexpected token: " + token;
        if (!expected.empty()) msg += ", expected: " + expected;
        throw error_t(line, msg);
    }

    void unexpected_token(size_t line, std::string_view token, std::string_view expected = "") {
        std::string msg = "Unexpected token: " + std::string(token);
        if (!expected.empty()) msg += ", expected: " + std::string(expected);
        throw error_t(line, msg);
    }



};