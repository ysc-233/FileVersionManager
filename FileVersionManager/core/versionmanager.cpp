#include "versionmanager.h"
#include "utils/Logger.h"
#include "filehasher.h"
#include "storage/filestorage.h"
#include <QDir>
#include <QtDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QDirIterator>
VersionManager::VersionManager(const QString& rootPath, QObject* parent)
    : QObject(parent),
      m_metadata(rootPath),
      m_storage(rootPath)
{
    Logger::init(rootPath);
    m_rootPath = rootPath;
}

void VersionManager::initializeWorkspace()
{
    QDirIterator it(
                m_rootPath,
                QDir::Files | QDir::NoSymLinks | QDir::Readable,
                QDirIterator::Subdirectories
                );

    while (it.hasNext())
    {
        it.next();
        QFileInfo fi = it.fileInfo();

        // 忽略 .fvm 目录下的任何文件（关键）
        if (fi.absoluteFilePath().contains("/.fvm/") ||
                fi.absoluteFilePath().contains("\\.fvm\\"))
            continue;

        const QString absPath = fi.absoluteFilePath();
        const QString relPath = QDir(m_rootPath).relativeFilePath(absPath);

        if (!m_metadata.versions(relPath).isEmpty())
            continue;

        QFile file(absPath);
        if (!file.open(QIODevice::ReadOnly))
            continue;

        QByteArray data = file.readAll();
        file.close();

        const QString hash = FileHasher::sha256(data);
        if (hash.isEmpty())
            continue;

        m_storage.save(hash, data);

        VersionInfo info;
        info.filePath  = relPath;
        info.versionId = hash;
        info.timestamp = fi.lastModified();
        info.fileSize  = fi.size();

        m_metadata.addVersion(info);
    }

    m_metadata.save();
}



QList<VersionInfo> VersionManager::versions(const QString& filePath) const
{
    return m_metadata.versions(filePath);
}

QMap<QString, QList<VersionInfo>> VersionManager::allVersions() const
{
    return m_metadata.allVersions();
}

void VersionManager::onFileChanged(const QString& relPath)
{
    QString absPath = QDir(m_rootPath).filePath(relPath); // 用 workspace 拼绝对路径
    QFile file(absPath);
    if (!file.exists()) {
        Logger::info(QString("File does not exist: %1").arg(absPath));
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray content = file.readAll();
    QString hash = FileHasher::sha256(content);
    if (hash.isEmpty())
        return;

    if (m_metadata.hasVersion(relPath, hash)) {
        Logger::info(QString("Version exists: %1").arg(relPath));
        return;
    }

    m_storage.save(hash, content);

    VersionInfo info;
    info.filePath = relPath;
    info.versionId = hash;
    info.timestamp = QDateTime::currentDateTime();
    info.fileSize = file.size();

    m_metadata.addVersion(info);
    m_metadata.save();
    Logger::info(QString("Version created: %1 [%2]").arg(relPath).arg(hash.left(8)));
}

void VersionManager::onFileDeleted(const QString &relPath)
{
    // 取该文件的最后一个版本 hash
    const QString lastHash = m_metadata.latestVersionHash(relPath);
    if (lastHash.isEmpty())
        return;
    m_pendingDeletes.append({relPath,lastHash,QDateTime::currentDateTime()});
    qDebug() << __FUNCTION__ <<"Pending delete:" << relPath;
}

void VersionManager::onFileAdded(const QString &relPath)
{
    const QString absPath = m_rootPath + "/" + relPath;

    QFile file(absPath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray content = file.readAll();
    file.close();

    const QString hash = FileHasher::sha256(content);
    if (hash.isEmpty())
        return;

    // 尝试匹配 pending delete
    for (int i = 0; i < m_pendingDeletes.size(); ++i)
    {
        const auto& pd = m_pendingDeletes[i];
        // hash 相同 + 时间接近 → rename
        if (pd.lastHash == hash && pd.time.msecsTo(QDateTime::currentDateTime()) < 2000)
        {
            qDebug() << __FUNCTION__ <<"Rename detected:" << pd.relPath << "->" << relPath;

            m_metadata.renameFile(pd.relPath, relPath);
            m_pendingDeletes.removeAt(i);
            return; //不生成新版本
        }
    }
    //真正的新文件
    createInitialVersion(relPath, content);
}

bool VersionManager::rollback(const QString& filePath,const QString& versionId,RollbackError* error)
{
    Logger::info(QString("Rollback requested: file=%1 target=%2").arg(filePath).arg(versionId.left(8)));
    if (error) *error = RollbackError::None;
    // 1. 查元数据，确保版本存在
    const VersionInfo info = m_metadata.find(filePath, versionId);
    if (info.versionId.isEmpty())
    {
        if (error) *error = RollbackError::VersionNotFound;
        return false;
    }

    // 2. 读取版本内容
    const QByteArray data = m_storage.load(versionId);
    if (data.isEmpty())
    {
        if (error) *error = RollbackError::LoadFailed;
        return false;
    }

    // 3. 临时文件（同目录，保证 rename 原子性）
    QString absPath = m_rootPath + "/" + filePath;
    const QString tmpPath = absPath + ".fvm_tmp";

    {
        QFile tmp(tmpPath);
        if (!tmp.open(QIODevice::WriteOnly))
        {
            if (error) *error = RollbackError::WriteFailed;
            return false;
        }

        if (tmp.write(data) != data.size())
        {
            if (error) *error = RollbackError::WriteFailed;
            return false;
        }

        tmp.flush();
        tmp.close();
    }

    // 4. 原子替换
    QFile::remove(absPath);            // Windows 需要先删
    if (!QFile::rename(tmpPath, absPath))
    {
        QFile::remove(tmpPath);
        if (error) *error = RollbackError::RenameFailed;
        return false;
    }

    return true;
}

QMap<QString, QString> VersionManager::currentVersions() const
{
    QMap<QString, QString> result;

    const auto versions = m_metadata.allVersions();  // 接住临时对象

    for (auto it = versions.cbegin(); it != versions.cend(); ++it)
    {
        const QString& relPath = it.key();
        const QString absPath = m_rootPath + "/" + relPath;
        QString hash = FileHasher::sha256File(absPath);
        if (!hash.isEmpty())
            result.insert(relPath, hash);
    }

    return result;
}

VersionInfo VersionManager::currentVersionInfo(const QString &filePath) const
{
    VersionInfo info;
    info.filePath = filePath;

    // 1. 计算当前文件 hash
    QString absPath = m_rootPath + "/" + filePath;
    QString hash = FileHasher::sha256File(absPath);
    if (hash.isEmpty())
        return {};

    info.versionId = hash;

    // 2. 文件系统信息
    QFileInfo fi(absPath);
    if (fi.exists())
    {
        info.fileSize  = fi.size();
        info.timestamp = fi.lastModified();
    }

    return info;
}

QString VersionManager::buildDiffText(const VersionInfo &current, const VersionInfo &target)
{
    QString text;

    text += "Version Compare:\n";

    if (current.versionId == target.versionId)
        text += "Content is consistent\n";
    else
        text += "Content is different\n";

    text += QString("Current: %1\n").arg(current.versionId.left(12));
    text += QString("Target: %1\n\n").arg(target.versionId.left(12));

    text += "File size:\n";
    text += QString("Current: %1 KB\n").arg(current.fileSize / 1024.0, 0, 'f', 2);
    text += QString("Target: %1 KB\n\n").arg(target.fileSize / 1024.0, 0, 'f', 2);

    text += "Modification Time:\n";
    text += QString("Current: %1\n").arg(current.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
    text += QString("Target: %1\n").arg(target.timestamp.toString("yyyy-MM-dd HH:mm:ss"));

    return text;
}

void VersionManager::flushPendingDeletes()
{
    const QDateTime now = QDateTime::currentDateTime();

    for (int i = 0; i < m_pendingDeletes.size(); )
    {
        if (m_pendingDeletes[i].time.msecsTo(now) > 3000)
        {
            m_metadata.markDeleted(m_pendingDeletes[i].relPath);
            qDebug() << "File deleted:" << m_pendingDeletes[i].relPath;
            m_pendingDeletes.removeAt(i);
        }
        else
        {
            ++i;
        }
    }

    m_metadata.save();
}

void VersionManager::createInitialVersion(const QString &relPath, const QByteArray &content)
{
    if (content.isEmpty())
        return;

    // 已存在版本 → 不是初始版本
    if (!m_metadata.versions(relPath).isEmpty())
        return;

    // 计算 hash（用内容，不要再用路径）
    const QString hash = FileHasher::sha256(content);
    if (hash.isEmpty())
        return;

    // 保存对象内容
    m_storage.save(hash, content);

    // 构建版本信息
    VersionInfo info;
    info.filePath  = relPath;
    info.versionId = hash;
    info.timestamp = QDateTime::currentDateTime();
    info.fileSize  = content.size();
    info.state     = FileState::Normal;

    // 写入 metadata
    m_metadata.addVersion(info);
    m_metadata.save();

    Logger::info(QString("Initial version created: %1 [%2]").arg(relPath).arg(hash.left(8)));
}

void VersionManager::markDeleted(const QString &relPath)
{
    m_metadata.markDeleted(relPath);
}

bool VersionManager::restoreDeletedFile(const QString &filePath, RollbackError *error)
{
    if (error) *error = RollbackError::None;

    // 1. 找最近一个 Normal 版本
    QString versionId = m_metadata.latestVersionHash(filePath);
    if (versionId.isEmpty()) {
        if (error) *error = RollbackError::VersionNotFound;
        return false;
    }

    // 2. 从对象存储读取内容
    QByteArray data = m_storage.load(versionId);
    if (data.isEmpty()) {
        if (error) *error = RollbackError::LoadFailed;
        return false;
    }

    // 3. 写回磁盘
    QString absPath = m_rootPath + "/" + filePath;
    QDir().mkpath(QFileInfo(absPath).absolutePath());

    QFile file(absPath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = RollbackError::WriteFailed;
        return false;
    }
    file.write(data);
    file.close();

    // 4. 追加一条 Normal 记录（关键）
    VersionInfo restored;
    restored.filePath  = filePath;
    restored.versionId = versionId;
    restored.timestamp = QDateTime::currentDateTime();
    restored.fileSize  = data.size();
    restored.state     = FileState::Normal;

    m_metadata.addVersion(restored);
    m_metadata.save();

    Logger::info(QString("File restored: %1 [%2]").arg(filePath).arg(versionId.left(8)));
    emit fileRestored(filePath);
    return true;
}

