#include "asttreedelegate.h"
#include <QApplication>

ASTTreeDelegate::ASTTreeDelegate(QObject* parent)
	: QStyledItemDelegate(parent) {}

void ASTTreeDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
	QStyledItemDelegate::initStyleOption(option, index);

	// Column 0 gets type-coloured text; other columns stay dim
	if (index.column() == 0) {
		QVariant colorVar = index.data(Qt::UserRole + 10);
		if (colorVar.isValid()) {
			QColor c = colorVar.value<QColor>();
			option->palette.setColor(QPalette::Text, c);
			option->palette.setColor(QPalette::HighlightedText, c.lighter(150));
		}
	} else {
		option->palette.setColor(QPalette::Text, QColor("#858585"));
		option->palette.setColor(QPalette::HighlightedText, QColor("#AAAAAA"));
	}
}
