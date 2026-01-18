#include "versionmanager.h"
#include "utils/logger.h"
#include "filehasher.h"
#include "storage/filestorage.h"
#include <QDir>
#include <QtDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

VersionManager::VersionManager(const QString& rootPath, QObject* parent)
    : QObject(parent),
      m_metadata(rootPath),
      m_storage(rootPath)
{
    Logger::init(rootPath);
}

QList<VersionInfo> VersionManager::versions(const QString& filePath) const
{
    return m_metadata.versions(filePath);
}

QMap<QString, QList<VersionInfo>> VersionManager::allVersions() const
{
    return m_metadata.allVersions();
}

void VersionManager::onFileChanged(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray content = file.readAll();
    QString hash = FileHasher::sha256(filePath);
    if (hash.isEmpty())
        return;

    if (m_metadata.hasVersion(filePath, hash))
    {
        qDebug()<<"Version exited";
        return;
    }

    m_storage.save(hash, content);

    VersionInfo info;
    info.filePath = filePath;
    info.versionId = hash;
    info.timestamp = QDateTime::currentDateTime();

    m_metadata.addVersion(info);
    m_metadata.save();
    qDebug()<<"Version created:"+hash.left(8);
    Logger::info(QString("Version created: file=%1 version=%2").arg(info.filePath).arg(info.versionId.left(8)));
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
    const QString tmpPath = filePath + ".fvm_tmp";
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
    QFile::remove(filePath);            // Windows 需要先删
    if (!QFile::rename(tmpPath, filePath))
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
        const QString& filePath = it.key();

        QString hash = FileHasher::sha256(filePath);
        if (!hash.isEmpty())
            result.insert(filePath, hash);
    }

    return result;
}

VersionInfo VersionManager::currentVersionInfo(const QString &filePath) const
{
    VersionInfo info;
    info.filePath = filePath;

    // 1. 计算当前文件 hash
    QString hash = FileHasher::sha256(filePath);
    if (hash.isEmpty())
        return {};

    info.versionId = hash;

    // 2. 文件系统信息
    QFileInfo fi(filePath);
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

    text += "版本对比\n";

    if (current.versionId == target.versionId)
        text += "内容一致\n";
    else
        text += "内容不同\n";

    text += QString("  当前: %1\n").arg(current.versionId.left(12));
    text += QString("  目标: %1\n\n").arg(target.versionId.left(12));

    text += "文件大小:\n";
    text += QString("  当前: %1 KB\n").arg(current.fileSize / 1024.0, 0, 'f', 2);
    text += QString("  目标: %1 KB\n\n").arg(target.fileSize / 1024.0, 0, 'f', 2);

    text += "修改时间:\n";
    text += QString("  当前: %1\n").arg(current.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
    text += QString("  目标: %1\n").arg(target.timestamp.toString("yyyy-MM-dd HH:mm:ss"));

    return text;
}
