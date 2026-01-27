#ifndef FILEm_watcherH
#define FILEm_watcherH

#pragma once
#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>
#include <QSet>
#include <QMap>
class FileWatcher : public QObject {
    Q_OBJECT
public:
    explicit FileWatcher(QObject* parent = nullptr);
    void setWorkspace(const QString& workspaceRoot);
    void startWatch();
    void addFile(const QString& absPath);
signals:
    void fileAdded(const QString& relPath);
    void fileChanged(const QString& relPath);
    void fileDeleted(const QString& relPath);
private:
    void scanAndWatchDir(const QString& absDir);
    void snapshotDirectory(const QString& absDir);
    QString toRel(const QString& absPath);
private:
    QFileSystemWatcher m_watcher;
    QString m_workspaceRoot;
    QSet<QString> m_watchedFiles;
    QSet<QString> m_watchedDirs;
    QSet<QString> m_knownFiles; // 绝对路径
    QMap<QString, QSet<QString>> m_dirSnapshots;
};
#endif // FILEm_watcherH
