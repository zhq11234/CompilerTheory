#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <memory>
#include "compilermodel.h"
#include "compilerview.h"
#include "compilercontroller.h"
#include "sidepanel.h"
#include "welcomepage.h"

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow() override;

	void showCompilerView();
	void showWelcomePage();
	void toggleSidebar();

	SidePanel* sidePanel() const { return sidePanel_; }
	WelcomePage* welcomePage() const { return welcomePage_; }

private:
	void setupStatusBar();
	void setupConnections();

	CompilerModel* model = nullptr;
	CompilerView* view = nullptr;
	CompilerController* controller = nullptr;

	SidePanel* sidePanel_ = nullptr;
	WelcomePage* welcomePage_ = nullptr;
	QStackedWidget* centralStack = nullptr;

	// Status bar widgets
	QLabel* statusFileLabel = nullptr;
	QLabel* statusLexer = nullptr;
	QLabel* statusParser = nullptr;
	QLabel* statusSemantic = nullptr;
	QLabel* statusIR = nullptr;
};

#endif // MAINWINDOW_H
