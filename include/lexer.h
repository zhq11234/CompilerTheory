#pragma once

#include <vector>
#include <string>
#include "token.h"

class Lexer {
public:
	Lexer();
	std::vector<Token> analyze(const std::string& source);
	void printTokens(const std::vector<Token>& tokens, std::ostream& out);
private:
	int line;
	int pos;
	std::string source;
	char peek();
	char advance();
	void skipWhitespace();
	Token scanToken();
	Token scanIdOrKeyword();
	Token scanNumber();
	Token scanOperator();
};
