#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/filewatcher.h"
#include "core/versionmanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    watcher_ = new FileWatcher(this);
    versionManager_ = new VersionManager(this);
    connect(watcher_, &FileWatcher::fileChanged,versionManager_, &VersionManager::onFileChanged);
    // TODO: 改为 UI 选择
    watcher_->addWatchPath("D:/Projects/FileVersionManager/tests");
    versionManager_->rollback("D:/Projects/FileVersionManager/tests/test.txt", "62e628d1f8aedacd6ceaf7f0eea8b4dacb1e27f5524b0b16639be9fc56c81c2f");
}

MainWindow::~MainWindow()
{
    delete ui;
}

