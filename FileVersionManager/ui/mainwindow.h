#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include "core/filewatcher.h"
#include "core/versionmanager.h"

class FileWatcher;
class VersionManager;
class VersionTreeModel;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool setWorkspace(const QString& path);
    bool confirmRollbackWithDiff(const VersionInfo& current,const VersionInfo& target);

private slots:
    void rollBack();

private:
    Ui::MainWindow *ui;
    FileWatcher* m_watcher;
    VersionManager* m_versionManager;
    VersionTreeModel* m_versionModel;
    QTreeView* m_versionView;
};
#endif // MAINWINDOW_H
