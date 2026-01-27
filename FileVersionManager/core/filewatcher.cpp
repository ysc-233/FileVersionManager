#include "filewatcher.h"
#include <QFileInfo>
#include <QDebug>
#include <QDir>
FileWatcher::FileWatcher(QObject* parent)
    : QObject(parent)
{
}

void FileWatcher::setWorkspace(const QString& workspaceRoot)
{
    m_workspaceRoot = workspaceRoot;
    if(m_watchedFiles.size()>0)
        m_watchedFiles.clear();
    if(m_watchedDirs.size()>0)
        m_watchedDirs.clear();

    // 清空 QFileSystemWatcher
    if(m_watcher.files().size()>0)
    {
        m_watcher.removePaths(m_watcher.files());
        m_watcher.removePaths(m_watcher.directories());
    }
    startWatch();
}

void FileWatcher::startWatch()
{
    scanAndWatchDir(m_workspaceRoot);

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,this, [this](const QString& absDir)
    {
        qDebug()<<__FUNCTION__<<"directoryChanged";
        QSet<QString> oldSet = m_dirSnapshots.value(absDir);

        QDir dir(absDir);
        QSet<QString> newSet;
        QFileInfoList files = dir.entryInfoList(QDir::Files);

        for (const QFileInfo& fi : files)
            newSet.insert(fi.absoluteFilePath());

        // 删除
        for (const QString& f : oldSet - newSet)
            emit fileDeleted(toRel(f));

        // 新增
        for (const QString& f : newSet - oldSet)
        {
            if (!m_watchedFiles.contains(f))
            {
                m_watcher.addPath(f);
                m_watchedFiles.insert(f);
            }
            emit fileAdded(toRel(f));
        }

        m_dirSnapshots[absDir] = newSet;
    });

    connect(&m_watcher, &QFileSystemWatcher::fileChanged,this, [this](const QString& absFile)
    {
        qDebug()<<__FUNCTION__<<"fileChanged";
        QFileInfo fi(absFile);
        if (fi.exists())
            emit fileChanged(toRel(absFile));
    });
}

void FileWatcher::addFile(const QString &absPath)
{
    qDebug()<<__FUNCTION__<<"absPath"<<absPath;
    qDebug()<<m_watcher.files();
    QFileInfo fi(absPath);
    if (!fi.exists() || !fi.isFile())
        return;

    if (m_watchedFiles.contains(absPath))
        return;

    m_watcher.addPath(absPath);
    m_watchedFiles.insert(absPath);

    qDebug() << __FUNCTION__ << "FileWatcher re-added file:" << absPath;
}

void FileWatcher::scanAndWatchDir(const QString &absDir)
{
    if (m_watchedDirs.contains(absDir))
        return;

    m_watcher.addPath(absDir);
    m_watchedDirs.insert(absDir);

    snapshotDirectory(absDir);

    QFileInfoList entries = QDir(absDir).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo& fi : entries)
    {
        if (fi.isDir() && fi.fileName() != ".fvm")
            scanAndWatchDir(fi.absoluteFilePath());
        else if (fi.isFile())
        {
            QString absFile = fi.absoluteFilePath();
            if (!m_watchedFiles.contains(absFile))
            {
                m_watcher.addPath(absFile);
                m_watchedFiles.insert(absFile);
            }
        }
    }
}

QString FileWatcher::toRel(const QString &absPath)
{
    return QDir(m_workspaceRoot).relativeFilePath(absPath);
}

void FileWatcher::snapshotDirectory(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
        return;

    QSet<QString> files;

    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fi : list)
    {
        files.insert(fi.absoluteFilePath());
    }

    m_dirSnapshots[dirPath] = files;
}
