#pragma once

#include <vector>
#include <string>

struct Symbol {
	std::string name;   // 标识符名
	std::string type;   // 类型，如 "int"
	int scope;     // 作用域层级（当前文法只有一层，值为 1）
	int line;      // 声明所在行号
};

//类    SymTab（符号表）
//文件	symtab.h / symtab.cpp
//职责	维护符号表，支持插入、查找、按声明检查
class SymTab {
public:
	SymTab();
	bool insert(const std::string& name, const std::string& type, int line);  // 插入符号；重复声明返回false
	Symbol* lookup(const std::string& name);   // 查找符号；未找到返回nullptr
	bool isDeclared(const std::string& name);  // 判断标识符是否已声明
	std::vector<Symbol> getAllSymbols();       // 返回全部符号（GUI展示用）
	void display(std::ostream& out);           // 输出符号表到文件/屏幕
private:
	std::vector<Symbol> table;  // 当前文法无嵌套作用域，用 vector 即可
};
