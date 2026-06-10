#ifndef TOKENCARDWIDGET_H
#define TOKENCARDWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <vector>
#include "token.h"

class TokenCardWidget : public QWidget {
	Q_OBJECT
	Q_PROPERTY(qreal animationProgress READ animationProgress WRITE setAnimationProgress)
public:
	explicit TokenCardWidget(QWidget* parent = nullptr);

	void setTokens(const std::vector<Token>& tokens);
	void setTypeFilter(int tokenType);  // -1 = all
	int typeFilter() const { return filterType; }

	qreal animationProgress() const { return m_animProgress; }
	void setAnimationProgress(qreal p);

signals:
	void tokenClicked(int sourceLine);

protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;

private:
	struct Card {
		Token token;
		QRectF rect;
		bool visible = true;
	};
	std::vector<Card> cards;
	int filterType = -1;
	qreal m_animProgress = 1.0;
	int hoveredIndex = -1;

	void computeLayout();
	int cardAtPosition(const QPointF& pos) const;
	QColor cardColor(int tokenType) const;
};

class TokenTypeBar : public QWidget {
	Q_OBJECT
public:
	explicit TokenTypeBar(QWidget* parent = nullptr);
signals:
	void filterChanged(int type);  // -1 = all
private:
	QPushButton* makeBtn(const QString& text, int type);
	std::vector<QPushButton*> btns;
	int currentFilter = -1;
	void updateHighlight();
};

#endif // TOKENCARDWIDGET_H
