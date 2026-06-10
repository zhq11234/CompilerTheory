#include "compilerview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <sstream>

CompilerView::CompilerView(QWidget* parent)
	: QWidget(parent)
{
	setupUI();
}

void CompilerView::setupUI()
{
	auto* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// ---- 菜单栏 ----
	menuBar_ = new QMenuBar(this);
	QMenu* fileMenu = menuBar_->addMenu(QStringLiteral("文件(&F)"));
	actOpenFile = fileMenu->addAction(QStringLiteral("打开源文件(&O)"));
	actOpenFile->setShortcut(QKeySequence(QStringLiteral("Ctrl+O")));

	QMenu* runMenu = menuBar_->addMenu(QStringLiteral("运行(&R)"));
	actRunLexer    = runMenu->addAction(QStringLiteral("词法分析"));
	actRunParser   = runMenu->addAction(QStringLiteral("语法分析"));
	actRunSemantic = runMenu->addAction(QStringLiteral("语义分析"));
	actRunIR       = runMenu->addAction(QStringLiteral("四元式生成"));
	runMenu->addSeparator();
	actRunAll      = runMenu->addAction(QStringLiteral("运行全部"));
	actRunAll->setShortcut(QKeySequence(QStringLiteral("F5")));

	mainLayout->addWidget(menuBar_);

	// ---- 左右分栏 ----
	auto* splitter = new QSplitter(Qt::Horizontal, this);

	// 左半区
	auto* leftWidget = new QWidget(splitter);
	auto* leftLayout = new QVBoxLayout(leftWidget);
	leftLayout->setContentsMargins(4, 4, 4, 4);

	auto* srcLabel = new QLabel(QStringLiteral("源代码"), leftWidget);
	sourceEdit = new QTextEdit(leftWidget);
	sourceEdit->setFont(QFont(QStringLiteral("Consolas"), 12));
	sourceEdit->setPlaceholderText(QStringLiteral("请通过 文件→打开源文件 加载 .src 文件..."));

	auto* conLabel = new QLabel(QStringLiteral("控制台 / 分析日志"), leftWidget);
	consoleEdit = new QTextEdit(leftWidget);
	consoleEdit->setReadOnly(true);
	consoleEdit->setFont(QFont(QStringLiteral("Consolas"), 10));
	consoleEdit->setMaximumHeight(150);

	leftLayout->addWidget(srcLabel);
	leftLayout->addWidget(sourceEdit, 3);
	leftLayout->addWidget(conLabel);
	leftLayout->addWidget(consoleEdit, 1);

	// 右半区 —— QTabWidget
	resultTabs = new QTabWidget(splitter);

	// Tab 1: Token 流
	tokenDisplay = new QTextEdit;
	tokenDisplay->setReadOnly(true);
	tokenDisplay->setFont(QFont(QStringLiteral("Consolas"), 11));
	resultTabs->addTab(tokenDisplay, QStringLiteral("Token 流"));

	// Tab 2: 语法树
	astDisplay = new QTreeWidget;
	astDisplay->setHeaderLabels({QStringLiteral("节点")});
	resultTabs->addTab(astDisplay, QStringLiteral("语法树"));

	// Tab 3: 符号表
	symtabDisplay = new QTableWidget;
	symtabDisplay->setColumnCount(4);
	symtabDisplay->setHorizontalHeaderLabels(
	    {QStringLiteral("名称"), QStringLiteral("类型"),
	     QStringLiteral("作用域"), QStringLiteral("行号")});
	symtabDisplay->horizontalHeader()->setStretchLastSection(true);
	symtabDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
	symtabDisplay->setSelectionBehavior(QAbstractItemView::SelectRows);
	resultTabs->addTab(symtabDisplay, QStringLiteral("符号表"));

	// Tab 4: 四元式
	quadDisplay = new QTableWidget;
	quadDisplay->setColumnCount(5);
	quadDisplay->setHorizontalHeaderLabels(
	    {QStringLiteral("#"), QStringLiteral("op"),
	     QStringLiteral("arg1"), QStringLiteral("arg2"),
	     QStringLiteral("result")});
	quadDisplay->horizontalHeader()->setStretchLastSection(true);
	quadDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
	quadDisplay->setSelectionBehavior(QAbstractItemView::SelectRows);
	resultTabs->addTab(quadDisplay, QStringLiteral("四元式"));

	// Tab 5: 错误信息
	errorDisplay = new QTextEdit;
	errorDisplay->setReadOnly(true);
	errorDisplay->setFont(QFont(QStringLiteral("Consolas"), 11));
	resultTabs->addTab(errorDisplay, QStringLiteral("错误信息"));

	splitter->addWidget(leftWidget);
	splitter->addWidget(resultTabs);
	splitter->setStretchFactor(0, 2);
	splitter->setStretchFactor(1, 3);

	mainLayout->addWidget(splitter);
}

// ---------- 展示方法 ----------

void CompilerView::showSource(const std::string& source)
{
	sourceEdit->setPlainText(QString::fromStdString(source));
}

std::string CompilerView::getSourceText() const
{
	return sourceEdit->toPlainText().toStdString();
}

void CompilerView::showTokens(const std::vector<Token>& tokens)
{
	std::ostringstream oss;
	oss << "共 " << tokens.size() << " 个 Token\n\n";
	for (size_t i = 0; i < tokens.size(); ++i) {
		auto& t = tokens[i];
		oss << "#" << (i + 1) << "  type=" << t.type
		    << "  value='" << t.value << "'  line=" << t.line << "\n";
	}
	tokenDisplay->setPlainText(QString::fromStdString(oss.str()));
}

void CompilerView::showAST(ASTNode* root)
{
	astDisplay->clear();
	if (!root) return;
	buildASTTree(root, astDisplay->invisibleRootItem());
	astDisplay->expandAll();
}

void CompilerView::buildASTTree(ASTNode* node, QTreeWidgetItem* parent)
{
	if (!node) return;

	QString label;
	switch (node->type) {
	case NODE_IF:     label = QStringLiteral("if-else"); break;
	case NODE_COND:   label = QStringLiteral("条件: ") + QString::fromStdString(node->op); break;
	case NODE_ASSIGN: label = QStringLiteral("赋值: ="); break;
	case NODE_ID:     label = QStringLiteral("id: ")   + QString::fromStdString(node->token ? node->token->value : "?"); break;
	case NODE_NUM:    label = QStringLiteral("num: ")  + QString::fromStdString(node->token ? node->token->value : "?"); break;
	default:          label = QStringLiteral("?"); break;
	}

	auto* item = new QTreeWidgetItem(parent, {label});

	switch (node->type) {
	case NODE_IF:
		buildASTTree(node->left, item);
		buildASTTree(node->thenBranch, item);
		buildASTTree(node->elseBranch, item);
		break;
	case NODE_COND:
	case NODE_ASSIGN:
		buildASTTree(node->left, item);
		buildASTTree(node->right, item);
		break;
	default: break;
	}
}

void CompilerView::showSymTab(SymTab& symtab)
{
	auto symbols = symtab.getAllSymbols();
	symtabDisplay->setRowCount(static_cast<int>(symbols.size()));
	for (size_t i = 0; i < symbols.size(); ++i) {
		auto& s = symbols[i];
		symtabDisplay->setItem(static_cast<int>(i), 0,
		    new QTableWidgetItem(QString::fromStdString(s.name)));
		symtabDisplay->setItem(static_cast<int>(i), 1,
		    new QTableWidgetItem(QString::fromStdString(s.type)));
		symtabDisplay->setItem(static_cast<int>(i), 2,
		    new QTableWidgetItem(QString::number(s.scope)));
		symtabDisplay->setItem(static_cast<int>(i), 3,
		    new QTableWidgetItem(QString::number(s.line)));
	}
}

void CompilerView::showQuads(const std::vector<Quadruple>& quads)
{
	quadDisplay->setRowCount(static_cast<int>(quads.size()));
	for (size_t i = 0; i < quads.size(); ++i) {
		auto& q = quads[i];
		quadDisplay->setItem(static_cast<int>(i), 0,
		    new QTableWidgetItem(QString::number(static_cast<int>(i + 1))));
		quadDisplay->setItem(static_cast<int>(i), 1,
		    new QTableWidgetItem(QString::fromStdString(q.op)));
		quadDisplay->setItem(static_cast<int>(i), 2,
		    new QTableWidgetItem(QString::fromStdString(q.arg1)));
		quadDisplay->setItem(static_cast<int>(i), 3,
		    new QTableWidgetItem(QString::fromStdString(q.arg2)));
		quadDisplay->setItem(static_cast<int>(i), 4,
		    new QTableWidgetItem(QString::fromStdString(q.result)));
	}
	quadDisplay->resizeColumnsToContents();
}

void CompilerView::showErrors(const std::vector<std::string>& errors)
{
	if (errors.empty()) {
		errorDisplay->setPlainText(QStringLiteral("无错误。"));
		return;
	}
	std::ostringstream oss;
	for (size_t i = 0; i < errors.size(); ++i)
		oss << "[" << (i + 1) << "] " << errors[i] << "\n";
	errorDisplay->setPlainText(QString::fromStdString(oss.str()));
}

void CompilerView::appendLog(const std::string& msg)
{
	consoleEdit->append(QString::fromStdString(msg));
}

void CompilerView::clearAll()
{
	tokenDisplay->clear();
	astDisplay->clear();
	symtabDisplay->setRowCount(0);
	quadDisplay->setRowCount(0);
	errorDisplay->clear();
}

// ---------- 菜单访问器 ----------

QMenuBar* CompilerView::menuBar() const { return menuBar_; }
QAction* CompilerView::actionOpenFile() const   { return actOpenFile; }
QAction* CompilerView::actionRunLexer() const    { return actRunLexer; }
QAction* CompilerView::actionRunParser() const   { return actRunParser; }
QAction* CompilerView::actionRunSemantic() const { return actRunSemantic; }
QAction* CompilerView::actionRunIR() const       { return actRunIR; }
QAction* CompilerView::actionRunAll() const      { return actRunAll; }
