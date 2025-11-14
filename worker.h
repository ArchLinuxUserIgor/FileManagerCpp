#ifndef WORKER_H
#define WORKER_H
#pragma once

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QProcess>

class Worker : public QObject {
    Q_OBJECT

public slots:
    void recRemove(const QString &path);
    void createDirFunc(const QString &parentPath, const QString &name);
    void createFileFunc(const QString &parentPath, const QString &name);
    void renameFunc(const QString &oldPath, const QString &newPath);
    void moveItem(const QString &source, const QString &destination);
    void copyItem(const QString &source, const QString &destination);

signals:
    void taskFinished(const QString &action, bool ok);
};

#endif // WORKER_H
