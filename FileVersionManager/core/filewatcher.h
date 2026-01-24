#ifndef FILEm_watcherH
#define FILEm_watcherH

#pragma once
#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>

class FileWatcher : public QObject {
    Q_OBJECT
public:
    explicit FileWatcher(QObject* parent = nullptr);

    void addWatchPath(const QString& path);

    void clear();

signals:
    void fileChanged(const QString& path);

private slots:
    void onFileChanged(const QString& path);
    void onDirectoryChanged(const QString& path);

private:
    QFileSystemWatcher m_watcher;
};
#endif // FILEm_watcherH
