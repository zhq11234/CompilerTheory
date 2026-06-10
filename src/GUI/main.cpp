#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName(QStringLiteral("CompilerTheory"));
	app.setApplicationDisplayName(QStringLiteral("Compiler Theory — Compiler Principles Visualizer"));

	// Load JetBrains Mono font
	QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Regular.ttf");
	QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Bold.ttf");
	app.setFont(QFont("JetBrains Mono", 11));

	// Load VS Code Dark+ stylesheet
	QFile styleFile(":/style.qss");
	if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
		app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
		styleFile.close();
	}

	MainWindow w;
	w.show();

	return app.exec();
}
