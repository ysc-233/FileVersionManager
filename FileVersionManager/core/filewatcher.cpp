#include "filewatcher.h"
#include <QDir>
#include <QDebug>

FileWatcher::FileWatcher(QObject* parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged,this, &FileWatcher::onFileChanged);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,this, &FileWatcher::onDirectoryChanged);
}

void FileWatcher::addWatchPath(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "Watch path not exists:" << path;
        return;
    }
    // 监听目录本身
    m_watcher.addPath(path);
    // 监听目录下已有文件（重要）
    const auto files = dir.entryList(QDir::Files);
    for (const auto& file : files) {
        m_watcher.addPath(dir.absoluteFilePath(file));
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
        if (!m_watcher.files().contains(abs)) {
            m_watcher.addPath(abs);
            emit fileChanged(abs);
        }
    }
}
