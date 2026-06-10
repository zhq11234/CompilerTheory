#pragma once

#include <vector>
#include <string>
#include "ast.h"
#include "symtab.h"

class SemanticAnalyzer {
public:
	SemanticAnalyzer();
	void analyze(ASTNode* ast, SymTab& symtab);
	std::vector<std::string> getErrors();
	void printErrors(std::ostream& out);
private:
	std::vector<std::string> errors;
	void checkNode(ASTNode* node, SymTab& symtab);
	void checkCondExpr(ASTNode* node, SymTab& symtab);
	void checkAssignExpr(ASTNode* node, SymTab& symtab);
};
