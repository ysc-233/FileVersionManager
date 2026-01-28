#include "filewatcher.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

FileWatcher::FileWatcher(QObject* parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,this, [this](const QString& absDir)
    {
        if (!m_dirSnapshots.contains(absDir))
            return;

        QSet<QString> oldSet = m_dirSnapshots.value(absDir);

        QDir dir(absDir);
        QSet<QString> newSet;
        QFileInfoList files = dir.entryInfoList(QDir::Files);

        for (const QFileInfo& fi : files)
            newSet.insert(fi.absoluteFilePath());

        // 删除文件，重命名逻辑：根据删除列表对比新增文件，必须先发送删除信号
        for (const QString& f : oldSet - newSet)
        {
            m_watcher.removePath(f);
            m_watchedFiles.remove(f);
            emit fileDeleted(toRel(f));
        }
        // 新增文件
        for (const QString& f : newSet - oldSet)
        {
            m_watcher.addPath(f);
            m_watchedFiles.insert(f);
            emit fileAdded(toRel(f));
        }
        m_dirSnapshots[absDir] = newSet;
    });

    connect(&m_watcher, &QFileSystemWatcher::fileChanged,this, [this](const QString& absFile)
    {
        QFileInfo fi(absFile);
//        if (!fi.exists())
//        {
//            emit fileDeleted(toRel(absFile));
//            m_watchedFiles.remove(absFile);
//            return;
//        }
                if (fi.exists())
                {
                    emit fileChanged(toRel(absFile));
                }
//        emit fileChanged(toRel(absFile));

        // Windows：fileChanged 只触发一次，需要重新加
        if (!m_watchedFiles.contains(absFile))
        {
            m_watcher.addPath(absFile);
            m_watchedFiles.insert(absFile);
        }
    });
}

void FileWatcher::setWorkspace(const QString& workspaceRoot)
{
    m_workspaceRoot = QDir(workspaceRoot).absolutePath();

    m_watcher.removePaths(m_watcher.files());
    m_watcher.removePaths(m_watcher.directories());

    m_watchedDirs.clear();
    m_watchedFiles.clear();
    m_dirSnapshots.clear();

    scanAndWatchDir(m_workspaceRoot);
}

void FileWatcher::scanAndWatchDir(const QString& absDir)
{
    if (m_watchedDirs.contains(absDir))
        return;

    QDir dir(absDir);
    if (!dir.exists())
        return;

    if (dir.dirName() == ".fvm")
        return;

    m_watcher.addPath(absDir);
    m_watchedDirs.insert(absDir);

    snapshotDirectory(absDir);

    QFileInfoList entries = dir.entryInfoList(
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo& fi : entries)
    {
        if (fi.isDir())
            scanAndWatchDir(fi.absoluteFilePath());
        else if (fi.isFile())
        {
            QString absFile = fi.absoluteFilePath();
            m_watcher.addPath(absFile);
            m_watchedFiles.insert(absFile);
        }
    }
}

void FileWatcher::snapshotDirectory(const QString& absDir)
{
    QDir dir(absDir);
    QSet<QString> files;

    QFileInfoList list = dir.entryInfoList(QDir::Files);
    for (const QFileInfo& fi : list)
        files.insert(fi.absoluteFilePath());

    m_dirSnapshots[absDir] = files;
}

QString FileWatcher::toRel(const QString& absPath) const
{
    return QDir(m_workspaceRoot).relativeFilePath(absPath);
}


