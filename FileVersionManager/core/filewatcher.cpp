#include "filewatcher.h"
#include <QDir>
#include <QDebug>

FileWatcher::FileWatcher(QObject* parent)
{
    connect(&watcher_, &QFileSystemWatcher::fileChanged,this, &FileWatcher::onFileChanged);
    connect(&watcher_, &QFileSystemWatcher::directoryChanged,this, &FileWatcher::onDirectoryChanged);
}

void FileWatcher::addWatchPath(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "Watch path not exists:" << path;
        return;
    }
    // 监听目录本身
    watcher_.addPath(path);
    // 监听目录下已有文件（重要）
    const auto files = dir.entryList(QDir::Files);
    for (const auto& file : files) {
        watcher_.addPath(dir.absoluteFilePath(file));
    }
    qDebug() << "Watching directory:" << path;
}

void FileWatcher::onFileChanged(const QString &path)
{
    emit fileChanged(path);
}

void FileWatcher::onDirectoryChanged(const QString &path)
{
    QDir dir(path);
    const auto files = dir.entryList(QDir::Files);

    // 新文件加入监听
    for (const auto& file : files) {
        const QString abs = dir.absoluteFilePath(file);
        if (!watcher_.files().contains(abs)) {
            watcher_.addPath(abs);
            emit fileChanged(abs);
        }
    }
}
