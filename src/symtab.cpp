#include "symtab.h"
#include <ostream>

SymTab::SymTab() = default;

bool SymTab::insert(const std::string& name, const std::string& type,int scope ,int line)
{
	if (lookup(name)) return false;
	table.push_back({name, type, 1, line});
	return true;
}

Symbol* SymTab::lookup(const std::string& name)
{
	for (auto& s : table)
		if (s.name == name) return &s;
	return nullptr;
}

bool SymTab::isDeclared(const std::string& name)
{
	return lookup(name) != nullptr;
}

std::vector<Symbol> SymTab::getAllSymbols()
{
	return table;
}

void SymTab::display(std::ostream& out)
{
	for (auto& s : table)
		out << s.name << " : " << s.type << " (line " << s.line << ")\n";
}
