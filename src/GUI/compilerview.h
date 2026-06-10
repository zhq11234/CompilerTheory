#ifndef COMPILERVIEW_H
#define COMPILERVIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QMenuBar>
#include <QAction>
#include <QLabel>
#include <QSplitter>
#include <string>
#include <vector>
#include "token.h"
#include "ast.h"
#include "symtab.h"
#include "quadruple.h"

class CompilerView : public QWidget {
	Q_OBJECT
public:
	explicit CompilerView(QWidget* parent = nullptr);

	void showSource(const std::string& source);
	std::string getSourceText() const;
	void showTokens(const std::vector<Token>& tokens);
	void showAST(ASTNode* root);
	void showSymTab(SymTab& symtab);
	void showQuads(const std::vector<Quadruple>& quads);
	void showErrors(const std::vector<std::string>& errors);
	void appendLog(const std::string& msg);
	void clearAll();

	QMenuBar* menuBar() const;
	QAction* actionOpenFile() const;
	QAction* actionRunLexer() const;
	QAction* actionRunParser() const;
	QAction* actionRunSemantic() const;
	QAction* actionRunIR() const;
	QAction* actionRunAll() const;

private:
	void setupUI();

	// --- 左侧：源代码 + 控制台 ---
	QTextEdit* sourceEdit = nullptr;
	QTextEdit* consoleEdit = nullptr;

	// --- 右侧：结果分页 ---
	QTabWidget* resultTabs = nullptr;
	QTextEdit*   tokenDisplay = nullptr;
	QTreeWidget* astDisplay = nullptr;
	QTableWidget* symtabDisplay = nullptr;
	QTableWidget* quadDisplay = nullptr;
	QTextEdit*   errorDisplay = nullptr;

	// --- 菜单 ---
	QMenuBar* menuBar_ = nullptr;
	QAction* actOpenFile = nullptr;
	QAction* actRunLexer = nullptr;
	QAction* actRunParser = nullptr;
	QAction* actRunSemantic = nullptr;
	QAction* actRunIR = nullptr;
	QAction* actRunAll = nullptr;

	void buildASTTree(ASTNode* node, QTreeWidgetItem* parent);
};

#endif // COMPILERVIEW_H
