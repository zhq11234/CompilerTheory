#include "irgen.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <iostream>

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

static std::string escapeJSON(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
}

// 静态函数：将四元式序列写入 JSON 文件（符合规范）
static void writeJSON(const std::string& filename, const std::vector<Quadruple>& quads) {
    std::ofstream out(filename);
    if (!out.is_open())
        return;

    out << "{\n";
    out << "  \"source\": \"source.src\",\n";
    out << "  \"timestamp\": \"" << getCurrentISO8601Time() << "\",\n";
    out << "  \"quads\": [\n";
    for (size_t i = 0; i < quads.size(); ++i) {
        const auto& q = quads[i];
        out << "    {\"op\": \"" << escapeJSON(q.op) << "\", "
            << "\"arg1\": \"" << escapeJSON(q.arg1.empty() ? "-" : q.arg1) << "\", "
            << "\"arg2\": \"" << escapeJSON(q.arg2.empty() ? "-" : q.arg2) << "\", "
            << "\"result\": \"" << escapeJSON(q.result.empty() ? "-" : q.result) << "\"}";
        if (i != quads.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ],\n";
    out << "  \"errors\": []\n";   // 规范要求 errors 字段，当前无错误
    out << "}\n";
}

static void resolveLabels(std::vector<Quadruple>& quads, const std::map<std::string, size_t>& labelMap) {
    for (auto& q : quads) {
        if (q.op == "j" || q.op == "j>" || q.op == "j<" || q.op == "j=") {
            auto it = labelMap.find(q.result);
            if (it != labelMap.end()) {
                q.result = std::to_string(it->second);
            }
        }
    }
}


// ==================== IRGenerator 类方法实现 ====================

IRGenerator::IRGenerator() : labelCounter(0) {}

std::string IRGenerator::newLabel() {
    return "L" + std::to_string(++labelCounter);
}

std::vector<Quadruple> IRGenerator::generate(ASTNode* ast, SymTab& symtab, const std::string& srcPath) {
    std::vector<Quadruple> quads;
    labelCounter = 0;
    labelMap.clear();
    if (ast && ast->type == NODE_IF) {
        genIfElse(ast, quads, symtab);
    }
    resolveLabels(quads, labelMap);   // 替换标签为数字索引
    std::string outFile = srcPath;
    if (outFile.size() >= 4 && outFile.compare(outFile.size() - 4, 4, ".src") == 0) {
        outFile.replace(outFile.size() - 4, 4, "_ir.json");
    }
    else {
        outFile += "_ir.json";
    }
    writeJSON(outFile, quads);
    return quads;
}

void IRGenerator::printQuads(const std::vector<Quadruple>& quads, std::ostream& out) {
    for (size_t i = 0; i < quads.size(); ++i) {
        const auto& q = quads[i];
        out << i << ": (" << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << ")\n";
    }
}

void IRGenerator::genIfElse(ASTNode* node, std::vector<Quadruple>& quads, SymTab& symtab) {
    ASTNode* cond = node->left;
    ASTNode* thenStmt = node->thenBranch;
    ASTNode* elseStmt = node->elseBranch;

    std::string trueLabel = newLabel();
    std::string falseLabel = newLabel();
    std::string endLabel = newLabel();

    // 1. 生成条件跳转（目标 trueLabel）
    genCond(cond, quads, trueLabel, falseLabel, symtab);

    // 2. 无条件跳转到 else（目标 falseLabel）
    quads.push_back({ "j", "", "", falseLabel });

    // 3. 记录 trueLabel 指向 then 分支的第一条指令（即当前位置）
    labelMap[trueLabel] = quads.size();

    // 4. 生成 then 分支
    if (thenStmt) {
        if (thenStmt->type == NODE_ASSIGN)
            genAssign(thenStmt, quads, symtab);
    }
    quads.push_back({ "j", "", "", endLabel });

    // 5. 记录 falseLabel 指向 else 分支的第一条指令
    labelMap[falseLabel] = quads.size();

    // 6. 生成 else 分支
    if (elseStmt) {
        if (elseStmt->type == NODE_ASSIGN)
            genAssign(elseStmt, quads, symtab);
    }

    // 7. 记录 endLabel 指向整个 if-else 结束后的位置（即当前 quads 大小）
    labelMap[endLabel] = quads.size();
}

void IRGenerator::genCond(ASTNode* node, std::vector<Quadruple>& quads,
    std::string trueLabel, std::string falseLabel, SymTab& symtab) {
    /*(void)falseLabel;*/
    if (node->type != NODE_COND) return;
    std::string left = node->left->token->value;
    std::string right = node->right->token->value;
    std::string op = node->op;
    if (op == ">")      quads.push_back({ "j>", left, right, trueLabel });
    else if (op == "<") quads.push_back({ "j<", left, right, trueLabel });
    else if (op == "=") quads.push_back({ "j=", left, right, trueLabel });
    else                quads.push_back({ "j=", left, right, trueLabel });
}

void IRGenerator::genAssign(ASTNode* node, std::vector<Quadruple>& quads, SymTab& symtab) {
    if (node->type != NODE_ASSIGN) return;
    if (node->op != "=") return;
    std::string left = node->left->token->value;
    std::string right = node->right->token->value;
    quads.push_back({ "=", right, "", left });
}