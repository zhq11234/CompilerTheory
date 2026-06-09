#ifndef ASTTREEDELEGATE_H
#define ASTTREEDELEGATE_H

#include <QStyledItemDelegate>
#include <QColor>

class ASTTreeDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	explicit ASTTreeDelegate(QObject* parent = nullptr);

protected:
	void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};

#endif
