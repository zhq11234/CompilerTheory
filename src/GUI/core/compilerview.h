#ifndef COMPILERVIEW_H
#define COMPILERVIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QStackedWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMenuBar>
#include <QAction>
#include <QLabel>
#include <QSplitter>
#include <QPushButton>
#include <string>
#include <vector>
#include <map>
#include "token.h"
#include "ast.h"
#include "symtab.h"
#include "quadruple.h"

class TokenCardWidget;
class TokenTypeBar;
class ASTTreeDelegate;

class CompilerView : public QWidget {
	Q_OBJECT
public:
	explicit CompilerView(QWidget* parent = nullptr);

	void showSource(const std::string& source);
	void highlightSourceLine(int line, const QColor& bg = QColor(180, 30, 30));
	std::string getSourceText() const;
	void showTokens(const std::vector<Token>& tokens);
	void showAST(ASTNode* root);
	void showSymTab(SymTab& symtab);
	void showQuads(const std::vector<Quadruple>& quads);
	void setStageErrors(const std::string& stage, const std::vector<std::string>& errors);
	void appendLog(const std::string& msg);
	void clearAll();
	void setResultTab(int idx);

	QMenuBar* menuBar() const;
	QAction* actionOpenFile() const;
	QAction* actionRunLexer() const;
	QAction* actionRunParser() const;
	QAction* actionRunSemantic() const;
	QAction* actionRunIR() const;
	QAction* actionRunAll() const;

signals:
	void tokenLineClicked(int line);
	void astLineClicked(int line);
	void recentFileSelected(const QString& path);

private:
	void setupUI();
	void setupTokenTab(QTabWidget* tabs);
	void setupASTTab(QTabWidget* tabs);
	void showTokensTable(const std::vector<Token>& tokens);
	void refreshErrorDisplay();
	void refreshTabLabels();
	void buildASTModel(ASTNode* node, QStandardItem* parent);

	// --- 左侧 ---
	QTextEdit* sourceEdit = nullptr;
	QTextEdit* consoleEdit = nullptr;

	// --- 右侧 ---
	QTabWidget* resultTabs = nullptr;
	QStackedWidget* tokenStack = nullptr;
	QTableWidget* tokenTable = nullptr;
	TokenCardWidget* tokenCardView = nullptr;
	QTreeView* astTree = nullptr;
	QStandardItemModel* astModel = nullptr;
	ASTTreeDelegate* astDelegate = nullptr;
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

	// --- 错误存储 ---
	std::map<std::string, std::vector<std::string>> stageErrors;

	// --- Tab 索引 ---
	static const int TAB_TOKEN = 0;
	static const int TAB_AST = 1;
	static const int TAB_SYMTAB = 2;
	static const int TAB_IR = 3;
	static const int TAB_ERRORS = 4;
};

#endif
