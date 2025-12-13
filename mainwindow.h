#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pathBar.h"
#include "worker.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QListView>
#include <QPushButton>
#include <QWidget>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QString>
#include <QFileIconProvider>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <QDateTime>
#include <QInputDialog>
#include <QTreeView>
#include <QThread>
#include <QItemSelectionModel>
#include <QLocale>
#include <QCoreApplication>
#include <QSettings>
#include <QDirIterator>
#include <QFontMetrics>
#include <QTimer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void changeThemeFunc();
    QString detectStylePath();
    void styleReader(const QString &filename);
    void changeDir(const QModelIndex &index);
    void changeDir(const QString& path);
    void goToParentOrChildDir();
    void showContextMenu(const QPoint &pos);
    void renameFunc(const QModelIndex &index, const QString &newName, const QString &originalName);
    QDateTime getCreatedTime(const QString &path) {
        struct statx stx;
        memset(&stx, 0, sizeof(stx));

        int ret = statx(AT_FDCWD, path.toUtf8().constData(), AT_STATX_SYNC_AS_STAT, STATX_BTIME, &stx);

        if (ret == 0 && (stx.stx_mask & STATX_BTIME)) {
            return QDateTime::fromSecsSinceEpoch(stx.stx_btime.tv_sec);
        }

        return QDateTime();
    }
    void createDirFunc(const QModelIndex &index);
    void createFileFunc(const QModelIndex &index);
    void moveFunc(const QModelIndex &index);
    void copyPasteFunc(const QModelIndex &index);
    void showFileInf(const QModelIndex &index);

private:
    Worker *worker;
    QThread *workerThread;
    QString filename;
    QVBoxLayout *fileInfLayout;
    QFileSystemModel *fileSystem;
    QListView *fileListView;
    QAction *changeTheme;
    QString currentPath;
    QAction *goToParent;
    QStringList historyBack;
    QStringList historyForward;
    QPushButton *picBtn;
    QAction *showHidden;
    bool checked;
    QString fileNameStr;
    QAction *toolbarCreatingDir;
    QAction *toolbarCreatingFile;
    QHBoxLayout *pathLayout;
    PathBar *pathBar;
    QAction *toolbarMoveAction;
    QString clipboardPath;
    bool isCut;
    QAction *toolbarCopyAct;
    QAction *toolbarCutAct;
    QAction *toolbarPasteAct;
    QAction *goToHome;

signals:
    void requestDelete(const QString &path);
    void requestCreateDir(const QString &parent, const QString &name);
    void requestCreateFile(const QString &parent, const QString &name);
    void requestRename(const QString &oldPath, const QString &newPath);
    void requestMove(const QString &source, const QString &destination);
    void requestCopy(const QString &source, const QString &destination);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openPath(const QString& path);
};
#endif // MAINWINDOW_H
