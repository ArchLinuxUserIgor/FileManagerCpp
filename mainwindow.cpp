#include "mainwindow.h"
#include "fileitemdelegate.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("File Manager");

    fileSystem = new QFileSystemModel();
    fileSystem->setRootPath(QDir::homePath());
    fileSystem->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    pathLayout = new QHBoxLayout();
    pathBar = new PathBar(this);
    pathBar->setPath(QDir::homePath());
    pathBar->setFixedHeight(30);
    pathLayout->addWidget(pathBar);

    changeTheme = new QAction(tr("Change Theme"), this);

    goToParent = new QAction(this);
    goToParent->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));

    showHidden = new QAction(tr("Show Hidden"), this);
    showHidden->setCheckable(true);

    toolbarCreatingDir = new QAction(this);
    toolbarCreatingDir->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    toolbarCreatingDir->setToolTip("New Directory");

    toolbarCreatingFile = new QAction(this);
    toolbarCreatingFile->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    toolbarCreatingFile->setToolTip("New File");

    toolbarMoveAction = new QAction(this);
    toolbarMoveAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogStart));
    toolbarMoveAction->setToolTip("Move");

    toolbarCopyAct = new QAction(this);
    toolbarCopyAct->setText("🗗");
    toolbarCopyAct->setToolTip("Copy");

    toolbarCutAct = new QAction(this);
    toolbarCutAct->setText("✂");
    toolbarCutAct->setToolTip("Cut");

    toolbarPasteAct = new QAction(this);
    toolbarPasteAct->setText("📋");
    toolbarPasteAct->setToolTip("Paste");

    goToHome = new QAction(this);
    goToHome->setIcon(style()->standardIcon(QStyle::SP_DirHomeIcon));
    goToHome->setToolTip("Go to home directory");

    QToolBar *toolbar = new QToolBar();
    toolbar->addAction(changeTheme);
    toolbar->addSeparator();
    toolbar->addAction(goToHome);
    toolbar->addAction(goToParent);
    toolbar->addAction(showHidden);
    toolbar->addSeparator();
    toolbar->addAction(toolbarCreatingDir);
    toolbar->addAction(toolbarCreatingFile);
    toolbar->addAction(toolbarMoveAction);
    toolbar->addSeparator();
    toolbar->addAction(toolbarCopyAct);
    toolbar->addAction(toolbarCutAct);
    toolbar->addAction(toolbarPasteAct);

    connect(toolbarCreatingDir, &QAction::triggered, this, [this]() {
        createDirFunc(QModelIndex());
    });
    connect(toolbarCreatingFile, &QAction::triggered, this, [this]() {
        createFileFunc(QModelIndex());
    });

    connect(toolbarMoveAction, &QAction::triggered, this, [this]() {
        QItemSelectionModel *selectionModel = fileListView->selectionModel();

        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

        moveFunc(selectedIndexes.first());
    });

    connect(toolbarCopyAct, &QAction::triggered, this, [this]() {
        QItemSelectionModel *sel = fileListView->selectionModel();
        QModelIndexList idxs = sel->selectedIndexes();
        clipboardPaths.clear();
        for (const QModelIndex &idx : idxs) {
            if (idx.column() == 0) {
                clipboardPaths << fileSystem->filePath(idx);
            }
        }
        isCut = false;
    });
    connect(toolbarCutAct, &QAction::triggered, this, [this]() {
        QItemSelectionModel *sel = fileListView->selectionModel();
        QModelIndexList idxs = sel->selectedIndexes();
        clipboardPaths.clear();
        for (const QModelIndex &idx : idxs) {
            if (idx.column() == 0) {
                clipboardPaths << fileSystem->filePath(idx);
            }
        }
        isCut = true;
    });

    connect(toolbarPasteAct, &QAction::triggered, this, [this]() {
        copyPasteFunc(fileListView->rootIndex());
    });

    connect(goToHome, &QAction::triggered, this, [this]() {
        QModelIndex index = fileSystem->index(QDir::homePath());
        changeDir(index);
    });

    QHBoxLayout *listLayout = new QHBoxLayout();


    fileListView = new FileListView(this);
    fileListView->setModel(fileSystem);
    fileListView->setItemDelegate(new FileItemDelegate(fileListView));
    fileListView->setFlow(QListView::TopToBottom);
    fileListView->setWrapping(false);
    fileListView->setSpacing(2);
    fileListView->setUniformItemSizes(false);
    fileListView->setViewMode(QListView::IconMode);
    fileListView->setWordWrap(false);
    fileListView->setResizeMode(QListView::Adjust);
    fileListView->setMovement(QListView::Snap);
    fileListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileListView->setSelectionBehavior(QAbstractItemView::SelectItems);
    fileListView->setDragEnabled(true);
    fileListView->setAcceptDrops(true);
    fileListView->setDragDropMode(QAbstractItemView::DragDrop);

    QModelIndex rootIndexView = fileSystem->index(QDir::homePath());
    fileListView->setRootIndexAnimated(rootIndexView);
    currentPath = QDir::homePath();

    connect(fileListView, &FileListView::customContextMenuRequestedAnimated, this, &MainWindow::showContextMenu);

    QVBoxLayout *treeLayout = new QVBoxLayout();

    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    buttonsLayout->addSpacing(30);

    QIcon downloadsIcon = style()->standardIcon(QStyle::SP_FileDialogToParent);

    QPushButton *downloadsBtn = new QPushButton("Downloads");

    downloadsBtn->setIcon(downloadsIcon);

    buttonsLayout->addWidget(downloadsBtn);

    buttonsLayout->setAlignment(Qt::AlignTop);

    picBtn = new QPushButton("Pictures");

    QFileIconProvider provider;

    QString fullPath = QDir::homePath() + QDir::separator() + "Pictures";

    QIcon picIcon = provider.icon(QFileInfo(fullPath));
    picBtn->setIcon(picIcon);

    buttonsLayout->addWidget(picBtn);

    QPushButton *videosBtn = new QPushButton("Videos");

    QString vidFullPath = QDir::homePath() + QDir::separator() + "Videos";

    videosBtn->setIcon(provider.icon(QFileInfo(vidFullPath)));

    buttonsLayout->addWidget(videosBtn);

    QString docFullPath = QDir::homePath() + QDir::separator() + "Documents";

    QPushButton *docBtn = new QPushButton("Documents");

    docBtn->setIcon(provider.icon(QFileInfo(docFullPath)));

    buttonsLayout->addWidget(docBtn);

    connect(showHidden, &QAction::triggered, this, [=] {
        bool checked = showHidden->isChecked();
        if (checked) {
            fileSystem->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);
        } else {
            fileSystem->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
        }
    });

    connect(changeTheme, &QAction::triggered, this, &MainWindow::changeThemeFunc);

    connect(fileListView, &QAbstractItemView::doubleClicked, this, QOverload<const QModelIndex &>::of(&MainWindow::changeDir));
    connect(goToParent, &QAction::triggered, this, &MainWindow::goToParentOrChildDir);
    connect(downloadsBtn, &QPushButton::clicked, this, [=]() {
        QString homePath = QDir::homePath();

        QString fullPath = homePath + QDir::separator() + "Downloads";

        QModelIndex index = fileSystem->index(fullPath);

        if (!index.isValid()) return;

        changeDir(index);
    });

    connect(picBtn, &QPushButton::clicked, this, [this, fullPath]() {
        QModelIndex index = fileSystem->index(fullPath);
        if (!index.isValid()) return;
        changeDir(index);
    });

    connect(videosBtn, &QPushButton::clicked, this, [this, vidFullPath]() {
        QModelIndex index = fileSystem->index(vidFullPath);
        if(!index.isValid()) return;
        changeDir(index);
    });

    connect(docBtn, &QPushButton::clicked, this, [this, docFullPath]() {
        QModelIndex index = fileSystem->index(docFullPath);
        if(!index.isValid()) return;
        changeDir(index);
    });

    connect(pathBar, &PathBar::pathClicked, this, [this](const QString &path) {
        QModelIndex index = fileSystem->index(path);
        if (index.isValid()) {
            changeDir(index);
            pathBar->setPath(path);
        } else {
            QMessageBox::critical(this, "Fail", "No such path", QMessageBox::Ok);
        }
    });

    connect(pathBar, &PathBar::pathEntered, this, [this](const QString &path) {
        QModelIndex index = fileSystem->index(path);
        if (index.isValid()) {
            changeDir(index);
            pathBar->setPath(path);
        } else {
            QMessageBox::critical(this, "Fail", "No such path", QMessageBox::StandardButton::Ok);
        }
    });

    worker = new Worker();
    workerThread = new QThread(this);
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(worker, &Worker::taskFinished, this, [this](const QString &action, bool ok) {
        if (ok && isCut && (action.contains("Move") || action == "Move completed")) {
            clipboardPaths.clear();
            isCut = false;
        }
        if (ok) {
            QMessageBox::information(this, "Done", action + " completed successfully");
        } else {
            QMessageBox::critical(this, "Failed", action + " failed", QMessageBox::StandardButton::Ok);
        }

        fileListView->setRootIndexAnimated(fileSystem->setRootPath(currentPath));
    });

    workerThread->start();

    connect(this, &MainWindow::requestDelete, worker, &Worker::recRemove);
    connect(this, &MainWindow::requestCreateDir, worker, &Worker::createDirFunc);
    connect(this, &MainWindow::requestCreateFile, worker, &Worker::createFileFunc);
    connect(this, &MainWindow::requestRename, worker, &Worker::renameFunc);
    connect(this, &MainWindow::requestMove, worker, &Worker::moveItems);
    connect(this, &MainWindow::requestCopy,  worker, &Worker::copyItems);
    connect(fileListView, &FileListView::filesDropped, this, [this](const QStringList &sources, const QString &targetDir, Qt::DropAction action) {
        //const QString platform = QApplication::platformName();
        //if (platform.contains("wayland", Qt::CaseInsensitive)) {
            //action = Qt::CopyAction;
        //}

        for (const QString &src : sources) {
            if (targetDir.startsWith(src + QDir::separator())) {
                return;
            }
        }

        if (action == Qt::MoveAction) {
            emit requestMove(sources, targetDir);
        } else {
            emit requestCopy(sources, targetDir);
        }
    });

    QAction *delAction = new QAction(this);
    delAction->setShortcut(QKeySequence::Delete);
    fileListView->addAction(delAction);
    connect(delAction, &QAction::triggered, this, [this]() {
        QModelIndexList indexes = fileListView->selectionModel()->selectedIndexes();
        QSet<QString> paths;
        for (const QModelIndex &idx : indexes) {
            if (idx.column() == 0) {
                paths.insert(fileSystem->filePath(idx));
            }
        }
        if (!paths.isEmpty()) {
            if (QMessageBox::question(this, "Delete", QString("Delete %1 item(s)?").arg(paths.size())) == QMessageBox::Yes) {
                emit requestDelete(QList<QString>(paths.begin(), paths.end()));
            }
        }
    });

    fileInfoContainer = new QFrame(this);
    fileInfoContainer->setFrameShape(QFrame::NoFrame);
    fileInfoContainer->setVisible(false);

    fileInfLayout = new QVBoxLayout(fileInfoContainer);
    fileInfLayout->setContentsMargins(0, 0, 0, 0);


    treeLayout->addLayout(buttonsLayout);
    treeLayout->addWidget(fileInfoContainer);
    showFileInf(QModelIndex());


    connect(fileListView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex &current, const QModelIndex &) {
        showFileInf(current);
    });

    QAction *clearSelectionAction = new QAction(this);
    clearSelectionAction->setShortcut(Qt::Key_Escape);
    fileListView->addAction(clearSelectionAction);

    connect(clearSelectionAction, &QAction::triggered, this, [this]() {
        fileListView->selectionModel()->clearSelection();
        fileListView->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate);
        showFileInf(QModelIndex());
    });

    listLayout->addLayout(treeLayout);
    QFrame *listContainer = new QFrame(this);
    listContainer->setFrameShape(QFrame::NoFrame);
    listContainer->setContentsMargins(0, 0, 40, 0);

    QHBoxLayout *containerLayout = new QHBoxLayout(listContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->addWidget(fileListView);

    listLayout->addWidget(listContainer);

    mainLayout->addWidget(toolbar);
    mainLayout->addLayout(pathLayout);
    mainLayout->addLayout(listLayout);

    QSettings settings("DadyaIgor", "FileManager");
    QString defStyle = settings.value("style", "darkStyle.qss").toString();
    filename = defStyle;
    styleReader(defStyle);
}

QString MainWindow::detectStylePath() {
    QString appDir = QCoreApplication::applicationDirPath();

    QString localPath = appDir + "/styles";
    if (QDir(localPath).exists())
        return localPath;

    QString optPath = "/opt/fileManager/share/fileManager/styles";
    if (QDir(optPath).exists())
        return optPath;

    QString usrPath = "/usr/share/filemanager/styles";
    if (QDir(usrPath).exists())
        return usrPath;

    return QString();
}

void MainWindow::styleReader(const QString &filename) {
    if (filename.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Empty style name");
        return;
    }

    QFileInfo fi(filename);
    QString baseName = fi.fileName();
    if (!baseName.endsWith(".qss", Qt::CaseInsensitive)) {
        baseName += ".qss";
    }

    QFile file;
    if (fi.isAbsolute() && QFile::exists(fi.absoluteFilePath())) {
        file.setFileName(fi.absoluteFilePath());
    } else {
        QString stylesDir = detectStylePath();
        if (!stylesDir.isEmpty()) {
            QString candidate = stylesDir + QDir::separator() + baseName;
            if (QFile::exists(candidate)) {
                file.setFileName(candidate);
            }
        }

        if (file.fileName().isEmpty()) {
            QString resName = ":/styles/" + baseName;
            if (QFile::exists(resName)) {
                file.setFileName(resName);
            }
        }
    }

    if (file.fileName().isEmpty() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Failed", "Failed to open style file: " + filename, QMessageBox::StandardButton::Ok);
        return;
    }

    QString style = QString::fromUtf8(file.readAll());
    setStyleSheet(style);
    file.close();
}

void MainWindow::changeThemeFunc() {
    QString startDir = detectStylePath();
    if (startDir.isEmpty()) startDir = QDir::homePath();

    QString selected = QFileDialog::getOpenFileName(this, "Load Style", startDir, "QSS Files (*.qss)");
    if (selected.isEmpty()) return;

    QFileInfo fi(selected);
    QString baseName = fi.fileName();
    filename = baseName;

    styleReader(selected);
}


void MainWindow::changeDir(const QModelIndex &index) {
    if (!index.isValid()) return;

    QString path = fileSystem->filePath(index);
    QFileInfo info(path);

    if (info.isDir()) {
        if (!currentPath.isEmpty()) {
            historyBack.append(currentPath);
        }

        historyForward.clear();
        currentPath = path;
        fileListView->setRootIndexAnimated(fileSystem->setRootPath(path));
        goToParent->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));

        if (pathBar) {
            pathBar->setPath(currentPath);
        }

    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void MainWindow::changeDir(const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        QMessageBox::critical(this, "Failed", QString("Fail to open '%1': No such file or directory").arg(path), QMessageBox::StandardButton::Ok);
        return;
    }

    pathBar->setPath(dir.absolutePath());
    fileListView->setRootIndexAnimated(fileSystem->setRootPath(dir.absolutePath()));
}

void MainWindow::goToParentOrChildDir() {
    if (!historyForward.isEmpty()) {
        QString nextPath = historyForward.takeLast();
        historyBack.append(currentPath);
        currentPath = nextPath;
        fileListView->setRootIndexAnimated(fileSystem->setRootPath(currentPath));

        if (historyForward.isEmpty()) {
            goToParent->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
        }

        if (pathBar) {
            pathBar->setPath(currentPath);
        }

        return;
    }



    if (currentPath.isEmpty()) return;

    QDir dir(currentPath);
    if (dir.isRoot() || !dir.cdUp()) return;

    historyForward.append(currentPath);
    currentPath = dir.absolutePath();
    fileListView->setRootIndexAnimated(fileSystem->setRootPath(currentPath));

    if (pathBar) {
        pathBar->setPath(currentPath);
    }

    goToParent->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));

}

void MainWindow::renameFunc(const QModelIndex &index, const QString &newName, const QString &originalName) {
    if (!index.isValid()) return;

    QString oldPath = fileSystem->filePath(index);
    QFileInfo info(oldPath);

    if (newName.isEmpty() || newName == originalName) return;

    QString parentPath = info.dir().absolutePath();
    QString newPath = parentPath + QDir::separator() + newName;

    emit requestRename(oldPath, newPath);
}

void MainWindow::createDirFunc(const QModelIndex &index) {
    QString parentPath;
    if (index.isValid()) {
        parentPath = fileSystem->filePath(index);
        QFileInfo info(parentPath);

        if (info.isFile()) {
            parentPath = info.dir().absolutePath();
        }
    } else {
        parentPath = fileSystem->filePath(fileListView->rootIndex());
    }

    QInputDialog *inputDialog = new QInputDialog(this);
    inputDialog->setWindowTitle("Create new directory");
    inputDialog->setLabelText("Enter name for the new directory:");
    inputDialog->setOkButtonText("Create");
    inputDialog->setCancelButtonText("Cancel");

    if (inputDialog->exec() == QDialog::Accepted) {
        QString nameDir = inputDialog->textValue().trimmed();

        if (nameDir.isEmpty()) {
            QMessageBox::warning(inputDialog, "Warning", "Directory name cannot be empty");
            delete inputDialog;
            return;
        }

        emit requestCreateDir(parentPath, nameDir);
    }

    delete inputDialog;
}

void MainWindow::createFileFunc(const QModelIndex &index) {
    QString parentPath;
    if (index.isValid()) {
        parentPath = fileSystem->filePath(index);
        QFileInfo info(parentPath);

        if (info.isFile()) {
            parentPath = info.dir().absolutePath();
        }
    } else {
        parentPath = fileSystem->filePath(fileListView->rootIndex());
    }

    QInputDialog *inputDialog = new QInputDialog(this);
    inputDialog->setWindowTitle("Create new file");
    inputDialog->setLabelText("Enter name for the new file:");
    inputDialog->setOkButtonText("Create");
    inputDialog->setCancelButtonText("Cancel");

    if (inputDialog->exec() == QInputDialog::Accepted) {
        QString nameFile = inputDialog->textValue().trimmed();

        if (nameFile.isEmpty()) {
            QMessageBox::warning(inputDialog, "Warning", "File name cannot be empty");
            delete inputDialog;
            return;
        }

        emit requestCreateFile(parentPath, nameFile);
    }

    delete inputDialog;
}

void MainWindow::moveFunc(const QModelIndex &index) {
    if (!index.isValid()) {
        QMessageBox::warning(this, "Warning", "Please select a file or directory to move");
        return;
    }
    QString source = fileSystem->filePath(index);
    QString destinationDir = QFileDialog::getExistingDirectory(this, "Select a destination to move", QDir::homePath());
    if (destinationDir.isEmpty()) {
        return;
    }

    QFileInfo sourceInfo(source);
    QString proposedDest = destinationDir + QDir::separator() + sourceInfo.fileName();
    if (QFile::exists(proposedDest)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Overwrite",
                                                                  QString("'%1' already exists in the destination. Replace it?").arg(sourceInfo.fileName()),
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }

    emit requestMove({source}, destinationDir);
}

void MainWindow::copyPasteFunc(const QModelIndex &index) {
    if (clipboardPaths.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Nothing to paste");
        return;
    }

    QString destinationDir;
    if (index.isValid()) {
        destinationDir = fileSystem->filePath(index);
        QFileInfo fi(destinationDir);
        if (fi.isFile()) {
            destinationDir = fi.dir().absolutePath();
        }
    } else {
        destinationDir = currentPath;
    }

    if (destinationDir.isEmpty()) {
        QMessageBox::critical(this, "Error", "Invalid destination");
        return;
    }

    if (isCut) {
        emit requestMove(clipboardPaths, destinationDir);
        clipboardPaths.clear();
        isCut = false;
    } else {
        emit requestCopy(clipboardPaths, destinationDir);
    }
}

void MainWindow::showContextMenu(const QPoint &globalPos, const QModelIndex &index) {

    QString path = fileSystem->filePath(index);
    QFileInfo info(path);

    QMenu contextMenu(this);

    QAction *createDirAction = contextMenu.addAction("New Directory");
    QAction *createFileAction = contextMenu.addAction("New File");
    QAction *pasteAction = contextMenu.addAction("Paste");
    if (index.isValid()) {
        QAction *openAction = contextMenu.addAction("Open");
        QAction *removeAction = contextMenu.addAction("Delete");
        QAction *propsAction = contextMenu.addAction("Properties");
        QAction *renameAction = contextMenu.addAction("Rename");
        QAction *moveAction = contextMenu.addAction("Move");
        QAction *copyAction = contextMenu.addAction("Copy");
        QAction *cutAction = contextMenu.addAction("Cut");
        contextMenu.addSeparator();

        QAction *selectedAction = contextMenu.exec(globalPos);
        if(!selectedAction) return;

        if (selectedAction == openAction) {
            if (info.isFile()) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            } else {
                changeDir(index);
            }
        } else if (selectedAction == removeAction) {
            QModelIndexList allIndexes = fileListView->selectionModel()->selectedIndexes();
            QSet<QString> uniquePaths;
            for (const QModelIndex &idx : allIndexes) {
                if (idx.column() == 0) {
                    uniquePaths.insert(fileSystem->filePath(idx));
                }
            }

            if (uniquePaths.isEmpty()) return;

            QString summary;
            if (uniquePaths.size() == 1) {
                summary = QString("Are you sure you want to delete '%1'?").arg(*uniquePaths.begin());
            } else {
                summary = QString("Are you sure you want to delete %1 items?").arg(uniquePaths.size());
            }

            QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete", summary, QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                emit requestDelete(QList<QString>(uniquePaths.begin(), uniquePaths.end()));
            }
        } else if (selectedAction == propsAction) {
            QDialog *props = new QDialog(fileListView);

            props->setWindowTitle("Properties");
            props->setFixedSize(500, 500);

            QVBoxLayout *propsMainLayout = new QVBoxLayout(props);

            QIcon entryIcon = style()->standardIcon(info.isDir() ? QStyle::SP_DirIcon : QStyle::SP_FileIcon);

            QLineEdit *fileName = new QLineEdit(props);

            QPixmap pixIcon = entryIcon.pixmap(QSize(64, 64));

            QHBoxLayout *fileNameLayout = new QHBoxLayout();
            fileNameLayout->setAlignment(Qt::AlignTop);
            fileNameLayout->setSpacing(5);

            QLabel *iconLabel = new QLabel(props);
            iconLabel->setPixmap(pixIcon);

            iconLabel->setFixedSize(64, 64);

            fileName->setText(info.fileName());
            fileName->setStyleSheet("font-size: 25px");

            fileNameLayout->addWidget(iconLabel);
            fileNameLayout->addWidget(fileName);


            QPushButton *okBtn = new QPushButton("✔ OK", props);
            connect(okBtn, &QPushButton::clicked, this, [this, index, fileName, info, props]() {
                this->renameFunc(index, fileName->text(), info.fileName());
                props->accept();
            });

            connect(fileName, &QLineEdit::returnPressed, this, [this, index, fileName, info, props]() {
                this->renameFunc(index, fileName->text(), info.fileName());
                props->accept();
            });

            QLabel *typeLabel = new QLabel(props);
            typeLabel->setText(info.isDir() ? "Type: Directory" : "Type: File");

            QDateTime created = getCreatedTime(info.absoluteFilePath());
            if (!created.isValid()) {
                created = info.lastModified();
            }

            QDateTime modified = info.lastModified();

            QDateTime accessed = info.fileTime(QFile::FileAccessTime);

            QString timeText = QString("Created: %1\nModified: %2\nAccessed: %3").arg(created.toString("dd.MM.yyyy hh:mm:ss"), modified.toString("dd.MM.yyyy hh:mm:ss"), accessed.toString("dd.MM.yyyy hh:mm:ss"));

            QLabel *timeLabel = new QLabel(props);
            timeLabel->setText(timeText);
            timeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            timeLabel->setAlignment(Qt::AlignLeft);

            QLabel *sizeLabel = new QLabel(props);
            sizeLabel->setText("Size: " + QLocale().formattedDataSize(info.size()));
            sizeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
            sizeLabel->setAlignment(Qt::AlignLeft);

            propsMainLayout->addLayout(fileNameLayout);
            propsMainLayout->addWidget(typeLabel);
            propsMainLayout->addWidget(timeLabel);
            propsMainLayout->addWidget(sizeLabel);
            propsMainLayout->addWidget(okBtn);

            props->exec();
        } else if (selectedAction == renameAction) {
            QInputDialog *inputDialog = new QInputDialog(this);

            QString originalName = info.fileName();

            inputDialog->setWindowTitle("Rename");
            inputDialog->setLabelText(QString("Rename '%1' to:").arg(originalName));
            inputDialog->setTextValue(originalName);
            inputDialog->setOkButtonText("Rename");
            inputDialog->setCancelButtonText("Cancel");

            if (inputDialog->exec() == QInputDialog::Accepted) {
                QString newName = inputDialog->textValue();

                renameFunc(index, newName, originalName);
            }

            delete inputDialog;
        } else if (selectedAction == createDirAction) {
            createDirFunc(index);
        } else if (selectedAction == createFileAction) {
            createFileFunc(index);
        } else if (selectedAction == moveAction) {
            moveFunc(index);
        } else if (selectedAction == copyAction || selectedAction == cutAction) {
            QItemSelectionModel *sel = fileListView->selectionModel();
            QModelIndexList idxs = sel->selectedIndexes();
            clipboardPaths.clear();
            for (const QModelIndex &idx : idxs) {
                if (idx.column() == 0) {
                    clipboardPaths << fileSystem->filePath(idx);
                }
            }
            isCut = (selectedAction == cutAction);
        } else if (selectedAction == pasteAction)  {
            copyPasteFunc(index);
        }
    } else {
        QAction *selectedAction = contextMenu.exec(globalPos);

        if (selectedAction == createDirAction) {
            createDirFunc(QModelIndex());
        } else if (selectedAction == createFileAction) {
            createFileFunc(QModelIndex());
        } else if (selectedAction == pasteAction) {
            copyPasteFunc(QModelIndex());
        }
    }
}

void MainWindow::openPath(const QString& path) {
    QString dir = QFileInfo(path).absoluteFilePath();

    if (dir.startsWith("~")) {
        dir.replace(0, 1, QDir::homePath());
    }

    changeDir(dir);
}

void MainWindow::showFileInf(const QModelIndex &index)
{
    QLayoutItem *child;
    while ((child = fileInfLayout->takeAt(0))) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    fileInfoContainer->setVisible(true);
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(fileInfoContainer);
    fileInfoContainer->setGraphicsEffect(effect);

    if (!index.isValid()) {
        QLabel *label = new QLabel("Select file", fileInfoContainer);
        label->setFixedWidth(203);
        fileInfLayout->addWidget(label);
    } else {
        QString path = fileSystem->filePath(index);
        QFileInfo info(path);

        QFileIconProvider iconProvider;
        QIcon icon = iconProvider.icon(info);
        QLabel *fileIcon = new QLabel(fileInfoContainer);
        fileIcon->setPixmap(icon.pixmap(96, QIcon::Normal, QIcon::On));
        fileInfLayout->addWidget(fileIcon);

        QLabel *fileName = new QLabel(fileInfoContainer);
        fileName->setMaximumWidth(440);
        QString full = info.fileName();
        QFontMetrics fm(fileName->font());
        QString el = fm.elidedText(full, Qt::ElideMiddle, fileName->width() + 20);
        el.replace(QChar(0x2026), "...");
        fileName->setText(el);
        fileName->setToolTip(full);
        fileInfLayout->addWidget(fileName);

        QLabel *entType = new QLabel(info.isDir() ? "Type: Directory" : "Type: File", fileInfoContainer);
        fileInfLayout->addWidget(entType);

        QDateTime created = getCreatedTime(path);
        if (!created.isValid()) created = info.lastModified();
        QString timeText = QString("Created: %1\nModified: %2\nAccessed: %3").arg(created.toString("dd.MM.yyyy hh:mm:ss"), info.lastModified().toString("dd.MM.yyyy hh:mm:ss"), info.fileTime(QFile::FileAccessTime).toString("dd.MM.yyyy hh:mm:ss"));
        QLabel *fileTimes = new QLabel(timeText, fileInfoContainer);
        fileTimes->setTextInteractionFlags(Qt::TextSelectableByMouse);
        fileInfLayout->addWidget(fileTimes);

        int fileCount = 0;
        if (info.isDir()) {
            QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::NoIteratorFlags);
            while (it.hasNext()) {
                it.next();
                ++fileCount;
            }
        }
        QLabel *fileSize = new QLabel(info.isDir()
                                          ? QString("Size: %1 item%2").arg(fileCount).arg(fileCount != 1 ? "s" : "")
                                          : QString("Size: %1").arg(QLocale().formattedDataSize(info.size())),
                                      fileInfoContainer);
        fileSize->setTextInteractionFlags(Qt::TextSelectableByMouse);
        fileInfLayout->addWidget(fileSize);
    }

    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity", this);
    fadeIn->setDuration(150);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    connect(fadeIn, &QPropertyAnimation::finished, this, [effect]() {
        effect->deleteLater();
    });
}

MainWindow::~MainWindow() {
    QSettings settings("DadyaIgor", "FileManager");
    settings.setValue("style", filename);

    workerThread->quit();
    workerThread->wait();
}
