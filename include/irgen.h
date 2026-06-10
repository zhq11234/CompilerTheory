#pragma once
#include <vector>
#include <string>
#include "ast.h"
#include "symtab.h"
#include "quadruple.h"

class IRGenerator {
public:
	IRGenerator();
	std::vector<Quadruple> generate(ASTNode* ast, SymTab& symtab);
	void printQuads(const std::vector<Quadruple>& quads, std::ostream& out);
private:
	int labelCounter;
	std::string newLabel();
	void genIfElse(ASTNode* node, std::vector<Quadruple>& quads, SymTab& symtab);
	void genCond(ASTNode* node, std::vector<Quadruple>& quads, std::string trueLabel, std::string falseLabel, SymTab& symtab);
	void genAssign(ASTNode* node, std::vector<Quadruple>& quads, SymTab& symtab);
};
