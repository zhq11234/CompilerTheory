#pragma once

#include "ast.h"
#include <vector>
#include <map>
#include <stack>
#include <string>


// 符号编号（内部使用，与词法分析器输出无关）
namespace Symbol {
	// 终结符
	const int IF = 1;
	const int THEN = 2;
	const int ELSE = 3;
	const int ID = 4;
	const int NUM = 5;
	const int GT = 6;   // '>'
	const int EQ = 7;   // '='
	const int LT = 8;   // '<'
	const int EOF_ = 9;   // '#'
	// 非终结符
	const int S1 = 100; // S'
	const int S = 101;
	const int E = 102;
	const int P = 103;
	const int N = 104;
}


//类2：  LRAnalysisTable（LR(1)分析表）
//文件	parser.h / parser.cpp
//职责	构造并存储 LR(1) Action 表和 Goto 表，提供查表接口
class LRAnalysisTable {
public:
	LRAnalysisTable();
	void buildTable();  // 构造LR(1)项目集规范族，生成Action/Goto表
	int getAction(int state, int tokenType) const;  // 返回值：>0=移进目标状态 / <0=归约产生式编号 / 0=接受 / -1=出错
	int getGoto(int state, int nonTerminal)const;  // 返回 Goto 目标状态
	std::string getActionString(int state, int tokenType)const;  // 用于GUI展示分析过程
	std::vector<std::string> getItemSets() const { return itemSets; };  // 返回项目集规范族（GUI展示用）
	
	
	
	// 用于 Parser 归约时的辅助数据
	const std::vector<int>& getRhsLength() const { return rhsLength; }
	const std::vector<int>& getLhsNonTerminal() const { return lhsNonTerminal; }

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
	std::string getProcessLog()const;  // 获取移进-归约过程日志（GUI状态栈展示用）
	std::vector<std::string> getErrors()const;  // 获取语法错误列表

	void writeASTToJSON(ASTNode* root, const std::string& filename, const std::string& srcPath);
private:
	LRAnalysisTable table;
	std::stack<int> stateStack;       // 状态栈
	std::stack<ASTNode*> nodeStack;   // AST节点栈（归约时同步出栈构建子节点）
	std::string processLog;           // 分析过程日志字符串
	std::vector<std::string> errors;       // 错误列表

    int getSymbol(const Token& tok) const;          // Token → 内部符号编号
    ASTNode* createLeafNode(const Token& tok);      // 为终结符创建叶子节点
    ASTNode* reduce(int prodIdx);                   // 归约并构建AST节点
    void logStep(int state, int symbol, int action, const std::string& desc);
    void addError(const std::string& msg);
};
