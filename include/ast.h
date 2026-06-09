#pragma once

#include "token.h"
#include <string>

enum ASTNodeType { NODE_IF, NODE_COND, NODE_ASSIGN, NODE_ID, NODE_NUM };

struct ASTNode {
	ASTNodeType type;                       // 节点类型
	Token* token;                           // 指向对应 Token（叶子节点用）
	std::string op;                              // N 的值：">" / "<" / "="
	ASTNode* left, * right;                  // 左右子节点（E/P 的左右操作数）
	ASTNode* thenBranch, * elseBranch;       // if-else 的 P1 和 P2 分支（仅 NODE_IF 用）
};
