<<<<<<< HEAD
锘?include "parser.h"
=======
#include "parser.h"
>>>>>>> origin/master
#include <sstream>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <iostream>
<<<<<<< HEAD

LRAnalysisTable::LRAnalysisTable() {
	buildTable();
}

void LRAnalysisTable::buildTable() {
	// 浜х敓寮忕紪鍙蜂粠0寮€濮?
	// 0: S' -> S
	// 1: S -> if E then P else P
	// 2: E -> id N id
	// 3: P -> id N NUM
	// 4: N -> >
	// 5: N -> =
	// 6: N -> <
	rhsLength = { 1, 6, 3, 3, 1, 1, 1 };   // 鍙抽儴闀垮害
	lhsNonTerminal = { S1, S, E, P, N, N, N }; // 宸﹂儴闈炵粓缁撶

	// ---- 2. Action 琛?----
	// 琛ㄩ」鏍煎紡: (state, symbol) -> action
	// action >0 绉昏繘鍒扮姸鎬侊紝<0 褰掔害缂栧彿锛? (prodIdx+1)锛夛紝0 鎺ュ彈锛?1 鍑洪敊锛堢渷鐣ワ級
	// 浠ヤ笅鏍规嵁鎵嬪姩鎺ㄥ鐨勮〃鏍煎～鍐?
	actionTable[{0, IF}] = 2;
	actionTable[{2, ID}] = 4;
	actionTable[{3, THEN}] = 5;
	actionTable[{4, GT}] = 7;
	actionTable[{4, EQ}] = 8;
	actionTable[{4, LT}] = 9;
	actionTable[{5, ID}] = 11;
	actionTable[{6, ID}] = 12;
	actionTable[{7, ID}] = -4;   // 褰掔害浜х敓寮? (N->>)
	actionTable[{8, ID}] = -5;   // 褰掔害浜х敓寮? (N->=)
	actionTable[{9, ID}] = -6;   // 褰掔害浜х敓寮? (N-><)
	actionTable[{10, ELSE}] = 13;
	actionTable[{11, GT}] = 15;
	actionTable[{11, EQ}] = 16;
	actionTable[{11, LT}] = 17;
	actionTable[{12, THEN}] = -2; // 褰掔害浜х敓寮? (E->id N id)
	actionTable[{13, ID}] = 19;
	actionTable[{14, NUM}] = 20;
	actionTable[{15, NUM}] = -4;  // 褰掔害浜х敓寮? (N->>)
	actionTable[{16, NUM}] = -5;
	actionTable[{17, NUM}] = -6;
	actionTable[{18, EOF_}] = -1; // 褰掔害浜х敓寮? (S->...)
	actionTable[{19, GT}] = 15;
	actionTable[{19, EQ}] = 16;
	actionTable[{19, LT}] = 17;
	actionTable[{20, ELSE}] = -3; // 褰掔害浜х敓寮? (P->id N NUM)
	actionTable[{21, NUM}] = 22;
	actionTable[{22, EOF_}] = -3; // 褰掔害浜х敓寮? (P->id N NUM)

	// 鎺ュ彈鍔ㄤ綔
	actionTable[{1, EOF_}] = 1000;

	// ---- 3. Goto 琛?----
	// (state, nonTerminal) -> nextState
	gotoTable[{0, S}] = 1;
	gotoTable[{2, E}] = 3;
	gotoTable[{4, N}] = 6;
	gotoTable[{5, P}] = 10;
	gotoTable[{11, N}] = 14;
	gotoTable[{13, P}] = 18;
	gotoTable[{19, N}] = 21;

	// ---- 4. 椤圭洰闆嗘弿杩帮紙鐢ㄤ簬GUI锛?----
	// 鎸夌収鎵嬪姩鎺ㄥ鐨?I0~I22 濉啓
	itemSets = {
		// I0
		"S' -> 路 S , #\nS -> 路 if E then P else P , #\n",
		// I1
		"S' -> S 路 , #\n",
		// I2
		"S -> if 路 E then P else P , #\nE -> 路 id N id , then\n",
		// I3
		"S -> if E 路 then P else P , #\n",
		// I4
		"E -> id 路 N id , then\nN -> 路 > , id\nN -> 路 = , id\nN -> 路 < , id\n",
		// I5
		"S -> if E then 路 P else P , #\nP -> 路 id N NUM , else\n",
		// I6
		"E -> id N 路 id , then\n",
		// I7
		"N -> > 路 , id\n",
		// I8
		"N -> = 路 , id\n",
		// I9
		"N -> < 路 , id\n",
		// I10
		"S -> if E then P 路 else P , #\n",
		// I11
		"P -> id 路 N NUM , else\nN -> 路 > , NUM\nN -> 路 = , NUM\nN -> 路 < , NUM\n",
		// I12
		"E -> id N id 路 , then\n",
		// I13
		"S -> if E then P else 路 P , #\nP -> 路 id N NUM , #\n",
		// I14
		"P -> id N 路 NUM , else\n",
		// I15
		"N -> > 路 , NUM\n",
		// I16
		"N -> = 路 , NUM\n",
		// I17
		"N -> < 路 , NUM\n",
		// I18
		"S -> if E then P else P 路 , #\n",
		// I19
		"P -> id 路 N NUM , #\nN -> 路 > , NUM\nN -> 路 = , NUM\nN -> 路 < , NUM\n",
		// I20
		"P -> id N NUM 路 , else\n",
		// I21
		"P -> id N 路 NUM , #\n",
		// I22
		"P -> id N NUM 路 , #\n"
	};
}

int LRAnalysisTable::getAction(int state, int tokenType) {
	auto it = actionTable.find({ state, tokenType });
	if (it != actionTable.end()) return it->second;
	return -999; // 鍑洪敊
}

int LRAnalysisTable::getGoto(int state, int nonTerminal) {
	auto it = gotoTable.find({ state, nonTerminal });
	if (it != gotoTable.end()) return it->second;
	return -1;
}

std::string LRAnalysisTable::getActionString(int state, int tokenType) {
	int act = getAction(state, tokenType);
	if (act > 0) return "shift " + std::to_string(act);
	if (act < 0) return "reduce " + std::to_string(-act - 1); // 浜х敓寮忕紪鍙?= -act - 1
	if (act == 0) return "accept";
	return "error";
=======
using namespace Symbol;

LRAnalysisTable::LRAnalysisTable() {
    buildTable();
}


void LRAnalysisTable::buildTable() {
    // 产生式编号从0开始
    // 0: S' -> S
    // 1: S -> if E then P else P
    // 2: E -> id N id
    // 3: P -> id N NUM
    // 4: N -> >
    // 5: N -> =
    // 6: N -> <
    rhsLength = { 1, 6, 3, 3, 1, 1, 1 };   // 右部长度
    lhsNonTerminal = { S1, S, E, P, N, N, N }; // 左部非终结符

    // ---- 2. Action 表 ----
    // 表项格式: (state, symbol) -> action
    // action >0 移进到状态，<0 归约编号（- (prodIdx+1)），0 接受，-1 出错（省略）
    // 以下根据手动推导的表格填写
    actionTable[{0, IF}] = 2;
    actionTable[{2, ID}] = 4;
    actionTable[{3, THEN}] = 5;
    actionTable[{4, GT}] = 7;
    actionTable[{4, EQ}] = 8;
    actionTable[{4, LT}] = 9;
    actionTable[{5, ID}] = 11;
    actionTable[{6, ID}] = 12;
    actionTable[{7, ID}] = -4;   // 归约产生式4 (N->>)
    actionTable[{8, ID}] = -5;   // 归约产生式5 (N->=)
    actionTable[{9, ID}] = -6;   // 归约产生式6 (N-><)
    actionTable[{10, ELSE}] = 13;
    actionTable[{11, GT}] = 15;
    actionTable[{11, EQ}] = 16;
    actionTable[{11, LT}] = 17;
    actionTable[{12, THEN}] = -2; // 归约产生式2 (E->id N id)
    actionTable[{13, ID}] = 19;
    actionTable[{14, NUM}] = 20;
    actionTable[{15, NUM}] = -4;  // 归约产生式4 (N->>)
    actionTable[{16, NUM}] = -5;
    actionTable[{17, NUM}] = -6;
    actionTable[{18, EOF_}] = -1; // 归约产生式1 (S->...)
    actionTable[{19, GT}] = 15;
    actionTable[{19, EQ}] = 16;
    actionTable[{19, LT}] = 17;
    actionTable[{20, ELSE}] = -3; // 归约产生式3 (P->id N NUM)
    actionTable[{21, NUM}] = 22;
    actionTable[{22, EOF_}] = -3; // 归约产生式3 (P->id N NUM)

    // 接受动作
    actionTable[{1, EOF_}] = 1000;

    // ---- 3. Goto 表 ----
    // (state, nonTerminal) -> nextState
    gotoTable[{0, S}] = 1;
    gotoTable[{2, E}] = 3;
    gotoTable[{4, N}] = 6;
    gotoTable[{5, P}] = 10;
    gotoTable[{11, N}] = 14;
    gotoTable[{13, P}] = 18;
    gotoTable[{19, N}] = 21;

    // ---- 4. 项目集描述（用于GUI） ----
    // 按照手动推导的 I0~I22 填写
    itemSets = {
        // I0
        "S' -> · S , #\nS -> · if E then P else P , #\n",
        // I1
        "S' -> S · , #\n",
        // I2
        "S -> if · E then P else P , #\nE -> · id N id , then\n",
        // I3
        "S -> if E · then P else P , #\n",
        // I4
        "E -> id · N id , then\nN -> · > , id\nN -> · = , id\nN -> · < , id\n",
        // I5
        "S -> if E then · P else P , #\nP -> · id N NUM , else\n",
        // I6
        "E -> id N · id , then\n",
        // I7
        "N -> > · , id\n",
        // I8
        "N -> = · , id\n",
        // I9
        "N -> < · , id\n",
        // I10
        "S -> if E then P · else P , #\n",
        // I11
        "P -> id · N NUM , else\nN -> · > , NUM\nN -> · = , NUM\nN -> · < , NUM\n",
        // I12
        "E -> id N id · , then\n",
        // I13
        "S -> if E then P else · P , #\nP -> · id N NUM , #\n",
        // I14
        "P -> id N · NUM , else\n",
        // I15
        "N -> > · , NUM\n",
        // I16
        "N -> = · , NUM\n",
        // I17
        "N -> < · , NUM\n",
        // I18
        "S -> if E then P else P · , #\n",
        // I19
        "P -> id · N NUM , #\nN -> · > , NUM\nN -> · = , NUM\nN -> · < , NUM\n",
        // I20
        "P -> id N NUM · , else\n",
        // I21
        "P -> id N · NUM , #\n",
        // I22
        "P -> id N NUM · , #\n"
    };
}

int LRAnalysisTable::getAction(int state, int tokenType) const {
    auto it = actionTable.find({ state, tokenType });
    if (it != actionTable.end()) return it->second;
    return -999; // 出错
}

int LRAnalysisTable::getGoto(int state, int nonTerminal) const {
    auto it = gotoTable.find({ state, nonTerminal });
    if (it != gotoTable.end()) return it->second;
    return -1;
}

std::string LRAnalysisTable::getActionString(int state, int tokenType) const {
    int act = getAction(state, tokenType);
    if (act > 0) return "shift " + std::to_string(act);
    if (act < 0) return "reduce " + std::to_string(-act - 1); // 产生式编号 = -act - 1
    if (act == 0) return "accept";
    return "error";
>>>>>>> origin/master
}

//-------------------

Parser::Parser() {}

<<<<<<< HEAD
// 瀹炵幇 Parser::getProcessLog
std::string Parser::getProcessLog() {
	return processLog;
}

int Parser::getSymbol(const Token& tok) {
	if (tok.type == 0) return -1;   // 閿欒Token

	if (tok.type == 1) { // 鍏抽敭瀛?
		if (tok.value == "if")   return IF;
		if (tok.value == "then") return THEN;
		if (tok.value == "else") return ELSE;
		// 鍏朵粬鍏抽敭瀛楋紙odd, begin绛夛級涓嶆敮鎸侊紝鎶ラ敊
		return -1;
	}
	if (tok.type == 2) return ID;          // 鏍囪瘑绗?
	if (tok.type == 3) return NUM;         // 鏁板瓧
	if (tok.type == 4) {                   // 杩愮畻绗?
		if (tok.value == ">")  return GT;
		if (tok.value == "=")  return EQ;
		if (tok.value == "<")  return LT;
		return -1;  // 涓嶆敮鎸佺殑杩愮畻绗?
	}
	// 鍒嗛殧绗︾瓑涓嶅叧蹇?
	return -1;
}

ASTNode* Parser::createLeafNode(const Token& tok) {
	ASTNode* node = new ASTNode;
	node->token = new Token(tok);
	node->op = "";
	node->left = node->right = nullptr;
	node->thenBranch = node->elseBranch = nullptr;

	if (tok.type == 2) node->type = NODE_ID;
	else if (tok.type == 3) node->type = NODE_NUM;
	else node->type = NODE_ID; // 鍏朵粬鍗犱綅
	return node;
}

ASTNode* Parser::reduce(int prodIdx) {
	int len = table.getRhsLength()[prodIdx];
	std::vector<ASTNode*> children(len);
	// 浠庢爤涓脊鍑哄彸閮ㄧ鍙峰搴旂殑鑺傜偣锛堥€嗗簭锛?
	for (int i = len - 1; i >= 0; --i) {
		children[i] = nodeStack.top();
		nodeStack.pop();
	}

	ASTNode* node = nullptr;
	switch (prodIdx) {
	case 1: { // S -> if E then P else P
		node = new ASTNode;
		node->type = NODE_IF;
		node->left = children[1];   // E
		node->right = children[3];  // P1 (then鍒嗘敮)
		node->elseBranch = children[5]; // P2 (else鍒嗘敮)
		// 鍒犻櫎鍗犱綅鑺傜偣 if, then, else
		delete children[0];
		delete children[2];
		delete children[4];
		break;
	}
	case 2: { // E -> id N id
		node = new ASTNode;
		node->type = NODE_COND;      // 鏀逛负 NODE_COND
		node->left = children[0];    // id
		node->right = children[2];   // id
		node->op = children[1]->op;  // 姣旇緝绗?
		delete children[1];          // N 鑺傜偣锛屼粎鐢ㄤ簬鑾峰彇 op
		break;
	}
	case 3: { // P -> id N NUM
		node = new ASTNode;
		node->type = NODE_COND;      // 鏀逛负 NODE_COND
		node->left = children[0];    // id
		node->right = children[2];   // NUM
		node->op = children[1]->op;  // 姣旇緝绗?
		delete children[1];          // N 鑺傜偣锛屼粎鐢ㄤ簬鑾峰彇 op
		break;
	}
	case 4: // N -> >
	case 5: // N -> =
	case 6: // N -> <
	{
		node = new ASTNode;
		node->type = NODE_COND;      // 涔熸敼涓?NODE_COND锛堜粎鐢ㄤ簬瀛樺偍 op锛?
		node->op = children[0]->token->value;
		node->left = node->right = nullptr;
		delete children[0];
		break;
	}
	default:
		assert(false); // 涓嶅簲鍙戠敓
	}
	return node;
}

ASTNode* Parser::parse(const std::vector<Token>& tokens) {
	// 娓呯┖鐘舵€?
	while (!stateStack.empty()) stateStack.pop();
	while (!nodeStack.empty()) { delete nodeStack.top(); nodeStack.pop(); }
	processLog.clear();
	errors.clear();

	stateStack.push(0);  // 鍒濆鐘舵€?

	size_t idx = 0;
	while (true) {
		int symbol;
		if (idx >= tokens.size()) {
			symbol = EOF_;
		}
		else {
			const Token& tok = tokens[idx];
			if (tok.type == 0) {
				addError("Lexical error at line " + std::to_string(tok.line) + ": " + tok.value);
				return nullptr;
			}
			symbol = getSymbol(tok);
			if (symbol == -1) {
				addError("Unexpected token '" + tok.value + "' at line " + std::to_string(tok.line));
				return nullptr;
			}
		}

		int state = stateStack.top();
		int action = table.getAction(state, symbol);
		logStep(state, symbol, action, "");
		if (action == 1000) {   // 鎺ュ彈
			if (nodeStack.size() == 1)
				return nodeStack.top();
			else {
				addError("Accept with non-single node stack");
				return nullptr;
			}
		}
		else if (action > 0) {   // 绉昏繘锛堟鏁帮紝涓嶆槸 1000锛?
			if (symbol != EOF_) {
				ASTNode* leaf = createLeafNode(tokens[idx]);
				nodeStack.push(leaf);
			}
			stateStack.push(action);
			++idx;
		}
		else if (action < 0 && action >= -6) {   // 褰掔害锛?1 ~ -6 瀵瑰簲浜х敓寮?~6锛?
			int prodIdx = -action;   // 鍥犱负 action = -prodIdx
			int rhsLen = table.getRhsLength()[prodIdx];
			for (int i = 0; i < rhsLen; ++i)
				stateStack.pop();
			ASTNode* newNode = reduce(prodIdx);
			nodeStack.push(newNode);
			int nonTerm = table.getLhsNonTerminal()[prodIdx];
			int gotoState = table.getGoto(stateStack.top(), nonTerm);
			if (gotoState == -1) {
				addError("Goto error: state " + std::to_string(stateStack.top()) +
					" non-terminal " + std::to_string(nonTerm));
				return nullptr;
			}
			stateStack.push(gotoState);
		}
		else {   // action == -999 鎴栧叾浠栨湭瀹氫箟 鈫?璇硶閿欒
			std::string msg = "Syntax error at state " + std::to_string(state) +
				", symbol " + std::to_string(symbol);
			if (idx < tokens.size())
				msg += " (token: " + tokens[idx].value + ")";
			addError(msg);
			return nullptr;
		}
	}
}

void Parser::printAST(ASTNode* root, std::ostream& out, int depth) {
	if (!root) return;
	std::string indent(depth * 2, ' ');

	switch (root->type) {
	case NODE_IF:
		out << indent << "IfStatement\n";
		out << indent << "  Condition:\n";
		printAST(root->left, out, depth + 2);
		out << indent << "  ThenBranch:\n";
		printAST(root->right, out, depth + 2);
		out << indent << "  ElseBranch:\n";
		printAST(root->elseBranch, out, depth + 2);
		break;
	case NODE_ASSIGN:
		out << indent << "Compare/Assign (op=" << root->op << ")\n";
		out << indent << "  Left:\n";
		printAST(root->left, out, depth + 2);
		out << indent << "  Right:\n";
		printAST(root->right, out, depth + 2);
		break;
	case NODE_ID:
		out << indent << "Identifier: " << root->token->value << "\n";
		break;
	case NODE_NUM:
		out << indent << "Number: " << root->token->value << "\n";
		break;
	default:
		out << indent << "Unknown node\n";
		break;
	}
}

void Parser::logStep(int state, int symbol, int action, const std::string& desc) {
	std::ostringstream oss;
	oss << "State " << state << ", symbol " << symbol << " -> ";
	if (action > 0) oss << "shift " << action;
	else if (action < 0) oss << "reduce " << (-action - 1);
	else if (action == 0) oss << "accept";
	else oss << "error";
	if (!desc.empty()) oss << " (" << desc << ")";
	processLog += oss.str() + "\n";
}

std::vector<std::string> Parser::getErrors() const {
	return errors;
}

void Parser::addError(const std::string& msg) {
	errors.push_back(msg);
}

// ---------- 杈呭姪鍑芥暟锛堜粠 lexer.cpp 澶嶅埗锛屼繚鎸佺粺涓€锛?----------
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
			}
			else {
				oss << c;
			}
			break;
		}
	}
	return oss.str();
}

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

static std::string jsonizeASTNode(ASTNode* node, int indent = 2) {
	if (!node) return "null";
	std::ostringstream oss;
	std::string pad(indent, ' ');
	std::string padChild(indent + 2, ' ');

	switch (node->type) {
	case NODE_IF: {
		oss << "{\n";
		oss << padChild << "\"type\": \"NODE_IF\",\n";
		oss << padChild << "\"cond\": " << jsonizeASTNode(node->left, indent + 2) << ",\n";
		oss << padChild << "\"thenBranch\": " << jsonizeASTNode(node->right, indent + 2) << ",\n";
		oss << padChild << "\"elseBranch\": " << jsonizeASTNode(node->elseBranch, indent + 2) << "\n";
		oss << pad << "}";
		break;
	}
	case NODE_COND: {
		oss << "{\n";
		oss << padChild << "\"type\": \"NODE_COND\",\n";
		oss << padChild << "\"op\": \"" << escape_json(node->op) << "\",\n";
		oss << padChild << "\"left\": " << jsonizeASTNode(node->left, indent + 2) << ",\n";
		oss << padChild << "\"right\": " << jsonizeASTNode(node->right, indent + 2) << "\n";
		oss << pad << "}";
		break;
	}
	case NODE_ID: {
		oss << "{\n";
		oss << padChild << "\"type\": \"NODE_ID\",\n";
		oss << padChild << "\"value\": \"" << escape_json(node->token->value) << "\",\n";
		oss << padChild << "\"line\": " << node->token->line << "\n";
		oss << pad << "}";
		break;
	}
	case NODE_NUM: {
		oss << "{\n";
		oss << padChild << "\"type\": \"NODE_NUM\",\n";
		oss << padChild << "\"value\": \"" << escape_json(node->token->value) << "\",\n";
		oss << padChild << "\"line\": " << node->token->line << "\n";
		oss << pad << "}";
		break;
	}
	default:
		oss << "null";
	}
	return oss.str();
}

void Parser::writeASTToJSON(ASTNode* root, const std::string& filename, const std::string& srcPath) {
	std::ofstream out(filename, std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "Error: cannot open file " << filename << " for writing.\n";
		return;
	}

	out << "{\n";
	out << "  \"source\": \"" << escape_json(srcPath) << "\",\n";
	out << "  \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
	out << "  \"ast\": " << jsonizeASTNode(root, 2) << ",\n";

	const auto& errs = getErrors();
	out << "  \"errors\": [\n";
	for (size_t i = 0; i < errs.size(); ++i) {
		out << "    \"" << escape_json(errs[i]) << "\"";
		if (i != errs.size() - 1) out << ",";
		out << "\n";
	}
	out << "  ]\n";
	out << "}\n";
	out.close();
=======
// 实现 Parser::getProcessLog
std::string Parser::getProcessLog() const {
    return processLog;
}

int Parser::getSymbol(const Token& tok) const {
    if (tok.type == 0) return -1;   // 错误Token

    if (tok.type == 1) { // 关键字
        if (tok.value == "if")   return IF;
        if (tok.value == "then") return THEN;
        if (tok.value == "else") return ELSE;
        // 其他关键字（odd, begin等）不支持，报错
        return -1;
    }
    if (tok.type == 2) return ID;          // 标识符
    if (tok.type == 3) return NUM;         // 数字
    if (tok.type == 4) {                   // 运算符
        if (tok.value == ">")  return GT;
        if (tok.value == "=")  return EQ;
        if (tok.value == "<")  return LT;
        return -1;  // 不支持的运算符
    }
    // 分隔符等不关心
    return -1;
}

ASTNode* Parser::createLeafNode(const Token& tok) {
    ASTNode* node = new ASTNode;
    node->token = new Token(tok);
    node->op = "";
    node->left = node->right = nullptr;
    node->thenBranch = node->elseBranch = nullptr;

    if (tok.type == 2) node->type = NODE_ID;
    else if (tok.type == 3) node->type = NODE_NUM;
    else node->type = NODE_ID; // 其他占位
    return node;
}

ASTNode* Parser::reduce(int prodIdx) {
    int len = table.getRhsLength()[prodIdx];
    std::vector<ASTNode*> children(len);
    // 从栈中弹出右部符号对应的节点（逆序）
    for (int i = len - 1; i >= 0; --i) {
        children[i] = nodeStack.top();
        nodeStack.pop();
    }

    ASTNode* node = nullptr;
    switch (prodIdx) {
    case 1: { // S -> if E then P else P
        node = new ASTNode;
        node->type = NODE_IF;
        node->left = children[1];   // E
        node->right = children[3];  // P1 (then分支)
        node->elseBranch = children[5]; // P2 (else分支)
        // 删除占位节点 if, then, else
        delete children[0];
        delete children[2];
        delete children[4];
        break;
    }
    case 2: { // E -> id N id
        node = new ASTNode;
        node->type = NODE_COND;      // 改为 NODE_COND
        node->left = children[0];    // id
        node->right = children[2];   // id
        node->op = children[1]->op;  // 比较符
        delete children[1];          // N 节点，仅用于获取 op
        break;
    }
    case 3: { // P -> id N NUM
        node = new ASTNode;
        node->type = NODE_COND;      // 改为 NODE_COND
        node->left = children[0];    // id
        node->right = children[2];   // NUM
        node->op = children[1]->op;  // 比较符
        delete children[1];          // N 节点，仅用于获取 op
        break;
    }
    case 4: // N -> >
    case 5: // N -> =
    case 6: // N -> <
    {
        node = new ASTNode;
        node->type = NODE_COND;      // 也改为 NODE_COND（仅用于存储 op）
        node->op = children[0]->token->value;
        node->left = node->right = nullptr;
        delete children[0];
        break;
    }
    default:
        assert(false); // 不应发生
    }
    return node;
}

ASTNode* Parser::parse(const std::vector<Token>& tokens) {
    // 清空状态
    while (!stateStack.empty()) stateStack.pop();
    while (!nodeStack.empty()) { delete nodeStack.top(); nodeStack.pop(); }
    processLog.clear();
    errors.clear();

    stateStack.push(0);  // 初始状态

    size_t idx = 0;
    while (true) {
        int symbol;
        if (idx >= tokens.size()) {
            symbol = EOF_;
        } else {
            const Token& tok = tokens[idx];
            if (tok.type == 0) {
                addError("Lexical error at line " + std::to_string(tok.line) + ": " + tok.value);
                return nullptr;
            }
            symbol = getSymbol(tok);
            if (symbol == -1) {
                addError("Unexpected token '" + tok.value + "' at line " + std::to_string(tok.line));
                return nullptr;
            }
        }

        int state = stateStack.top();
        int action = table.getAction(state, symbol);
        logStep(state, symbol, action, "");
        if (action == 1000) {   // 接受
            if (nodeStack.size() == 1)
                return nodeStack.top();
            else {
                addError("Accept with non-single node stack");
                return nullptr;
            }
        }
        else if (action > 0) {   // 移进（正数，不是 1000）
            if (symbol != EOF_) {
                ASTNode* leaf = createLeafNode(tokens[idx]);
                nodeStack.push(leaf);
            }
            stateStack.push(action);
            ++idx;
        }
        else if (action <0 && action >= -6) {   // 归约（-1 ~ -6 对应产生式0~6）
            int prodIdx = -action;   // 因为 action = -prodIdx
            int rhsLen = table.getRhsLength()[prodIdx];
            for (int i = 0; i < rhsLen; ++i)
                stateStack.pop();
            ASTNode* newNode = reduce(prodIdx);
            nodeStack.push(newNode);
            int nonTerm = table.getLhsNonTerminal()[prodIdx];
            int gotoState = table.getGoto(stateStack.top(), nonTerm);
            if (gotoState == -1) {
                addError("Goto error: state " + std::to_string(stateStack.top()) +
                    " non-terminal " + std::to_string(nonTerm));
                return nullptr;
            }
            stateStack.push(gotoState);
        }
        else {   // action == -999 或其他未定义 → 语法错误
            std::string msg = "Syntax error at state " + std::to_string(state) +
                ", symbol " + std::to_string(symbol);
            if (idx < tokens.size())
                msg += " (token: " + tokens[idx].value + ")";
            addError(msg);
            return nullptr;
        }
    }
}

void Parser::printAST(ASTNode* root, std::ostream& out, int depth) {
    if (!root) return;
    std::string indent(depth * 2, ' ');

    switch (root->type) {
        case NODE_IF:
            out << indent << "IfStatement\n";
            out << indent << "  Condition:\n";
            printAST(root->left, out, depth + 2);
            out << indent << "  ThenBranch:\n";
            printAST(root->right, out, depth + 2);
            out << indent << "  ElseBranch:\n";
            printAST(root->elseBranch, out, depth + 2);
            break;
        case NODE_ASSIGN:
            out << indent << "Compare/Assign (op=" << root->op << ")\n";
            out << indent << "  Left:\n";
            printAST(root->left, out, depth + 2);
            out << indent << "  Right:\n";
            printAST(root->right, out, depth + 2);
            break;
        case NODE_ID:
            out << indent << "Identifier: " << root->token->value << "\n";
            break;
        case NODE_NUM:
            out << indent << "Number: " << root->token->value << "\n";
            break;
        default:
            out << indent << "Unknown node\n";
            break;
    }
}

void Parser::logStep(int state, int symbol, int action, const std::string& desc) {
    std::ostringstream oss;
    oss << "State " << state << ", symbol " << symbol << " -> ";
    if (action > 0) oss << "shift " << action;
    else if (action < 0) oss << "reduce " << (-action - 1);
    else if (action == 0) oss << "accept";
    else oss << "error";
    if (!desc.empty()) oss << " (" << desc << ")";
    processLog += oss.str() + "\n";
}

std::vector<std::string> Parser::getErrors() const {
    return errors;
}

void Parser::addError(const std::string& msg) {
    errors.push_back(msg);
}


// ---------- 辅助函数（从 lexer.cpp 复制，保持统一） ----------
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
            }
            else {
                oss << c;
            }
            break;
        }
    }
    return oss.str();
}

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

static std::string jsonizeASTNode(ASTNode* node, int indent = 2) {
    if (!node) return "null";
    std::ostringstream oss;
    std::string pad(indent, ' ');
    std::string padChild(indent + 2, ' ');

    switch (node->type) {
    case NODE_IF: {
        oss << "{\n";
        oss << padChild << "\"type\": \"NODE_IF\",\n";
        oss << padChild << "\"cond\": " << jsonizeASTNode(node->left, indent + 2) << ",\n";
        oss << padChild << "\"thenBranch\": " << jsonizeASTNode(node->right, indent + 2) << ",\n";
        oss << padChild << "\"elseBranch\": " << jsonizeASTNode(node->elseBranch, indent + 2) << "\n";
        oss << pad << "}";
        break;
    }
    case NODE_COND: {
        oss << "{\n";
        oss << padChild << "\"type\": \"NODE_COND\",\n";
        oss << padChild << "\"op\": \"" << escape_json(node->op) << "\",\n";
        oss << padChild << "\"left\": " << jsonizeASTNode(node->left, indent + 2) << ",\n";
        oss << padChild << "\"right\": " << jsonizeASTNode(node->right, indent + 2) << "\n";
        oss << pad << "}";
        break;
    }
    case NODE_ID: {
        oss << "{\n";
        oss << padChild << "\"type\": \"NODE_ID\",\n";
        oss << padChild << "\"value\": \"" << escape_json(node->token->value) << "\",\n";
        oss << padChild << "\"line\": " << node->token->line << "\n";
        oss << pad << "}";
        break;
    }
    case NODE_NUM: {
        oss << "{\n";
        oss << padChild << "\"type\": \"NODE_NUM\",\n";
        oss << padChild << "\"value\": \"" << escape_json(node->token->value) << "\",\n";
        oss << padChild << "\"line\": " << node->token->line << "\n";
        oss << pad << "}";
        break;
    }
    default:
        oss << "null";
    }
    return oss.str();
}


void Parser::writeASTToJSON(ASTNode* root, const std::string& filename, const std::string& srcPath) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error: cannot open file " << filename << " for writing.\n";
        return;
    }

    out << "{\n";
    out << "  \"source\": \"" << escape_json(srcPath) << "\",\n";
    out << "  \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
    out << "  \"ast\": " << jsonizeASTNode(root, 2) << ",\n";

    const auto& errs = getErrors();
    out << "  \"errors\": [\n";
    for (size_t i = 0; i < errs.size(); ++i) {
        out << "    \"" << escape_json(errs[i]) << "\"";
        if (i != errs.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    out.close();
>>>>>>> origin/master
}