#include "worker.h"
#include <QDebug>

void Worker::recRemove(const QStringList &paths) {
    if (paths.isEmpty()) return;

    for (const QString &p : paths) {
        QString can = QFileInfo(p).canonicalFilePath();
        if (can.isEmpty() || can == "/" || can.startsWith("/proc") || can.startsWith("/sys")) {
            emit taskFinished("Unsafe delete attempt", false);
            return;
        }
    }

    QProcess proc;
    proc.start("rm", QStringList{"-rf"} << paths);
    proc.waitForFinished(-1);
    bool ok = (proc.exitCode() == 0);
    emit taskFinished("Delete", ok);
}

void Worker::createDirFunc(const QString &parentPath, const QString &name) {
    QString fullPath = parentPath + QDir::separator() + name;
    QDir dir;
    bool ok = dir.mkdir(fullPath);
    emit taskFinished(QString("Create directory %1").arg(fullPath), ok);
}

void Worker::createFileFunc(const QString &parentPath, const QString &name) {
    QString fullPath = parentPath + QDir::separator() + name;
    QFile file(fullPath);
    bool ok = file.open(QIODevice::WriteOnly);
    if (ok) file.close();
    emit taskFinished(QString("Create file %1").arg(fullPath), ok);
}

void Worker::renameFunc(const QString &oldPath, const QString &newPath) {
    bool ok = QFile::rename(oldPath, newPath);
    emit taskFinished(QString("Rename %1 to %2").arg(oldPath, newPath), ok);
}

void Worker::moveItems(const QStringList &sources, const QString &destinationDir) {
    qDebug() << "Moving to:" << destinationDir;
    qDebug() << "Sources:" << sources;

    for (const QString &src : sources) {
        QProcess process;
        QStringList args{"-f", src, destinationDir};
        process.start("mv", args);
        process.waitForFinished(-1);

        if (process.exitCode() != 0) {
            qDebug() << "mv failed:" << process.readAllStandardError();
            emit taskFinished("Move", false);
            return;
        }
    }
    emit taskFinished("Move completed", true);
}

void Worker::copyItems(const QStringList &sources, const QString &destinationDir) {
    for (const QString &src : sources) {
        QProcess proc;
        proc.start("cp", {"-rf", src, destinationDir});
        proc.waitForFinished(-1);
        if (proc.exitCode() != 0) {
            emit taskFinished("Copy", false);
            return;
        }
    }
    emit taskFinished("Copy completed", true);
}
