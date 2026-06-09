#ifndef COMPILERCONTROLLER_H
#define COMPILERCONTROLLER_H

#include <QObject>
#include <string>
#include "compilermodel.h"
#include "compilerview.h"

class CompilerController : public QObject {
	Q_OBJECT
public:
	explicit CompilerController(CompilerModel* model, CompilerView* view,
		QObject* parent = nullptr);
	void init();
	void openSpecificFile(const QString& path);

public slots:
	void handleOpenFile();
	void handleRunLexer();
	void handleRunParser();
	void handleRunSemantic();
	void handleRunIR();
	void handleRunAll();
	void saveSourceFile();
private:
	CompilerModel* model;

	CompilerView* view;

	std::string baseName;
	std::string baseDir;

	std::string pathTokens;
	std::string pathAST;
	std::string pathSemantic;
	std::string pathIR;

	void derivePaths(const std::string& srcPath);
	void refreshAll();
	std::string sourceFilePath() const;
};

#endif // COMPILERCONTROLLER_H
