#pragma once

#include "ast.h"
#include <vector>
#include <map>
#include <stack>

class LRAnalysisTable {
public:
	LRAnalysisTable();
	void buildTable();
	int getAction(int state, int tokenType);
	int getGoto(int state, int nonTerminal);
	std::string getActionString(int state, int tokenType);
	std::vector<std::string> getItemSets();
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
	ASTNode* parse(const std::vector<Token>& tokens);
	void printAST(ASTNode* root, std::ostream& out, int depth = 0);
	std::string getProcessLog();
	std::vector<std::string> getErrors();
private:
	LRAnalysisTable table;
	std::stack<int> stateStack;
	std::stack<ASTNode*> nodeStack;
	std::string processLog;
	std::vector<std::string> errors;
	ASTNode* reduce(int productionIndex);
};
