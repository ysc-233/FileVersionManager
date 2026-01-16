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
    connect(ui->btn_rollBack, &QPushButton::clicked,this, [=]()
    {
        const QModelIndex index = m_versionView->currentIndex();
        if (!index.isValid())
            return;

        const VersionInfo info = m_versionModel->versionAt(index);
        VersionInfo current = m_versionManager->currentVersionInfo(info.filePath);
        if (!confirmRollbackWithDiff(current, info))
            return;

        if (!m_versionManager->rollback(info.filePath, info.versionId))
        {
            QMessageBox::warning(this, "Rollback", "Rollback failed");
        }
        else
        {
            m_versionModel->setCurrentVersions(m_versionManager->currentVersions());
        }
    });

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

    // 🔒 Disable rollback when versions are identical
    btnRollback->setEnabled(!sameVersion);

    box.exec();

    return box.clickedButton() == btnRollback;
}
