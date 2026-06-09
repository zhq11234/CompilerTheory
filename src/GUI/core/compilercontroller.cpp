#include "compilercontroller.h"
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "irgen.h"

CompilerController::CompilerController(CompilerModel* m, CompilerView* v,
	QObject* parent)
	: QObject(parent), model(m), view(v)
{
}

void CompilerController::init()
{
	connect(view->actionOpenFile(), &QAction::triggered,
		this, &CompilerController::handleOpenFile);
	connect(view->actionRunLexer(), &QAction::triggered,
		this, &CompilerController::handleRunLexer);
	connect(view->actionRunParser(), &QAction::triggered,
		this, &CompilerController::handleRunParser);
	connect(view->actionRunSemantic(), &QAction::triggered,
		this, &CompilerController::handleRunSemantic);
	connect(view->actionRunIR(), &QAction::triggered,
		this, &CompilerController::handleRunIR);
	connect(view->actionRunAll(), &QAction::triggered,
		this, &CompilerController::handleRunAll);

	connect(model, &CompilerModel::tokensLoaded, this, [this]() {
		view->showTokens(model->getTokens());
		view->setStageErrors("Lexer", model->getLexerErrors());
		view->appendLog("[OK] Token stream loaded (" +
			std::to_string(model->getTokens().size()) + " tokens)");
		});
	connect(model, &CompilerModel::astLoaded, this, [this]() {
		view->showAST(model->getAST());
		view->setStageErrors("Parser", model->getParserErrors());
		view->appendLog("[OK] AST loaded");
		});
	connect(model, &CompilerModel::semanticLoaded, this, [this]() {
		view->showSymTab(model->getSymTab());
		view->setStageErrors("Semantic", model->getSemanticErrors());
		view->appendLog("[OK] Semantic analysis loaded");
		});
	connect(model, &CompilerModel::irLoaded, this, [this]() {
		view->showQuads(model->getQuads());
		view->setStageErrors("IR", model->getIRErrors());
		view->appendLog("[OK] IR loaded (" +
			std::to_string(model->getQuads().size()) + " quads)");
		});
	connect(model, &CompilerModel::anyStepDone, this, [this](const std::string& step) {
		view->appendLog("[Done] " + step);
		});
	connect(model, &CompilerModel::stepStarted, this, [this](const QString& step) {
		view->appendLog("[Loading] " + step.toStdString() + "...");
		});
}

void CompilerController::derivePaths(const std::string& srcPath)
{
	QFileInfo fi(QString::fromStdString(srcPath));
	baseDir = fi.absolutePath().toStdString();
	baseName = fi.completeBaseName().toStdString();

	auto makePath = [&](const std::string& suffix) -> std::string {
		QDir d(QString::fromStdString(baseDir));
		return d.filePath(QString::fromStdString(baseName + suffix)).toStdString();
		};
	pathTokens = makePath("_tokens.json");
	pathAST = makePath("_ast.json");
	pathSemantic = makePath("_semantic.json");
	pathIR = makePath("_ir.json");
}

void CompilerController::openSpecificFile(const QString& path)
{
	if (path.isEmpty()) return;

	derivePaths(path.toStdString());
	model->setSourceDir(path.toStdString());

	QFile f(path);
	if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&f);
		view->showSource(in.readAll().toStdString());
	}

	view->clearAll();
	view->appendLog("Source: " + path.toStdString());
	view->appendLog("Token JSON:  " + pathTokens);
	view->appendLog("AST JSON:    " + pathAST);
	view->appendLog("Semantic JSON: " + pathSemantic);
	view->appendLog("IR JSON:     " + pathIR);

	handleRunAll();
}

void CompilerController::handleOpenFile()
{
	QString filter = QStringLiteral("Source files (*.src);;All files (*)");
	QString path = QFileDialog::getOpenFileName(view,
		QStringLiteral("Open source file"), QString(), filter);
	if (path.isEmpty()) return;
	openSpecificFile(path);
}

void CompilerController::handleRunLexer()
{
	if (baseDir.empty() || baseName.empty()) return;
	view->appendLog("\n======== Run Lexer ========");

	std::string srcPath = sourceFilePath();
	std::string source = view->getSourceText();

	Lexer lexer;
	lexer.analyze(source, srcPath);

	if (!pathTokens.empty()) model->loadTokens(pathTokens);
}

void CompilerController::handleRunParser()
{
	if (baseDir.empty() || baseName.empty()) return;
	view->appendLog("\n======== Run Parser ========");

	std::string srcPath = sourceFilePath();
	std::string source = view->getSourceText();

	Lexer lexer;
	auto tokens = lexer.analyze(source, srcPath);
	if (!pathTokens.empty()) model->loadTokens(pathTokens);

	Parser parser;
	parser.parse(tokens, srcPath);
	if (!pathAST.empty()) model->loadAST(pathAST);
}

void CompilerController::handleRunSemantic()
{
	if (baseDir.empty() || baseName.empty()) return;
	view->appendLog("\n======== Run Semantic Analysis ========");

	std::string srcPath = sourceFilePath();
	std::string source = view->getSourceText();

	Lexer lexer;
	auto tokens = lexer.analyze(source, srcPath);
	if (!pathTokens.empty()) model->loadTokens(pathTokens);

	Parser parser;
	auto* ast = parser.parse(tokens, srcPath);
	if (!pathAST.empty()) model->loadAST(pathAST);

	SymTab symtab;
	SemanticAnalyzer sema;
	sema.analyze(ast, symtab, srcPath);
	if (!pathSemantic.empty()) model->loadSemantic(pathSemantic);
}

void CompilerController::handleRunIR()
{
	if (baseDir.empty() || baseName.empty()) return;
	view->appendLog("\n======== Run IR ========");

	std::string srcPath = sourceFilePath();
	std::string source = view->getSourceText();

	Lexer lexer;
	auto tokens = lexer.analyze(source, srcPath);
	if (!pathTokens.empty()) model->loadTokens(pathTokens);

	Parser parser;
	auto* ast = parser.parse(tokens, srcPath);
	if (!pathAST.empty()) model->loadAST(pathAST);

	SymTab symtab;
	SemanticAnalyzer sema;
	sema.analyze(ast, symtab, srcPath);
	if (!pathSemantic.empty()) model->loadSemantic(pathSemantic);

	IRGenerator irgen;
	irgen.generate(ast, symtab, srcPath);
	if (!pathIR.empty()) model->loadIR(pathIR);
}

void CompilerController::saveSourceFile()
{
	if (baseDir.empty() || baseName.empty()) return;
	QString srcPath = QDir(QString::fromStdString(baseDir))
		.filePath(QString::fromStdString(baseName + ".src"));
	QFile f(srcPath);
	if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&f);
		out << QString::fromStdString(view->getSourceText());
		view->appendLog("[Saved] Source written to " + srcPath.toStdString());
	}
}

void CompilerController::handleRunAll()
{
	saveSourceFile();
	view->appendLog("\n======== Run All ========");
	view->clearAll();

	std::string srcPath = sourceFilePath();
	std::string source = view->getSourceText();

	// 先调用编译器组件生成 JSON 文件
	Lexer lexer;
	auto tokens = lexer.analyze(source, srcPath);

	Parser parser;
	auto* ast = parser.parse(tokens, srcPath);

	SymTab symtab;
	SemanticAnalyzer sema;
	sema.analyze(ast, symtab, srcPath);

	IRGenerator irgen;
	irgen.generate(ast, symtab, srcPath);

	// 再从 JSON 文件加载结果
	if (!pathTokens.empty())   model->loadTokens(pathTokens);
	if (!pathAST.empty())      model->loadAST(pathAST);
	if (!pathSemantic.empty()) model->loadSemantic(pathSemantic);
	if (!pathIR.empty())       model->loadIR(pathIR);
}

std::string CompilerController::sourceFilePath() const
{
	return QDir(QString::fromStdString(baseDir))
		.filePath(QString::fromStdString(baseName + ".src")).toStdString();
}

void CompilerController::refreshAll()
{
	view->showTokens(model->getTokens());
	view->showAST(model->getAST());
	view->showSymTab(model->getSymTab());
	view->showQuads(model->getQuads());
	view->setStageErrors("Semantic", model->getSemanticErrors());
}