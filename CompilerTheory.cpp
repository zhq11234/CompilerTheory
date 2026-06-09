#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//// 递归释放 AST 节点（防止内存泄漏）
//void deleteAST(ASTNode* node) {
//    if (!node) return;
//    deleteAST(node->left);
//    deleteAST(node->right);
//    deleteAST(node->thenBranch);
//    deleteAST(node->elseBranch);
//    if (node->token) delete node->token;
//    delete node;
//}

int main() {
 /*    1. 准备测试源码（符合文法）
    std::string sourceCode =
        "if a > b then a = 10 else b < 10";

    std::cout << "========== 源代码 ==========\n";
    std::cout << sourceCode << "\n\n";

     2. 词法分析
    Lexer lexer;
    std::vector<Token> tokens = lexer.analyze(sourceCode);

    std::cout << "========== Token 流 ==========\n";
    lexer.printTokens(tokens, std::cout);
    std::cout << "\n";

     3. 语法分析
    Parser parser;
    ASTNode* root = parser.parse(tokens);

     4. 输出结果
    if (root) {
        std::cout << "========== 语法分析成功 ==========\n";
        std::cout << "========== 抽象语法树 (AST) ==========\n";
        parser.printAST(root, std::cout);

        std::cout << "\n========== 分析过程日志 ==========\n";
        std::cout << parser.getProcessLog();

         释放 AST 内存
        deleteAST(root);
    }
    else {
        std::cout << "========== 语法分析失败 ==========\n";
        std::vector<std::string> errors = parser.getErrors();
        for (const auto& err : errors) {
            std::cerr << "[ERROR] " << err << std::endl;
        }
    }*/

    std::string source =
        "if x > z then\n"
        "  y = 1\n"
        "else\n"
        "  z = 2\n";

    Lexer lexer;
    auto tokens = lexer.analyze(source, "test.src");        // 传入源文件路径（或仅文件名）
   

   
    lexer.writeTokensToJSON(tokens, "../test/test_tokens.json");

    // 语法分析
    Parser parser;
    ASTNode* root = parser.parse(tokens);

    parser.writeASTToJSON(root, "../test/test_ast.json", "test.src");

    return 0;
}
