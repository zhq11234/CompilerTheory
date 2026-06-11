#include "lexer.h"
#include <iostream>
#include <string>
using namespace std;

int main() {
    string source =
        "const var1 := 123456789;\n"
        "// 这是单行注释\n"
        "/* 块注释\n"
        "   多行 */\n"
        "if a123 <= b then\n"
        "    call proc( x, y );\n"
        "odd 999999999\n"
        "  while # do\n"
        "  123abc illegal\n"
        "  @ invalid\n"
        "  longid\n";  // 标识符超长测试

    Lexer lexer;
    vector<Token> tokens = lexer.analyze(source);
    lexer.writeTokensToJSON(tokens, "E:/BIANYIYUANLI/test/test_tokens.json");
    return 0;
}