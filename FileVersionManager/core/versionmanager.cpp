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
    const QByteArray content = m_storage.load(versionId);
    if (content.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    file.write(content);
    file.close();
    return true;
}
