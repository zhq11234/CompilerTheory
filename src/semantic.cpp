// semantic.cpp
#include "semantic.h"
#include "symtab.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

// 辅助函数：获取当前时间的 ISO 8601 格式字符串
static std::string getCurrentISO8601Time() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
#if defined(_MSC_VER)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

// 辅助函数：转义 JSON 字符串中的特殊字符
static std::string escapeJSON(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else result += c;
    }
    return result;
}

// 静态函数：将语义分析结果写入 JSON 文件
static void writeJSONToFile(const std::string& filename, SymTab& symtab,
    const std::vector<std::string>& errors) {
    std::ofstream out(filename);
    if (!out.is_open()) return;

    out << "{\n";
    out << "  \"source\": \"source.src\",\n";
    out << "  \"timestamp\": \"" << getCurrentISO8601Time() << "\",\n";

    // 输出符号表
    out << "  \"symtab\": [\n";
    std::vector<Symbol> symbols = symtab.getAllSymbols();
    for (size_t i = 0; i < symbols.size(); ++i) {
        const auto& sym = symbols[i];
        out << "    {\"name\": \"" << escapeJSON(sym.name) << "\", "
            << "\"type\": \"" << escapeJSON(sym.type) << "\", "
            << "\"scope\": " << sym.scope << ", "
            << "\"line\": " << sym.line << "}";
        if (i != symbols.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    // 输出错误列表
    out << "  \"errors\": [\n";
    for (size_t i = 0; i < errors.size(); ++i) {
        out << "    \"" << escapeJSON(errors[i]) << "\"";
        if (i != errors.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
}

// 构造函数
SemanticAnalyzer::SemanticAnalyzer() {}

// 主入口：清空错误，遍历 AST，最后输出 JSON 文件
void SemanticAnalyzer::analyze(ASTNode* ast, SymTab& symtab, const std::string& srcPath) {
    errors.clear();
    if (ast) {
        checkNode(ast, symtab);
    }
    // 调用静态函数输出 JSON
    writeJSONToFile("srcPath/test_semantic.json", symtab, errors);
}

// 获取错误列表
std::vector<std::string> SemanticAnalyzer::getErrors() {
    return errors;
}

// 打印错误到输出流
void SemanticAnalyzer::printErrors(std::ostream& out) {
    for (const auto& err : errors) {
        out << err << std::endl;
    }
}

// 递归分发检查
void SemanticAnalyzer::checkNode(ASTNode* node, SymTab& symtab) {
    if (!node) return;

    switch (node->type) {
    case NODE_IF:
        checkNode(node->left, symtab);
        checkNode(node->thenBranch, symtab);
        checkNode(node->elseBranch, symtab);
        break;
    case NODE_COND:
        checkCondExpr(node, symtab);
        break;
    case NODE_ASSIGN:
        checkAssignExpr(node, symtab);
        break;
    case NODE_ID:
    {
        std::string name = node->token->value;
        if (!symtab.isDeclared(name)) {
            symtab.insert(name, "int", 1,node->token->line);
        }
    }
    break;
    case NODE_NUM:
        // 数字节点无需检查
        break;
    default:
        break;
    }
}

// 检查条件表达式 E → id1 N id2
void SemanticAnalyzer::checkCondExpr(ASTNode* node, SymTab& symtab) {
    ASTNode* leftId = node->left;
    ASTNode* rightId = node->right;

    if (leftId && leftId->type == NODE_ID) {
        std::string name = leftId->token->value;
        if (!symtab.isDeclared(name)) {
            symtab.insert(name, "int", 1,leftId->token->line);
        }
    }
    else {
        errors.push_back("行 " + std::to_string(node->token ? node->token->line : 0) +
            ": 条件表达式左边不是标识符");
    }

    if (rightId && rightId->type == NODE_ID) {
        std::string name = rightId->token->value;
        if (!symtab.isDeclared(name)) {
            symtab.insert(name, "int", 1,rightId->token->line);
        }
    }
    else {
        errors.push_back("行 " + std::to_string(node->token ? node->token->line : 0) +
            ": 条件表达式右边不是标识符");
    }
}

// 检查赋值/比较语句 P → id N NUM
void SemanticAnalyzer::checkAssignExpr(ASTNode* node, SymTab& symtab) {
    ASTNode* leftId = node->left;
    ASTNode* rightNum = node->right;

    bool isAssign = (node->op == "=");
    std::string stmtType = isAssign ? "赋值语句" : "比较语句";

    if (leftId && leftId->type == NODE_ID) {
        std::string name = leftId->token->value;
        if (!symtab.isDeclared(name)) {
            symtab.insert(name, "int", 1,leftId->token->line);
        }
    }
    else {
        errors.push_back("行 " + std::to_string(node->token ? node->token->line : 0) +
            ": " + stmtType + "左边不是标识符");
    }

    if (rightNum && rightNum->type == NODE_NUM) {
        // 正确
    }
    else {
        errors.push_back("行 " + std::to_string(node->token ? node->token->line : 0) +
            ": " + stmtType + "右边不是整数常量");
    }

    if (node->op != "=" && node->op != ">" && node->op != "<") {
        errors.push_back("行 " + std::to_string(node->token ? node->token->line : 0) +
            ": " + stmtType + "存在未知的操作符 '" + node->op + "'");
    }
}