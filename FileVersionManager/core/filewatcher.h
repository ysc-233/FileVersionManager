#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#pragma once
#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>

class FileWatcher : public QObject {
    Q_OBJECT
public:
    explicit FileWatcher(QObject* parent = nullptr);

    void addWatchPath(const QString& path);

signals:
    void fileChanged(const QString& path);

private slots:
    void onFileChanged(const QString& path);
    void onDirectoryChanged(const QString& path);

private:
    QFileSystemWatcher watcher_;
};
#endif // FILEWATCHER_H
