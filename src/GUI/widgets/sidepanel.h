#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QPointer>

class SidePanel : public QWidget {
	Q_OBJECT
	Q_PROPERTY(int panelWidth READ panelWidth WRITE setPanelWidth)
public:
	explicit SidePanel(QWidget* parent = nullptr);

	void setStageStatus(int stage, bool running, bool success, bool hasErrors);
	// stage: 0=Lexer, 1=Parser, 2=Semantic, 3=IR

	bool isExpanded() const { return expanded; }
	void setExpanded(bool e) { expanded = e; }

	int panelWidth() const { return m_panelWidth; }
	void setPanelWidth(int w);

signals:
	void stageClicked(int stage);
	void navigationRequested(int tabIndex);
	void toggleRequested();

private:
	void setupUI();
	QLabel* makeDot(int stage);
	int m_panelWidth = 240;
	bool expanded = true;

	// Step panel
	QWidget* stepPanel;
	struct StageRow {
		QWidget* widget;
		QLabel* dot;
		QLabel* name;
		QLabel* status;
	};
	StageRow stageRows[4];

	// Navigation
	QWidget* navPanel;
	QPushButton* btnToggle;

	// Pulse animations for loading stages
	QPointer<QVariantAnimation> pulseAnims[4];
	void startPulse(int stage);
	void stopPulse(int stage);
};

#endif // SIDEPANEL_H
