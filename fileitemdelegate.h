#ifndef FILEITEMDELEGATE_H
#define FILEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QString>
#include <QModelIndex>
#include <QFontMetrics>
#include <QPainter>

class FileItemDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override;

    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const override;
};

#endif // FILEITEMDELEGATE_H
