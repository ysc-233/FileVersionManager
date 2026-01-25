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
    m_watchedFiles.clear();
    m_watchedDirs.clear();

    // 清空 QFileSystemWatcher
    m_watcher.removePaths(m_watcher.files());
    m_watcher.removePaths(m_watcher.directories());

    addWatchPath(workspaceRoot);
}

void FileWatcher::addWatchPath(const QString& path)
{
    QFileInfo fi(path);
    if (!fi.exists()) return;

    if (fi.isDir()) {
        watchDirectory(fi.absoluteFilePath());
    } else {
        if (!m_watchedFiles.contains(fi.absoluteFilePath())) {
            m_watcher.addPath(fi.absoluteFilePath());
            m_watchedFiles.insert(fi.absoluteFilePath());
        }
    }

    // 只连接一次
    static bool connected = false;
    if (!connected) {
        connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [=](const QString& absPath) {
            QFileInfo fi(absPath);
            QString relPath = QDir(m_workspaceRoot).relativeFilePath(absPath);

            if (!fi.exists()) {
                // 文件被删除
                if (m_watchedFiles.contains(absPath)) {
                    m_watchedFiles.remove(absPath);
                    emit fileDeleted(relPath);
                }
            } else {
                emit fileChanged(relPath);
                // 重新添加到 watcher（Windows 上可能只触发一次）
                if (!m_watchedFiles.contains(absPath)) {
                    m_watcher.addPath(absPath);
                    m_watchedFiles.insert(absPath);
                }
            }
        });

        connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [=](const QString& absDir){
            QDir dir(absDir);
            QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QFileInfo& fi : entries) {
                if (fi.isDir() && fi.fileName() != ".fvm") {
                    watchDirectory(fi.absoluteFilePath());
                } else if (fi.isFile()) {
                    QString absFile = fi.absoluteFilePath();
                    if (!m_watchedFiles.contains(absFile)) {
                        m_watcher.addPath(absFile);
                        m_watchedFiles.insert(absFile);
                        emit fileChanged(QDir(m_workspaceRoot).relativeFilePath(absFile)); // 新增文件
                    }
                }
            }

            // 检查已删除文件
            for (auto it = m_watchedFiles.begin(); it != m_watchedFiles.end();) {
                QFileInfo fi(*it);
                if (!fi.exists()) {
                    emit fileDeleted(QDir(m_workspaceRoot).relativeFilePath(*it));
                    it = m_watchedFiles.erase(it);
                } else {
                    ++it;
                }
            }
        });

        connected = true;
    }
}

void FileWatcher::watchDirectory(const QString& absPath)
{
    if (m_watchedDirs.contains(absPath)) return;

    QDir dir(absPath);
    if (!dir.exists()) return;

    m_watcher.addPath(absPath);
    m_watchedDirs.insert(absPath);

    // 递归子目录
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fi : entries) {
        if (fi.isDir() && fi.fileName() != ".fvm") {
            watchDirectory(fi.absoluteFilePath());
        } else if (fi.isFile()) {
            if (!m_watchedFiles.contains(fi.absoluteFilePath())) {
                m_watcher.addPath(fi.absoluteFilePath());
                m_watchedFiles.insert(fi.absoluteFilePath());
            }
        }
    }
}
