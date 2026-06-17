#ifndef COMPILERMODEL_H
#define COMPILERMODEL_H

#include <QObject>
#include <QFutureWatcher>
#include <string>
#include <vector>
#include <memory>
#include "token.h"
#include "ast.h"
#include "symtab.h"
#include "quadruple.h"
#include "parser.h"

class CompilerModel : public QObject {
	Q_OBJECT
public:
	explicit CompilerModel(QObject* parent = nullptr);
	~CompilerModel() override;

	void setSourceDir(const std::string& basePath);
	std::string sourceDir() const;

	void runCompiler(const std::string& source, const std::string& srcPath);

	void loadTokens(const std::string& jsonPath);
	void loadAST(const std::string& jsonPath);
	void loadSemantic(const std::string& jsonPath);
	void loadIR(const std::string& jsonPath);

	std::string getSource() const;
	std::vector<Token> getTokens() const;
	ASTNode* getAST() const;
	std::vector<Quadruple> getQuads() const;
	SymTab& getSymTab();
	std::vector<std::string> getLexerErrors() const;
	std::vector<std::string> getParserErrors() const;
	std::vector<std::string> getSemanticErrors() const;
	std::vector<std::string> getIRErrors() const;
	std::string getParserLog() const;

	std::string getTokenSource() const { return tokenSource; }
	std::string getTokenTimestamp() const { return tokenTimestamp; }
	std::string getASTSource() const { return astSource; }
	std::string getASTTimestamp() const { return astTimestamp; }
	std::string getSemanticSource() const { return semanticSource; }
	std::string getSemanticTimestamp() const { return semanticTimestamp; }
	std::string getIRSource() const { return irSource; }
	std::string getIRTimestamp() const { return irTimestamp; }

signals:
	void stepStarted(const QString& stepName);
	void stepFinished(const QString& stepName);
	void tokensLoaded();
	void astLoaded();
	void semanticLoaded();
	void irLoaded();
	void compilationFinished();
	void sourceFileSet(const std::string& filename);
	void anyStepDone(const std::string& stepName);

private slots:
	void onTokensFinished();
	void onASTFinished();
	void onSemanticFinished();
	void onIRFinished();
	void onCompilationFinished();

private:
	struct TokenPayload {
		std::string source;
		std::string timestamp;
		std::vector<Token> tokens;
		std::vector<std::string> errors;
	};
	struct ASTPayload {
		std::string source;
		std::string timestamp;
		ASTNode* root = nullptr;
		std::vector<Token*> tokenPtrs;
		std::vector<std::string> errors;
	};
	struct SemanticPayload {
		std::string source;
		std::string timestamp;
		std::vector<symbol_enum_symbol> symbols;
		std::vector<std::string> errors;
	};
	struct IRPayload {
		std::string source;
		std::string timestamp;
		std::vector<Quadruple> quads;
		std::vector<std::string> errors;
	};

	static TokenPayload parseTokensJson(const QString& path);
	static ASTPayload   parseASTJson(const QString& path);
	static SemanticPayload parseSemanticJson(const QString& path);
	static IRPayload    parseIRJson(const QString& path);

	struct CompileResult {
		std::vector<Token> tokens;
		std::unique_ptr<ASTNode> astRoot;
		std::vector<symbol_enum_symbol> symbols;
		std::vector<Quadruple> quads;
		std::vector<std::string> lexerErrors;
		std::vector<std::string> parserErrors;
		std::vector<std::string> semanticErrors;
		std::vector<std::string> irErrors;
		std::string parserLog;
	};
	static CompileResult runCompilerStatic(const std::string& source, const std::string& srcPath);

	std::string sourceDir_;
	SymTab symtab;

	std::vector<Token> tokens;
	std::vector<Quadruple> quads;

	ASTNode* astRoot = nullptr;
	std::unique_ptr<ASTNode> astOwned;
	std::vector<std::unique_ptr<Token>> astTokenPool;

	std::vector<std::string> lexerErrors;
	std::vector<std::string> parserErrors;
	std::vector<std::string> semanticErrors;
	std::vector<std::string> irErrors;
	std::string parserLog;

	std::string tokenSource, tokenTimestamp;
	std::string astSource, astTimestamp;
	std::string semanticSource, semanticTimestamp;
	std::string irSource, irTimestamp;

	QFutureWatcher<TokenPayload>    tokenWatcher;
	QFutureWatcher<ASTPayload>      astWatcher;
	QFutureWatcher<SemanticPayload> semanticWatcher;
	QFutureWatcher<IRPayload>       irWatcher;
	QFutureWatcher<CompileResult>   compileWatcher;
};

#endif // COMPILERMODEL_H
