#include "fileitemdelegate.h"
#include <QApplication>

QSize FileItemDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const {
    QFontMetrics fm(opt.font);
    int height = qMax(opt.decorationSize.height(), fm.height()) + 8;

    const QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    const QString text = index.data(Qt::DisplayRole).toString();

    int textWidth = fm.horizontalAdvance(text);

    return QSize(
        opt.decorationSize.width() + 8 + textWidth + 16,
        height
        );

}

void FileItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const {
    QStyleOptionViewItem option(opt);
    initStyleOption(&option, index);

    option.decorationPosition = QStyleOptionViewItem::Left;
    option.displayAlignment = Qt::AlignCenter | Qt::AlignLeft;

    QStyle *style = option.widget ? option.widget->style() : QApplication::style();

    style->drawControl(QStyle::CE_ItemViewItem, &option, p, option.widget);
}
