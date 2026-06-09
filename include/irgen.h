#pragma once
#include <vector>
#include <string>
#include "ast.h"
#include "symtab.h"
#include "quadruple.h"

//类2：IRGenerator（四元式生成器）
//文件	irgen.h / irgen.cpp
//职责	递归遍历 AST，语法制导翻译生成四元式序列
class IRGenerator {
public:
	IRGenerator();
	std::vector<Quadruple> generate(ASTNode* ast, SymTab& symtab);  // 主接口：AST+符号表 → 四元式序列
	void printQuads(const std::vector<Quadruple>& quads, std::ostream& out);  // 输出四元式
private:
	int labelCounter;              // 跳转标签计数器 L1, L2, ...
	std::string newLabel();             // 生成新标签
	void genIfElse(ASTNode* node, std::vector<Quadruple>& quads, SymTab& symtab);
	void genCond(ASTNode* node, std::vector<Quadruple>& quads, std::string trueLabel, std::string falseLabel, SymTab& symtab);
	void genAssign(ASTNode* node, std::vector<Quadruple>& quads, SymTab& symtab);
};
