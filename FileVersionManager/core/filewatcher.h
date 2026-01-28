#ifndef FILEm_watcherH
#define FILEm_watcherH

#pragma once
#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>
#include <QSet>
#include <QMap>
class FileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcher(QObject* parent = nullptr);

    void setWorkspace(const QString& workspaceRoot);

signals:
    void fileAdded(const QString& relPath);
    void fileChanged(const QString& relPath);
    void fileDeleted(const QString& relPath);

private:
    void scanAndWatchDir(const QString& absDir);
    void snapshotDirectory(const QString& absDir);
    QString toRel(const QString& absPath) const;

private:
    QFileSystemWatcher m_watcher;
    QString m_workspaceRoot;

    QSet<QString> m_watchedDirs;      // abs dir
    QSet<QString> m_watchedFiles;     // abs file
    QMap<QString, QSet<QString>> m_dirSnapshots; // absDir -> absFiles
};
#endif // FILEm_watcherH
