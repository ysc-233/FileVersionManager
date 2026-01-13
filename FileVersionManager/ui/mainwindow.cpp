#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    watcher_ = new FileWatcher(this);
    versionManager_ = new VersionManager(this);
    connect(watcher_, &FileWatcher::fileChanged,versionManager_, &VersionManager::onFileChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

