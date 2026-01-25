#ifndef FILEm_watcherH
#define FILEm_watcherH

#pragma once
#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>
#include <QSet>
class FileWatcher : public QObject {
    Q_OBJECT
public:
    explicit FileWatcher(QObject* parent = nullptr);

    void setWorkspace(const QString& workspaceRoot);
    void addWatchPath(const QString& path);

signals:
    void fileChanged(const QString& relPath);
    void fileDeleted(const QString &relPath);

private:
    QFileSystemWatcher m_watcher;
    QString m_workspaceRoot;
    QSet<QString> m_watchedFiles;
    QSet<QString> m_watchedDirs;
    void watchDirectory(const QString& absPath);
};
#endif // FILEm_watcherH
