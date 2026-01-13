#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core/FileWatcher.h"
#include "core/versionmanager.h"

class FileWatcher;
class VersionManager;

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
    Ui::MainWindow *ui;
    FileWatcher* watcher_;
    VersionManager* versionManager_;
};
#endif // MAINWINDOW_H
