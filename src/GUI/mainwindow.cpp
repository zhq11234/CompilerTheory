#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("编译器 GUI — CompilerTheory"));
	resize(1200, 750);

	model      = new CompilerModel(this);
	view       = new CompilerView(this);
	controller = new CompilerController(model, view, this);

	setCentralWidget(view);
	setMenuBar(view->menuBar());

	controller->init();
}

MainWindow::~MainWindow() = default;
