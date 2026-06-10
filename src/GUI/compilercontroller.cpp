#include "compilercontroller.h"
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

CompilerController::CompilerController(CompilerModel* m, CompilerView* v,
                                       QObject* parent)
	: QObject(parent), model(m), view(v)
{
}

void CompilerController::init()
{
	// ---- View 菜单 → Controller 槽 ----
	connect(view->actionOpenFile(),   &QAction::triggered,
	        this, &CompilerController::handleOpenFile);
	connect(view->actionRunLexer(),    &QAction::triggered,
	        this, &CompilerController::handleRunLexer);
	connect(view->actionRunParser(),   &QAction::triggered,
	        this, &CompilerController::handleRunParser);
	connect(view->actionRunSemantic(), &QAction::triggered,
	        this, &CompilerController::handleRunSemantic);
	connect(view->actionRunIR(),       &QAction::triggered,
	        this, &CompilerController::handleRunIR);
	connect(view->actionRunAll(),      &QAction::triggered,
	        this, &CompilerController::handleRunAll);

	// ---- Model 信号 → View 刷新 ----
	connect(model, &CompilerModel::tokensLoaded, this, [this]() {
		view->showTokens(model->getTokens());
		view->showErrors(model->getLexerErrors());
		view->appendLog("[OK] Token 流加载完成");
	});
	connect(model, &CompilerModel::astLoaded, this, [this]() {
		view->showAST(model->getAST());
		view->showErrors(model->getParserErrors());
		view->appendLog("[OK] AST 加载完成");
	});
	connect(model, &CompilerModel::semanticLoaded, this, [this]() {
		view->showSymTab(model->getSymTab());
		view->showErrors(model->getSemanticErrors());
		view->appendLog("[OK] 语义分析结果加载完成");
	});
	connect(model, &CompilerModel::irLoaded, this, [this]() {
		view->showQuads(model->getQuads());
		view->showErrors(model->getIRErrors());
		view->appendLog("[OK] 四元式加载完成");
	});
	connect(model, &CompilerModel::anyStepDone, this, [this](const std::string& step) {
		view->appendLog("[完成] " + step);
	});
}

// ---------- 路径推断 ----------

void CompilerController::derivePaths(const std::string& srcPath)
{
	QFileInfo fi(QString::fromStdString(srcPath));
	baseDir  = fi.absolutePath().toStdString();
	baseName = fi.completeBaseName().toStdString();  // "test" from "test.src"

	auto makePath = [&](const std::string& suffix) -> std::string {
		QDir d(QString::fromStdString(baseDir));
		return d.filePath(QString::fromStdString(baseName + suffix)).toStdString();
	};
	pathTokens   = makePath("_tokens.json");
	pathAST      = makePath("_ast.json");
	pathSemantic = makePath("_semantic.json");
	pathIR       = makePath("_ir.json");
}

// ---------- 槽实现 ----------

void CompilerController::handleOpenFile()
{
	QString filter = QStringLiteral("源文件 (*.src);;所有文件 (*)");
	QString path = QFileDialog::getOpenFileName(view,
	              QStringLiteral("打开源文件"), QString(), filter);
	if (path.isEmpty()) return;

	derivePaths(path.toStdString());

	// 读取源文件内容
	QFile f(path);
	if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&f);
		QString content = in.readAll();
		view->showSource(content.toStdString());
	}

	model->setSourceDir(path.toStdString());
	view->clearAll();
	view->appendLog("源文件: " + path.toStdString());
	view->appendLog("Token JSON:  " + pathTokens);
	view->appendLog("AST JSON:    " + pathAST);
	view->appendLog("语义 JSON:   " + pathSemantic);
	view->appendLog("IR JSON:     " + pathIR);

	// 加载所有可用的 JSON
	handleRunAll();
}

void CompilerController::handleRunLexer()
{
	if (pathTokens.empty()) return;
	view->appendLog("\n======== 加载 Token 流 ========");
	model->loadTokens(pathTokens);
}

void CompilerController::handleRunParser()
{
	if (pathAST.empty()) return;
	view->appendLog("\n======== 加载语法树 ========");
	if (!pathTokens.empty()) model->loadTokens(pathTokens);
	model->loadAST(pathAST);
}

void CompilerController::handleRunSemantic()
{
	if (pathSemantic.empty()) return;
	view->appendLog("\n======== 加载语义分析 ========");
	model->loadSemantic(pathSemantic);
}

void CompilerController::handleRunIR()
{
	if (pathIR.empty()) return;
	view->appendLog("\n======== 加载四元式 ========");
	model->loadIR(pathIR);
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
		view->appendLog("[保存] 源代码已写入 " + srcPath.toStdString());
	}
}

void CompilerController::handleRunAll()
{
	saveSourceFile();
	view->appendLog("\n======== 运行全部 ========");
	view->clearAll();
	if (!pathTokens.empty())   model->loadTokens(pathTokens);
	if (!pathAST.empty())      model->loadAST(pathAST);
	if (!pathSemantic.empty()) model->loadSemantic(pathSemantic);
	if (!pathIR.empty())       model->loadIR(pathIR);
}

void CompilerController::refreshAll()
{
	view->showTokens(model->getTokens());
	view->showAST(model->getAST());
	view->showSymTab(model->getSymTab());
	view->showQuads(model->getQuads());
	view->showErrors(model->getSemanticErrors());
}
