#pragma once

#include "token.h"
#include <string>

enum ASTNodeType { NODE_IF, NODE_COND, NODE_ASSIGN, NODE_ID, NODE_NUM };

struct ASTNode {
	ASTNodeType type;
	Token* token;
	std::string op;
	ASTNode* left, * right;
	ASTNode* thenBranch, * elseBranch;
};
