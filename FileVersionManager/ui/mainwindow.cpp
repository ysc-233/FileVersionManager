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
}

MainWindow::~MainWindow()
{
    delete ui;
}

