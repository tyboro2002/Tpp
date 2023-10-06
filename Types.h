#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

#define NewLine "\n"

enum class TokenType {
    _exit,
    int_lit,
    semi,
    open_Paren,
    closed_Paren,
    say,
    shout,
    string_lit,
    open_Quote,
    closed_Quote,
    _return,
    identifier,
    equals,
    open_curly,
    closed_curly,
    _if,
    _else,
    _elif,
    _import,
    request,
    test_equal,
    test_not_equal,
    test_equal_greater,
    test_equal_smaller,
    test_greater,
    test_smaller,
    comma,
    var_dump,
    //tppinp
};

struct Token {
    TokenType type;
    std::optional<std::string> value{};
};

// Function to convert TokenType to string for printing
std::string TokenTypeToString(TokenType type);

// Define the output stream operator for Token
std::ostream& operator<<(std::ostream& os, const Token& token);