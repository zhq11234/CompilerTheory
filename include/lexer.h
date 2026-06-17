#pragma once

#include <vector>
#include <string>
#include "token.h"

class Lexer {
public:
	Lexer();
	std::vector<Token> analyze(const std::string& source, const std::string& srcPath);
	void printTokens(const std::vector<Token>& tokens, std::ostream& out);
	void writeTokensToJSON(const std::vector<Token>& tokens, const std::string& filename);
private:
	int line;
	int pos;
	std::string source;
	std::string sourceName;        // 存储源文件名（不含路径）

	char peek();
	char advance();
	void skipWhitespace();
	void skipComment();
	Token scanToken();
	Token scanIdOrKeyword();
	Token scanNumber();
	Token scanOperator();
	Token scanDelimiter();
};
