#include "lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>


// 生成 ISO 8601 时间戳字符串
static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
#if defined(_WIN32)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

// Helper functions
static bool isKeyword(const std::string& s) {
    static const char* keywords[] = {
       "const", "var", "procedure", "begin", "end", "odd",
       "if", "then", "else", "call", "while", "do", "read", "write"
    };
    for (int i = 0; i < 14; ++i)
        if (s == keywords[i]) return true;
    return false;
}

// Check if character is a delimiter
static bool isDelimiter(char c) {
    static const char delimiters[] = { '(', ')', ',', ';', '.' };
    for (char d : delimiters)
        if (c == d) return true;
    return false;
}

// Check if string is an operator
static bool isOperator(const std::string& s) {
    static const char* operators[] = {
        "+", "-", "*", "/", "<", "<=", ">", ">=", "#", "=", ":=", "!="
    };
    for (int i = 0; i < 12; ++i)
        if (s == operators[i]) return true;
    return false;
}


// Lexer implementation
Lexer::Lexer() : line(1), pos(0) {}

char Lexer::peek() {
    if (pos >= source.size()) return '\0';
    return source[pos];
}

char Lexer::advance() {
    if (pos >= source.size()) return '\0';
    char c = source[pos++];
    if (c == '\n') ++line;
    return c;
}

void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        if (c == '\0') break;
        if (std::isspace(c)) {
            advance();
            continue;
        }
        break;
    }
}

// Skip comments: // or /* ... */ Caller confirmed current char is '/' and next is '/' or '*'
void Lexer::skipComment() {
    char c = advance();  // skip '/'
    if (peek() == '/') { // single-line comment
        advance();       // skip '/'
        while (true) {
            c = advance();
            if (c == '\0' || c == '\n') break;
        }
    }
    else if (peek() == '*') { // block comment
        advance();       // skip '*'
        while (true) {
            c = advance();
            if (c == '\0') break;
            if (c == '*' && peek() == '/') {
                advance(); // skip '/'
                break;
            }
        }
    }
}

Token Lexer::scanIdOrKeyword() {
    Token tok;
    tok.line = line;
    std::string s;
    char c = peek();
    while (std::isalnum(c)) {
        s += c;
        advance();
        c = peek();
    }
    tok.value = s;

    // Check identifier length (max 8 characters)
    if (s.size() > 8) {
        tok.type = 0;          // error
        return tok;
    }

    if (isKeyword(s)) {
        tok.type = 1;          // keyword
    }
    else {
        tok.type = 2;          // identifier
    }
    return tok;
}

Token Lexer::scanNumber() {
    Token tok;
    tok.line = line;
    std::string s;
    char c = peek();
    while (std::isdigit(c)) {
        s += c;
        advance();
        c = peek();
    }

    // Digits followed by letters => invalid (e.g. 123abc)
    if (std::isalpha(c)) {
        while (std::isalnum(c)) {
            s += c;
            advance();
            c = peek();
        }
        tok.value = s;
        tok.type = 0;          // error: invalid word
        return tok;
    }

    tok.value = s;
    if (s.size() > 8) {
        tok.type = 0;          // error: number too long
    }
    else {
        tok.type = 3;          // unsigned integer
    }
    return tok;
}

Token Lexer::scanOperator() {
    Token tok;
    tok.line = line;
    std::string op;
    char c = advance();  // read first character
    op += c;

    // Two-character operators: <= >= :=
    if ((c == '<' || c == '>' || c == ':'||c=='!') && peek() == '=') {
        op += advance();
    }

    tok.value = op;
    if (isOperator(op)) {
        tok.type = 4;          // operator
    }
    else {
        tok.type = 0;          // invalid character
    }
    return tok;
}


Token Lexer::scanDelimiter() {
    Token tok;
    tok.line = line;
    char c = advance();  // consume current delimiter
    tok.value = std::string(1, c);
    tok.type = 5;        // delimiter
    return tok;
}

Token Lexer::scanToken() {
    skipWhitespace();
    char c = peek();
    if (c == '\0') {
        Token eof;
        eof.type = 0;   // end marker
        eof.value = "";
        eof.line = line;
        return eof;
    }

    // Comment handling
    if (c == '/') {
        char next = (pos + 1 < source.size()) ? source[pos + 1] : '\0';
        if (next == '/' || next == '*') {
            skipComment();
            return scanToken();   // continue after skipping comment
        }
        // Otherwise it's division operator
        Token tok;
        tok.line = line;
        tok.value = "/";
        tok.type = 4;   // operator
        advance();      // consume '/'
        return tok;
    }

    // Identifier or keyword
    if (std::isalpha(c)) {
        return scanIdOrKeyword();
    }

    // Number
    if (std::isdigit(c)) {
        return scanNumber();
    }

    // Operator
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '#' || c == '=' ||
        c == '<' || c == '>' || c == ':') {
        return scanOperator();
    }

    // Delimiter
    if (isDelimiter(c)) {
        return scanDelimiter();
    }

    // Invalid character
    Token tok;
    tok.line = line;
    tok.value = std::string(1, c);
    tok.type = 0;       // error
    advance();
    return tok;
}

std::vector<Token> Lexer::analyze(const std::string& src, const std::string& srcPath) {
    source = src;
    pos = 0;
    line = 1;

    // 从 srcPath 中提取文件名（去掉路径）
    size_t lastSlash = srcPath.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        sourceName = srcPath.substr(lastSlash + 1);
    else
        sourceName = srcPath;

    // 清空符号表
   // symTab.clear();

    std::vector<Token> tokens;
    while (true) {
        Token t = scanToken();
        if (t.type == 0 && t.value.empty()) break;
        tokens.push_back(t);

        // 插入符号表（标识符和关键字）
        if (t.type == 1 || t.type == 2) {
            std::string symbolType = (t.type == 1) ? "Keyword" : "Identifier";
            symTab.insert(t.value, symbolType, 1, t.line);
        }
    }

    // 自动生成 JSON 文件
    // 根据源文件名生成对应的 tokens.json 路径
    std::string baseName = sourceName.substr(0, sourceName.find_last_of('.'));
    if (baseName.empty()) baseName = "test";

    // 使用 "../test/" 回到项目根目录
    std::string jsonPath = "../test/" + baseName + "_tokens.json";
    writeTokensToJSON(tokens, jsonPath);

    return tokens;
}




// 辅助函数：转义 JSON 字符串中的双引号和反斜杠
static std::string escape_json(const std::string& s) {
    std::ostringstream oss;
    for (char c : s) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b";  break;
            case '\f': oss << "\\f";  break;
            case '\n': oss << "\\n";  break;
            case '\r': oss << "\\r";  break;
            case '\t': oss << "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    oss << c;
                }
                break;
        }
    }
    return oss.str();
}

// 错误信息描述（英文版，避免乱码）
static std::string tokenDescription(const Token& tok) {
    if (tok.type == 0) {
        if (tok.value.empty()) return "EOF";
        if (tok.value.size() > 8 && std::isdigit(tok.value[0])) {
            bool allDigit = true;
            for (char ch : tok.value) if (!std::isdigit(ch)) { allDigit = false; break; }
            if (allDigit) return "Number too long (>8 digits)";
        }
        if (tok.value.size() > 8 && std::isalpha(tok.value[0])) {
            return "Identifier too long (>8 chars)";
        }
        if (!tok.value.empty() && std::isdigit(tok.value[0])) {
            for (char ch : tok.value) if (std::isalpha(ch)) return "Illegal word (digit+letter)";
        }
        return "Illegal character";
    }
    switch (tok.type) {
    case 1: return "Keyword";
    case 2: return "Identifier";
    case 3: return "Number";
    case 4: return "Operator";
    case 5: return "Delimiter";
    default: return "Unknown";
    }
}

void Lexer::writeTokensToJSON(const std::vector<Token>& tokens, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error: cannot open file " << filename << " for writing.\n";
        return;
    }

    std::vector<Token> goodTokens, errorTokens;
    for (const auto& tok : tokens) {
        if (tok.type == 0 && tok.value.empty()) continue;
        if (tok.type == 0)
            errorTokens.push_back(tok);
        else
            goodTokens.push_back(tok);
    }

    out << "{\n";
    out << "  \"source\": \"" << escape_json(sourceName) << "\",\n";
    out << "  \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";

    out << "  \"tokens\": [\n";
    for (size_t i = 0; i < goodTokens.size(); ++i) {
        const Token& tok = goodTokens[i];
        out << "    {";
        out << "\"type\": " << tok.type << ", ";
        out << "\"value\": \"" << escape_json(tok.value) << "\", ";
        out << "\"line\": " << tok.line;
        out << "}";
        if (i != goodTokens.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    out << "  \"errors\": [\n";
    for (size_t i = 0; i < errorTokens.size(); ++i) {
        const Token& tok = errorTokens[i];
        std::string errMsg = tokenDescription(tok);
        out << "    {";
        out << "\"line\": " << tok.line << ", ";
        out << "\"lexeme\": \"" << escape_json(tok.value) << "\", ";
        out << "\"message\": \"" << escape_json(errMsg) << "\"";
        out << "}";
        if (i != errorTokens.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";

    out.close();
}


void Lexer::printTokens(const std::vector<Token>& tokens, std::ostream& out) {
    for (const auto& tok : tokens) {
        std::string desc = tokenDescription(tok);  // 先存储为 string
        out << "( " << tok.value << " , " << desc << " )\n";
    }
}