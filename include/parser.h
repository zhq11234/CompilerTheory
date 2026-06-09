#pragma once

#include "ast.h"
#include <vector>
#include <map>
#include <stack>

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
	std::map<std::pair<int, int>, int> actionTable;  // (状态, 终结符) → 动作
	std::map<std::pair<int, int>, int> gotoTable;    // (状态, 非终结符) → 目标状态
	std::vector<int> rhsLength;                 // 各产生式右部长度（归约时弹出状态数）
	std::vector<int> lhsNonTerminal;            // 各产生式左部非终结符编号
	std::vector<std::string> itemSets;               // 项目集规范族描述
};

//类3：Parser（LR(1)语法分析器）
//文件	parser.h / parser.cpp
//职责	读入 Token 流，LR(1)移进 - 归约，同步构建 AST
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
