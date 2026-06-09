#include "mainwindow.h"
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("Compiler Theory — Compiler Principles Visualizer"));
	resize(1400, 850);

	// MVC
	model      = new CompilerModel(this);
	view       = new CompilerView(this);
	controller = new CompilerController(model, view, this);

	// Welcome page
	welcomePage_ = new WelcomePage(this);

	// Central stacked widget
	centralStack = new QStackedWidget(this);
	centralStack->addWidget(welcomePage_);  // index 0
	centralStack->addWidget(view);          // index 1
	centralStack->setCurrentIndex(0);

	// Sidebar
	sidePanel_ = new SidePanel(this);

	// Layout: Sidebar | Stack
	auto* centralArea = new QWidget(this);
	auto* hLayout = new QHBoxLayout(centralArea);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(0);
	hLayout->addWidget(sidePanel_);
	hLayout->addWidget(centralStack, 1);

	setCentralWidget(centralArea);
	setMenuBar(view->menuBar());

	// Status bar
	setupStatusBar();

	// Wire connections
	setupConnections();
	connect(view, &CompilerView::tokenLineClicked, this, [this](int line) { view->highlightSourceLine(line, QColor(30, 80, 180)); });
	connect(view, &CompilerView::astLineClicked, this, [this](int line) { view->highlightSourceLine(line, QColor(30, 80, 180)); });
	connect(view, &CompilerView::recentFileSelected, this, [this](const QString& path) { controller->openSpecificFile(path); });
	controller->init();
}

MainWindow::~MainWindow() = default;

void MainWindow::showCompilerView()
{
	centralStack->setCurrentIndex(1);
}

void MainWindow::showWelcomePage()
{
	centralStack->setCurrentIndex(0);
}

void MainWindow::toggleSidebar()
{
	bool visible = sidePanel_->isExpanded();
	sidePanel_->setExpanded(!visible);

	auto* anim = new QPropertyAnimation(sidePanel_, "panelWidth", this);
	anim->setDuration(200);
	anim->setStartValue(visible ? sidePanel_->width() : 0);
	anim->setEndValue(visible ? 0 : sidePanel_->width());
	anim->setEasingCurve(QEasingCurve::InOutCubic);
	anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::setupStatusBar()
{
	auto* sb = statusBar();
	sb->setStyleSheet(QStringLiteral(
		"QStatusBar { background-color: #007ACC; color: #FFFFFF;"
		" border-top: none; padding: 0px 8px; min-height: 26px; }"
		"QStatusBar QLabel { color: #FFFFFF; padding: 0px 6px; font-size: 11px; }"
		"QStatusBar QPushButton {"
		"  background-color: transparent; color: #FFFFFF;"
		"  border: 1px solid rgba(255,255,255,0.3); border-radius: 3px;"
		"  padding: 2px 10px; font-size: 11px;"
		"}"
		"QStatusBar QPushButton:hover { background-color: rgba(255,255,255,0.15); }"
	));

	// Left: filename
	statusFileLabel = new QLabel(QStringLiteral("No file loaded"));
	sb->addWidget(statusFileLabel);

	// Per-stage indicators
	statusLexer = new QLabel(QStringLiteral("Lexer"));
	statusLexer->setStyleSheet(QStringLiteral("color: rgba(255,255,255,0.5); font-size: 11px;"));
	sb->addPermanentWidget(statusLexer);

	statusParser = new QLabel(QStringLiteral("Parser"));
	statusParser->setStyleSheet(QStringLiteral("color: rgba(255,255,255,0.5); font-size: 11px;"));
	sb->addPermanentWidget(statusParser);

	statusSemantic = new QLabel(QStringLiteral("Semantic"));
	statusSemantic->setStyleSheet(QStringLiteral("color: rgba(255,255,255,0.5); font-size: 11px;"));
	sb->addPermanentWidget(statusSemantic);

	statusIR = new QLabel(QStringLiteral("IR"));
	statusIR->setStyleSheet(QStringLiteral("color: rgba(255,255,255,0.5); font-size: 11px;"));
	sb->addPermanentWidget(statusIR);

	// Right: quick actions
	auto* btnClear = new QPushButton(QStringLiteral("Clear All"));
	sb->addPermanentWidget(btnClear);

	auto* btnSave = new QPushButton(QStringLiteral("Save"));
	sb->addPermanentWidget(btnSave);

	auto* btnRunAll = new QPushButton(QStringLiteral("Run All (F5)"));
	btnRunAll->setStyleSheet(QStringLiteral(
		"QPushButton {"
		"  background-color: rgba(255,255,255,0.2); color: #FFFFFF;"
		"  border: 1px solid rgba(255,255,255,0.4); border-radius: 3px;"
		"  padding: 2px 10px; font-size: 11px; font-weight: bold;"
		"}"
		"QPushButton:hover { background-color: rgba(255,255,255,0.3); }"
	));
	sb->addPermanentWidget(btnRunAll);

	// Quick action connections
	connect(btnClear, &QPushButton::clicked, view, &CompilerView::clearAll);
	connect(btnRunAll, &QPushButton::clicked, controller, &CompilerController::handleRunAll);
	connect(btnSave, &QPushButton::clicked, controller, &CompilerController::saveSourceFile);
}

void MainWindow::setupConnections()
{
	// Sidebar toggle
	connect(sidePanel_, &SidePanel::toggleRequested,
	        this, &MainWindow::toggleSidebar);

	// Sidebar navigation → switch tabs
	connect(sidePanel_, &SidePanel::navigationRequested, this, [this](int idx) {
		view->setResultTab(idx);
	});

	// Sidebar stage clicks → run that stage
	connect(sidePanel_, &SidePanel::stageClicked, this, [this](int stage) {
		switch (stage) {
		case 0: controller->handleRunLexer(); break;
		case 1: controller->handleRunParser(); break;
		case 2: controller->handleRunSemantic(); break;
		case 3: controller->handleRunIR(); break;
		}
	});

	// Welcome page → open file dialog
	connect(welcomePage_, &WelcomePage::openFileClicked,
	        controller, &CompilerController::handleOpenFile);

	// Welcome page recent file selected → open directly
	connect(welcomePage_, &WelcomePage::fileSelected, this, [this](const QString& path) {
		controller->openSpecificFile(path);
	});

	// Model step signals → sidebar + status bar
	connect(model, &CompilerModel::stepStarted, this, [this](const QString& step) {
		if (step == "Lexer")    sidePanel_->setStageStatus(0, true, false, false);
		if (step == "Parser")   sidePanel_->setStageStatus(1, true, false, false);
		if (step == "Semantic") sidePanel_->setStageStatus(2, true, false, false);
		if (step == "IR")       sidePanel_->setStageStatus(3, true, false, false);
	});
	connect(model, &CompilerModel::stepFinished, this, [this](const QString& step) {
		if (step == "Lexer") {
			bool ok = model->getLexerErrors().empty();
			sidePanel_->setStageStatus(0, false, ok, !ok);
			statusLexer->setStyleSheet(QStringLiteral("color: %1; font-size: 11px;")
			    .arg(ok ? "#6A9955" : "#F44747"));
		}
		if (step == "Parser") {
			bool ok = model->getParserErrors().empty();
			sidePanel_->setStageStatus(1, false, ok, !ok);
			statusParser->setStyleSheet(QStringLiteral("color: %1; font-size: 11px;")
			    .arg(ok ? "#6A9955" : "#F44747"));
		}
		if (step == "Semantic") {
			bool ok = model->getSemanticErrors().empty();
			sidePanel_->setStageStatus(2, false, ok, !ok);
			statusSemantic->setStyleSheet(QStringLiteral("color: %1; font-size: 11px;")
			    .arg(ok ? "#6A9955" : "#F44747"));
		}
		if (step == "IR") {
			bool ok = model->getIRErrors().empty();
			sidePanel_->setStageStatus(3, false, ok, !ok);
			statusIR->setStyleSheet(QStringLiteral("color: %1; font-size: 11px;")
			    .arg(ok ? "#6A9955" : "#F44747"));
		}
	});

	// File loaded → switch to compiler view + recent files
	connect(model, &CompilerModel::sourceFileSet, this, [this](const std::string& fname) {
		statusFileLabel->setText(QString::fromStdString(fname));
		showCompilerView();
		welcomePage_->addRecentFile(
			QString::fromStdString(model->sourceDir()));
	});
}
