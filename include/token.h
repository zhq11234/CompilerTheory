#pragma once

#include <string>

enum TokenType {
	TOKEN_KEYWORD    = 1,
	TOKEN_IDENTIFIER = 2,
	TOKEN_NUMBER     = 3,
	TOKEN_OPERATOR   = 4,
	TOKEN_DELIMITER  = 5
};

struct Token {
	int type;       // TokenType enum
	std::string value;
	int line;
};
