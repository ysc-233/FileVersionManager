#include "filewatcher.h"
#include <QDir>
#include <QDebug>
#include <QDirIterator>

FileWatcher::FileWatcher(QObject* parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged,this, &FileWatcher::onFileChanged);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,this, &FileWatcher::onDirectoryChanged);
}

void FileWatcher::addWatchPath(const QString& rootPath)
{
    QDir root(rootPath);
    if (!root.exists()) {
        qWarning() << "Watch path not exists:" << rootPath;
        return;
    }

    // 先清理旧监听（防止重复）
    m_watcher.removePaths(m_watcher.directories());
    m_watcher.removePaths(m_watcher.files());

    // 递归遍历目录
    QDirIterator it(
        rootPath,
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories
    );

    while (it.hasNext())
    {
        it.next();
        QFileInfo fi = it.fileInfo();
        const QString absPath = fi.absoluteFilePath();

        // 忽略 .fvm
        if (absPath.contains("/.fvm/") ||
            absPath.contains("\\.fvm\\"))
            continue;

        // 目录也要监听（用于捕获新增/删除）
        if (fi.isDir()) {
            m_watcher.addPath(absPath);
        }
        // 文件监听内容变化
        else if (fi.isFile()) {
            m_watcher.addPath(absPath);
        }
    }

    // 根目录监听
    m_watcher.addPath(rootPath);

    qDebug() << "Watching workspace recursively:" << rootPath;
}
void FileWatcher::clear()
{
    auto filePathList = m_watcher.files();
    if(!filePathList.isEmpty())
        m_watcher.removePaths(filePathList);
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
