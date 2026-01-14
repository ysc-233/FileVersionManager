#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDir>
#include "versiontreemodel.h"
#include "core/filewatcher.h"
#include "core/versionmanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const QString watchPath = "D:/Projects/FileVersionManager/tests";

    // core
    m_watcher = new FileWatcher(this);
    m_versionManager = new VersionManager(watchPath, this);

    // model / view
    m_versionModel = new VersionTreeModel(this);
    m_versionView = new QTreeView(this);
    m_versionView->setModel(m_versionModel);

    auto* layout = new QVBoxLayout(ui->gpb_version);
    layout->addWidget(m_versionView);
    ui->gpb_version->setLayout(layout);

    // 启动时加载所有历史版本
    m_versionModel->setAllVersions(
        m_versionManager->allVersions()
    );

    // 文件变化 → 版本生成 → UI 刷新
    connect(m_watcher, &FileWatcher::fileChanged,this, [=](const QString& path)
    {
        m_versionManager->onFileChanged(path);
        m_versionModel->setAllVersions(m_versionManager->allVersions());
        m_versionView->expandAll();
    });

    // 回滚
    connect(ui->btn_rollBack, &QPushButton::clicked,this, [=]()
    {
        const QModelIndex index = m_versionView->currentIndex();
        if (!index.isValid())
            return;

        const VersionInfo info = m_versionModel->versionAt(index);

        if (!m_versionManager->rollback(info.filePath, info.versionId))
        {
            QMessageBox::warning(
                this, "Rollback", "Rollback failed");
        }
    });

    // watcher
    m_watcher->addWatchPath(watchPath);
}

MainWindow::~MainWindow()
{
    delete ui;
}
