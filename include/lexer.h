#pragma once

#include <vector>
#include <string>
#include "token.h"
#include "symtab.h"

class Lexer {
public:
    Lexer();
    std::vector<Token> analyze(const std::string& source, const std::string& srcPath);
    void printTokens(const std::vector<Token>& tokens, std::ostream& out);

    // 获取符号表
    const SymTab& getSymbolTable() const { return symTab; }
    std::string getSourceName() const { return sourceName; }

private:
    int line;
    int pos;
    std::string source;
    std::string sourceName;
    SymTab symTab;

    char peek();
    char advance();
    void skipWhitespace();
    void skipComment();
    Token scanToken();
    Token scanIdOrKeyword();
    Token scanNumber();
    Token scanOperator();
    Token scanDelimiter();

    // JSON 输出方法（私有）
    void writeTokensToJSON(const std::vector<Token>& tokens, const std::string& filename);
};