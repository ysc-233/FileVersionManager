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
        Logger::info("Version exited");
        return;
    }

    m_storage.save(hash, content);

    VersionInfo info;
    info.filePath = filePath;
    info.versionId = hash;
    info.timestamp = QDateTime::currentDateTime();

    m_metadata.addVersion(info);
    m_metadata.save();
    Logger::info("Version created:"+hash.left(8));
}

bool VersionManager::rollback(const QString& filePath,const QString& versionId)
{
    // 1. 查元数据，确保版本存在
    const VersionInfo info = m_metadata.find(filePath, versionId);
    if (info.versionId.isEmpty())
        return false;

    // 2. 读取版本内容
    const QByteArray data = m_storage.load(versionId);
    if (data.isEmpty())
        return false;

    QFileInfo fi(filePath);
    QDir dir = fi.dir();

    // 3. 临时文件（同目录，保证 rename 原子性）
    const QString tmpPath = filePath + ".fvm_tmp";

    {
        QFile tmp(tmpPath);
        if (!tmp.open(QIODevice::WriteOnly))
            return false;

        if (tmp.write(data) != data.size())
            return false;

        tmp.flush();
        tmp.close();
    }

    // 4. 原子替换
    QFile::remove(filePath);            // Windows 需要先删
    if (!QFile::rename(tmpPath, filePath))
    {
        QFile::remove(tmpPath);
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
