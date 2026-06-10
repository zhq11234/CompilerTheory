#include "compilermodel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent/QtConcurrent>
#include <sstream>

CompilerModel::CompilerModel(QObject* parent)
	: QObject(parent), astRoot(nullptr)
{
	connect(&tokenWatcher,    &QFutureWatcher<TokenPayload>::finished,
	        this, &CompilerModel::onTokensFinished);
	connect(&astWatcher,      &QFutureWatcher<ASTPayload>::finished,
	        this, &CompilerModel::onASTFinished);
	connect(&semanticWatcher, &QFutureWatcher<SemanticPayload>::finished,
	        this, &CompilerModel::onSemanticFinished);
	connect(&irWatcher,       &QFutureWatcher<IRPayload>::finished,
	        this, &CompilerModel::onIRFinished);
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
	auto future = QtConcurrent::run(&CompilerModel::parseTokensJson,
	                                QString::fromStdString(jsonPath));
	tokenWatcher.setFuture(future);
}

void CompilerModel::loadAST(const std::string& jsonPath)
{
	auto future = QtConcurrent::run(&CompilerModel::parseASTJson,
	                                QString::fromStdString(jsonPath));
	astWatcher.setFuture(future);
}

void CompilerModel::loadSemantic(const std::string& jsonPath)
{
	auto future = QtConcurrent::run(&CompilerModel::parseSemanticJson,
	                                QString::fromStdString(jsonPath));
	semanticWatcher.setFuture(future);
}

void CompilerModel::loadIR(const std::string& jsonPath)
{
	auto future = QtConcurrent::run(&CompilerModel::parseIRJson,
	                                QString::fromStdString(jsonPath));
	irWatcher.setFuture(future);
}

std::string CompilerModel::getSource() const { return sourceDir_; }
std::vector<Token> CompilerModel::getTokens() const { return tokens; }
ASTNode* CompilerModel::getAST() const { return astRoot; }
std::vector<Quadruple> CompilerModel::getQuads() const { return quads; }
SymTab& CompilerModel::getSymTab() { return symtab; }

std::vector<std::string> CompilerModel::getLexerErrors() const    { return lexerErrors; }
std::vector<std::string> CompilerModel::getParserErrors() const   { return parserErrors; }
std::vector<std::string> CompilerModel::getSemanticErrors() const { return semanticErrors; }
std::vector<std::string> CompilerModel::getIRErrors() const       { return irErrors; }
std::string CompilerModel::getParserLog() const { return parserLog; }

void CompilerModel::onTokensFinished()
{
	const auto& payload = tokenWatcher.result();
	tokens = payload.tokens;
	lexerErrors = payload.errors;
	std::ostringstream oss;
	oss << "From " << payload.source << " loaded " << tokens.size() << " tokens";
	parserLog = oss.str();
	emit tokensLoaded();
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
	std::ostringstream oss;
	oss << "From " << payload.source << " loaded AST";
	parserLog = oss.str();
	emit astLoaded();
	emit anyStepDone("Parser");
}

void CompilerModel::onSemanticFinished()
{
	const auto& payload = semanticWatcher.result();
	symtab = SymTab{};
	for (const auto& s : payload.symbols)
		symtab.insert(s.name, s.type, s.line);
	semanticErrors = payload.errors;
	emit semanticLoaded();
	emit anyStepDone("Semantic");
}

void CompilerModel::onIRFinished()
{
	const auto& payload = irWatcher.result();
	quads = payload.quads;
	irErrors = payload.errors;
	emit irLoaded();
	emit anyStepDone("IR");
}

// =====================================================================
//  static JSON parsers  (worker thread via QtConcurrent)
// =====================================================================

CompilerModel::TokenPayload CompilerModel::parseTokensJson(const QString& path)
{
	TokenPayload out;
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) return out;

	QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
	QJsonObject root = doc.object();
	out.source    = root["source"].toString().toStdString();
	out.timestamp = root["timestamp"].toString().toStdString();

	const QJsonArray arr = root["tokens"].toArray();
	for (const auto v : arr) {
		QJsonObject o = v.toObject();
		Token t;
		t.type  = o["type"].toInt();
		t.value = o["value"].toString().toStdString();
		t.line  = o["line"].toInt();
		out.tokens.push_back(std::move(t));
	}
	for (const auto v : root["errors"].toArray())
		out.errors.push_back(v.toString().toStdString());

	return out;
}

namespace {
ASTNode* buildNode(const QJsonObject& obj, std::vector<Token*>& pool)
{
	if (obj.isEmpty()) return nullptr;

	auto* node = new ASTNode;
	std::string typeStr = obj["type"].toString().toStdString();

	if (typeStr == "NODE_IF") {
		node->type = NODE_IF;
		node->left  = buildNode(obj["cond"].toObject(), pool);
		node->thenBranch = buildNode(obj["thenBranch"].toObject(), pool);
		node->elseBranch = buildNode(obj["elseBranch"].toObject(), pool);
	} else if (typeStr == "NODE_COND") {
		node->type = NODE_COND;
		node->op   = obj["op"].toString().toStdString();
		node->left  = buildNode(obj["left"].toObject(), pool);
		node->right = buildNode(obj["right"].toObject(), pool);
	} else if (typeStr == "NODE_ASSIGN") {
		node->type = NODE_ASSIGN;
		node->op   = obj["op"].toString().toStdString();
		node->left  = buildNode(obj["left"].toObject(), pool);
		node->right = buildNode(obj["right"].toObject(), pool);
	} else if (typeStr == "NODE_ID") {
		node->type = NODE_ID;
		auto* tok = new Token;
		tok->type  = 2;
		tok->value = obj["value"].toString().toStdString();
		tok->line  = obj["line"].toInt();
		node->token = tok;
		pool.push_back(tok);
	} else if (typeStr == "NODE_NUM") {
		node->type = NODE_NUM;
		auto* tok = new Token;
		tok->type  = 3;
		tok->value = obj["value"].toString().toStdString();
		tok->line  = obj["line"].toInt();
		node->token = tok;
		pool.push_back(tok);
	}
	return node;
}
} // anonymous namespace

CompilerModel::ASTPayload CompilerModel::parseASTJson(const QString& path)
{
	ASTPayload out;
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) return out;

	QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
	QJsonObject root = doc.object();
	out.source    = root["source"].toString().toStdString();
	out.timestamp = root["timestamp"].toString().toStdString();
	out.root = buildNode(root["ast"].toObject(), out.tokenPtrs);
	for (const auto v : root["errors"].toArray())
		out.errors.push_back(v.toString().toStdString());

	return out;
}

CompilerModel::SemanticPayload CompilerModel::parseSemanticJson(const QString& path)
{
	SemanticPayload out;
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) return out;

	QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
	QJsonObject root = doc.object();
	out.source    = root["source"].toString().toStdString();
	out.timestamp = root["timestamp"].toString().toStdString();

	const QJsonArray syms = root["symtab"].toArray();
	for (const auto v : syms) {
		QJsonObject o = v.toObject();
		Symbol s;
		s.name  = o["name"].toString().toStdString();
		s.type  = o["type"].toString().toStdString();
		s.scope = o["scope"].toInt();
		s.line  = o["line"].toInt();
		out.symbols.push_back(s);
	}
	for (const auto v : root["errors"].toArray())
		out.errors.push_back(v.toString().toStdString());

	return out;
}

CompilerModel::IRPayload CompilerModel::parseIRJson(const QString& path)
{
	IRPayload out;
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) return out;

	QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
	QJsonObject root = doc.object();
	out.source    = root["source"].toString().toStdString();
	out.timestamp = root["timestamp"].toString().toStdString();

	const QJsonArray arr = root["quads"].toArray();
	for (const auto v : arr) {
		QJsonObject o = v.toObject();
		Quadruple q;
		q.op     = o["op"].toString().toStdString();
		q.arg1   = o["arg1"].toString().toStdString();
		q.arg2   = o["arg2"].toString().toStdString();
		q.result = o["result"].toString().toStdString();
		out.quads.push_back(q);
	}
	for (const auto v : root["errors"].toArray())
		out.errors.push_back(v.toString().toStdString());

	return out;
}
