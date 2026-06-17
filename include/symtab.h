#pragma once

#include <vector>
#include <string>

struct Symbol {
	std::string name;
	std::string type;
	int scope;
	int line;
};

class SymTab {
public:
	SymTab();
	bool insert(const std::string& name, const std::string& type, int scope, int line);
	Symbol* lookup(const std::string& name);
	bool isDeclared(const std::string& name);
	std::vector<Symbol> getAllSymbols();
	void display(std::ostream& out);
private:
	static std::vector<Symbol> table; //静态变量，存储符号表;; //静态变量，存储符号表;
};
