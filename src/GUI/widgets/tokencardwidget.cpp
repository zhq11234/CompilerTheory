#include "tokencardwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <cmath>

// ==================== TokenTypeBar ====================

TokenTypeBar::TokenTypeBar(QWidget* parent) : QWidget(parent)
{
	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(2, 2, 2, 2);
	layout->setSpacing(2);

	layout->addWidget(makeBtn(QStringLiteral("All"), -1));
	layout->addWidget(makeBtn(QStringLiteral("Keyword"), TOKEN_KEYWORD));
	layout->addWidget(makeBtn(QStringLiteral("ID"), TOKEN_IDENTIFIER));
	layout->addWidget(makeBtn(QStringLiteral("Num"), TOKEN_NUMBER));
	layout->addWidget(makeBtn(QStringLiteral("Op"), TOKEN_OPERATOR));
	layout->addWidget(makeBtn(QStringLiteral("Delim"), TOKEN_DELIMITER));
	layout->addStretch();

	// Default: "All" checked
	btns[0]->setChecked(true);
}

QPushButton* TokenTypeBar::makeBtn(const QString& text, int type)
{
	auto* btn = new QPushButton(text);
	btn->setCheckable(true);
	int fh = fontMetrics().height(); btn->setFixedHeight(fh * 2);
	btn->setStyleSheet(QStringLiteral(
		"QPushButton {"
		"  background: #2D2D2D; color: #858585; border: 1px solid #3C3C3C;"
		"  border-radius: 3px; padding: 2px 8px; font-size: 10px;"
		"}"
		"QPushButton:hover { color: #D4D4D4; border-color: #555; }"
		"QPushButton:checked { background: #094771; color: #FFFFFF; border-color: #007ACC; }"
	));
	int tag = type;
	connect(btn, &QPushButton::clicked, this, [this, tag]() {
		currentFilter = tag;
		// Update all buttons: only the matching one is checked
		for (auto* b : btns) b->setChecked(false);
		// Find the button for this filter and check it
		for (size_t i = 0; i < btns.size(); ++i) {
			int btnType = (i == 0) ? -1 : static_cast<int>(i); // 0=All(-1), 1=Keyword(1), 2=ID(2), 3=Num(3), 4=Op(4), 5=Delim(5)
			if (i == 0 && tag == -1) { btns[i]->setChecked(true); break; }
			if (static_cast<int>(i) == tag) { btns[i]->setChecked(true); break; }
		}
		emit filterChanged(tag);
	});
	btns.push_back(btn);
	return btn;
}

void TokenTypeBar::updateHighlight() {}

// ==================== TokenCardWidget ====================

TokenCardWidget::TokenCardWidget(QWidget* parent)
	: QWidget(parent)
{
	setMouseTracking(true);
	setMinimumHeight(120);
	setStyleSheet(QStringLiteral("background-color: #1E1E1E;"));
}

void TokenCardWidget::setAnimationProgress(qreal p)
{
	m_animProgress = p;
	update();
}

QColor TokenCardWidget::cardColor(int type) const
{
	switch (type) {
	case TOKEN_KEYWORD:    return QColor("#569CD6");
	case TOKEN_IDENTIFIER: return QColor("#4EC9B0");
	case TOKEN_NUMBER:     return QColor("#CE9178");
	case TOKEN_OPERATOR:   return QColor("#D4D4D4");
	case TOKEN_DELIMITER:  return QColor("#D4D4D4");
	default:               return QColor("#D4D4D4");
	}
}

void TokenCardWidget::setTokens(const std::vector<Token>& tokens)
{
	cards.clear();
	for (const auto& t : tokens) {
		Card c;
		c.token = t;
		c.visible = (filterType == -1 || t.type == filterType);
		cards.push_back(c);
	}
	computeLayout();

	// Staggered entrance: animate progress 0→1 over 500ms
	m_animProgress = 0.0;
	auto* anim = new QPropertyAnimation(this, "animationProgress");
	anim->setDuration(500);
	anim->setStartValue(0.0);
	anim->setEndValue(1.0);
	anim->setEasingCurve(QEasingCurve::OutCubic);

	// Use QTimer to delay start slightly, ensuring widget is ready
	QTimer::singleShot(50, this, [anim]() {
		anim->start(QAbstractAnimation::DeleteWhenStopped);
	});

	update();
}

void TokenCardWidget::setTypeFilter(int type)
{
	filterType = type;
	for (auto& c : cards)
		c.visible = (filterType == -1 || c.token.type == filterType);
	computeLayout();
	m_animProgress = 1.0;
	update();
}

void TokenCardWidget::computeLayout()
{
	int charW = fontMetrics().horizontalAdvance(QLatin1Char('M'));
	const qreal margin = fontMetrics().height() * 0.6;
	const qreal cardW = qMax(80.0, charW * 8.0);
	const qreal cardH = fontMetrics().height() * 2.2;
	const qreal spacing = fontMetrics().height() * 0.4;
	qreal x = margin;
	qreal y = margin;
	qreal maxW = qMax(100.0, width() - 2.0 * margin);

	for (auto& c : cards) {
		if (!c.visible) continue;
		if (x + cardW > maxW) {
			x = margin;
			y += cardH + spacing;
		}
		c.rect = QRectF(x, y, cardW, cardH);
		x += cardW + spacing;
	}
}

int TokenCardWidget::cardAtPosition(const QPointF& pos) const
{
	for (size_t i = 0; i < cards.size(); ++i) {
		if (cards[i].visible && cards[i].rect.contains(pos))
			return static_cast<int>(i);
	}
	return -1;
}

void TokenCardWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(rect(), QColor("#1E1E1E"));

	if (cards.empty()) {
		painter.setPen(QColor("#858585"));
		painter.setFont(QFont("JetBrains Mono", 12));
		painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("No tokens loaded"));
		return;
	}

	QFont font("JetBrains Mono", 10);
	painter.setFont(font);

	for (size_t i = 0; i < cards.size(); ++i) {
		const auto& card = cards[i];
		if (!card.visible) continue;

		// Better stagger: each card appears when progress reaches its threshold
		qreal staggerStart = static_cast<qreal>(i) / qMax((size_t)1, cards.size());
		qreal cardProgress = qBound(0.0, (m_animProgress - staggerStart) / (1.0 - staggerStart + 0.01), 1.0);
		if (cardProgress <= 0.0) continue;

		painter.setOpacity(cardProgress);

		QColor bg = cardColor(card.token.type);
		QRectF r = card.rect;

		// Hover scale + lighten
		if (static_cast<int>(i) == hoveredIndex) {
			QPointF c = r.center();
			r.setSize(r.size() * 1.08);
			r.moveCenter(c);
			painter.setBrush(bg.lighter(130));
		} else {
			painter.setBrush(bg.darker(180));
		}

		painter.setPen(QPen(bg, 1.5));

		QPainterPath path;
		path.addRoundedRect(r, 5, 5);
		painter.drawPath(path);

		// Text
		painter.setPen(QColor("#FFFFFF"));
		QString text = QString::fromStdString(card.token.value);
		QRectF textRect = r.adjusted(3, 2, -3, -2);
		painter.drawText(textRect, Qt::AlignCenter,
		                 painter.fontMetrics().elidedText(text, Qt::ElideRight,
		                                                 static_cast<int>(textRect.width())));
	}
	painter.setOpacity(1.0);
}

void TokenCardWidget::resizeEvent(QResizeEvent*)
{
	computeLayout();
	update();
}

void TokenCardWidget::mouseMoveEvent(QMouseEvent* event)
{
	hoveredIndex = cardAtPosition(event->pos());
	update();
}

void TokenCardWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		int idx = cardAtPosition(event->pos());
		if (idx >= 0) {
			emit tokenClicked(cards[idx].token.line);
		}
	}
}
