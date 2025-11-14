#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("File Manager");
    setFixedSize(1000, 800);

    fileSystem = new QFileSystemModel();
    fileSystem->setRootPath(QDir::homePath());
    fileSystem->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    pathLayout = new QHBoxLayout(centralWidget);
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

    toolbarCreatingFile = new QAction(this);
    toolbarCreatingFile->setIcon(style()->standardIcon(QStyle::SP_FileIcon));

    toolbarMoveAction = new QAction(this);
    toolbarMoveAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogStart));

    toolbarCopyAct = new QAction(this);
    toolbarCopyAct->setText("🗗");

    toolbarCutAct = new QAction(this);
    toolbarCutAct->setText("✂");

    toolbarPasteAct = new QAction(this);
    toolbarPasteAct->setText("📋");

    goToHome = new QAction(this);
    goToHome->setIcon(style()->standardIcon(QStyle::SP_DirHomeIcon));

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
        QItemSelectionModel *selectionModel = fileListView->selectionModel();

        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

        clipboardPath = fileSystem->filePath(selectedIndexes.first());
        isCut = false;
    });

    connect(toolbarCutAct, &QAction::triggered, this, [this]() {
        QItemSelectionModel *selectionModel = fileListView->selectionModel();

        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

        clipboardPath = fileSystem->filePath(selectedIndexes.first());
        isCut = true;
    });

    connect(toolbarPasteAct, &QAction::triggered, this, [this]() {
        QItemSelectionModel *selectionModel = fileListView->selectionModel();

        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

        copyPasteFunc(selectedIndexes.first());
    });

    connect(goToHome, &QAction::triggered, this, [this]() {
        QModelIndex index = fileSystem->index(QDir::homePath());
        changeDir(index);
    });

    QHBoxLayout *listLayout = new QHBoxLayout(centralWidget);

    fileListView = new QListView(this);
    fileListView->setModel(fileSystem);

    QModelIndex rootIndexView = fileSystem->index(QDir::homePath());
    fileListView->setRootIndex(rootIndexView);
    currentPath = QDir::homePath();

    fileListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fileListView, &QListView::customContextMenuRequested, this, &MainWindow::showContextMenu);

    fileSystemTree = new QFileSystemModel(this);
    fileSystemTree->setRootPath(QDir::homePath());
    fileSystemTree->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

    fileTreeView = new QTreeView(this);
    QModelIndex rootIndexTree = fileSystemTree->index(QDir::homePath());

    fileTreeView->setModel(fileSystemTree);
    fileTreeView->setRootIndex(rootIndexTree);
    fileTreeView->setFixedHeight(500);
    fileTreeView->setFixedWidth(200);

    QVBoxLayout *treeLayout = new QVBoxLayout(centralWidget);

    QVBoxLayout *buttonsLayout = new QVBoxLayout(centralWidget);
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
            fileSystemTree->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden);
        } else {
            fileSystem->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
            fileSystemTree->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
        }
    });

    connect(changeTheme, &QAction::triggered, this, &MainWindow::changeThemeFunc);

    connect(fileListView, &QListView::doubleClicked, this, &MainWindow::changeDir);
    connect(fileTreeView, &QTreeView::doubleClicked, this, &MainWindow::changeDir);
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
        if (ok) {
            QMessageBox::information(this, "Done", action + " completed successfully");
        } else {
            QMessageBox::critical(this, "Failed", action + " failed", QMessageBox::StandardButton::Ok);
        }

        fileListView->setRootIndex(fileSystem->setRootPath(currentPath));
        fileTreeView->setRootIndex(fileSystemTree->setRootPath(currentPath));
    });

    workerThread->start();

    connect(this, &MainWindow::requestDelete, worker, &Worker::recRemove);
    connect(this, &MainWindow::requestCreateDir, worker, &Worker::createDirFunc);
    connect(this, &MainWindow::requestCreateFile, worker, &Worker::createFileFunc);
    connect(this, &MainWindow::requestRename, worker, &Worker::renameFunc);
    connect(this, &MainWindow::requestMove, worker, &Worker::moveItem);
    connect(this, &MainWindow::requestCopy,  worker, &Worker::copyItem);

    treeLayout->addLayout(buttonsLayout);
    treeLayout->addWidget(fileTreeView);

    listLayout->addLayout(treeLayout);
    listLayout->addWidget(fileListView);

    mainLayout->addWidget(toolbar);
    mainLayout->addLayout(pathLayout);
    mainLayout->addLayout(listLayout);

    QSettings settings("DadyaIgor", "FileManager");
    QString defStyle = settings.value("style", "darkStyle.qss").toString();
    styleReader(defStyle);
    filename = "darkStyle.qss";
}

void MainWindow::styleReader(const QString &filename) {
    QString localPath = QCoreApplication::applicationDirPath() + "/styles/" + filename;
    QFile file(localPath);
    if (!file.exists()) {
        file.setFileName(":/styles/" + filename);
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QString::fromUtf8(file.readAll());
        setStyleSheet(style);
        file.close();
    } else {
        QMessageBox::critical(this, "Failed", "Failed to open file", QMessageBox::StandardButton::Ok);
    }
}

void MainWindow::changeThemeFunc() {
    filename = QFileDialog::getOpenFileName(this, "Load Style", "", "QSS File (*.qss)");
    styleReader(filename);
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
        fileListView->setRootIndex(fileSystem->setRootPath(path));
        fileTreeView->setRootIndex(fileSystemTree->setRootPath(path));
        goToParent->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));

        if (pathBar) {
            pathBar->setPath(currentPath);
        }

    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void MainWindow::goToParentOrChildDir() {
    if (!historyForward.isEmpty()) {
        QString nextPath = historyForward.takeLast();
        historyBack.append(currentPath);
        currentPath = nextPath;
        fileListView->setRootIndex(fileSystem->setRootPath(currentPath));
        fileTreeView->setRootIndex(fileSystemTree->setRootPath(currentPath));

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
    fileListView->setRootIndex(fileSystem->setRootPath(currentPath));
    fileTreeView->setRootIndex(fileSystemTree->setRootPath(currentPath));

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
        QMessageBox::warning(this, "Warning", "Please selecet a file or directory to move");
        return;
    }

    QString source = fileSystem->filePath(index);

    QString destinationDir = QFileDialog::getExistingDirectory(this, "Select a destination to move", QDir::homePath());

    if (destinationDir.isEmpty()) {
        return;
    }

    QFileInfo sourceInfo(source);
    QString destination = destinationDir + QDir::separator() + sourceInfo.fileName();

    if (QFile::exists(destination)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Overwirte", QString("'%1' already exists in the destination. Replace it?").arg(sourceInfo.fileName()), QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return;
        }
    }

    emit requestMove(source, destination);
}

void MainWindow::copyPasteFunc(const QModelIndex &index) {
    if (clipboardPath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Nothing to paste");
        return;
    }

    QString destinationDir = fileSystem->filePath(index);
    QFileInfo destInfo(destinationDir);

    if (destInfo.isFile()) {
        destinationDir = destInfo.dir().absolutePath();
    }

    QFileInfo srcInfo(clipboardPath);
    QString destination = destinationDir + QDir::separator() + srcInfo.fileName();


    if (QFile::exists(destination)) {
        auto reply = QMessageBox::question(this, "Overwrite", QString("'%1' already exists. Replace it?").arg(srcInfo.fileName()), QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;
    }

    if (isCut) {
        emit requestMove(clipboardPath, destination);
        clipboardPath.clear();
        isCut = false;
    } else {
        emit requestCopy(clipboardPath, destination);
    }
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QModelIndex index = fileListView->indexAt(pos);

    QMenu contextMenu(this);

    QString path = fileSystem->filePath(index);
    QFileInfo info(path);

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
        QAction *selectedAction = contextMenu.exec(fileListView->viewport()->mapToGlobal(pos));

        if (selectedAction == openAction) {
            if (info.isFile()) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            } else {
                changeDir(index);
            }
        } else if (selectedAction == removeAction) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete", QString("Are you sure you want to delete '%1'?").arg(info.fileName()), QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                emit requestDelete(path);
            }
        } else if (selectedAction == propsAction) {
            QDialog *props = new QDialog(fileListView);

            props->setWindowTitle("Properties");
            props->setFixedSize(500, 500);

            QVBoxLayout *propsMainLayout = new QVBoxLayout(props);

            QIcon entryIcon = style()->standardIcon(info.isDir() ? QStyle::SP_DirIcon : QStyle::SP_FileIcon);

            QLineEdit *fileName = new QLineEdit(props);

            QPixmap pixIcon = entryIcon.pixmap(QSize(64, 64));

            QHBoxLayout *fileNameLayout = new QHBoxLayout(props);
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
        } else if (selectedAction == copyAction) {
            clipboardPath = path;
            isCut = false;
        } else if (selectedAction == cutAction) {
            clipboardPath = path;
            isCut = true;
        } else if (selectedAction == pasteAction)  {
            copyPasteFunc(index);
        }
    } else {
        QAction *selectedAction = contextMenu.exec(fileListView->viewport()->mapToGlobal(pos));

        if (selectedAction == createDirAction) {
            createDirFunc(index);
        } else if (selectedAction == createFileAction) {
            createFileFunc(index);
        } else if (selectedAction == pasteAction) {
            copyPasteFunc(index);
        }
    }
}

MainWindow::~MainWindow() {
    QSettings settings("DadyaIgor", "FileManager");
    settings.setValue("style", filename);

    workerThread->quit();
    workerThread->wait();
}
