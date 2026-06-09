#include "compilerview.h"
#include "asttreedelegate.h"
#include "tokencardwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QFileDialog>
#include <QTextBlock>
#include <QTextCursor>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <sstream>
#include <map>
#include <QSettings>
#include <QFileInfo>

// ==================== Line Number Area ====================

class LineNumberArea : public QWidget {
public:
	explicit LineNumberArea(QTextEdit* editor) : QWidget(editor), m_editor(editor) {
		setFont(editor->font());
	}

	QSize sizeHint() const override {
		int digits = 1, max = qMax(1, m_editor->document()->blockCount());
		while (max >= 10) { max /= 10; ++digits; }
		int w = 8 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
		return {w, 0};
	}

	void paintEvent(QPaintEvent*) override {
		QPainter painter(this);
		painter.fillRect(rect(), QColor("#1E1E1E"));
		QTextBlock block = m_editor->document()->begin();
		int top = (int)m_editor->document()->documentMargin()
		          - m_editor->verticalScrollBar()->value();
		while (block.isValid()) {
			QRectF r = m_editor->document()->documentLayout()->blockBoundingRect(block);
			if (r.isValid()) {
				int y = top + (int)r.top();
				if (y > height()) break;
				painter.setPen(QColor("#858585"));
				painter.drawText(0, y, width() - 4, (int)r.height(),
				                 Qt::AlignRight | Qt::AlignVCenter,
				                 QString::number(block.blockNumber() + 1));
			}
			block = block.next();
		}
	}

private:
	QTextEdit* m_editor;
};

CompilerView::CompilerView(QWidget* parent) : QWidget(parent) { setupUI(); }

// ==================== Token Tab ====================

void CompilerView::setupTokenTab(QTabWidget* tabs)
{
	auto* container = new QWidget;
	auto* layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(2);

	auto* topBar = new QWidget;
	auto* topLayout = new QHBoxLayout(topBar);
	topLayout->setContentsMargins(4, 2, 4, 2);
	topLayout->setSpacing(6);

	auto* cardLabel = new QLabel(QStringLiteral("Table"));
	cardLabel->setStyleSheet("color:#D4D4D4; font-size:11px;");
	topLayout->addWidget(cardLabel);

	auto* toggleBtn = new QPushButton(QStringLiteral("Switch to Card View"));
	toggleBtn->setMaximumHeight(fontMetrics().height() * 2);
	toggleBtn->setStyleSheet(
		"QPushButton{background:#333;color:#D4D4D4;border:1px solid #3C3C3C;border-radius:3px;padding:2px 10px;font-size:10px;}"
		"QPushButton:hover{background:#3C3C3C;}");
	topLayout->addWidget(toggleBtn);
	topLayout->addStretch();
	layout->addWidget(topBar);

	auto* filterBar = new TokenTypeBar;
	layout->addWidget(filterBar);

	tokenStack = new QStackedWidget;
	tokenTable = new QTableWidget;
	tokenTable->setColumnCount(3);
	tokenTable->setHorizontalHeaderLabels({QStringLiteral("#"), QStringLiteral("Value"), QStringLiteral("Line")});
	tokenTable->horizontalHeader()->setStretchLastSection(true);
	tokenTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tokenTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	tokenTable->setAlternatingRowColors(true);
	tokenStack->addWidget(tokenTable);

	tokenCardView = new TokenCardWidget;
	tokenStack->addWidget(tokenCardView);
	layout->addWidget(tokenStack);
	tabs->addTab(container, QStringLiteral("Token"));

	connect(filterBar, &TokenTypeBar::filterChanged, tokenCardView, &TokenCardWidget::setTypeFilter);
	connect(toggleBtn, &QPushButton::clicked, this, [this, toggleBtn, cardLabel]() {
		bool card = (tokenStack->currentIndex() == 0);
		tokenStack->setCurrentIndex(card ? 1 : 0);
		toggleBtn->setText(card ? QStringLiteral("Switch to Table") : QStringLiteral("Switch to Card View"));
		cardLabel->setText(card ? QStringLiteral("Card") : QStringLiteral("Table"));
	});
	connect(tokenCardView, &TokenCardWidget::tokenClicked, this, [this](int line) { emit tokenLineClicked(line); });
	connect(tokenTable, &QTableWidget::cellClicked, this, [this](int row, int) {
		auto* item = tokenTable->item(row, 2);
		if (item) emit tokenLineClicked(item->text().toInt());
	});
}

// ==================== AST Tab ====================

void CompilerView::setupASTTab(QTabWidget* tabs)
{
	auto* container = new QWidget;
	auto* layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	astTree = new QTreeView;
	astTree->setRootIsDecorated(true);
	astTree->setAnimated(true);
	astTree->setExpandsOnDoubleClick(true);
	astTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	astTree->setSelectionBehavior(QAbstractItemView::SelectRows);
	astTree->setAlternatingRowColors(true);
	astTree->setContextMenuPolicy(Qt::CustomContextMenu);
	astTree->setStyleSheet(
		"QTreeView{background:#252526;color:#D4D4D4;border:1px solid #3C3C3C;outline:none;}"
		"QTreeView::item{padding:2px 4px;}"
		"QTreeView::item:hover{background:#2A2D2E;}"
		"QTreeView::item:selected{background:#094771;}");

	astModel = new QStandardItemModel;
	astModel->setHorizontalHeaderLabels({QStringLiteral("Type"), QStringLiteral("Value"), QStringLiteral("Line")});
	astDelegate = new ASTTreeDelegate(this);
	astTree->setModel(astModel);
	astTree->setItemDelegate(astDelegate);
	astTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	astTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
	astTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

	layout->addWidget(astTree);
	tabs->addTab(container, QStringLiteral("AST"));

	connect(astTree->selectionModel(), &QItemSelectionModel::currentChanged,
	        this, [this](const QModelIndex& idx, const QModelIndex&) {
		if (!idx.isValid()) return;
		QModelIndex lineIdx = astModel->index(idx.row(), 2, idx.parent());
		int line = astModel->itemFromIndex(lineIdx) ? astModel->itemFromIndex(lineIdx)->text().toInt() : 0;
		if (line > 0) emit astLineClicked(line);
	});

	connect(astTree, &QTreeView::customContextMenuRequested,
	        this, [this](const QPoint& pos) {
		QModelIndex idx = astTree->indexAt(pos);
		if (!idx.isValid()) return;
		QMenu menu(astTree);
		QAction* actExpand   = menu.addAction(QStringLiteral("Expand subtree"));
		QAction* actCollapse = menu.addAction(QStringLiteral("Collapse subtree"));
		menu.addSeparator();
		QAction* actGoto     = menu.addAction(QStringLiteral("Go to source line"));
		QAction* chosen = menu.exec(astTree->viewport()->mapToGlobal(pos));
		if (chosen == actExpand) astTree->expandRecursively(idx);
		else if (chosen == actCollapse) astTree->collapse(idx);
		else if (chosen == actGoto) {
			QModelIndex lineIdx = astModel->index(idx.row(), 2, idx.parent());
			int line = astModel->itemFromIndex(lineIdx) ? astModel->itemFromIndex(lineIdx)->text().toInt() : 0;
			if (line > 0) emit astLineClicked(line);
		}
	});
}

void CompilerView::buildASTModel(ASTNode* node, QStandardItem* parent)
{
	if (!node) return;

	const char* icons[]  = {"◇ if", "◎ cond:", "← assign:", "▸ id:", "# num:"};
	const QColor colors[] = {QColor("#569CD6"), QColor("#4EC9B0"), QColor("#CE9178"),
	                         QColor("#DCDCAA"), QColor("#6A9955")};

	int ti = static_cast<int>(node->type);
	QString typeStr = (ti >= 0 && ti < 5) ? QString::fromUtf8(icons[ti]) : QStringLiteral("?");
	if (ti == 1) typeStr += QString::fromStdString(node->op);
	if (ti == 2) typeStr = QStringLiteral("← assign");

	QString valStr;
	int lineNum = 0;
	if (node->token) { valStr = QString::fromStdString(node->token->value); lineNum = node->token->line; }
	else if (!node->op.empty()) { valStr = QString::fromStdString(node->op); }


		// Inherit line from left child for internal nodes
		if (lineNum == 0) {
			ASTNode* cur = node->left;
			while (cur && !cur->token) cur = cur->left;
			if (cur && cur->token) lineNum = cur->token->line;
		}
	auto* typeItem  = new QStandardItem(typeStr);
	auto* valueItem = new QStandardItem(valStr);
	auto* lineItem  = new QStandardItem(lineNum > 0 ? QString::number(lineNum) : QString());

	QColor c = (ti >= 0 && ti < 5) ? colors[ti] : QColor("#D4D4D4");
	typeItem->setData(c, Qt::UserRole + 10);
	valueItem->setForeground(QColor("#D4D4D4"));
	lineItem->setForeground(QColor("#858585"));

	if (parent) parent->appendRow({typeItem, valueItem, lineItem});
	else astModel->appendRow({typeItem, valueItem, lineItem});

	switch (node->type) {
	case NODE_IF:
		buildASTModel(node->left, typeItem);
		buildASTModel(node->thenBranch, typeItem);
		buildASTModel(node->elseBranch, typeItem);
		break;
	case NODE_COND: case NODE_ASSIGN:
		buildASTModel(node->left, typeItem);
		buildASTModel(node->right, typeItem);
		break;
	default: break;
	}
}

// ==================== Main UI ====================

void CompilerView::setupUI()
{
	auto* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	menuBar_ = new QMenuBar(this);
	QMenu* fileMenu = menuBar_->addMenu(QStringLiteral("File(&F)"));
	actOpenFile = fileMenu->addAction(QStringLiteral("Open Source File(&O)"));
	actOpenFile->setShortcut(QKeySequence(QStringLiteral("Ctrl+O")));

		// Recent files from QSettings
		{
			QSettings settings("CompilerTheory", "CompilerGUI");
			QStringList recent = settings.value("recentFiles").toStringList();
			if (!recent.isEmpty()) {
				fileMenu->addSeparator();
				for (int i = 0; i < qMin(5, recent.size()); ++i) {
					QString path = recent[i];
					QAction* act = fileMenu->addAction(QString::number(i+1) + ". " + QFileInfo(path).fileName());
					connect(act, &QAction::triggered, this, [this, path]() { emit recentFileSelected(path); });
				}
			}
		}
	QMenu* runMenu = menuBar_->addMenu(QStringLiteral("Run(&R)"));
	actRunLexer    = runMenu->addAction(QStringLiteral("Lexer"));
	actRunParser   = runMenu->addAction(QStringLiteral("Parser"));
	actRunSemantic = runMenu->addAction(QStringLiteral("Semantic"));
	actRunIR       = runMenu->addAction(QStringLiteral("IR Generation"));
	runMenu->addSeparator();
	actRunAll      = runMenu->addAction(QStringLiteral("Run All"));
	actRunAll->setShortcut(QKeySequence(QStringLiteral("F5")));
	mainLayout->addWidget(menuBar_);

	auto* splitter = new QSplitter(Qt::Horizontal, this);

	auto* leftWidget = new QWidget(splitter);
	auto* leftLayout = new QVBoxLayout(leftWidget);
	leftLayout->setContentsMargins(4, 4, 4, 4);

	auto* srcLabel = new QLabel(QStringLiteral("Source Code"), leftWidget);

	auto* srcWrapper = new QWidget(leftWidget);
	auto* srcLayout = new QHBoxLayout(srcWrapper);
	srcLayout->setContentsMargins(0, 0, 0, 0);
	srcLayout->setSpacing(0);
	sourceEdit = new QTextEdit(srcWrapper);
	sourceEdit->setFont(QFont("JetBrains Mono", 12));
	sourceEdit->setPlaceholderText(QStringLiteral("Open a .src file via File -> Open...  or from the Start page"));

	auto* lineNum = new LineNumberArea(sourceEdit);
	srcLayout->addWidget(lineNum);
	srcLayout->addWidget(sourceEdit, 1);

	connect(sourceEdit->verticalScrollBar(), &QScrollBar::valueChanged, lineNum,
	        static_cast<void(QWidget::*)()>(&QWidget::update));
	connect(sourceEdit, &QTextEdit::textChanged, lineNum,
	        static_cast<void(QWidget::*)()>(&QWidget::update));

	auto* conLabel = new QLabel(QStringLiteral("Console / Analysis Log"), leftWidget);
	consoleEdit = new QTextEdit(leftWidget);
	consoleEdit->setReadOnly(true);
	consoleEdit->setFont(QFont("JetBrains Mono", 10));
	consoleEdit->setMinimumHeight(80);

	leftLayout->addWidget(srcLabel);
	leftLayout->addWidget(srcWrapper, 3);
	leftLayout->addWidget(conLabel);
	leftLayout->addWidget(consoleEdit, 1);

	resultTabs = new QTabWidget(splitter);
	setupTokenTab(resultTabs);
	setupASTTab(resultTabs);

	symtabDisplay = new QTableWidget;
	symtabDisplay->setColumnCount(4);
	symtabDisplay->setHorizontalHeaderLabels({QStringLiteral("Name"), QStringLiteral("Type"), QStringLiteral("Scope"), QStringLiteral("Line")});
	symtabDisplay->horizontalHeader()->setStretchLastSection(true);
	symtabDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
	symtabDisplay->setSelectionBehavior(QAbstractItemView::SelectRows);
	resultTabs->addTab(symtabDisplay, QStringLiteral("SymTab"));

	quadDisplay = new QTableWidget;
	quadDisplay->setColumnCount(5);
	quadDisplay->setHorizontalHeaderLabels({QStringLiteral("#"), QStringLiteral("op"), QStringLiteral("arg1"), QStringLiteral("arg2"), QStringLiteral("result")});
	quadDisplay->horizontalHeader()->setStretchLastSection(true);
	quadDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
	quadDisplay->setSelectionBehavior(QAbstractItemView::SelectRows);
	resultTabs->addTab(quadDisplay, QStringLiteral("IR"));
	connect(quadDisplay, &QTableWidget::cellClicked, this, [this](int row, int) { auto* item = quadDisplay->item(row, 0); if (item) highlightSourceLine(item->text().toInt(), QColor(30, 80, 180)); });

	errorDisplay = new QTextEdit;
	errorDisplay->setReadOnly(true);
	errorDisplay->setFont(QFont("JetBrains Mono", 11));
	resultTabs->addTab(errorDisplay, QStringLiteral("Errors"));

	// Click error → jump to source line
	connect(errorDisplay, &QTextEdit::cursorPositionChanged, this, [this]() {
		QString text = errorDisplay->textCursor().block().text();
		int p = text.indexOf("line ");
		if (p < 0) return;
		p += 5;
		QString num;
		while (p < text.length() && text[p].isDigit()) num += text[p++];
		int line = num.toInt();
		if (line > 0) highlightSourceLine(line);
	});

	splitter->addWidget(leftWidget);
	splitter->addWidget(resultTabs);
	splitter->setStretchFactor(0, 2);
	splitter->setStretchFactor(1, 3);
	mainLayout->addWidget(splitter);
}

// ==================== Show Methods ====================

void CompilerView::showSource(const std::string& source) {
	sourceEdit->setPlainText(QString::fromStdString(source));
}

void CompilerView::highlightSourceLine(int line, const QColor& bg) {
	QTextBlock block = sourceEdit->document()->findBlockByLineNumber(line - 1);
	if (!block.isValid()) return;
	QTextCursor cursor(block);
	sourceEdit->setTextCursor(cursor);
	sourceEdit->ensureCursorVisible();

	QTextEdit::ExtraSelection sel;
	sel.format.setBackground(bg);
	sel.format.setProperty(QTextFormat::FullWidthSelection, true);
	sel.cursor = cursor;
	sourceEdit->setExtraSelections({sel});
}

std::string CompilerView::getSourceText() const {
	return sourceEdit->toPlainText().toStdString();
}

void CompilerView::showTokens(const std::vector<Token>& tokens) {
	showTokensTable(tokens);
	tokenCardView->setTokens(tokens);
}

void CompilerView::showTokensTable(const std::vector<Token>& tokens) {
	tokenTable->setRowCount(static_cast<int>(tokens.size()));
	const QColor colors[] = {QColor("#569CD6"), QColor("#4EC9B0"), QColor("#CE9178"), QColor("#D4D4D4"), QColor("#D4D4D4")};
	for (size_t i = 0; i < tokens.size(); ++i) {
		const auto& t = tokens[i]; int row = static_cast<int>(i);
		auto* item0 = new QTableWidgetItem(QString::number(i + 1));
		auto* item1 = new QTableWidgetItem(QString::fromStdString(t.value));
		auto* item2 = new QTableWidgetItem(QString::number(t.line));
		QColor c = (t.type >= 1 && t.type <= 5) ? colors[t.type - 1] : QColor("#D4D4D4");
		item0->setForeground(QColor("#858585")); item1->setForeground(c); item2->setForeground(QColor("#858585"));
		tokenTable->setItem(row, 0, item0); tokenTable->setItem(row, 1, item1); tokenTable->setItem(row, 2, item2);
	}
	tokenTable->resizeColumnsToContents();
}

void CompilerView::showAST(ASTNode* root) {
	astModel->removeRows(0, astModel->rowCount());
	if (!root) return;
	buildASTModel(root, nullptr);
	astTree->expandAll();
}

void CompilerView::showSymTab(SymTab& symtab) {
	auto symbols = symtab.getAllSymbols();
	symtabDisplay->setRowCount(static_cast<int>(symbols.size()));
	for (size_t i = 0; i < symbols.size(); ++i) {
		auto& s = symbols[i]; int row = static_cast<int>(i);
		symtabDisplay->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(s.name)));
		symtabDisplay->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(s.type)));
		symtabDisplay->setItem(row, 2, new QTableWidgetItem(QString::number(s.scope)));
		symtabDisplay->setItem(row, 3, new QTableWidgetItem(QString::number(s.line)));
	}
}

void CompilerView::showQuads(const std::vector<Quadruple>& quads) {
	quadDisplay->setRowCount(static_cast<int>(quads.size()));
	for (size_t i = 0; i < quads.size(); ++i) {
		auto& q = quads[i]; int row = static_cast<int>(i);
		QColor rowColor;
		if (q.op == "=") rowColor = QColor("#1A6A9955");
		else if (!q.op.empty() && q.op[0] == 'j') rowColor = QColor("#1A569CD6");

		auto makeItem = [&](const QString& t, const QColor& fg = QColor("#D4D4D4")) {
			auto* it = new QTableWidgetItem(t); it->setForeground(fg);
			if (rowColor.alpha() > 0) it->setBackground(rowColor);
			return it;
		};
		quadDisplay->setItem(row, 0, makeItem(QString::number(i + 1), QColor("#858585")));
		quadDisplay->setItem(row, 1, makeItem(QString::fromStdString(q.op),
			q.op == "=" ? QColor("#6A9955") : (!q.op.empty() && q.op[0]=='j') ? QColor("#569CD6") : QColor("#D4D4D4")));
		quadDisplay->setItem(row, 2, makeItem(QString::fromStdString(q.arg1)));
		quadDisplay->setItem(row, 3, makeItem(QString::fromStdString(q.arg2)));
		quadDisplay->setItem(row, 4, makeItem(QString::fromStdString(q.result),
			!q.result.empty() && q.result[0]=='L' ? QColor("#DCDCAA") : QColor("#D4D4D4")));
	}
	quadDisplay->resizeColumnsToContents();
}

// ==================== Errors ====================

void CompilerView::setStageErrors(const std::string& stage, const std::vector<std::string>& errors) {
	stageErrors[stage] = errors;
	refreshErrorDisplay();
	refreshTabLabels();
}

void CompilerView::refreshErrorDisplay() {
	int total = 0;
	for (const auto& [_, e] : stageErrors) total += static_cast<int>(e.size());

	if (total == 0) {
		errorDisplay->setHtml(QStringLiteral(
			"<div style='color:#6A9955;font-size:13px;padding:12px;'>"
			"<b>All checks passed</b><br>No errors in any stage.</div>"));
		resultTabs->setTabText(TAB_ERRORS, QStringLiteral("Errors"));
		return;
	}

	const char* names[] = {"Lexer", "Parser", "Semantic", "IR"};
	const char* cols[]  = {"#569CD6", "#4EC9B0", "#DCDCAA", "#CE9178"};
	std::ostringstream html;
	html << "<style>"
	     << ".s{margin:8px 0;}"
	     << ".h{font-weight:bold;font-size:13px;padding:4px 8px;border-radius:3px;}"
	     << ".e{color:#F44747;padding:2px 16px;font-size:12px;font-family:'JetBrains Mono',monospace;}"
	     << ".ok{color:#6A9955;padding:2px 16px;font-size:12px;}"
	     << "</style>";

	for (int i = 0; i < 4; ++i) {
		auto it = stageErrors.find(names[i]);
		bool has = (it != stageErrors.end() && !it->second.empty());
		html << "<div class='s'><div class='h' style='background:" << cols[i] << "30;color:" << cols[i] << ";'>"
		     << names[i] << " (" << (has ? std::to_string(it->second.size()) + " errors" : "OK") << ")</div>";
		if (has) {
			for (size_t j = 0; j < it->second.size(); ++j)
				html << "<div class='e'>[" << (j+1) << "] " << it->second[j] << "</div>";
		} else {
			html << "<div class='ok'>&#10003; No errors</div>";
		}
		html << "</div>";
	}
	errorDisplay->setHtml(QString::fromStdString(html.str()));
}

void CompilerView::refreshTabLabels() {
	const char* stageMap[] = {"Lexer", "Parser", "Semantic", "IR"};
	const char* tabNames[] = {"Token", "AST", "SymTab", "IR"};
	int tabs[] = {TAB_TOKEN, TAB_AST, TAB_SYMTAB, TAB_IR};
	int lastErr = TAB_ERRORS;

	for (int i = 0; i < 4; ++i) {
		auto it = stageErrors.find(stageMap[i]);
		int cnt = (it != stageErrors.end()) ? static_cast<int>(it->second.size()) : 0;
		QString label = QString::fromUtf8(tabNames[i]);
		if (cnt > 0) { label += " (" + QString::number(cnt) + ")"; lastErr = tabs[i]; }
		resultTabs->setTabText(tabs[i], label);
	}

	int total = 0;
	for (const auto& [_, e] : stageErrors) total += static_cast<int>(e.size());
	QString el = QStringLiteral("Errors");
	if (total > 0) el += " (" + QString::number(total) + ")";
	resultTabs->setTabText(TAB_ERRORS, el);
	if (total > 0) resultTabs->setCurrentIndex(lastErr);
}

void CompilerView::appendLog(const std::string& msg) {
	consoleEdit->append(QString::fromStdString(msg));
}

void CompilerView::clearAll() {
	tokenTable->setRowCount(0);
	tokenCardView->setTokens({});
	astModel->removeRows(0, astModel->rowCount());
	symtabDisplay->setRowCount(0);
	quadDisplay->setRowCount(0);
	stageErrors.clear();
	refreshTabLabels();
	errorDisplay->clear();
}

void CompilerView::setResultTab(int idx) {
	if (idx >= 0 && idx < resultTabs->count()) resultTabs->setCurrentIndex(idx); }

QMenuBar* CompilerView::menuBar() const { return menuBar_; }
QAction* CompilerView::actionOpenFile() const   { return actOpenFile; }
QAction* CompilerView::actionRunLexer() const    { return actRunLexer; }
QAction* CompilerView::actionRunParser() const   { return actRunParser; }
QAction* CompilerView::actionRunSemantic() const { return actRunSemantic; }
QAction* CompilerView::actionRunIR() const       { return actRunIR; }
QAction* CompilerView::actionRunAll() const      { return actRunAll; }
