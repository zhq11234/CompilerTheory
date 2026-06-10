#include "welcomepage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QFileInfo>
#include <QShowEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>

WelcomePage::WelcomePage(QWidget* parent)
	: QWidget(parent)
{
	setupUI();
	loadRecentFiles();
}

void WelcomePage::setupUI()
{
	auto* outerLayout = new QVBoxLayout(this);
	outerLayout->setAlignment(Qt::AlignCenter);

	int charW = fontMetrics().horizontalAdvance(QLatin1Char('M'));
	auto* inner = new QWidget;
	inner->setMaximumWidth(charW * 50);
	inner->setMinimumWidth(qMax(charW * 35, 300));

	auto* layout = new QVBoxLayout(inner);
	layout->setSpacing(16);

	// Title
	titleLabel = new QLabel(QStringLiteral("Compiler Theory"));
	titleLabel->setStyleSheet(QStringLiteral("font-size: 28px; font-weight: bold; color: #D4D4D4;"));
	titleLabel->setAlignment(Qt::AlignCenter);
	layout->addSpacing(40);
	layout->addWidget(titleLabel);

	auto* subLabel = new QLabel(QStringLiteral("Compiler Principles Visualizer"));
	subLabel->setStyleSheet(QStringLiteral("font-size: 14px; color: #858585;"));
	subLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(subLabel);
	layout->addSpacing(30);

	// Quick open search
	quickOpen = new QLineEdit;
	quickOpen->setPlaceholderText(QStringLiteral("Search files by name... (Ctrl+P)"));
	quickOpen->setMinimumHeight(36);
	quickOpen->setStyleSheet(QStringLiteral(
		"QLineEdit {"
		"  background-color: #3C3C3C;"
		"  color: #D4D4D4;"
		"  border: 1px solid #3C3C3C;"
		"  border-radius: 6px;"
		"  padding: 6px 14px;"
		"  font-size: 14px;"
		"}"
		"QLineEdit:focus { border-color: #007ACC; }"
	));
	layout->addWidget(quickOpen);

	// Recent files label
	auto* recentLabel = new QLabel(QStringLiteral("Recent files"));
	recentLabel->setStyleSheet(QStringLiteral("font-size: 12px; color: #858585; font-weight: bold; margin-top: 8px;"));
	layout->addWidget(recentLabel);

	// Recent files list
	recentList = new QListWidget;
	recentList->setMinimumHeight(150);
	recentList->setStyleSheet(QStringLiteral(
		"QListWidget {"
		"  background-color: #252526;"
		"  color: #D4D4D4;"
		"  border: 1px solid #3C3C3C;"
		"  border-radius: 6px;"
		"  font-size: 13px;"
		"}"
		"QListWidget::item { padding: 8px 14px; }"
		"QListWidget::item:hover { background-color: #333333; }"
		"QListWidget::item:selected { background-color: #094771; }"
	));
	layout->addWidget(recentList);

	// Open file button
	openBtn = new QPushButton(QStringLiteral("Open File..."));
	openBtn->setMinimumHeight(40);
	openBtn->setCursor(Qt::PointingHandCursor);
	openBtn->setStyleSheet(QStringLiteral(
		"QPushButton {"
		"  background-color: #007ACC;"
		"  color: #FFFFFF;"
		"  border: none;"
		"  border-radius: 6px;"
		"  font-size: 14px;"
		"  font-weight: bold;"
		"}"
		"QPushButton:hover { background-color: #1A8CDC; }"
		"QPushButton:pressed { background-color: #005A9E; }"
	));
	layout->addWidget(openBtn);

	// Shortcuts hint
	auto* hintLabel = new QLabel(QStringLiteral("Ctrl+O — Open file    F5 — Run all    Ctrl+S — Save source"));
	hintLabel->setStyleSheet(QStringLiteral("font-size: 11px; color: #858585;"));
	hintLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(hintLabel);

	outerLayout->addWidget(inner);

	// Connections
	connect(openBtn, &QPushButton::clicked, this, &WelcomePage::openFileClicked);
	connect(recentList, &QListWidget::itemDoubleClicked,
	        this, &WelcomePage::onRecentDoubleClicked);
	connect(quickOpen, &QLineEdit::returnPressed,
	        this, &WelcomePage::onQuickOpenReturn);
}

void WelcomePage::onRecentDoubleClicked(QListWidgetItem* item)
{
	QString path = item->data(Qt::UserRole).toString();
	if (!path.isEmpty())
		emit fileSelected(path);
}

void WelcomePage::onQuickOpenReturn()
{
	QString filter = quickOpen->text().trimmed();
	if (filter.isEmpty()) return;
	// Search in recent files
	for (int i = 0; i < recentList->count(); ++i) {
		QListWidgetItem* item = recentList->item(i);
		if (item->text().contains(filter, Qt::CaseInsensitive)) {
			QString path = item->data(Qt::UserRole).toString();
			if (!path.isEmpty())
				emit fileSelected(path);
			return;
		}
	}
}

void WelcomePage::setRecentFiles(const QStringList& files)
{
	recentPaths = files;
	recentList->clear();
	for (const auto& f : files) {
		auto* item = new QListWidgetItem(QFileInfo(f).fileName());
		item->setData(Qt::UserRole, f);
		item->setToolTip(f);
		recentList->addItem(item);
	}
	saveRecentFiles();
}

void WelcomePage::addRecentFile(const QString& path)
{
	recentPaths.removeAll(path);
	recentPaths.prepend(path);
	if (recentPaths.size() > MAX_RECENT)
		recentPaths.resize(MAX_RECENT);
	setRecentFiles(recentPaths);
}

QStringList WelcomePage::recentFiles() const
{
	return recentPaths;
}

void WelcomePage::loadRecentFiles()
{
	QSettings settings("CompilerTheory", "CompilerGUI");
	QStringList files = settings.value("recentFiles").toStringList();
	setRecentFiles(files);
}

void WelcomePage::saveRecentFiles()
{
	QSettings settings("CompilerTheory", "CompilerGUI");
	settings.setValue("recentFiles", recentPaths);
}

void WelcomePage::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	animateEntrance();
}

void WelcomePage::animateEntrance()
{
	QList<QWidget*> targets = {titleLabel, quickOpen, recentList, openBtn};
	auto* group = new QParallelAnimationGroup(this);

	for (int i = 0; i < targets.size(); ++i) {
		auto* w = targets[i];
		auto* effect = new QGraphicsOpacityEffect(w);
		effect->setOpacity(0.0);
		w->setGraphicsEffect(effect);

		auto* anim = new QPropertyAnimation(effect, "opacity");
		anim->setDuration(350);
		anim->setStartValue(0.0);
		anim->setEndValue(1.0);
		anim->setEasingCurve(QEasingCurve::OutCubic);
		// Stagger: 80ms delay per element
		anim->setCurrentTime(-i * 80);

		group->addAnimation(anim);
	}

	group->start(QAbstractAnimation::DeleteWhenStopped);
}
