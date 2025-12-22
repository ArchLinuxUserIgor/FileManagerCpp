#include "filelistview.h"
#include <QMimeData>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QDrag>
#include <QMouseEvent>
#include <QTimer>
#include <QApplication>

FileListView::FileListView(QWidget *parent) : QListView(parent) {
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void FileListView::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void FileListView::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

void FileListView::startDrag(Qt::DropActions supportedActions) {
    auto *fsm = qobject_cast<QFileSystemModel*>(model());
    if (!fsm)
        return;

    QSet<QString> uniquePaths;
    for (const QModelIndex &idx : selectedIndexes())
        uniquePaths.insert(fsm->filePath(idx));

    if (uniquePaths.isEmpty())
        return;

    QList<QUrl> urls;
    for (const QString &path : uniquePaths)
        urls << QUrl::fromLocalFile(path);

    auto *mime = new QMimeData;
    mime->setUrls(urls);

    auto *drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->exec(supportedActions);

}

void FileListView::dropEvent(QDropEvent *event) {
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    auto *fsm = qobject_cast<QFileSystemModel*>(model());
    if (!fsm) return;

    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPoint pos = event->position().toPoint();
    #else
        QPoint pos = event->pos();
    #endif

    QModelIndex idx = indexAt(pos);

    QString targetDir;

    if (idx.isValid()) {
        QFileInfo info(fsm->filePath(idx));
        targetDir = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    } else {
        targetDir = fsm->filePath(rootIndex());
    }

    QStringList sources;
    for (const QUrl &url : event->mimeData()->urls()) {
        sources << url.toLocalFile();
    }

    event->acceptProposedAction();
    emit filesDropped(sources, targetDir, event->proposedAction());
}

void FileListView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) dragStartPos = event->pos();
    QListView::mousePressEvent(event);
}

void FileListView::mouseMoveEvent(QMouseEvent *event) {
    if (dragInProgress) return;

    if (!(event->buttons() & Qt::LeftButton)) return;

    if ((event->pos() - dragStartPos).manhattanLength() < QApplication::startDragDistance()) return;

    dragInProgress = true;
    startDrag(Qt::CopyAction | Qt::MoveAction);
    dragInProgress = false;
}

void FileListView::setRootIndexAnimated(const QModelIndex &index) {
    if (isAnimating || !index.isValid()) return;

    isAnimating = true;
    pendingIndex = index;

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);

    QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(100);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);

    connect(fadeOut, &QPropertyAnimation::finished, this, &FileListView::onFadeOutFinished);
}

void FileListView::onFadeOutFinished() {
    setRootIndex(pendingIndex);
    pendingIndex = QModelIndex();

    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (!effect) {
        isAnimating = false;
        return;
    }

    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(120);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    connect(fadeIn, &QPropertyAnimation::finished, this, [this]() {
        setGraphicsEffect(nullptr);
        isAnimating = false;
    });
}

void FileListView::contextMenuEvent(QContextMenuEvent *event) {
    QModelIndex index = indexAt(event->pos());
    emit customContextMenuRequestedAnimated(event->globalPos(), index);
}
