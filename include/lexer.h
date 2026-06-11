#pragma once

#include <vector>
#include <string>
#include "token.h"

//类1：Lexer（词法分析器）
//文件	lexer.h / lexer.cpp
//职责	读入源代码字符串，输出 vector<Token>
class Lexer {
public:
	Lexer();
	std::vector<Token> analyze(const std::string& source);  // 主接口：源码 → Token流
	void printTokens(const std::vector<Token>& tokens, std::ostream& out);  // 输出Token流
	// lexer.h
	void writeTokensToJSON(const std::vector<Token>& tokens,
		const std::string& filename,
		const std::string& sourceName);
private:
	int line;       // 当前行号
	int pos;        // 当前字符位置
	std::string source;  // 源代码字符串
	char peek();    // 预读当前字符（不消费）
	char advance(); // 消费当前字符并前进
	void skipWhitespace();      // 跳过空白字符
	void skipComment();      // 跳过注释
	Token scanToken();          // 扫描下一个 Token
	Token scanIdOrKeyword();    // 识别标识符或关键字
	Token scanNumber();         // 识别无符号整数
	Token scanOperator();       // 识别运算符 + - * / < > <= >= != :=
	Token scanDelimiter();       // 识别界符
};
