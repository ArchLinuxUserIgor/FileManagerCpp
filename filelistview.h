#pragma once
#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <QListView>
#include <QDropEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QContextMenuEvent>
#include <QPoint>
#include <QMouseEvent>

class FileListView : public QListView {
    Q_OBJECT

public:
    explicit FileListView(QWidget *parent = nullptr);

    void setRootIndexAnimated(const QModelIndex &index);

signals:
    void filesDropped(const QStringList &sources, const QString &targetDir, Qt::DropAction action);
    void customContextMenuRequestedAnimated(const QPoint &globalPos, const QModelIndex &index);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onFadeOutFinished();

private:
    QModelIndex pendingIndex;
    bool isAnimating = false;
    QPoint dragStartPos;
    bool dragInProgress = false;
};

#endif // FILELISTVIEW_H
