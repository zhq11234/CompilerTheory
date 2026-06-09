#include "compilermodel.h"
#include <QtConcurrent/QtConcurrent>
#include <sstream>

#include "irgen.h"
#include "lexer.h"
#include "semantic.h"

CompilerModel::CompilerModel(QObject* parent)
	: QObject(parent), astRoot(nullptr)
{
	connect(&tokenWatcher, &QFutureWatcher<TokenPayload>::finished,
		this, &CompilerModel::onTokensFinished);
	connect(&astWatcher, &QFutureWatcher<ASTPayload>::finished,
		this, &CompilerModel::onASTFinished);
	connect(&semanticWatcher, &QFutureWatcher<SemanticPayload>::finished,
		this, &CompilerModel::onSemanticFinished);
	connect(&irWatcher, &QFutureWatcher<IRPayload>::finished,
		this, &CompilerModel::onIRFinished);
	connect(&compileWatcher, &QFutureWatcher<CompileResult>::finished,
		this, &CompilerModel::onCompilationFinished);
}

CompilerModel::~CompilerModel()
{
	tokenWatcher.waitForFinished();
	astWatcher.waitForFinished();
	semanticWatcher.waitForFinished();
	irWatcher.waitForFinished();
}

void CompilerModel::setSourceDir(const std::string& basePath)
{
	sourceDir_ = basePath;
	size_t pos = basePath.find_last_of("/\\");
	std::string fname = (pos != std::string::npos) ? basePath.substr(pos + 1) : basePath;
	emit sourceFileSet(fname);
}

std::string CompilerModel::sourceDir() const { return sourceDir_; }

void CompilerModel::loadTokens(const std::string& jsonPath)
{
	emit stepStarted(QStringLiteral("Lexer"));
	auto future = QtConcurrent::run(&CompilerModel::parseTokensJson,
		QString::fromStdString(jsonPath));
	tokenWatcher.setFuture(future);
}

void CompilerModel::loadAST(const std::string& jsonPath)
{
	emit stepStarted(QStringLiteral("Parser"));
	auto future = QtConcurrent::run(&CompilerModel::parseASTJson,
		QString::fromStdString(jsonPath));
	astWatcher.setFuture(future);
}

void CompilerModel::loadSemantic(const std::string& jsonPath)
{
	emit stepStarted(QStringLiteral("Semantic"));
	auto future = QtConcurrent::run(&CompilerModel::parseSemanticJson,
		QString::fromStdString(jsonPath));
	semanticWatcher.setFuture(future);
}

void CompilerModel::loadIR(const std::string& jsonPath)
{
	emit stepStarted(QStringLiteral("IR"));
	auto future = QtConcurrent::run(&CompilerModel::parseIRJson,
		QString::fromStdString(jsonPath));
	irWatcher.setFuture(future);
}

std::string CompilerModel::getSource() const { return sourceDir_; }
std::vector<Token> CompilerModel::getTokens() const { return tokens; }
ASTNode* CompilerModel::getAST() const { return astRoot; }
std::vector<Quadruple> CompilerModel::getQuads() const { return quads; }
SymTab& CompilerModel::getSymTab() { return symtab; }

std::vector<std::string> CompilerModel::getLexerErrors() const { return lexerErrors; }
std::vector<std::string> CompilerModel::getParserErrors() const { return parserErrors; }
std::vector<std::string> CompilerModel::getSemanticErrors() const { return semanticErrors; }
std::vector<std::string> CompilerModel::getIRErrors() const { return irErrors; }
std::string CompilerModel::getParserLog() const { return parserLog; }

void CompilerModel::runCompiler(const std::string& source, const std::string& srcPath)
{
	auto future = QtConcurrent::run(&CompilerModel::runCompilerStatic,
		source, srcPath);
	compileWatcher.setFuture(future);
}

CompilerModel::CompileResult CompilerModel::runCompilerStatic(
	const std::string& source, const std::string& srcPath)
{
	CompileResult result;

	// 1. Lexer
	Lexer lexer;
	result.tokens = lexer.analyze(source, srcPath);

	// 2. Parser
	Parser parser;
	result.astRoot.reset(parser.parse(result.tokens));
	result.parserErrors = parser.getErrors();
	result.parserLog = parser.getProcessLog();

	// 3. Semantic
	SymTab symtab;
	SemanticAnalyzer sema;
	sema.analyze(result.astRoot.get(), symtab, srcPath);
	result.semanticErrors = sema.getErrors();
	for (const auto& s : symtab.getAllSymbols())
		result.symbols.push_back(s);

	// 4. IR
	IRGenerator irgen;
	result.quads = irgen.generate(result.astRoot.get(), symtab, srcPath);

	return result;
}

void CompilerModel::onCompilationFinished()
{
	auto r = compileWatcher.future().takeResult();

	tokens = std::move(r.tokens);
	lexerErrors = std::move(r.lexerErrors);
	astOwned = std::move(r.astRoot);
	astRoot = astOwned.get();
	parserErrors = std::move(r.parserErrors);
	parserLog = std::move(r.parserLog);
	symtab = SymTab{};
	for (const auto& s : r.symbols)
		symtab.insert(s.name, s.type, s.scope, s.line);
	semanticErrors = std::move(r.semanticErrors);
	quads = std::move(r.quads);

	emit tokensLoaded();
	emit astLoaded();
	emit semanticLoaded();
	emit irLoaded();
	emit compilationFinished();
}

void CompilerModel::onTokensFinished()
{
	const auto& payload = tokenWatcher.result();
	tokens = payload.tokens;
	lexerErrors = payload.errors;
	tokenSource = payload.source;
	tokenTimestamp = payload.timestamp;
	std::ostringstream oss;
	oss << "From " << payload.source << " loaded " << tokens.size() << " tokens";
	parserLog = oss.str();
	emit tokensLoaded();
	emit stepFinished(QStringLiteral("Lexer"));
	emit anyStepDone("Lexer");
}

void CompilerModel::onASTFinished()
{
	const auto& payload = astWatcher.result();
	astTokenPool.clear();
	for (auto* t : payload.tokenPtrs)
		astTokenPool.emplace_back(t);
	astOwned.reset(payload.root);
	astRoot = astOwned.get();
	parserErrors = payload.errors;
	astSource = payload.source;
	astTimestamp = payload.timestamp;
	std::ostringstream oss;
	oss << "From " << payload.source << " loaded AST";
	parserLog = oss.str();
	emit astLoaded();
	emit stepFinished(QStringLiteral("Parser"));
	emit anyStepDone("Parser");
}

void CompilerModel::onSemanticFinished()
{
	const auto& payload = semanticWatcher.result();
	symtab = SymTab{};
	for (const auto& s : payload.symbols)
		symtab.insert(s.name, s.type, s.scope, s.line);
	semanticErrors = payload.errors;
	semanticSource = payload.source;
	semanticTimestamp = payload.timestamp;
	emit semanticLoaded();
	emit stepFinished(QStringLiteral("Semantic"));
	emit anyStepDone("Semantic");
}

void CompilerModel::onIRFinished()
{
	const auto& payload = irWatcher.result();
	quads = payload.quads;
	irErrors = payload.errors;
	irSource = payload.source;
	irTimestamp = payload.timestamp;
	emit irLoaded();
	emit stepFinished(QStringLiteral("IR"));
	emit anyStepDone("IR");
}

// =====================================================================
//  static JSON parsers  (worker thread via QtConcurrent)
// =====================================================================

namespace {
	std::optional<QJsonObject> openAndParseJson(const QString& path, QString& errorOut)
	{
		QFile f(path);
		if (!f.open(QIODevice::ReadOnly)) {
			errorOut = QStringLiteral("Failed to open: ") + path;
			return std::nullopt;
		}
		QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
		if (doc.isNull()) {
			errorOut = QStringLiteral("Invalid JSON: ") + path;
			return std::nullopt;
		}
		return doc.object();
	}

	void extractMetaAndErrors(const QJsonObject& root,
		std::string& source, std::string& timestamp,
		std::vector<std::string>& errors)
	{
		source = root["source"].toString().toStdString();
		timestamp = root["timestamp"].toString().toStdString();
		for (const auto v : root["errors"].toArray())
			errors.push_back(v.toString().toStdString());
	}

	ASTNode* buildNode(const QJsonObject& obj, std::vector<Token*>& pool)
	{
		if (obj.isEmpty()) return nullptr;

		auto* node = new ASTNode{};
		std::string typeStr = obj["type"].toString().toStdString();

		if (typeStr == "NODE_IF") {
			node->type = NODE_IF;
			node->left = buildNode(obj["cond"].toObject(), pool);
			node->thenBranch = buildNode(obj["thenBranch"].toObject(), pool);
			node->elseBranch = buildNode(obj["elseBranch"].toObject(), pool);
		}
		else if (typeStr == "NODE_COND") {
			node->type = NODE_COND;
			node->op = obj["op"].toString().toStdString();
			node->left = buildNode(obj["left"].toObject(), pool);
			node->right = buildNode(obj["right"].toObject(), pool);
		}
		else if (typeStr == "NODE_ASSIGN") {
			node->type = NODE_ASSIGN;
			node->op = obj["op"].toString().toStdString();
			node->left = buildNode(obj["left"].toObject(), pool);
			node->right = buildNode(obj["right"].toObject(), pool);
		}
		else if (typeStr == "NODE_ID") {
			node->type = NODE_ID;
			auto* tok = new Token;
			tok->type = TOKEN_IDENTIFIER;
			tok->value = obj["value"].toString().toStdString();
			tok->line = obj["line"].toInt();
			node->token = tok;
			pool.push_back(tok);
		}
		else if (typeStr == "NODE_NUM") {
			node->type = NODE_NUM;
			auto* tok = new Token;
			tok->type = TOKEN_NUMBER;
			tok->value = obj["value"].toString().toStdString();
			tok->line = obj["line"].toInt();
			node->token = tok;
			pool.push_back(tok);
		}
		return node;
	}
} // anonymous namespace

CompilerModel::TokenPayload CompilerModel::parseTokensJson(const QString& path)
{
	TokenPayload out;
	QString error;
	auto rootOpt = openAndParseJson(path, error);
	if (!rootOpt) { out.errors.push_back(error.toStdString()); return out; }
	extractMetaAndErrors(*rootOpt, out.source, out.timestamp, out.errors);

	const QJsonArray arr = rootOpt->value("tokens").toArray();
	for (const auto v : arr) {
		QJsonObject o = v.toObject();
		Token t;
		t.type = o["type"].toInt();
		t.value = o["value"].toString().toStdString();
		t.line = o["line"].toInt();
		out.tokens.push_back(std::move(t));
	}
	return out;
}

CompilerModel::ASTPayload CompilerModel::parseASTJson(const QString& path)
{
	ASTPayload out;
	QString error;
	auto rootOpt = openAndParseJson(path, error);
	if (!rootOpt) { out.errors.push_back(error.toStdString()); return out; }
	extractMetaAndErrors(*rootOpt, out.source, out.timestamp, out.errors);

	out.root = buildNode(rootOpt->value("ast").toObject(), out.tokenPtrs);
	return out;
}

CompilerModel::SemanticPayload CompilerModel::parseSemanticJson(const QString& path)
{
	SemanticPayload out;
	QString error;
	auto rootOpt = openAndParseJson(path, error);
	if (!rootOpt) { out.errors.push_back(error.toStdString()); return out; }
	extractMetaAndErrors(*rootOpt, out.source, out.timestamp, out.errors);

	const QJsonArray syms = rootOpt->value("symtab").toArray();
	for (const auto v : syms) {
		QJsonObject o = v.toObject();
		Symbol s;
		s.name = o["name"].toString().toStdString();
		s.type = o["type"].toString().toStdString();
		s.scope = o["scope"].toInt();
		s.line = o["line"].toInt();
		out.symbols.push_back(s);
	}
	return out;
}

CompilerModel::IRPayload CompilerModel::parseIRJson(const QString& path)
{
	IRPayload out;
	QString error;
	auto rootOpt = openAndParseJson(path, error);
	if (!rootOpt) { out.errors.push_back(error.toStdString()); return out; }
	extractMetaAndErrors(*rootOpt, out.source, out.timestamp, out.errors);

	const QJsonArray arr = rootOpt->value("quads").toArray();
	for (const auto v : arr) {
		QJsonObject o = v.toObject();
		Quadruple q;
		q.op = o["op"].toString().toStdString();
		q.arg1 = o["arg1"].toString().toStdString();
		q.arg2 = o["arg2"].toString().toStdString();
		q.result = o["result"].toString().toStdString();
		out.quads.push_back(q);
	}
	return out;
}