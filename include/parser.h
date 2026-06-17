#pragma once

#include "ast.h"
#include <vector>
#include <map>
#include <stack>
#include <string>

// 符号编号（内部使用，与词法分析器输出无关）
enum Symbol1 {
	// 终结符
	IF = 1,
	THEN = 2,
	ELSE = 3,
	ID = 4,
	NUM = 5,
	GT = 6,   // '>'
	EQ = 7,   // '='
	LT = 8,   // '<'
	EOF_ = 9,   // '#'
	// 非终结符
	S1 = 100, // S'
	S = 101,
	E = 102,
	P = 103,
	N = 104,
};

//类2：  LRAnalysisTable（LR(1)分析表）
//文件	parser.h / parser.cpp
//职责	构造并存储 LR(1) Action 表和 Goto 表，提供查表接口
class LRAnalysisTable {
public:
	LRAnalysisTable();
	void buildTable();  // 构造LR(1)项目集规范族，生成Action/Goto表
	int getAction(int state, int tokenType);  // 返回值：>0=移进目标状态 / <0=归约产生式编号 / 0=接受 / -1=出错
	int getGoto(int state, int nonTerminal);  // 返回 Goto 目标状态
	std::string getActionString(int state, int tokenType);  // 用于GUI展示分析过程
	std::vector<std::string> getItemSets();  // 返回项目集规范族（GUI展示用）
private:
	std::map<std::pair<int, int>, int> actionTable;
	std::map<std::pair<int, int>, int> gotoTable;
	std::vector<int> rhsLength;
	std::vector<int> lhsNonTerminal;
	std::vector<std::string> itemSets;
};

class Parser {
public:
	Parser();
	ASTNode* parse(const std::vector<Token>& tokens);  // 主接口：Token流 → AST根节点
	void printAST(ASTNode* root, std::ostream& out, int depth = 0);  // 输出AST
	std::string getProcessLog();  // 获取移进-归约过程日志（GUI状态栈展示用）
	std::vector<std::string> getErrors();  // 获取语法错误列表
private:
	LRAnalysisTable table;
	std::stack<int> stateStack;       // 状态栈
	std::stack<ASTNode*> nodeStack;   // AST节点栈（归约时同步出栈构建子节点）
	std::string processLog;           // 分析过程日志字符串
	std::vector<std::string> errors;       // 错误列表
	ASTNode* reduce(int productionIndex);  // 按产生式归约，从nodeStack弹出子节点构建AST
};
