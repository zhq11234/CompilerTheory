#include "sidepanel.h"
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPixmap>

SidePanel::SidePanel(QWidget* parent)
	: QWidget(parent)
{
	setupUI();
}

void SidePanel::setPanelWidth(int w)
{
	m_panelWidth = w;
	setFixedWidth(w);
}

static QPixmap dotPixmap(int size, const QColor& color)
{
	QPixmap pm(size, size);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing);
	p.setBrush(color);
	p.setPen(Qt::NoPen);
	p.drawEllipse(1, 1, size - 2, size - 2);
	p.end();
	return pm;
}

void SidePanel::setupUI()
{
	int sidebarW = fontMetrics().horizontalAdvance(QLatin1Char('M')) * 22;
	setFixedWidth(sidebarW);
	setMinimumWidth(0);
	setStyleSheet(QStringLiteral(
		"SidePanel { background-color: #252526; border-right: 1px solid #3C3C3C; }"
	));

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	// --- Toggle button (top-right) ---
	auto* topBar = new QWidget;
	auto* topLayout = new QHBoxLayout(topBar);
	topLayout->setContentsMargins(8, 6, 6, 6);

	auto* title = new QLabel(QStringLiteral("EXPLORER"));
	title->setStyleSheet(QStringLiteral("font-size: 11px; color: #858585; font-weight: bold;"));
	topLayout->addWidget(title);
	topLayout->addStretch();

	btnToggle = new QPushButton(QStringLiteral("☰"));
	int btnSz = fontMetrics().height() * 2;
	btnToggle->setFixedSize(btnSz, btnSz);
	btnToggle->setCursor(Qt::PointingHandCursor);
	btnToggle->setStyleSheet(QStringLiteral(
		"QPushButton {"
		"  background: transparent; color: #858585; border: none; font-size: 14px;"
		"}"
		"QPushButton:hover { color: #D4D4D4; }"
	));
	topLayout->addWidget(btnToggle);
	layout->addWidget(topBar);

	connect(btnToggle, &QPushButton::clicked, this, &SidePanel::toggleRequested);

	// --- Step panel ---
	auto* stepLabel = new QLabel(QStringLiteral("  STEPS"));
	stepLabel->setStyleSheet(QStringLiteral("font-size: 10px; color: #858585; font-weight: bold; padding: 8px 0 4px 8px;"));
	layout->addWidget(stepLabel);

	stepPanel = new QWidget;
	auto* stepLayout = new QVBoxLayout(stepPanel);
	stepLayout->setContentsMargins(4, 0, 4, 4);
	stepLayout->setSpacing(2);

	const char* stageNames[] = {"Lexer", "Parser", "Semantic", "IR"};
	for (int i = 0; i < 4; ++i) {
		auto* row = new QWidget;
		row->setCursor(Qt::PointingHandCursor);
		row->setStyleSheet(QStringLiteral("QWidget:hover { background-color: #2D2D2D; border-radius: 4px; }"));
		auto* rl = new QHBoxLayout(row);
		rl->setContentsMargins(8, 4, 8, 4);

		stageRows[i].dot = new QLabel;
		int dotSz = fontMetrics().height();
		stageRows[i].dot->setFixedSize(dotSz, dotSz);
		stageRows[i].dot->setPixmap(dotPixmap(12, QColor("#3C3C3C")));
		rl->addWidget(stageRows[i].dot);

		stageRows[i].name = new QLabel(QString::fromUtf8(stageNames[i]));
		stageRows[i].name->setStyleSheet(QStringLiteral("color: #D4D4D4; font-size: 12px;"));
		rl->addWidget(stageRows[i].name);
		rl->addStretch();

		stageRows[i].status = new QLabel;
		stageRows[i].status->setStyleSheet(QStringLiteral("color: #858585; font-size: 10px;"));
		rl->addWidget(stageRows[i].status);

		stageRows[i].widget = row;
		int stageIdx = i;
		row->installEventFilter(this);
		// Use a simple clickable widget approach
		row->setProperty("stageIndex", stageIdx);

		stepLayout->addWidget(row);

		// Make row clickable via mouse press
		row->setAttribute(Qt::WA_Hover);
	}

	layout->addWidget(stepPanel);

	// --- Separator ---
	auto* sep = new QFrame;
	sep->setFrameShape(QFrame::HLine);
	sep->setStyleSheet(QStringLiteral("QFrame { color: #3C3C3C; margin: 8px 12px; }"));
	layout->addWidget(sep);

	// --- Navigation panel ---
	auto* navLabel = new QLabel(QStringLiteral("  NAVIGATION"));
	navLabel->setStyleSheet(QStringLiteral("font-size: 10px; color: #858585; font-weight: bold; padding: 4px 0 4px 8px;"));
	layout->addWidget(navLabel);

	navPanel = new QWidget;
	auto* navLayout = new QVBoxLayout(navPanel);
	navLayout->setContentsMargins(4, 0, 4, 4);
	navLayout->setSpacing(2);

	struct NavItem { QString name; int tab; };
	const NavItem navItems[] = {
		{QStringLiteral("Token"), 0},
		{QStringLiteral("AST"), 1},
		{QStringLiteral("Symbol Table"), 2},
		{QStringLiteral("IR"), 3},
		{QStringLiteral("Errors"), 4}
	};

	for (const auto& item : navItems) {
		auto* btn = new QPushButton(item.name);
		btn->setCursor(Qt::PointingHandCursor);
		btn->setStyleSheet(QStringLiteral(
			"QPushButton {"
			"  background: transparent; color: #D4D4D4; border: none;"
			"  text-align: left; padding: 5px 12px; font-size: 11px;"
			"}"
			"QPushButton:hover { background-color: #2D2D2D; }"
			"QPushButton:pressed { background-color: #094771; }"
		));
		int tabIdx = item.tab;
		connect(btn, &QPushButton::clicked, this, [this, tabIdx]() {
			emit navigationRequested(tabIdx);
		});
		navLayout->addWidget(btn);
	}

	layout->addWidget(navPanel);
	layout->addStretch();
}

void SidePanel::startPulse(int stage)
{
	if (pulseAnims[stage]) return;
	auto* anim = new QVariantAnimation(this);
	anim->setDuration(800);
	anim->setStartValue(0.3);
	anim->setEndValue(1.0);
	anim->setLoopCount(-1);
	anim->setEasingCurve(QEasingCurve::InOutSine);
	connect(anim, &QVariantAnimation::valueChanged, this, [this, stage](const QVariant& v) {
		qreal alpha = v.toReal();
		QColor c("#DCDCAA");
		c.setAlphaF(alpha);
		stageRows[stage].dot->setPixmap(dotPixmap(12, c));
	});
	anim->start();
	pulseAnims[stage] = anim;
}

void SidePanel::stopPulse(int stage)
{
	if (pulseAnims[stage]) {
		pulseAnims[stage]->stop();
		pulseAnims[stage]->deleteLater();
		pulseAnims[stage] = nullptr;
	}
}

void SidePanel::setStageStatus(int stage, bool running, bool success, bool hasErrors)
{
	if (stage < 0 || stage > 3) return;

	StageRow& row = stageRows[stage];

	if (running) {
		startPulse(stage);
		row.status->setText(QStringLiteral("loading"));
		row.status->setStyleSheet(QStringLiteral("color: #DCDCAA; font-size: 10px;"));
	} else {
		stopPulse(stage);
	}

	if (!running) {
		if (hasErrors) {
			row.dot->setPixmap(dotPixmap(12, QColor("#F44747")));
			row.status->setText(QStringLiteral("errors"));
			row.status->setStyleSheet(QStringLiteral("color: #F44747; font-size: 10px;"));
		} else if (success) {
			row.dot->setPixmap(dotPixmap(12, QColor("#6A9955")));
			row.status->setText(QStringLiteral("done"));
			row.status->setStyleSheet(QStringLiteral("color: #6A9955; font-size: 10px;"));
		} else {
			row.dot->setPixmap(dotPixmap(12, QColor("#3C3C3C")));
			row.status->setText(QStringLiteral(""));
		}
	}
}
