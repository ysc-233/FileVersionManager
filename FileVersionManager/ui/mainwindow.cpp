#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDir>
#include "versiontreemodel.h"
#include "core/filewatcher.h"
#include "core/versionmanager.h"
#include "utils/logger.h"

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
    m_versionView->setHeaderHidden(true);
    m_versionView->setRootIsDecorated(true);
    m_versionView->setItemsExpandable(true);

    auto* layout = new QVBoxLayout(ui->gpb_version);
    layout->addWidget(m_versionView);
    ui->gpb_version->setLayout(layout);

    // 启动时加载所有历史版本
    m_versionModel->setAllVersions(m_versionManager->allVersions());
    m_versionModel->setCurrentVersions(m_versionManager->currentVersions());
    // 文件变化 → 版本生成 → UI 刷新
    connect(m_watcher, &FileWatcher::fileChanged,this, [=](const QString& path)
    {
        m_versionManager->onFileChanged(path);
        m_versionModel->setAllVersions(m_versionManager->allVersions());
        m_versionView->expandAll();

        m_versionModel->setCurrentVersions(m_versionManager->currentVersions());
    });

    // 回滚
    connect(ui->btn_rollBack, &QPushButton::clicked,this,&MainWindow::rollBack);

    // watcher
    m_watcher->addWatchPath(watchPath);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::confirmRollbackWithDiff(const VersionInfo &current, const VersionInfo &target)
{
    QString diffText = VersionManager::buildDiffText(current, target);

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Confirm Rollback");

    const bool sameVersion =
            !current.versionId.isEmpty() &&
            current.versionId == target.versionId;

    if (sameVersion)
    {
        box.setText(
                    "The selected version is identical to the current version.\n"
                    "Rollback is not available."
                    );
    }
    else
    {
        box.setText(
                    "You are about to rollback to the selected version.\n"
                    "Do you want to continue?"
                    );
    }

    box.setDetailedText(diffText);

    QPushButton* btnRollback =
            box.addButton("Rollback", QMessageBox::AcceptRole);
    QPushButton* btnCancel =
            box.addButton("Cancel", QMessageBox::RejectRole);

    // Disable rollback when versions are identical
    btnRollback->setEnabled(!sameVersion);

    box.exec();

    return box.clickedButton() == btnRollback;
}

void MainWindow::rollBack()
{
    const QModelIndex index = m_versionView->currentIndex();
    if (!index.isValid())
        return;

    const VersionInfo target = m_versionModel->versionAt(index);
    VersionInfo current = m_versionManager->currentVersionInfo(target.filePath);
    if (!confirmRollbackWithDiff(current, target))
        return;

    VersionManager::RollbackError err;
    const bool ok = m_versionManager->rollback(target.filePath,target.versionId,&err);

    if (!ok)
    {
        QString reason;
        switch (err)
        {
        case VersionManager::RollbackError::VersionNotFound:
            reason = "Target version not found.";
            break;
        case VersionManager::RollbackError::LoadFailed:
            reason = "Failed to load version data.";
            break;
        case VersionManager::RollbackError::WriteFailed:
            reason = "Failed to write temporary file.";
            break;
        case VersionManager::RollbackError::RenameFailed:
            reason = "Failed to replace original file.";
            break;
        default:
            reason = "Unknown error.";
            break;
        }
        Logger::error(QString("Rollback failed: file=%1 target=%2 reason=%3")
                      .arg(target.filePath).arg(target.versionId.left(8)).arg(reason));
        QMessageBox::critical(this,"Rollback Failed",reason);
        return;
    }

    QMessageBox::information(
                this,
                "Rollback Completed",
                QString("File:\n%1\n\nRolled back to version:\n%2")
                .arg(QFileInfo(target.filePath).fileName())
                .arg(target.versionId.left(8))
                );
    Logger::info(QString("Rollback succeeded: file=%1 target=%2")
                 .arg(target.filePath).arg(target.versionId.left(8)));

    m_versionModel->setCurrentVersions(m_versionManager->currentVersions());
}

void MainWindow::switchWorkspace(const QString& newPath)
{
    // 1. 停止 watcher
//    m_watcher->clear();

    // 2. 关闭旧仓库日志
    Logger::shutdown();

    // 3. 销毁旧 manager（可选但推荐）
    delete m_versionManager;

    // 4. 创建新仓库
    m_versionManager = new VersionManager(newPath, this);

    // 5. 初始化日志（在 VM 构造中或这里）
    Logger::init(newPath);

    // 6. 重新 watch
    m_watcher->addWatchPath(newPath);
}
