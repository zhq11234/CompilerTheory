#pragma once

#include "ast.h"
#include <vector>
#include <map>
#include <stack>

// ���ű�ţ��ڲ�ʹ�ã���ʷ�����������޹أ�
enum Symbol {
	// �ս��
	const int IF = 1;
const int THEN = 2;
const int ELSE = 3;
const int ID = 4;
const int NUM = 5;
const int GT = 6;   // '>'
const int EQ = 7;   // '='
const int LT = 8;   // '<'
const int EOF_ = 9;   // '#'
// ���ս��
const int S1 = 100; // S'
const int S = 101;
const int E = 102;
const int P = 103;
const int N = 104;
}

//��2��  LRAnalysisTable��LR(1)�������
//�ļ�	parser.h / parser.cpp
//ְ��	���첢�洢 LR(1) Action ��� Goto ����ṩ���ӿ�
class LRAnalysisTable {
public:
	LRAnalysisTable();
	void buildTable();  // ����LR(1)��Ŀ���淶�壬����Action/Goto��
	int getAction(int state, int tokenType) const;  // ����ֵ��>0=�ƽ�Ŀ��״̬ / <0=��Լ����ʽ��� / 0=���� / -1=����
	int getGoto(int state, int nonTerminal)const;  // ���� Goto Ŀ��״̬
	std::string getActionString(int state, int tokenType)const;  // ����GUIչʾ��������
	std::vector<std::string> getItemSets() const { return itemSets; };  // ������Ŀ���淶�壨GUIչʾ�ã�

	// ���� Parser ��Լʱ�ĸ�������
	const std::vector<int>& getRhsLength() const { return rhsLength; }
	const std::vector<int>& getLhsNonTerminal() const { return lhsNonTerminal; }

private:
	std::map<std::pair<int, int>, int> actionTable;
	std::map<std::pair<int, int>, int> gotoTable;
	std::vector<int> rhsLength;
	std::vector<int> lhsNonTerminal;
	std::vector<std::string> itemSets;
};

class Parser {
public:
	Parser();
	ASTNode* parse(const std::vector<Token>& tokens);  // ���ӿڣ�Token�� �� AST���ڵ�
	void printAST(ASTNode* root, std::ostream& out, int depth = 0);  // ���AST
	std::string getProcessLog()const;  // ��ȡ�ƽ�-��Լ������־��GUI״̬ջչʾ�ã�
	std::vector<std::string> getErrors()const;  // ��ȡ�﷨�����б�

	void writeASTToJSON(ASTNode* root, const std::string& filename, const std::string& srcPath);
private:
	LRAnalysisTable table;
	std::stack<int> stateStack;       // ״̬ջ
	std::stack<ASTNode*> nodeStack;   // AST�ڵ�ջ����Լʱͬ����ջ�����ӽڵ㣩
	std::string processLog;           // ����������־�ַ���
	std::vector<std::string> errors;       // �����б�

	int getSymbol(const Token& tok) const;          // Token �� �ڲ����ű��
	ASTNode* createLeafNode(const Token& tok);      // Ϊ�ս������Ҷ�ӽڵ�
	ASTNode* reduce(int prodIdx);                   // ��Լ������AST�ڵ�
	void logStep(int state, int symbol, int action, const std::string& desc);
	void addError(const std::string& msg);
};
