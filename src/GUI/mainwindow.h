#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "compilermodel.h"
#include "compilerview.h"
#include "compilercontroller.h"

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow() override;

private:
	CompilerModel* model = nullptr;
	CompilerView* view = nullptr;
	CompilerController* controller = nullptr;
};

#endif // MAINWINDOW_H
