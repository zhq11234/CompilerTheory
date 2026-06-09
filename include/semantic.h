#pragma	once

#include <vector>
#include <string>
#include "ast.h"
#include "symtab.h"

//类1：SemanticAnalyzer（语义分析器）
//文件	semantic.h / semantic.cpp
//职责	递归遍历 AST，检查变量是否已声明、类型是否匹配
class SemanticAnalyzer {
public:
	SemanticAnalyzer();
	// 主接口：遍历AST+符号表，做语义检查
	void analyze(ASTNode* ast, SymTab& symtab);
	// 返回语义错误列表
	std::vector<std::string> getErrors();
	// 输出错误信息
	void printErrors(std::ostream& out);
private:
	std::vector<std::string> errors;
	// 递归分发
	void checkNode(ASTNode* node, SymTab& symtab);
	// 检查 E→id1 N id2：两边 id 是否已声明
	void checkCondExpr(ASTNode* node, SymTab& symtab);
	// 检查 P→id N NUM：id 是否已声明，类型是否匹配（int←int）
	void checkAssignExpr(ASTNode* node, SymTab& symtab);
};
