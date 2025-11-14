#include "worker.h"

void Worker::recRemove(const QString &path) {
    bool ok = true;
    QFileInfo fileInfo(path);

    if (fileInfo.isFile() || fileInfo.isSymLink()) {
        ok = QFile::remove(path);
    } else {
        QDir dir(path);
        if (dir.exists()) {
            for (const QFileInfo &info : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden)) {
                QString sub = info.absoluteFilePath();
                if (info.isDir()) {
                    recRemove(sub);
                } else {
                    QFile::remove(sub);
                }
            }
            ok = dir.rmdir(path);
        } else {
            ok = false;
        }
    }

    emit taskFinished(QString("Delete %1").arg(path), ok);
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

void Worker::moveItem(const QString &source, const QString &destination) {
    QProcess process;
    QStringList args;
    args << source << destination;
    process.start("mv", args);
    process.waitForFinished(-1);

    bool ok = (process.exitCode() == 0);
    emit taskFinished("Move", ok);
}

void Worker::copyItem(const QString &source, const QString &destination) {
    QProcess process;
    QStringList args;
    args << "-r" << source << destination;
    process.start("cp", args);
    process.waitForFinished(-1);

    bool ok = (process.exitCode() == 0);
    emit taskFinished("Copy", ok);
}
