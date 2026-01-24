#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QTimer>
#include "versiontreemodel.h"
#include "core/filewatcher.h"
#include "core/versionmanager.h"
#include "utils/Logger.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 先创建 core 对象
    m_watcher = new FileWatcher(this);
    m_versionManager = nullptr;

    // 2. 创建 model / view
    m_versionModel = new VersionTreeModel(this);
    m_versionView = new QTreeView(this);
    m_versionView->setModel(m_versionModel);
    m_versionView->setHeaderHidden(true);
    m_versionView->setRootIsDecorated(true);
    m_versionView->setItemsExpandable(true);

    auto* layout = new QVBoxLayout(ui->gpb_version);
    layout->addWidget(m_versionView);
    ui->gpb_version->setLayout(layout);

    // 3. 连接信号
    setConnection();

    // 4. 选择 workspace
    QString watchPath = QFileDialog::getExistingDirectory(this, "Select Workspace");

    if (watchPath.isEmpty())
    {
        QTimer::singleShot(0, qApp, &QApplication::quit);
        return;
    }
    setWorkspace(watchPath);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::setWorkspace(const QString &path)
{
    ui->led_workspace->setText(path);
    // 1. 彻底停 watcher
    m_watcher->blockSignals(true);
    m_watcher->clear();

    // 2. 安全释放旧 manager
    if (m_versionManager) {
        m_versionManager->deleteLater();
        m_versionManager = nullptr;
    }

    // 3. 重建 manager
    m_versionManager = new VersionManager(path, this);
    m_versionManager->initializeWorkspace();
    // 4. Logger 跟随 workspace
    Logger::init(path);

    // 5. 恢复 watcher
    m_watcher->blockSignals(false);
    m_watcher->addWatchPath(path);

    // 6. 刷 UI
    m_versionModel->setAllVersions(
        m_versionManager->allVersions());
    m_versionModel->setCurrentVersions(
        m_versionManager->currentVersions());

    m_versionView->expandAll();
    return true;
}

bool MainWindow::confirmRollbackWithDiff(const VersionInfo &current, const VersionInfo &target)
{
    QString diffText = VersionManager::buildDiffText(current, target);

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Confirm Rollback");

    const bool sameVersion = !current.versionId.isEmpty() && current.versionId == target.versionId;

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

    QPushButton* btnRollback = box.addButton("Rollback", QMessageBox::AcceptRole);
    QPushButton* btnCancel = box.addButton("Cancel", QMessageBox::RejectRole);

    // Disable rollback when versions are identical
    btnRollback->setEnabled(!sameVersion);

    box.exec();

    return box.clickedButton() == btnRollback;
}

void MainWindow::setConnection()
{
    connect(m_watcher, &FileWatcher::fileChanged,this, [this](const QString& path)
    {
        if (!m_versionManager)
            return;
        m_versionManager->onFileChanged(path);
        m_versionModel->setAllVersions(m_versionManager->allVersions());
        m_versionModel->setCurrentVersions(m_versionManager->currentVersions());
        m_versionView->expandAll();
    });

    connect(ui->btn_rollBack, &QPushButton::clicked,this, &MainWindow::rollBack);

    connect(ui->btn_changeWorkspace, &QPushButton::clicked,this, [=]
    {
        QString watchPath = QFileDialog::getExistingDirectory(this, "Select Workspace");

        if (watchPath.isEmpty())
        {
            return;
        }
        setWorkspace(watchPath);
    });
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
