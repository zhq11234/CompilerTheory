#pragma once

#include <string>

struct Token {
	int type;       // 种别编码：1=关键字  2=标识符  3=整数  4=运算符  5=分隔符
	std::string value;   // 原始字符串，如 "if"、"x"、"42"、">"、";"
	int line;       // 行号，用于错误报告定位
};
